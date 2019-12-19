#include "processor.h"
#include "base.h"
#include "sc_disk.h"
#include "sc_engine.h"
#include "sc_application.h"
#include "camera_capture.h"
#include "job_manager.h"
#include "tag_manager.h"
#include "sc_io_manager.h"
#include "sc_db_manager.h"
#include "proc/image_proc.h"
#include <QtConcurrent/QtConcurrent>

#if !defined(POR_PRODUCT)
#define POR_TESTMODE
#endif

//////////////////////////////////////////////////////////////////////////
i32 qtProcessSnap(CSCEngine* sc_engine_ptr, CJobUnit* job_ptr, i32 oper_mode, i32 result_keep_mode, i64 time_stamp)
{
	if (!job_ptr || !sc_engine_ptr)
	{
		return kEngineResultFail;
	}
	return sc_engine_ptr->processSnap(job_ptr, oper_mode, result_keep_mode, time_stamp);
}

//////////////////////////////////////////////////////////////////////////
CProcessor::CProcessor()
{
	m_cam_capture_ptr = NULL;
	m_job_manager_ptr = NULL;
	m_io_manager_ptr = NULL;
	m_tag_manager_ptr = NULL;

	m_cam_param_ptr = NULL;
	m_engine_param_ptr = NULL;
	m_device_param_ptr = NULL;
	m_general_param_ptr = NULL;

	m_pu_id = 0;
	m_pu_state = kSCStateNone;
	m_pu_mode = kSCModeNone;
	m_result_code = kPOSuccess;

	m_is_inited = false;
	m_is_admin_used = false;
	m_is_thread_used = false;

	m_pu_semaphore_ptr = NULL;
	moveToThread(this);
}

CProcessor::~CProcessor()
{
	exitProcessor();
	POSAFE_DELETE(m_pu_semaphore_ptr);
}

bool CProcessor::initProcessor(CCameraCapture* cam_capture_ptr, i32 id)
{
	//check processor index
	if (!cam_capture_ptr || !CPOBase::checkIndex(id, PO_UNIT_COUNT))
	{
		return false;
	}

	//processor initialize....
	if (!m_is_inited)
	{
		singlelog_lv0(QString("Processor[%1] InitInstance").arg(id));

		{
			QMutexLocker l(&m_mutex_processor);

			m_pu_id = id;
			m_cam_param_ptr = NULL;
			m_result_code = kPOSuccess;
		}

		m_pu_mode = kSCModeNone;
		m_pu_state = kSCStateNone;
		m_pu_semaphore_ptr = po_new QSemaphore(1);

		m_do_setting.init();
		m_modbus_comm.init();
		m_ftp_comm.init();
		m_scu_control.init();

		m_job_manager_ptr = g_main_app_ptr->m_job_manager_ptr;
		m_io_manager_ptr = g_main_app_ptr->m_io_manager_ptr;
		m_tag_manager_ptr = g_main_app_ptr->m_tag_manager_ptr;
		m_engine_param_ptr = &(g_main_app_ptr->m_engine_param);
		m_general_param_ptr = &(g_main_app_ptr->m_general_param);
		m_device_param_ptr = &(g_main_app_ptr->m_device_info);

		if (!m_sc_engine.initInstance(m_pu_id, m_engine_param_ptr))
		{
			return false;
		}

		m_is_inited = true;
		m_is_admin_used = false;
		m_is_thread_used = false;

		m_cam_capture_ptr = cam_capture_ptr;
		m_cam_capture_ptr->moveToThread(this);
		m_pu_idle_timer.moveToThread(this);
		QThread::start();
	}
	return true;
}

bool CProcessor::exitProcessor()
{
	if (m_is_inited)
	{
		singlelog_lv0(QString("ProcessUnit[%1] ExitInstance is").arg(m_pu_id));

		m_is_inited = false;
		m_is_admin_used = false;
		m_is_thread_used = false;
		m_sc_engine.exitInstance();

		QEventLoopStop();
		POSAFE_DELETE(m_pu_semaphore_ptr);
	}
	return true;
}

bool CProcessor::initInstance()
{
	m_sc_db_client.initInstance(&g_main_app_ptr->m_db_config, false, true);

	std::vector<CSCFtpClient*> ftp_client_vec;
	for (i32 i = 0; i < POFTP_MAX_DEV; i++)
	{
		ftp_client_vec.push_back(g_main_app_ptr->m_io_manager_ptr->getFtpClient(i));
	}

	if (!m_sc_output_manager.initInstance(&g_main_app_ptr->m_device_info,
								&g_main_app_ptr->m_general_param, &m_sc_db_client, ftp_client_vec))
	{ 
		return false;
	}
	return true;
}

bool CProcessor::exitInstance()
{
	m_sc_output_manager.exitInstance();
	m_sc_db_client.exitInstance(false);
	return true;
}

void CProcessor::initEvent()
{
	qRegisterMetaType<ScuControl>("SyncControl");
	qRegisterMetaType<CameraTrigger>("CamTrigger");
	qRegisterMetaType<CFtpDev>("FtpSetting");

	disconnect();
	connect(g_main_app_ptr, SIGNAL(broadcastPacket(i32, i32)), this, SLOT(onBroadcastPacket(i32, i32)));
	connect(&m_pu_idle_timer, SIGNAL(timeout()), this, SLOT(onSnapImage()));

	if (m_cam_capture_ptr)
	{
		m_cam_capture_ptr->disconnect();

		//signal between CameraCapture and Processor unit
		connect(this, SIGNAL(requireImage(ScuControl)), m_cam_capture_ptr, SLOT(onRequireImage(ScuControl)));
		connect(m_cam_capture_ptr, SIGNAL(syncCamState()), this, SLOT(onSyncCameraState()));
	}
	printlog_lv1(QString("Processor[%1] connect to eventloop.").arg(m_pu_id));
}

void CProcessor::run()
{
	singlelog_lv0(QString("The Processor[%1] thread is").arg(m_pu_id));

	initEvent();
	m_pu_idle_timer.start(1); //idle processing timer

	exec(); //start event loop

	m_pu_idle_timer.stop();
}

const bool CProcessor::isAcceptableSnap(const ScuControl& sync)
{
	if (!isUsedProcessor())
	{
		return false;
	}

	i32 sync_mode = sync.snap_mode;
	i32 sync_level = sync.snap_level;
	return (sync_mode == kCamSnapManualEx || sync_level == m_scu_control.snap_level);
}

void CProcessor::onSnapImage()
{
	if (!isUsedProcessor())
	{
		return;
	}

	//pop from queue & pre-processing image
	ScuControl sync;
	ImageData* img_data_ptr;
	if (!m_cam_capture_ptr->popImageFromQueue(sync, img_data_ptr))
	{
		POSAFE_DELETE(img_data_ptr);
		return;
	}
  	if (chk_logcond(LOG_LV4, LOG_SCOPE_APP))
  	{
  		saveImageFrameForDebug(img_data_ptr, -1);
  	}

  	//processing
 	i32 cam_id = getCamID();
 	singlelog_lvs4(QString("Processing one captured camera%1").arg(cam_id), LOG_SCOPE_APP);

  	if (isAcceptableSnap(sync))
  	{
  		m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutBusy, cam_id, true);
  		singlelog_lvs4("One snapped image processing", LOG_SCOPE_APP);
  
  		switch (sync.snap_mode)
  		{
  			case kCamSnapFree:
  			case kCamSnapManual:
  			case kCamSnapManualEx:
  			{
  				onProcessSnap(img_data_ptr, sync.time_stamp, isProcessorRun());
  				break;
  			}
  			case kCamSnapCalib:
  			{
  				onProcessSnapCalib(img_data_ptr);
  				break;
  			}
  			case kCamSnapTeach:
  			{
  				onProcessTeachJob(img_data_ptr);
  				break;
  			}
  			default:
  			{
  				onPrepareSnap(img_data_ptr, sync.time_stamp);
  				break;
  			}
  		}
  	}

  	m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutBusy, cam_id, false);
  	m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutChkFinish, cam_id, true);
	POSAFE_DELETE(img_data_ptr);
}

void CProcessor::onProcessTeachJob(ImageData* img_data_ptr)
{
	if (!img_data_ptr || !img_data_ptr->isValid())
	{
		printlog_lvs2("Can't capture image in onProcessTeachJob.", LOG_SCOPE_APP);
		uploadFailPacket(kSCCmdTeachJob, kPOErrInvalidOper);
		return;
	}

	//register image
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (m_sc_engine.preProcessFromImg(img_data_ptr, job_ptr) != kEngineResultOK)
	{
		printlog_lvs2("onProcessTeachJob can't register image.", LOG_SCOPE_APP);
		return;
	}

	if (!isProcessorStop() || !job_ptr)
	{
		if (m_is_admin_used)
		{
			printlog_lvs2(QString("Can't teach current job, PUState is %1, current job is %2")
								.arg(m_pu_state).arg(job_ptr != NULL), LOG_SCOPE_APP);
			uploadFailPacket(kSCCmdTeachJob, kSCErrInvalidCurJob);
		}
		return;
	}

	//replace camera calibration
	job_ptr->setCameraSetting(m_cam_param_ptr, kCamSettingUpdateAll);
	
	//update tool data and result
	onUpdatingTool(job_ptr);
	m_io_manager_ptr->updateJobTools(job_ptr);

	i32 code = kEngineResultOK;
#if defined(POR_TESTMODE)
	code = m_sc_engine.processSnap(job_ptr, kEngineOperStudy, kResultOperTypeAll, sys_cur_time);
#else
	if (chk_logcond(LOG_LV4, LOG_SCOPE_APP))
	{
		m_sc_engine.processSnap(job_ptr, kEngineOperStudy, kResultOperTypeAll, sys_cur_time);
	}
	else
	{
		QFuture<i32> fcode = QtConcurrent::run(qtProcessSnap, &m_sc_engine, job_ptr,
											kEngineOperStudy, kResultOperTypeAll, sys_cur_time);
		fcode.waitForFinished();
		code = fcode.result();
	}
#endif
	if (code != kEngineResultOK)
	{
		printlog_lv1("JobTeaching Process is failed!");
		if (m_is_admin_used)
		{
			uploadFailPacket(kSCCmdTeachJob, kPOErrInvalidOper);
		}
		return;
	}

	job_ptr->updateThumbnail(img_data_ptr);
	job_ptr->updateInfo(kJobEditUpdate);
	job_ptr->setChanged(kJobContent);
	m_job_manager_ptr->writeJobToFile();

	if (m_is_admin_used)
	{
		uploadJobResult(kSCCmdSnapResult, kResultWithAll, true);
	}
}

void CProcessor::onPrepareSnap(ImageData* img_data_ptr, i64 time_stamp)
{
	if (!isProcessorAvailable() || !img_data_ptr)
	{
		return;
	}

	//register image & check current job
	m_sc_engine.preProcessFromImg(img_data_ptr);
}

void CProcessor::onProcessSnap(ImageData* img_data_ptr, i64 time_stamp, bool is_trigger)
{
	if (!isProcessorAvailable() || !img_data_ptr || !img_data_ptr->isValid())
	{
		printlog_lvs3("onProcessSnap failed.", LOG_SCOPE_APP);
		return;
	}

	i32 code = kEngineResultFail;
	i32 result_keep_mode = m_general_param_ptr->getHistorySetting();
	bool is_upload = isAdminProcessor();
	if (is_upload)
	{
		result_keep_mode |= kResultOperTypeAll;
	}
	
	//register image & check current job
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (m_sc_engine.preProcessFromImg(img_data_ptr, job_ptr) != kEngineResultOK)
	{
		return;
	}
	if (!job_ptr)
	{
		return;
	}

	//process snapimage with current job
	sys_keep_time;
	{
		onUpdatingTool(job_ptr);
		m_io_manager_ptr->updateJobTools(job_ptr);

#if defined(POR_TESTMODE)
		code = m_sc_engine.processSnap(job_ptr, kEngineOperProcess, result_keep_mode, time_stamp);
#else
		if (chk_logcond(LOG_LV4, LOG_SCOPE_APP))
		{
			code = m_sc_engine.processSnap(job_ptr, kEngineOperProcess, result_keep_mode, time_stamp);
		}
		else
		{
			QFuture<i32> fcode = QtConcurrent::run(qtProcessSnap, &m_sc_engine, job_ptr,
												kEngineOperProcess, result_keep_mode, time_stamp);
			fcode.waitForFinished();
			code = fcode.result();
		}
#endif
	}
	printlog_lvs4(QString("Processing time is %1ms").arg(sys_get_time_ms), LOG_SCOPE_APP);
	if (code != kEngineResultOK)
	{
		printlog_lv1(QString("Process snapimage is failed by Engine, Engine result is %1").arg(code));
		uploadFailPacket(kSCCmdSnapResult, kPOSubTypeNone, code);
		return;
	}

	CJobResult* job_result_ptr = m_sc_engine.getJobResult();
	job_result_ptr->m_cam_id = getCamID();
	job_result_ptr->updateProcessTime();

	if (is_trigger)
	{
		sys_keep_time;

		//output job result history to local database and external storage such as Ftp
		m_sc_output_manager.outputResult(img_data_ptr, job_ptr, job_result_ptr, &m_ftp_comm);
		printlog_lvs4(QString("Output job result, write time:%1ms").arg(sys_get_time_ms), LOG_SCOPE_APP);

		//output job result to io/communication
		m_io_manager_ptr->output(&m_do_setting, &m_modbus_comm, job_result_ptr);

		//update job result
		job_result_ptr->updateProcessTime();
		job_result_ptr->updateResultStat();
	}

	if (is_upload)
	{
		uploadJobResult(kSCCmdSnapResult, kResultWithAll);
		printlog_lvs3(QString("onProcessSnap upload result:%1").arg(img_data_ptr->time_stamp), LOG_SCOPE_APP);
	}

	//update & upload runtime history
	if (is_trigger)
	{
		updateRuntimeHistory(job_result_ptr);
		uploadRuntimeHistory(kSCCmdRuntimeSync);
	}
}

void CProcessor::onProcessInteractive()
{
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (!job_ptr || !isProcessorStop() || !isAdminProcessor())
	{
		return;
	}

	sys_keep_time;
	{
		i64 time_stamp = sys_cur_time;
		onUpdatingTool(job_ptr);
		m_io_manager_ptr->updateJobTools(job_ptr);

#if defined(POR_TESTMODE)
		m_sc_engine.processSnap(job_ptr, kEngineOperProcess, kResultOperTypeAll, time_stamp);
#else
		if (chk_logcond(LOG_LV4, LOG_SCOPE_APP))
		{
			m_sc_engine.processSnap(job_ptr, kEngineOperProcess, kResultOperTypeAll, time_stamp);
		}
		else
		{
			QFuture<i32> fcode = QtConcurrent::run(qtProcessSnap, &m_sc_engine, job_ptr,
												kEngineOperProcess, kResultOperTypeAll, time_stamp);
			fcode.waitForFinished();
			fcode.result();
		}
#endif
	}
	printlog_lvs2(QString("Interactive processing time is %1ms").arg(sys_get_time_ms), LOG_SCOPE_APP);
}

void CProcessor::onProcessSnapCalib(ImageData* img_data_ptr)
{
	if (!img_data_ptr || !img_data_ptr->isValid())
	{
		uploadFailPacket(kSCCmdCalib, kPOErrInvalidOper);
		printlog_lv1("CalibSnap Process is failed.");
		return;
	}

	sendSnapImage(kSCCmdCalib, img_data_ptr, false);
	
	//register image & check current job
	if (m_sc_engine.preProcessFromImg(img_data_ptr) != kEngineResultOK)
	{
		return;
	}
	
	//detect board for calibration such as chessboard, circle-grid and so on...
	bool is_success = false;
	CalibBoard board;
	CameraCalib* cam_calib_ptr = m_tmp_cam_param.getCameraCalib();
	if (m_sc_engine.detectBoard(&board, cam_calib_ptr) == kEngineResultOK)
	{
		printlog_lv1("Calibsnap process is OK.");
		is_success = true;
	}

	//Upload Corner Points
	i32 len = board.memSize();
	i32 buffer_size = len;
	u8* buffer_ptr;
	
	Packet* pak = po_new Packet(kSCCmdCalib, kPOPacketRespOK);
	if (pak)
	{
		pak->setSubCmd(kSCSubTypeCalibSnap);
		pak->setReservedb8(0, is_success);
		pak->allocateBuffer(len, buffer_ptr);

		board.memWrite(buffer_ptr, buffer_size);
		sendPacketToNet(pak, buffer_ptr);
	}
	else
	{
		printlog_lvs2("Can't allocate onProcessSnapCalib Packet.", LOG_SCOPE_APP);
	}
}

i32 CProcessor::onProcessCameraCalib(u8*& buffer_ptr, i32& buffer_size)
{
	if (!buffer_ptr || !m_cam_param_ptr)
	{
		return kPOErrInvalidOper;
	}

	CalibBoardVector board_vec;
	CameraCalib* cam_calib_param_ptr = m_cam_param_ptr->getCameraCalib();
	CameraInfo* cam_info_ptr = m_cam_param_ptr->getCameraInfo();
	if (!cam_calib_param_ptr || !cam_info_ptr)
	{
		return kPOErrInvalidOper;
	}

	//lock calib param
	exlock_guard(cam_calib_param_ptr->m_mutex);

	//Read Calib parameter
	cam_calib_param_ptr->init();
	cam_calib_param_ptr->memParamRead(buffer_ptr, buffer_size);
	
	//Read CalibBoards
	switch (cam_calib_param_ptr->getCalibType())
	{
		case kCalibTypeGrid:
		{
			//Read Calib Board Count
			i32 i, count = -1;
			CPOBase::memRead(count, buffer_ptr, buffer_size);
			if (!CPOBase::isCount(count))
			{
				return kPOErrInvalidData;
			}

			CalibBoard* board_ptr;
			for (i = 0; i < count; i++)
			{
				board_ptr = CPOBase::pushBackNew(board_vec);
				board_ptr->memRead(buffer_ptr, buffer_size);
			}
			break;
		}
	}
	
	i32 w = cam_info_ptr->m_max_width;
	i32 h = cam_info_ptr->m_max_height;
	i32 code = m_sc_engine.cameraCalibration(board_vec, w, h);
	if (code != kEngineResultOK)
	{
		return kSCErrEngine0 + code;
	}
	return kPOSuccess;
}

i32 CProcessor::onProcessCamColorSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	i32 cam_flag = 0;
	CameraColor* cur_cam_color_ptr = m_cam_param_ptr->getCameraColor();
	CameraColor* cam_color_ptr = cam_param_ptr->getCameraColor();
	CameraInfo* cur_cam_info_ptr = m_cam_param_ptr->getCameraInfo();

	//set geometric parameters for capture
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamColorMode))
	{
		if (cam_color_ptr->m_color_mode != cur_cam_color_ptr->m_color_mode)
		{
			cam_flag |= kPOSubFlagCamColorMode;
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamColorWBMode))
	{
		if (cam_color_ptr->m_wb_mode != cur_cam_color_ptr->m_wb_mode)
		{
			cam_flag |= kPOSubFlagCamColorWBMode;
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamColorGain))
	{
		if (CPOBase::checkRange(cam_color_ptr->m_red_gain,
			cur_cam_info_ptr->m_red_gain_min, cur_cam_info_ptr->m_red_gain_max))
		{
			if (cam_color_ptr->m_red_gain != cur_cam_color_ptr->m_red_gain)
			{
				cam_flag |= kPOSubFlagCamColorGain;
			}
		}
		if (CPOBase::checkRange(cam_color_ptr->m_green_gain,
			cur_cam_info_ptr->m_green_gain_min, cur_cam_info_ptr->m_green_gain_max))
		{
			if (cam_color_ptr->m_green_gain != cur_cam_color_ptr->m_green_gain)
			{
				cam_flag |= kPOSubFlagCamColorGain;
			}
		}
		if (CPOBase::checkRange(cam_color_ptr->m_blue_gain,
			cur_cam_info_ptr->m_blue_gain_min, cur_cam_info_ptr->m_blue_gain_max))
		{
			if (cam_color_ptr->m_blue_gain != cur_cam_color_ptr->m_blue_gain)
			{
				cam_flag |= kPOSubFlagCamColorGain;
			}
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamColorWBAutoOnce))
	{
		cam_flag |= kPOSubFlagCamColorWBAutoOnce;
	}
	return cam_flag;
}

i32 CProcessor::onProcessCamShutterSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	i32 cam_flag = 0;
	CameraSpec* cur_cam_spec_ptr = m_cam_param_ptr->getCameraSpec();
	CameraSpec* cam_spec_ptr = cam_param_ptr->getCameraSpec();
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamShutter))
	{
		if (cam_spec_ptr->m_jitter_time != cur_cam_spec_ptr->m_jitter_time &&
			CPOBase::isCount(cam_spec_ptr->m_jitter_time))
		{
			cam_flag |= kPOSubFlagCamShutter;
		}
		if (cam_spec_ptr->m_shutter_mode != cur_cam_spec_ptr->m_shutter_mode &&
			CPOBase::checkIndex(cam_spec_ptr->m_shutter_mode, kShutterTypeCount))
		{
			cam_flag |= kPOSubFlagCamShutter;
		}
	}
	return cam_flag;
}

i32 CProcessor::onProcessCamRangeSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	i32 cam_flag = 0;
	CameraRange* cur_cam_range_ptr = m_cam_param_ptr->getCameraRange();
	CameraRange* cam_range_ptr = cam_param_ptr->getCameraRange();

	//set geometric parameters for capture
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamGeoInvert))
	{
		if (cam_range_ptr->m_is_invert != cur_cam_range_ptr->m_is_invert)
		{
			cam_flag |= kPOSubFlagCamGeoInvert;
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamGeoFlip))
	{
		if (cam_range_ptr->m_is_flip_x != cur_cam_range_ptr->m_is_flip_x ||
			cam_range_ptr->m_is_flip_y != cur_cam_range_ptr->m_is_flip_y)
		{
			cam_flag |= kPOSubFlagCamGeoFlip;
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamGeoRotation))
	{
		if (cam_range_ptr->m_rotation != cur_cam_range_ptr->m_rotation)
		{
			cam_flag |= kPOSubFlagCamGeoRotation;
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamGeoRange))
	{
		if (cam_range_ptr->m_range != cur_cam_range_ptr->m_range)
		{
			cam_flag |= kPOSubFlagCamGeoRange;
		}
	}
	return cam_flag;
}

i32 CProcessor::onProcessCamExposureSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	i32 cam_flag = 0;
	CameraInfo* cur_cam_info_ptr = m_cam_param_ptr->getCameraInfo();
	CameraExposure* cur_cam_exposure_ptr = m_cam_param_ptr->getCameraExposure();
	CameraExposure* cam_exposure_ptr = cam_param_ptr->getCameraExposure();

	//set gain for control
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamGain))
	{
		if (CPOBase::checkRange(cam_exposure_ptr->m_gain,
			cur_cam_info_ptr->m_gain_min, cur_cam_info_ptr->m_gain_max))
		{
			if (cam_exposure_ptr->m_gain != cur_cam_exposure_ptr->m_gain)
			{
				cam_flag |= kPOSubFlagCamGain;
			}
		}
	}
	//set exposure for control
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamExposure))
	{
		if (CPOBase::checkRange(cam_exposure_ptr->m_exposure,
			cur_cam_info_ptr->m_exposure_min, cur_cam_info_ptr->m_exposure_max))
		{
			if (cam_exposure_ptr->m_exposure != cur_cam_exposure_ptr->m_exposure)
			{
				cam_flag |= kPOSubFlagCamExposure;
			}
		}
	}
	//set auto_exposure mode for control
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamAEMode))
	{
		if (CPOBase::checkIndex(cam_exposure_ptr->m_autoexp_mode, kCamAEModeCount) &&
			cam_exposure_ptr->m_autoexp_mode != cur_cam_exposure_ptr->m_autoexp_mode)
		{
			cam_flag |= kPOSubFlagCamAEMode;
		}
	}
	//set region for AEControl
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamAEWindow))
	{
		if (cam_exposure_ptr->m_autoexp_window.isEmpty())
		{
			cam_exposure_ptr->m_autoexp_window = cur_cam_exposure_ptr->m_autoexp_window;
		}
		else if (cam_exposure_ptr->m_autoexp_window != cur_cam_exposure_ptr->m_autoexp_window)
		{
			cam_flag |= kPOSubFlagCamAEWindow;
		}
	}
	//set auto intensity for AEControl
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamAEBrightness))
	{
		if (!CPOBase::checkRange(cam_exposure_ptr->m_auto_brightness, cur_cam_info_ptr->m_brightness_min, cur_cam_info_ptr->m_brightness_max))
		{
			cam_exposure_ptr->m_auto_brightness = cur_cam_exposure_ptr->m_auto_brightness;
		}
		else if (cam_exposure_ptr->m_auto_brightness != cur_cam_exposure_ptr->m_auto_brightness)
		{
			cam_flag |= kPOSubFlagCamAEBrightness;
		}
	}
	//set AEGain
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamAEGain))
	{
		//set auto mingain for AEControl
		if (!CPOBase::checkRange(cam_exposure_ptr->m_autogain_min, cur_cam_info_ptr->m_gain_min, cur_cam_info_ptr->m_gain_max))
		{
			cam_exposure_ptr->m_autogain_min = cur_cam_exposure_ptr->m_autogain_min;
		}
		else if (cam_exposure_ptr->m_autogain_min != cur_cam_exposure_ptr->m_autogain_min)
		{
			cam_flag |= kPOSubFlagCamAEGain;
		}

		//set auto maxgain for AEControl
		if (!CPOBase::checkRange(cam_exposure_ptr->m_autogain_max, cur_cam_info_ptr->m_gain_min, cur_cam_info_ptr->m_gain_max))
		{
			cam_exposure_ptr->m_autogain_max = cur_cam_exposure_ptr->m_autogain_max;
		}
		else if (cam_exposure_ptr->m_autogain_max != cur_cam_exposure_ptr->m_autogain_max)
		{
			cam_flag |= kPOSubFlagCamAEGain;
		}
	}
	//set AEExposure
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamAEExposure))
	{
		//set auto minexptime for AEcontrol
		if (!CPOBase::checkRange(cam_exposure_ptr->m_autoexp_min, cur_cam_info_ptr->m_exposure_min, cur_cam_info_ptr->m_exposure_max))
		{
			cam_exposure_ptr->m_autoexp_min = cur_cam_exposure_ptr->m_autoexp_min;
		}
		else if (cam_exposure_ptr->m_autoexp_min != cur_cam_exposure_ptr->m_autoexp_min)
		{
			cam_flag |= kPOSubFlagCamAEExposure;
		}

		//set auto maxexptime for AEControl
		if (!CPOBase::checkRange(cam_exposure_ptr->m_autoexp_max, cur_cam_info_ptr->m_exposure_min, cur_cam_info_ptr->m_exposure_max))
		{
			cam_exposure_ptr->m_autoexp_max = cur_cam_exposure_ptr->m_autoexp_max;
		}
		else if (cam_exposure_ptr->m_autoexp_max != cur_cam_exposure_ptr->m_autoexp_max)
		{
			cam_flag |= kPOSubFlagCamAEExposure;
		}
	}
	return cam_flag;
}

i32 CProcessor::onProcessCamCorrectionSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	i32 cam_flag = 0;
	CameraCorrection* cur_cam_corr_ptr = m_cam_param_ptr->getCameraCorrection();
	CameraCorrection* cam_corr_ptr = cam_param_ptr->getCameraCorrection();
	CameraInfo* cur_cam_info_ptr = m_cam_param_ptr->getCameraInfo();

	if (CPOBase::bitCheck(sflag, kPOSubFlagCamCorrectionGamma))
	{
		if (CPOBase::checkRange(cam_corr_ptr->m_gamma,
			cur_cam_info_ptr->m_gamma_min, cur_cam_info_ptr->m_gamma_max))
		{
			if (cam_corr_ptr->m_gamma != cur_cam_corr_ptr->m_gamma)
			{
				cam_flag |= kPOSubFlagCamCorrectionGamma;
			}
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamCorrectionContrast))
	{
		if (CPOBase::checkRange(cam_corr_ptr->m_contrast,
			cur_cam_info_ptr->m_contrast_min, cur_cam_info_ptr->m_contrast_max))
		{
			if (cam_corr_ptr->m_contrast != cur_cam_corr_ptr->m_contrast)
			{
				cam_flag |= kPOSubFlagCamCorrectionContrast;
			}
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamCorrectionSaturation))
	{
		if (CPOBase::checkRange(cam_corr_ptr->m_saturation,
			cur_cam_info_ptr->m_saturation_min, cur_cam_info_ptr->m_saturation_max))
		{
			if (cam_corr_ptr->m_saturation != cur_cam_corr_ptr->m_saturation)
			{
				cam_flag |= kPOSubFlagCamCorrectionSaturation;
			}
		}
	}
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamCorrectionSharpness))
	{
		if (CPOBase::checkRange(cam_corr_ptr->m_sharpness,
			cur_cam_info_ptr->m_sharpness_min, cur_cam_info_ptr->m_sharpness_max))
		{
			if (cam_corr_ptr->m_sharpness != cur_cam_corr_ptr->m_sharpness)
			{
				cam_flag |= kPOSubFlagCamCorrectionSharpness;
			}
		}
	}
	return cam_flag;
}

i32 CProcessor::onProcessCamStrobeSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	i32 cam_flag = 0;
	CameraStrobe* cur_cam_strobe_ptr = m_cam_param_ptr->getCameraStrobe();
	CameraStrobe* cam_strobe_ptr = cam_param_ptr->getCameraStrobe();

	if (CPOBase::bitCheck(sflag, kPOSubFlagCamStrobe))
	{
		bool use_changed = (cam_strobe_ptr->m_use_strobe != cur_cam_strobe_ptr->m_use_strobe);
		bool mode_changed = (cam_strobe_ptr->m_is_auto_strobe != cur_cam_strobe_ptr->m_is_auto_strobe);
		bool level_changed = (CPOBase::isCount(cam_strobe_ptr->m_strobe_level) &&
			cam_strobe_ptr->m_strobe_level != cur_cam_strobe_ptr->m_strobe_level);
		bool delay_changed = (CPOBase::isCount(cam_strobe_ptr->m_strobe_pwm_delay) &&
			cam_strobe_ptr->m_strobe_pwm_delay != cur_cam_strobe_ptr->m_strobe_pwm_delay);
		bool width_changed = (CPOBase::isCount(cam_strobe_ptr->m_strobe_pwm_width) &&
			cam_strobe_ptr->m_strobe_pwm_width != cur_cam_strobe_ptr->m_strobe_pwm_width);

		if (use_changed || mode_changed || level_changed || delay_changed || width_changed)
		{
			cam_flag |= kPOSubFlagCamStrobe;
		}
	}
	return cam_flag;
}

i32 CProcessor::onProcessCameraSetting(i32 sflag, CameraSetting* cam_param_ptr)
{
	if (!cam_param_ptr || !m_cam_param_ptr)
	{
		return kPOErrInvalidData;
	}

	i32 cam_flag = 0;
	cam_flag |= onProcessCamColorSetting(sflag, cam_param_ptr);
	cam_flag |= onProcessCamShutterSetting(sflag, cam_param_ptr);
	cam_flag |= onProcessCamRangeSetting(sflag, cam_param_ptr);
	cam_flag |= onProcessCamExposureSetting(sflag, cam_param_ptr);
	cam_flag |= onProcessCamCorrectionSetting(sflag, cam_param_ptr);
	cam_flag |= onProcessCamStrobeSetting(sflag, cam_param_ptr);

	//add external flag to current camera setting
	if (CPOBase::bitCheck(sflag, kPOSubFlagCamClearFocus))
	{
		cam_flag |= kPOSubFlagCamClearFocus;
	}

	if (cam_flag != 0 && m_cam_capture_ptr)
	{
		if (m_cam_capture_ptr->setCameraSetting(cam_flag, cam_param_ptr))
		{
			return kPOSuccess;
		}
	}
	else
	{
		printlog_lv1(QString("Camera%1 change nothing, flag is %2").arg(m_cam_param_ptr->getCamID()).arg(sflag));
	}
	return kPOErrInvalidOper;
}

i32 CProcessor::onRequestTeachJob(Packet* packet_ptr)
{
	if (!isProcessorAvailable())
	{
		printlog_lv1(QString("PUstate:%1 is not compatibility[onRequestTeachModel].").arg(m_pu_state));
		return kSCErrInvalidPUState;
	}
	if (isProcessorFrozenMode())
	{
		printlog_lv1("Currently freezen mode.");
		return kSCErrInvalidPUState;
	}

	if (m_job_manager_ptr->getCurJob(m_pu_id) == NULL)
	{
		printlog_lv1(QString("Current job:%1 is not valid[onRequestTeachModel].").arg(m_pu_id));
		return kSCErrInvalidJob;
	}

	captureImage(kCamSnapTeach);
	return kPOSuccess;
}

i32 CProcessor::onRequestUpdateJobThumbnail(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 sub_cmd = packet_ptr->getSubCmd();
	if (sub_cmd != kSCSubTypeJobOperByID)
	{
		return kPOErrInvalidOper;
	}

	i32 job_id = packet_ptr->getReservedi32(0);
	CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id);
	if (!job_ptr || !isCompatibilityJob(job_ptr))
	{
		printlog_lvs2(QString("Can't update job:%1 thumbnail").arg(job_id), LOG_SCOPE_APP);
		return kSCErrInvalidJob;
	}

	if (!job_ptr->updateThumbnail(m_sc_engine.getImageData()))
	{
		printlog_lvs2(QString("Can't update job:%1 thumbnail with NULL image").arg(job_id), LOG_SCOPE_APP);
		return kPOErrInvalidData;
	}

	uploadJobMeta(packet_ptr, job_ptr);
	return kPOSuccess;
}

i32 CProcessor::onRequestJobInteractive(Packet* packet_ptr)
{
	//check PUState and current job
	if (!isProcessorStop() || !packet_ptr)
	{
		printlog_lvs2(QString("Can't accept interactive job with PUstate[%1].").arg(m_pu_state), LOG_SCOPE_APP);
		return kSCErrInvalidPUState;
	}

	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (!job_ptr)
	{
		printlog_lvs2("Can't interactive with invalid current job.", LOG_SCOPE_APP);
		return kSCErrInvalidJob;
	}

	//check and read packet
	i32 sub_cmd = packet_ptr->getSubCmd();
	i32 job_id = packet_ptr->getReservedi32(0);
	u8* buffer_ptr = packet_ptr->getData();

	if (job_ptr->getJobID() != job_id)
	{
		printlog_lvs2(QString("Can't interactive with job_id[%1].").arg(job_id), LOG_SCOPE_APP);
		return kSCErrInvalidJob;
	}

	//interactive
	i32 result = kPOErrCmdFail;
	bool is_tag_updated = false;
	m_tag_manager_ptr->removeJob(job_ptr);
	{
		switch (sub_cmd)
		{
			case kSCSubTypeAddTool:
			{
				result = onRequestAddTool(packet_ptr, job_ptr);
				break;
			}
			case kSCSubTypeDeleteTool:
			{
				result = onRequestDeleteTool(packet_ptr, job_ptr);
				break;
			}
			case kSCSubTypeModifyTool:
			{
				result = onRequestModifyTool(packet_ptr, job_ptr, is_tag_updated);
				break;
			}
			default:
			{
				return kPOErrInvalidOper;
			}
		}
	}
	m_tag_manager_ptr->addJob(job_ptr);
	m_tag_manager_ptr->addJobResult(job_ptr, m_sc_engine.getJobResult());

	//update job io settings(modbus, opc)
	if (is_tag_updated)
	{
		i32 update = job_ptr->updateIOSettings(m_tag_manager_ptr);
		if (update != kJobUpdateFlagNone)
		{
			if (CPOBase::bitCheck(update, kJobUpdateFlagModbus))
			{
				updateModBusSetting(job_ptr->m_modbus_comm);
			}
			if (CPOBase::bitCheck(update, kJobUpdateFlagOpc))
			{
				updateOpcSetting(job_ptr);
			}

			//upload changed job content
			uploadJobData(job_ptr, kJobDataIOSetting, i32vector());
		}
	}

	//upload interactive result
	if (result != kPOSuccess)
	{
		return result;
	}

	onProcessInteractive();
	uploadJobResult(kSCCmdSnapResult, kResultWithAll, true);
	job_ptr->setChanged(kJobContent);
	return kPOSuccess;
}

i32 CProcessor::onRequestAddTool(Packet* packet_ptr, CJobUnit* job_ptr)
{
	//check packet
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!job_ptr || !buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidPacket;
	}

	//add and train tool
	CBaseTool* new_tool_ptr = CBaseTool::newMemRead(kToolWithSimple, buffer_ptr, buffer_size);
	if (!new_tool_ptr)
	{
		POSAFE_DELETE(new_tool_ptr);
		return kPOErrInvalidData;
	}
	new_tool_ptr->setReserved(0, job_ptr->m_cam_id);

	//read & update tool name
	i32 tool_id;
	postring tool_name;
	CPOBase::memRead(buffer_ptr, buffer_size, tool_name);

	i32 seq_index = job_ptr->addTool(new_tool_ptr, tool_name);
	if (!CPOBase::checkIndex(seq_index, kJobToolMaxCount))
	{
		POSAFE_DELETE(new_tool_ptr);
		return kSCErrInvalidJob;
	}

	tool_id = new_tool_ptr->getToolID();
	if (!m_sc_engine.addOneTool(job_ptr, seq_index))
	{
		job_ptr->deleteTool(tool_id);
		return kSCErrEngine0;
	}

	//response packet
	i32 len = new_tool_ptr->memSize(kToolWithAll);
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	if (len > 0 && pak)
	{
		buffer_size = len;
		pak->allocateBuffer(len, buffer_ptr);

		new_tool_ptr->memWrite(kToolWithAll, buffer_ptr, buffer_size);
		sendPacketToNet(pak, buffer_ptr);
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestModifyTool(Packet* packet_ptr, CJobUnit* job_ptr, bool& is_tag_updated)
{
	//check packet
	i32 tool_id = packet_ptr->getReservedi32(1);
	i32 mode = packet_ptr->getReservedi32(2);
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!job_ptr || !buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidData;
	}

	//get modified tool and process
	i32 seq_index = -1;
	CBaseTool* mod_tool_ptr = NULL;

	if (CPOBase::bitCheck(mode, kToolInteractiveParam) || CPOBase::bitCheck(mode, kToolInteractiveRegion))
	{
		//read tool param
		seq_index = job_ptr->updateTool(tool_id, buffer_ptr, buffer_size);
		printlog_lvs2(QString("Tool[%1] interactive. mode %2").arg(tool_id).arg(mode), LOG_SCOPE_APP);

		if (!CPOBase::checkIndex(seq_index, kJobToolMaxCount))
		{
			printlog_lvs2(QString("Tool[%1] replace is failed.").arg(tool_id), LOG_SCOPE_APP);
			return kPOErrInvalidOper;
		}
		mod_tool_ptr = job_ptr->getToolData(seq_index);

		if (CPOBase::bitCheck(mode, kToolInteractiveRegion))
		{
			if (mod_tool_ptr->isRegionTool())
			{
				Recti range;
				CVirtualRegionTool* region_tool_ptr = dynamic_cast<CVirtualRegionTool*>(mod_tool_ptr);
				if (region_tool_ptr)
				{
					region_tool_ptr->clearSearchMaskImage();
					region_tool_ptr->makeSearchMaskImage(NULL, NULL, range);
				}
			}
		}
	}

	//when changed order of tool
	if (CPOBase::bitCheck(mode, kToolInteractiveReorder))
	{
		//read and check toolnum
		i32vector order_vec;
		CPOBase::memReadVector(order_vec, buffer_ptr, buffer_size);
		printlog_lvs2(QString("Reorder for tool[%1]").arg(tool_id), LOG_SCOPE_APP);

		if (order_vec.size() != job_ptr->getToolCount())
		{
			printlog_lvs2(QString("Reorder is failed, reorder vecsize[%1] cur jobid[%2]")
							.arg(order_vec.size()).arg(job_ptr->getToolCount()), LOG_SCOPE_APP);
			return kPOErrInvalidData;
		}

		if (!job_ptr->updateToolOrder(order_vec, m_sc_engine.getJobResult()->m_tool_result_vec))
		{
			printlog_lvs2(QString("Update for reorder is failed, tid[%1]").arg(tool_id), LOG_SCOPE_APP);
			return kPOErrInvalidData;
		}
	}

	//when tool need study
	seq_index = job_ptr->getToolIndex(tool_id);
	if (CPOBase::bitCheck(mode, kToolInteractiveStudy))
	{
		m_sc_engine.studyOneTool(job_ptr, seq_index);
	}

	mod_tool_ptr = job_ptr->getToolData(seq_index);
	if (!mod_tool_ptr)
	{
		return kPOErrInvalidOper;
	}

	//update tool tags
	is_tag_updated = mod_tool_ptr->updateToolTags(false);

	//response packet
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	if (pak)
	{
		if (CPOBase::bitCheck(mode, kToolInteractiveStudy) || is_tag_updated)
		{
			i32 len = mod_tool_ptr->memSize(kToolWithAll);
			pak->allocateBuffer(len, buffer_ptr);
			pak->setReservedi32(3, 0xFFFF); //tool modified flag

			buffer_size = len;
			mod_tool_ptr->memWrite(kToolWithAll, buffer_ptr, buffer_size);
		}
		sendPacketToNet(pak, buffer_ptr);
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestDeleteTool(Packet* packet_ptr, CJobUnit* job_ptr)
{
	//check packet
	i32 tool_id = packet_ptr->getReservedi32(1);
	u8* buffer_ptr = packet_ptr->getData();
	if (!job_ptr)
	{
		return kSCErrInvalidJob;
	}

	//delete tool
	i32 seq_index = job_ptr->deleteTool(tool_id);
	if (!CPOBase::checkIndex(seq_index, kJobToolMaxCount))
	{
		printlog_lvs2(QString("DeleteTool Oper Failed."), LOG_SCOPE_APP);
		return kPOErrInvalidOper;
	}

	//response
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	if (pak)
	{
		sendPacketToNet(pak);
	}

	//update opc, modbus, io input/output setting
	i32 update = job_ptr->removeIOSetting(tool_id);
	if (update != kJobUpdateFlagNone)
	{
		if (CPOBase::bitCheck(update, kJobUpdateFlagDo))
		{
			updateDOSetting(job_ptr->m_do_setting, false);
		}
		if (CPOBase::bitCheck(update, kJobUpdateFlagModbus))
		{
			updateModBusSetting(job_ptr->m_modbus_comm);
		}
		if (CPOBase::bitCheck(update, kJobUpdateFlagFtp))
		{
			updateFtpSetting(job_ptr->m_ftp_comm);
		}
		if (CPOBase::bitCheck(update, kJobUpdateFlagOpc))
		{
			updateOpcSetting(job_ptr);
		}

		//upload changed job content
		uploadJobData(job_ptr, kJobDataIOSetting, i32vector());
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestOperation(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 result = kPOErrInvalidOper;
	i32 sub_cmd = packet_ptr->getSubCmd();
	switch (sub_cmd)
	{
		//수학도구의 콤파일가능성을 평가한다.
		case kSCSubTypeCheckMath:
		{
			result = onRequestCheckMathExpression(packet_ptr);
			break;
		}
		//문자렬도구의 콤파일가능성을 평가한다.
		case kSCSubTypeCheckString:
		{
			result = onRequestCheckStringExpression(packet_ptr);
			break;
		}
		//론리도구의 콤파일가능성을 평가한다.
		case kSCSubTypeCheckLogic:
		{
			result = onRequestCheckLogicExpression(packet_ptr);
			break;
		}
		default:
		{
			result = kPOErrInvalidOper;
			break;
		}
	}
	return result;
}

i32 CProcessor::onRequestCheckMathExpression(Packet* packet_ptr)
{
	if (!packet_ptr || !packet_ptr->getData())
	{
		return kPOErrInvalidPacket;
	}

	postring math_expression;
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	CPOBase::memRead(buffer_ptr, buffer_size, math_expression);

	postring error_message;
	i32 code = m_sc_engine.checkMathExpression(math_expression, error_message);
	Packet* pak = Packet::makeRespPacket(packet_ptr);

	if (code == kEngineResultOK)
	{
		pak->setReservedi32(0, kPOSuccess);
	}
	else
	{
		pak->setReservedi32(0, kSCErrEngine0 + code);

		i32 len = CPOBase::getStringMemSize(error_message);
		u8* buffer_ptr;
		i32 buffer_size = len;
		pak->allocateBuffer(len, buffer_ptr);
		CPOBase::memWrite(buffer_ptr, buffer_size, error_message);
	}
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::onRequestCheckStringExpression(Packet* packet_ptr)
{
	if (!packet_ptr || !packet_ptr->getData())
	{
		return kPOErrInvalidPacket;
	}

	postring str_expression;
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	CPOBase::memRead(buffer_ptr, buffer_size, str_expression);

	postring error_message;
	i32 code = m_sc_engine.checkStringExpression(str_expression, error_message);
	Packet* pak = Packet::makeRespPacket(packet_ptr);

	if (code == kEngineResultOK)
	{
		pak->setReservedi32(0, kPOSuccess);
	}
	else
	{
		pak->setReservedi32(0, kSCErrEngine0 + code);

		i32 len = CPOBase::getStringMemSize(error_message);
		u8* buffer_ptr;
		i32 buffer_size = len;
		pak->allocateBuffer(len, buffer_ptr);
		CPOBase::memWrite(buffer_ptr, buffer_size, error_message);
	}
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::onRequestCheckLogicExpression(Packet* packet_ptr)
{
	if (!packet_ptr || !packet_ptr->getData())
	{
		return kPOErrInvalidPacket;
	}

	postring logic_expression;
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	CPOBase::memRead(buffer_ptr, buffer_size, logic_expression);

	postring error_message;
	i32 code = m_sc_engine.checkLogicExpression(logic_expression, error_message);
	Packet* pak = Packet::makeRespPacket(packet_ptr);

	if (code == kEngineResultOK)
	{
		pak->setReservedi32(0, kPOSuccess);
	}
	else
	{
		pak->setReservedi32(0, kSCErrEngine0 + code);

		i32 len = CPOBase::getStringMemSize(error_message);
		u8* buffer_ptr;
		i32 buffer_size = len;

		pak->allocateBuffer(len, buffer_ptr);
		CPOBase::memWrite(buffer_ptr, buffer_size, error_message);
	}
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::onRequestCalib(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !isProcessorStop())
	{
		printlog_lvs2("Can't accept calib with PUState.", LOG_SCOPE_APP);
		return kSCErrInvalidPUState;
	}

	bool is_update = false;
	i32 sub_cmd = packet_ptr->getSubCmd();
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	CameraCalib* calib_param_ptr = m_tmp_cam_param.getCameraCalib();
	CameraCalib* cur_calib_param_ptr = m_cam_param_ptr->getCameraCalib();

	switch (sub_cmd)
	{
		case kSCSubTypeCalibStart:
		{
			setProcessorCalibMode(true);
			uploadEndPacket(packet_ptr);
			printlog_lvs2("Calib subcmd is done.[kSCSubTypeCalibStart]", LOG_SCOPE_APP);
			break;
		}
		case kSCSubTypeCalibClose:
		{
			if (!isProcessorCalibMode())
			{
				return kSCErrInvalidPUState;
			}
			setProcessorCalibMode(false);
			uploadEndPacket(packet_ptr);
			printlog_lvs2("Calib subcmd is done.[kSCSubTypeCalibClose]", LOG_SCOPE_APP);
			break;
		}
		case kSCSubTypeCalibBoard:
		{
			if (!isProcessorCalibMode())
			{
				return kSCErrInvalidPUState;
			}
			calib_param_ptr->memBoardRead(buffer_ptr, buffer_size);
			uploadEndPacket(packet_ptr);
			printlog_lvs2("Calib subcmd is done.[kSCSubTypeCalibBoard]", LOG_SCOPE_APP);
			break;
		}
		case kSCSubTypeCalibSnap:
		{
			if (!isProcessorCalibMode())
			{
				return kSCErrInvalidPUState;
			}

			captureImage(kCamSnapCalib);
			printlog_lvs2("Calib subcmd is done.[kSCSubTypeCalibSnap]", LOG_SCOPE_APP);
			break;
		}
		case kSCSubTypeCalibProcess:
		{
			if (!buffer_ptr)
			{
				return kPOErrInvalidData;
			}

			i32 ret_code = onProcessCameraCalib(buffer_ptr, buffer_size);
			if (ret_code == kPOSuccess)
			{
				is_update = true;
				uploadCalibResult(kSCCmdCalib, kSCSubTypeCalibProcess, true);
				g_sc_disk.writeAllSettings(g_main_app_ptr);
				m_io_manager_ptr->outputCalib(m_cam_param_ptr, 1);
				printlog_lvs2("Calib process done.[kSCSubTypeCalibProcess]", LOG_SCOPE_APP);
			}
			else
			{
				printlog_lvs2(QString("Calib process failed.[%1].[kSCSubTypeCalibProcess]").arg(ret_code), LOG_SCOPE_APP);
				return ret_code;
			}
			break;
		}
		case kSCSubTypeCalibUse:
		{
			is_update = true;
			if (cur_calib_param_ptr->setUseCalib(packet_ptr->getReservedb8(0)))
			{
				uploadEndPacket(packet_ptr);
				g_sc_disk.writeAllSettings(g_main_app_ptr);
			}
			printlog_lvs2("Calib subcmd is done.[kSCSubTypeCalibUse]", LOG_SCOPE_APP);
			break;
		}
		case kSCSubTypeCalibRemove:
		{
			is_update = true;
			cur_calib_param_ptr->init();
			uploadCalibResult(kSCCmdCalib, kSCSubTypeCalibRemove, true);

			g_sc_disk.writeAllSettings(g_main_app_ptr);
			m_io_manager_ptr->outputCalib(m_cam_param_ptr, 1);
			printlog_lvs2("Calib subcmd is done.[kSCSubTypeCalibRemove]", LOG_SCOPE_APP);
			break;
		}
		case kSCSubTypeCalibImport:
		{
			if (!buffer_ptr)
			{
				return kPOErrInvalidData;
			}

			i32 calib_size;
			CameraCalib tmp_calib_param;
			CPOBase::memRead(calib_size, buffer_ptr, buffer_size);
			if (tmp_calib_param.memRead(buffer_ptr, buffer_size))
			{
				CameraInfo* cam_info_ptr = m_cam_param_ptr->getCameraInfo();
				if (cam_info_ptr->m_max_width == tmp_calib_param.m_calibed_width &&
					cam_info_ptr->m_max_height == tmp_calib_param.m_calibed_height)
				{
					is_update = true;
					cur_calib_param_ptr->setValue(tmp_calib_param);
					uploadCalibResult(kSCCmdCalib, kSCSubTypeCalibImport, true);

					m_sc_engine.setCalibParam(m_cam_param_ptr);
					m_io_manager_ptr->outputCalib(m_cam_param_ptr, 1);
					g_sc_disk.writeAllSettings(g_main_app_ptr);
					printlog_lvs2("Calib import done.[kSCSubTypeCalibImport]", LOG_SCOPE_APP);
				}
				else
				{
					printlog_lvs2("Calib import uncompatibility.[kSCSubTypeCalibImport]", LOG_SCOPE_APP);
					return kPOErrInvalidOper;
				}
			}
			else
			{
				printlog_lvs2("Calib import failed.[kSCSubTypeCalibImport]", LOG_SCOPE_APP);
				return kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeCalibExport:
		{
			uploadCalibExport(packet_ptr);
			printlog_lvs2("Calib export done.[kSCSubTypeCalibExport]", LOG_SCOPE_APP);
			break;
		}
		default:
		{
			i32 sub_cmd = packet_ptr->getSubCmd();
			printlog_lvs2(QString("Camera Calib is failed. subcmd is %1").arg(sub_cmd), LOG_SCOPE_APP);
			return kPOErrInvalidOper;
		}
	}

	//update calibration parameter to current job
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (is_update && job_ptr)
	{
		job_ptr->setCameraSetting(m_cam_param_ptr, kCamSettingUpdateCalib);
		job_ptr->setChanged(kJobContent);
		printlog_lvs2(QString("Calib info is updated in job[%1]").arg(job_ptr->getJobID()), LOG_SCOPE_APP);
	}
	return kPOSuccess;
}

void CProcessor::onBroadcastPacket(i32 cmd, i32 mode)
{
	//write log message
	printlog_lvs4(QString("Processor[%1] recived broadcast cmd[%2], md[%3]")
					.arg(m_pu_id).arg(cmd).arg(mode), LOG_SCOPE_APP);

	//change PUState for PUControl
	if (isUsedProcessor())
	{
		switch (cmd)
		{
			case kSCCmdIntRunSystem:
			{
				i32 mode = m_pu_mode;
				CPOBase::bitRemove(mode, kSCModeUserEdit);
				updatePUState(kSCStateRun, mode);
				break;
			}
			case kSCCmdIntStopSystem:
			{
				i32 mode = m_pu_mode;
				CPOBase::bitRemove(mode, kSCModeUserEdit);
				updatePUState(kSCStateStop, mode);
				break;
			}
			case kSCCmdIntIdle:
			{
				onInternalIdle(NULL, kPUModeThread);
				break;
			}
			case kSCCmdIntRunThread:
			{
				onInternalRunThread();
				break;
			}
			case kSCCmdIntStopThread:
			{
				onInternalStopThread();
				break;
			}
		}
	}

	if (cmd == kSCCmdIntDisconnect)
	{
		onInternalOffline();
	}

	//release semaphore for unblock cmd
	switch (mode)
	{
		case kPUCmdNonBlock:
		{
			break;
		}
		case kPUCmdBlock:
		{
			printlog_lv1(QString("Processor[%1] release global shemaphore[%2]")
							.arg(m_pu_id).arg(g_shp_manager.available()));

			g_shp_manager.release();

			i32 available = g_shp_manager.available();
			if (available > PO_UNIT_COUNT)
			{
				printlog_lv1(QString("g_shp_manager available is %1").arg(available));
				assert(false);
			}
			break;
		}
	}
}

void CProcessor::onReadPacket(Packet* packet_ptr, i32 mode, i32 conn_mode)
{
	//check processor queue
	g_main_app_ptr->decProcessorQueue(m_pu_id);

	if (!packet_ptr)
	{
		return;
	}

	bool is_used_cmd = true;
	i32 nid = packet_ptr->getID();
	i32 cmd = packet_ptr->getCmd();
	i32 code = kPOErrCmdFail;
	singlelog_lvs3(QString("Processor%1 recive packet, cmd[%2], md[%3]").arg(m_pu_id).arg(cmd).arg(mode), LOG_SCOPE_APP);

	switch (cmd)
	{
		case kSCCmdIntUsed:
		{
			singlelog_lv1("kSCCmdIntUsed is");
			code = onInternalUsed(packet_ptr, kPUModeThread);
			break;
		}
		case kSCCmdIntAdminUsed:
		{
			singlelog_lv1("kSCCmdIntAdminUsed is");
			code = onInternalUsed(packet_ptr, kPUModeAdmin);
			break;
		}
		case kSCCmdIntSelectCamera:
		{
			singlelog_lv1("kSCCmdIntSelectCamera is");
			code = onInternalSelectCamera(packet_ptr, mode);
			break;
		}
		default:
		{
			is_used_cmd = false;
			break;
		}
	}

	if (!is_used_cmd && isUsedProcessor())
	{
		switch (cmd)
		{
			//internal request from main application
			case kSCCmdIntAdminSync:
			{
				singlelog_lv1("kSCCmdIntAdminSync is");
				code = onInternalAdminSync(packet_ptr);
				break;
			}
			case kSCCmdIntExtTrigger:
			{
				singlelog_lv1("kSCCmdIntExtTrigger is");
				code = onInternalExtTrigger(packet_ptr);
				break;
			}
			case kSCCmdIntExtTeaching:
			{
				singlelog_lv1("kSCCmdIntExtTeaching is");
				cmd = kSCCmdTeachJob; //must need for exception
				code = onRequestTeachJob(NULL);
				break;
			}
			case kSCCmdIntJobSetting:
			{
				singlelog_lv1("kSCCmdIntJobSetting is");
				code = onInternalJobSetting(packet_ptr);
				break;
			}
			case kSCCmdIntSelectJob:
			{
				singlelog_lv1("kSCCmdIntSelectJob is");
				cmd = kSCCmdSelectJob; //must need for exception
				code = onInternalSelectJob(cmd, packet_ptr);
				break;
			}
			case kSCCmdIntSelectProgram:
			{
				singlelog_lv1("kSCCmdIntSelectProgram is");
				code = onInternalSelectProgram(packet_ptr);
				break;
			}
			case kSCCmdIntRunThread:
			{
				singlelog_lv1("kSCCmdIntRunThread is");
				code = onInternalRunThread();
				break;
			}
			case kSCCmdIntStopThread:
			{
				singlelog_lv1("kSCCmdIntStopThread is");
				code = onInternalStopThread();
				break;
			}
			case kSCCmdIntNetOpen:
			{
				singlelog_lv1("kSCCmdIntNetOpen is");
				code = onInternalNetOpen(packet_ptr);
				break;
			}
			case kSCCmdIntNetClose:
			{
				singlelog_lv1("kSCCmdIntNetClose is");
				code = onInternalNetClose(packet_ptr);
				break;
			}
			case kSCCmdIntIdle:
			{
				singlelog_lv1("kSCCmdIntIdle is");
				code = onInternalIdle(packet_ptr, kPUModeThread);
				break;
			}
			case kSCCmdIntDisconnect:
			{
				singlelog_lv1("kSCCmdIntDisconnect is");
				code = onInternalOffline();
				break;
			}
			case kSCCmdIntAdminIdle:
			{
				singlelog_lv1("kSCCmdIntAdminRemove is");
				code = onInternalIdle(packet_ptr, kPUModeAdmin);
				break;
			}

			//external request through network
			case kSCCmdRun:
			{
				singlelog_lv1("kSCCmdRun is");
				code = onRequestRunProcess(packet_ptr);
				break;
			}
			case kSCCmdStop:
			{
				singlelog_lv1("kSCCmdStop is");
				code = onRequestStopProcess(packet_ptr);
				break;
			}
			case kSCCmdFreezeMode:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdFreezeMode is");
					code = onRequestFreezeMode(packet_ptr);
				}
				break;
			}
			case kSCCmdTestMode:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdTest is");
					code = onRequestTestMode(packet_ptr);
				}
				break;
			}
			case kSCCmdInteractiveMode:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdInteractive is");
					code = onRequestInteractiveMode(packet_ptr);
				}
				break;
			}
			case kSCCmdTrigger:
			{
				singlelog_lv1("kSCCmdTrigger is");
				code = onRequestManualTrigger(packet_ptr);
				break;
			}
			case kSCCmdGetJob:
			{
				singlelog_lv1("kSCCmdGetJob is");
				code = onRequestGetJob(packet_ptr);
				break;
			}
			case kSCCmdAddJob:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdAddJob is");
					code = onRequestAddJob(packet_ptr);
				}
				break;
			}
			case kSCCmdSaveJob:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdSaveJob is");
					code = onRequestSaveJob(packet_ptr);
				}
				break;
			}
			case kSCCmdDeleteJob:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdDeleteJob is");
					code = onRequestDeleteJob(packet_ptr);
				}
				break;
			}
			case kSCCmdTeachJob:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdTeachJob is");
					code = onRequestTeachJob(packet_ptr);
				}
				break;
			}
			case kSCCmdFindJob:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdFindJob is");
					code = onRequestSearchJobs(packet_ptr);
				}
				break;
			}
			case kSCCmdGetJobMeta:
			{
				singlelog_lv1("kSCCmdGetJobMeta is");
				code = onRequestGetJobMeta(packet_ptr);
				break;
			}
			case kSCCmdSetJobMeta:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdSetJobMeta is");
					code = onRequestSetJobMeta(packet_ptr);
				}
				break;
			}
			case kSCCmdUpdateJobThumbnail:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdUpdateJobThumbnail is");
					code = onRequestUpdateJobThumbnail(packet_ptr);
				}
				break;
			}
			case kSCCmdInteractiveJob:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdInteractiveJob is");
					code = onRequestJobInteractive(packet_ptr);
				}
				break;
			}
			case kSCCmdCameraExposure:
			{
				singlelog_lv1(QString("kSCCmdCameraSetting[%1] is").arg(packet_ptr->getSubCmd()));
				code = onRequestCameraSetting(packet_ptr);
				responseCameraExposure(packet_ptr, code);
				break;
			}
			case kSCCmdCameraRange:
			{
				singlelog_lv1("kSCCmdCameraRange is");
				code = onRequestCameraSetting(packet_ptr);
				responseCameraRange(packet_ptr, code);
				break;
			}
			case kSCCmdCameraColor:
			{
				singlelog_lv1("kSCCmdCameraColor is");
				code = onRequestCameraSetting(packet_ptr);
				responseCameraColor(packet_ptr, code);
				break;
			}
			case kSCCmdCameraCorrection:
			{
				singlelog_lv1("kSCCmdCameraCorrection is");
				code = onRequestCameraSetting(packet_ptr);
				responseCameraCorrection(packet_ptr, code);
				break;
			}
			case kSCCmdCameraShutter:
			{
				singlelog_lv1("kSCCmdCameraShutter is");
				code = onRequestCameraSetting(packet_ptr);
				responseCameraShutter(packet_ptr, code);
				break;
			}
			case kSCCmdCameraStrobe:
			{
				singlelog_lv1("kSCCmdCameraStrobe is");
				code = onRequestCameraSetting(packet_ptr);
				responseCameraStrobe(packet_ptr, code);
				break;
			}
			case kSCCmdCameraTrigger:
			{
				singlelog_lv1("kSCCmdTriggerSetting is");
				code = onRequestCameraTrigger(packet_ptr);
				responseCameraTrigger(packet_ptr, code);
				break;
			}
			case kSCCmdCameraFocus:
			{
				singlelog_lv1("kSCCmdCameraFocus is");
				code = onRequestCameraFocus(packet_ptr);
				responseCameraFocus(packet_ptr, code);
				break;
			}
			case kSCCmdCameraHDR:
			{
				singlelog_lv1("kSCCmdCameraHDR is");
				code = onRequestCameraHDR(packet_ptr);
				responseCameraHDR(packet_ptr, code);
				break;
			}
			case kSCCmdCameraMultiCapture:
			{
				singlelog_lv1("kSCCmdCameraMultiCapture is");
				code = onRequestCameraMultiCapture(packet_ptr);
				responseCameraMultiCapture(packet_ptr, code);
				break;
			}
			case kSCCmdOperation:
			{
				singlelog_lv1("kSCCmdOperation is");
				code = onRequestOperation(packet_ptr);
				break;
			}
			case kSCCmdCalib:
			{
				if (conn_mode >= kPOConnAdmin)
				{
					singlelog_lv1("kSCCmdCalib is");
					code = onRequestCalib(packet_ptr);
				}
				break;
			}
			default:
			{
				printlog_lv1(QString("[Unknown Command*************] cmd:%1,sub:%2").arg(cmd).arg(packet_ptr->getSubCmd()));
				break;
			}
		}
	}
	if (code != kPOSuccess && cmd < kSCCmdIntNetOpen)
	{
		packet_ptr->setCmd(cmd);
		uploadFailPacket(packet_ptr, code);
		printlog_lv1(QString("Command Fail, pid:%1, cmd:%2, code:%3").arg(m_pu_id).arg(cmd).arg(code));
	}

	//release semaphore for block cmd
	m_result_code = code;
	switch (mode)
	{
		case kPUCmdNonBlock:
		{
			break;
		}
		case kPUCmdBlock:
		{
			unlock();
			break;
		}
	}
	POSAFE_DELETE(packet_ptr);
}

void CProcessor::onUpdatingTool(CJobUnit* job_ptr)
{
	if (!job_ptr || !isUsedProcessor())
	{
		return;
	}

	CBaseTool* tool_ptr;
	CToolVector& tool_vec = job_ptr->getToolVector();

	i32vector tool_id_vec;
	i32 mode, tool_id, tool_index;
	i32 i, count = (i32)tool_vec.size();

	for (i = 0; i < count; i++)
	{
		tool_ptr = dynamic_cast<CBaseTool*>(tool_vec[i]);
		if (!tool_ptr)
		{
			continue;
		}

		tool_id = tool_ptr->getToolID();
		if (tool_ptr->isToolExtUpdated())
		{
			mode = tool_ptr->getToolExtUpdated();
			if (CPOBase::bitCheck(mode, kTagTypeRestudy))
			{
				tool_index = job_ptr->getToolIndex(tool_id);
				m_sc_engine.studyOneTool(job_ptr, tool_index);
			}
			tool_ptr->m_tool_ext_updated = 0;
			tool_id_vec.push_back(tool_id);
		}
	}

	if (tool_id_vec.size() > 0)
	{
		uploadJobData(job_ptr, kJobDataUpdatedTools, tool_id_vec);
		job_ptr->setChanged(kJobContent);
	}
}

void CProcessor::onUpdatingToolFromOpc()
{
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	g_main_app_ptr->setOpcUpdating(false);

	if (!job_ptr)
	{
		return;
	}
	onUpdatingTool(job_ptr);
}

void CProcessor::onUpdatingToolFromModbus(i32 addr, i32 size)
{
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	g_main_app_ptr->setModbusUpdating(false);

	if (!job_ptr)
	{
		return;
	}
	if (m_io_manager_ptr->updateJobToolsFromModbus(job_ptr, addr, size))
	{
		onUpdatingTool(job_ptr);
	}
}

void CProcessor::onRequestCommand(i32 cmd, i32 data)
{
	if (!isUsedProcessor() || !isProcessorRunning() || !m_cam_capture_ptr)
	{
		return;
	}

	Packet pak(cmd, kPOPacketRequest);
	switch (cmd)
	{
		case kSCCmdCameraTrigger:
		{
			if (updateTriggerMode(m_cam_param_ptr->m_cam_trigger) == kPOSuccess)
			{
				responseCameraTrigger(&pak, kPOPacketRespOK);
			}
			break;
		}
		case kSCCmdCameraExposure:
		{
			if (m_cam_capture_ptr->setCameraSetting(kPOMixFlagCamExposure, m_cam_param_ptr))
			{
				pak.setReservedi32(0, kPOMixFlagCamExposure);
				responseCameraExposure(&pak, kPOPacketRespOK);
			}
			break;
		}
		case kSCCmdCameraColor:
		{
			if (m_cam_capture_ptr->setCameraSetting(kPOMixFlagCamColor, m_cam_param_ptr))
			{
				pak.setReservedi32(0, kPOMixFlagCamColor);
				responseCameraColor(&pak, kPOPacketRespOK);
			}
			break;
		}
		case kSCCmdCameraCorrection:
		{
			if (m_cam_capture_ptr->setCameraSetting(kPOMixFlagCamCorrection, m_cam_param_ptr))
			{
				pak.setReservedi32(0, kPOMixFlagCamCorrection);
				responseCameraCorrection(&pak, kPOPacketRespOK);
			}
			break;
		}
		case kSCCmdCameraStrobe:
		{
			if (m_cam_capture_ptr->setCameraSetting(kPOMixFlagCamStrobe, m_cam_param_ptr))
			{
				pak.setReservedi32(0, kPOMixFlagCamStrobe);
				responseCameraStrobe(&pak, kPOPacketRespOK);
			}
			break;
		}
		case kSCCmdCameraRange:
		{
			if (m_cam_capture_ptr->setCameraSetting(kPOMixFlagCamRange, m_cam_param_ptr))
			{
				pak.setReservedi32(0, kPOMixFlagCamRange);
				responseCameraRange(&pak, kPOPacketRespOK);
			}
			break;
		}
	}
}

i32 CProcessor::onInternalNetOpen(Packet* packet_ptr)
{
	if (!m_cam_capture_ptr)
	{
		return kPOErrInvalidOper;
	}

	// update camera mode
	DeviceInfo* dev_info_ptr = g_main_app_ptr->getDeviceInfo();
	i32 encoder_mode = dev_info_ptr->getVideoEncoder();
	//if (g_main_app_ptr->isLocalConnection()) //TODO: need update
	//{
	//	encoder_mode = kPOEncoderIPCRaw;
	//}

	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlAdmin, 1);
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlEncoder, encoder_mode);
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlSCUControl, m_scu_control.snap_level);

	// Admin카메라, 이전처리결과가 있으면 상위기로 전송한다.
	if (m_sc_engine.hasPrevResult())
	{
		uploadJobResult(kSCCmdSnapResult, kResultWithAll);
		m_cam_capture_ptr->sendCameraImage(m_sc_engine.getImageData());
	}
	else
	{
		onRequestManualTrigger(NULL, kManualTriggerAdmin);
	}
	return kPOSuccess;
}

i32 CProcessor::onInternalNetClose(Packet* packet_ptr)
{
	if (!m_cam_capture_ptr)
	{
		return kPOErrInvalidOper;
	}

	//remove interactive flag
	i32 mode = m_pu_mode;
	CPOBase::bitRemove(mode, kSCModeUserEdit);
	updatePUState(m_pu_state, mode);

	//remove camera flags
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlSettingSync, 0);
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlEncoder, kPOEncoderNone);
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlAdmin, 0);

	//remove io flags
	m_io_manager_ptr->setIORawCommand(kSCSubTypeIORawUnSync);
	return kPOSuccess;
}

i32 CProcessor::onInternalOffline()
{
	onInternalIdle(NULL, kPUModeAdmin | kPUModeThread | kPUFlagKeepJob);

	if (m_cam_capture_ptr)
	{
		m_cam_capture_ptr->exitInstance();
	}

	{
		QMutexLocker l(&m_mutex_processor);
		m_cam_param_ptr = NULL;
	}
	m_scu_control.snap_level++;
	return kPOSuccess;
}

i32 CProcessor::onInternalSelectCamera(Packet* packet_ptr, i32& blk_mode)
{
	i32 cmd_mode = packet_ptr->getReservedi32(2);
	CameraSetting* cam_param_ptr = (CameraSetting*)(packet_ptr->getReservedi64(0));

	if (!cam_param_ptr)
	{
		if (cmd_mode == kHLCommand)
		{
			uploadFailPacket(kSCCmdSelectCamera, kPOErrInvalidData);
		}
		printlog_lv1("SelectCamera Invalid.");
		return kPOErrInvalidData;
	}

	//check same camera setting
	i32 cam_id = cam_param_ptr->getCamID();
	{
		QMutexLocker l(&m_mutex_processor);
		if (m_cam_param_ptr && m_cam_param_ptr->getCamID() == cam_id)
		{
			printlog_lv1(QString("SelectCamera Duplicated: Cam%1").arg(cam_id));
			return kPOSuccess;
		}
	}

	//set camera capture parameters
	{
		QMutexLocker l(&m_mutex_processor);
		m_cam_param_ptr = cam_param_ptr; //카메라파라메터를 설정한다.
	}
	
	//kPUCmdBlock방식으로 명령이 전송된경우 Block을 해제한다.
	if (blk_mode == kPUCmdBlock)
	{
		blk_mode = kPUCmdNonBlock;
		m_result_code = kPOSuccess;
		unlock();
	}

	//set camera param
	if (!m_cam_capture_ptr->initInstance(cam_param_ptr))
	{
		if (cmd_mode == kHLCommand)
		{
			uploadFailPacket(kSCCmdSelectCamera, kPOErrInvalidOper);
		}
		{
			QMutexLocker l(&m_mutex_processor);
			m_cam_param_ptr = NULL; //카메라파라메터를 초기화한다.
		}
		printlog_lv1(QString("CameraInit[%1] fail in SelectCamera.").arg(cam_id));
		return kPOErrInvalidOper;
	}

	m_sc_engine.setCalibParam(m_cam_param_ptr);
	if (cmd_mode == kHLCommand)
	{
		Packet* pak = po_new Packet(kSCCmdSelectCamera, kPOPacketRespOK);
		pak->setReservedi32(0, cam_id);
		sendPacketToNet(pak);
	}
	printlog_lv1(QString("Processor%1 camera%2 selected").arg(m_pu_id).arg(cam_id));
	return kPOSuccess;
}

i32 CProcessor::selectJob(i32 job_id)
{
	CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id);
	if (!job_ptr || !isCompatibilityJob(job_ptr))
	{
		printlog_lvs2("SelectJob failed, invalid job", LOG_SCOPE_APP);
		return kPOErrInvalidPacket;
	}

	//load job
	if (!job_ptr->hasJobData(kJobContent))
	{
		//load job contents only for processing
		if (!m_job_manager_ptr->loadJobData(job_ptr, kJobContent))
		{
			if (job_ptr->hasJobData(kJobMeta))
			{
				printlog_lvs2(QString("LoadJobData is failed, DeleteJob cam:%1, job id:%2")
								.arg(job_ptr->getCamID()).arg(job_ptr->getJobID()), LOG_SCOPE_APP);
				m_job_manager_ptr->deleteJob(job_ptr);
				return kSCErrDeletedJob;
			}
			else
			{
				printlog_lvs2(QString("LoadJobData is failed, But NonData Job"), LOG_SCOPE_APP);
			}
			return kSCErrInvalidJob;
		}
	}

	updateJobSelect(job_ptr);

	//send camera trigger, if stop and admin processor
	if (isProcessorStop() && isAdminProcessor())
	{
		onProcessInteractive();
		uploadJobResult(kSCCmdSnapResult, kResultWithAll, true);
	}
	return kPOSuccess;
}

i32 CProcessor::onInternalSelectProgram(Packet* packet_ptr)
{
	//check PUState
	if (!isProcessorRun())
	{
		printlog_lv1(QString("Processor state[%1] is not compatibility for switching job").arg(m_pu_state));
		return kPOErrInvalidOper;
	}

	//check packet
	i32 job_id = packet_ptr->getReservedi32(0);
	return selectJob(job_id);
}

i32 CProcessor::onInternalSelectJob(i32& cmd, Packet* packet_ptr)
{
	i32 job_id = packet_ptr->getReservedi32(0);
	u8 job_loaded = packet_ptr->getReservedu8(4);
	u8 job_operator = packet_ptr->getReservedu8(5);

	//check processor state, if select job signal is accepted from thread
	if (job_operator != kOperationbyAdmin)
	{
		cmd = kSCCmdAdminSelectJob;
		if (!isProcessorRun())
		{
			printlog_lv1(QString("Admin state[%1] is not compatibility for select job").arg(m_pu_state));
			return kPOErrInvalidOper;
		}
	}

	i32 result = selectJob(job_id);
	if (result != kPOSuccess)
	{
		return result;
	}

	Packet* pak = po_new Packet(cmd, kPOPacketRespOK);
	pak->setReservedi32(0, job_id);
	sendPacketToNet(pak);

	printlog_lv1(QString("Processor%1 job_id[%2] is selected").arg(m_pu_id).arg(job_id));
	return kPOSuccess;
}

i32 CProcessor::onInternalJobSetting(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidPacket;
	}

	i32 sub_cmd = packet_ptr->getSubCmd();
	i32 buffer_size = packet_ptr->getDataLen();
	u8* buffer_ptr = packet_ptr->getData();

	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (!job_ptr || !buffer_ptr)
	{
		return kPOErrInvalidData;
	}

	switch (sub_cmd)
	{
		case kSCSubTypeIOOutput:
		{
			job_ptr->m_do_setting.memRead(buffer_ptr, buffer_size);
			updateDOSetting(job_ptr->m_do_setting);
			break;
		}
		case kSCSubTypeModbusComm:
		{
			job_ptr->m_modbus_comm.memRead(buffer_ptr, buffer_size);
			updateModBusSetting(job_ptr->m_modbus_comm);
			break;
		}
		case kSCSubTypeFtpComm:
		{
			job_ptr->m_ftp_comm.memRead(buffer_ptr, buffer_size);
			updateFtpSetting(job_ptr->m_ftp_comm);
			break;
		}
		case kSCSubTypeOpcComm:
		{
			job_ptr->m_opc_comm.memRead(buffer_ptr, buffer_size);
			updateOpcSetting(job_ptr);
			break;
		}
		default:
		{
			return kPOErrInvalidOper;
		}
	}

	job_ptr->setChanged(kJobContent);
	m_job_manager_ptr->writeJobToFile();
	printlog_lv1(QString("JobSetting updated and write to disk. sub_cmd:%1").arg(sub_cmd));
	return kPOSuccess;
}

i32 CProcessor::onInternalRunThread()
{
	if (!isAdminProcessor())
	{
		i32 mode = m_pu_mode;
		CPOBase::bitRemove(mode, kSCModeUserEdit);
		updatePUState(kSCStateRun, mode);
	}
	return kPOSuccess;
}

i32 CProcessor::onInternalStopThread()
{
	if (!isAdminProcessor())
	{
		i32 mode = m_pu_mode;
		CPOBase::bitRemove(mode, kSCModeUserEdit);
		updatePUState(kSCStateStop, mode);
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestAddJob(Packet* packet_ptr)
{
	//check packet and PUState
	if (!isProcessorStop())
	{
		return kSCErrInvalidPUState;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidPacket;
	}
	
	//read new job data
	powstring new_job_name;
	i32 new_job_cam_id = packet_ptr->getReservedi32(0);
	i32 new_job_sub_id = packet_ptr->getReservedi32(1);
	CPOBase::memRead(buffer_ptr, buffer_size, new_job_name);
	checkRead(buffer_ptr, packet_ptr);

	if (new_job_cam_id != m_cam_param_ptr->getCamID())
	{
		return kPOErrInvalidData;
	}

	//add new job and update
	i32 job_ext_id = new_job_cam_id << 8 | new_job_sub_id;
	if (m_job_manager_ptr->findJobByExternalID(job_ext_id))
	{
		return kPOErrDuplicateObject;
	}

	CJobUnit* job_ptr = m_job_manager_ptr->makeNewJob(new_job_name, new_job_cam_id, new_job_sub_id);
	if (!job_ptr)
	{
		printlog_lv1("New Job Failed");
		return kPOErrInvalidOper;
	}
	job_ptr->setCameraSetting(m_cam_param_ptr, kCamSettingUpdateAll);
	job_ptr->updateThumbnail(m_sc_engine.getImageData());

	//update job manager
	if (selectJob(job_ptr->getJobID()) != kPOSuccess)
	{
		return kPOErrInvalidOper;
	}
	m_job_manager_ptr->writeJobToFile();

	//upload job data
	if (!uploadJobContent(packet_ptr->getCmd(), job_ptr, true))
	{
		return kSCErrInvalidJob;
	}
	printlog_lv1(QString("Add Job is OK, cam_id:%1, sub_id:%2, job_id:%3")
					.arg(new_job_cam_id).arg(new_job_sub_id).arg(job_ptr->getJobID()));
	return kPOSuccess;
}

i32 CProcessor::onRequestGetJob(Packet* packet_ptr)
{
	//check job
	i32 job_id = packet_ptr->getReservedi32(0);
	CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id);
	if (!job_ptr || !isCompatibilityJob(job_ptr))
	{
		printlog_lvs2("GetJob failed, invalid job", LOG_SCOPE_APP);
		return kPOErrInvalidOper;
	}

	//load all data in job
	if (!job_ptr->hasJobData(kJobContent))
	{
		if (!m_job_manager_ptr->loadJobData(job_ptr, kJobContent))
		{
			if (job_ptr->hasJobData(kJobMeta))
			{
				printlog_lvs2(QString("GetJob is failed, DeleteJob cam:%1, job id:%2")
					.arg(job_ptr->getCamID()).arg(job_ptr->getJobID()), LOG_SCOPE_APP);
				m_job_manager_ptr->deleteJob(job_ptr);
				return kSCErrDeletedJob;
			}
			else
			{
				printlog_lvs2(QString("GetJob is failed, But NonData Job"), LOG_SCOPE_APP);
			}
			return kSCErrInvalidJob;
		}
	}

	//upload job data
	if (!uploadJobContent(packet_ptr->getCmd(), job_ptr, true))
	{
		return kSCErrInvalidJob;
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestSaveJob(Packet* packet_ptr)
{
	//save job to disk
	i32 job_id = packet_ptr->getReservedi32(0);
	if (!m_job_manager_ptr->writeJobToFile(job_id))
	{
		return kSCErrInvalidJob;
	}

	//upload response
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	if (pak)
	{
		sendPacketToNet(pak);
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestCameraSetting(Packet* packet_ptr)
{
	i32 flag = packet_ptr->getReservedi32(0);
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();

	//check syncronize flag for checking real-time camera status
	if (CPOBase::bitCheck(flag, kPOSubFlagCamSync))
	{
		printlog_lvs2("CameraSetting CamSync is accept.", LOG_SCOPE_APP);
		if (m_cam_capture_ptr->setCameraModeSelect(kCamCtrlSettingSync, 1))
		{
			return kPOSuccess;
		}
		return kPOErrInvalidOper;
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamUnSync))
	{
		printlog_lvs2("CameraSetting CamUnSync is accept.", LOG_SCOPE_APP);
		if (m_cam_capture_ptr->setCameraModeSelect(kCamCtrlSettingSync, 0))
		{
			return kPOSuccess;
		}
		return kPOErrInvalidOper;
	}

	//check processor running state...
	if (!isProcessorRunning()) //run + stop_testing
	{
		printlog_lvs2("Can't accept onRequestCameraSetting in Run/FreezenMode.", LOG_SCOPE_APP);
		return kPOErrCmdFail;
	}

	m_tmp_cam_param.memReadChanged(flag, buffer_ptr, buffer_size);
	checkRead(buffer_ptr, packet_ptr);
	return onProcessCameraSetting(flag, &m_tmp_cam_param);
}

i32 CProcessor::onRequestCameraTrigger(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !isProcessorStop())
	{
		return kSCErrInvalidPUState;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidPacket;
	}

	CameraTrigger trigger_mode;
	trigger_mode.memRead(buffer_ptr, buffer_size);
	checkRead(buffer_ptr, packet_ptr);

	if (!updateTriggerMode(trigger_mode))
	{
		return kPOErrInvalidOper;
	}

	//tune trigger setting into current job
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (job_ptr)
	{
		job_ptr->setCameraSetting(m_cam_param_ptr, kCamSettingUpdateTrigger);
		job_ptr->setChanged(kJobBody);
	}
	return kPOSuccess;
}

i32 CProcessor::onRequestCameraFocus(Packet* packet_ptr)
{
	return kPOErrInvalidOper;
}

i32 CProcessor::onRequestCameraHDR(Packet* packet_ptr)
{
	return kPOErrInvalidOper;
}

i32 CProcessor::onRequestCameraMultiCapture(Packet* packet_ptr)
{
	return kPOErrInvalidOper;
}

i32 CProcessor::responseCameraExposure(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}
	updateJobCamSetting(kCamSettingUpdateExposure);

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	i32 len = 0;
	i32 flag = packet_ptr->getReservedi32(0);
	CameraExposure* cam_exposure_ptr = m_cam_param_ptr->getCameraExposure();
	anlock_guard_ptr(cam_exposure_ptr);

	if (CPOBase::bitCheck(flag, kPOSubFlagCamGain))
	{
		len += sizeof(cam_exposure_ptr->m_gain);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamExposure))
	{
		len += sizeof(cam_exposure_ptr->m_exposure);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamAEMode))
	{
		len += sizeof(cam_exposure_ptr->m_autoexp_mode);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamAEGain))
	{
		len += sizeof(cam_exposure_ptr->m_autogain_min);
		len += sizeof(cam_exposure_ptr->m_autogain_max);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamAEExposure))
	{
		len += sizeof(cam_exposure_ptr->m_autoexp_min);
		len += sizeof(cam_exposure_ptr->m_autoexp_max);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamAEBrightness))
	{
		len += sizeof(cam_exposure_ptr->m_auto_brightness);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamAEWindow))
	{
		len += sizeof(cam_exposure_ptr->m_autoexp_window);
	}

	u8* buffer_ptr = NULL;
	i32 buffer_size = len;
	if (len > 0)
	{
		pak->allocateBuffer(len, buffer_ptr);

		if (CPOBase::bitCheck(flag, kPOSubFlagCamGain))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_gain, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamExposure))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_exposure, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamAEMode))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_autoexp_mode, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamAEGain))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_autogain_min, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_exposure_ptr->m_autogain_max, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamAEExposure))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_autoexp_min, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_exposure_ptr->m_autoexp_max, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamAEBrightness))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_auto_brightness, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamAEWindow))
		{
			CPOBase::memWrite(cam_exposure_ptr->m_autoexp_window, buffer_ptr, buffer_size);
		}
	}

	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraRange(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}
	updateJobCamSetting(kCamSettingUpdateGeometric);

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	i32 len = 0;
	i32 flag = packet_ptr->getReservedi32(0);
	CameraRange* cam_range_ptr = m_cam_param_ptr->getCameraRange();
	anlock_guard_ptr(cam_range_ptr);

	if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoInvert))
	{
		len += sizeof(cam_range_ptr->m_is_invert);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoFlip))
	{
		len += sizeof(cam_range_ptr->m_is_flip_x);
		len += sizeof(cam_range_ptr->m_is_flip_y);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoRotation))
	{
		len += sizeof(cam_range_ptr->m_rotation);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoRange))
	{
		len += sizeof(cam_range_ptr->m_range);
	}

	u8* buffer_ptr = NULL;
	i32 buffer_size = len;
	if (len > 0)
	{
		pak->allocateBuffer(len, buffer_ptr);

		if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoInvert))
		{
			CPOBase::memWrite(cam_range_ptr->m_is_invert, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoFlip))
		{
			CPOBase::memWrite(cam_range_ptr->m_is_flip_x, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_range_ptr->m_is_flip_y, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoRotation))
		{
			CPOBase::memWrite(cam_range_ptr->m_rotation, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamGeoRange))
		{
			CPOBase::memWrite(cam_range_ptr->m_range, buffer_ptr, buffer_size);
		}
	}

	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraColor(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}
	updateJobCamSetting(kCamSettingUpdateColor);

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	i32 len = 0;
	i32 flag = packet_ptr->getReservedi32(0);
	CameraColor* cam_color_ptr = m_cam_param_ptr->getCameraColor();
	anlock_guard_ptr(cam_color_ptr);

	if (CPOBase::bitCheck(flag, kPOSubFlagCamColorMode))
	{
		len += sizeof(cam_color_ptr->m_color_mode);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamColorWBMode))
	{
		len += sizeof(cam_color_ptr->m_wb_mode);
		len += sizeof(cam_color_ptr->m_red_gain);
		len += sizeof(cam_color_ptr->m_green_gain);
		len += sizeof(cam_color_ptr->m_blue_gain);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamColorWBAutoOnce))
	{
		len += sizeof(cam_color_ptr->m_red_gain);
		len += sizeof(cam_color_ptr->m_green_gain);
		len += sizeof(cam_color_ptr->m_blue_gain);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamColorGain))
	{
		len += sizeof(cam_color_ptr->m_red_gain);
		len += sizeof(cam_color_ptr->m_green_gain);
		len += sizeof(cam_color_ptr->m_blue_gain);
	}

	u8* buffer_ptr = NULL;
	i32 buffer_size = len;
	if (len > 0)
	{
		pak->allocateBuffer(len, buffer_ptr);

		if (CPOBase::bitCheck(flag, kPOSubFlagCamColorMode))
		{
			CPOBase::memWrite(cam_color_ptr->m_color_mode, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamColorWBMode))
		{
			CPOBase::memWrite(cam_color_ptr->m_wb_mode, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_red_gain, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_green_gain, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_blue_gain, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamColorWBAutoOnce))
		{
			CPOBase::memWrite(cam_color_ptr->m_red_gain, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_green_gain, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_blue_gain, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamColorGain))
		{
			CPOBase::memWrite(cam_color_ptr->m_red_gain, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_green_gain, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_color_ptr->m_blue_gain, buffer_ptr, buffer_size);
		}
	}

	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraCorrection(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}
	updateJobCamSetting(kCamSettingUpdateCorrection);

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	i32 len = 0;
	i32 flag = packet_ptr->getReservedi32(0);
	CameraCorrection* cam_corr_ptr = m_cam_param_ptr->getCameraCorrection();
	anlock_guard_ptr(cam_corr_ptr);

	if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionGamma))
	{
		len += sizeof(cam_corr_ptr->m_gamma);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionContrast))
	{
		len += sizeof(cam_corr_ptr->m_contrast);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionSaturation))
	{
		len += sizeof(cam_corr_ptr->m_saturation);
	}
	if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionSharpness))
	{
		len += sizeof(cam_corr_ptr->m_sharpness);
	}

	u8* buffer_ptr = NULL;
	i32 buffer_size = len;
	if (len > 0)
	{
		pak->allocateBuffer(len, buffer_ptr);

		if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionGamma))
		{
			CPOBase::memWrite(cam_corr_ptr->m_gamma, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionContrast))
		{
			CPOBase::memWrite(cam_corr_ptr->m_contrast, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionSaturation))
		{
			CPOBase::memWrite(cam_corr_ptr->m_saturation, buffer_ptr, buffer_size);
		}
		if (CPOBase::bitCheck(flag, kPOSubFlagCamCorrectionSharpness))
		{
			CPOBase::memWrite(cam_corr_ptr->m_sharpness, buffer_ptr, buffer_size);
		}
	}

	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraShutter(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}
	updateJobCamSetting(kCamSettingUpdateShutter);

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	i32 len = 0;
	i32 flag = packet_ptr->getReservedi32(0);
	CameraSpec* cam_spec_ptr = m_cam_param_ptr->getCameraSpec();
	anlock_guard_ptr(cam_spec_ptr);

	if (CPOBase::bitCheck(flag, kPOSubFlagCamShutter))
	{
		len += sizeof(cam_spec_ptr->m_shutter_mode);
		len += sizeof(cam_spec_ptr->m_jitter_time);
	}

	u8* buffer_ptr = NULL;
	i32 buffer_size = len;
	if (len > 0)
	{
		pak->allocateBuffer(len, buffer_ptr);

		if (CPOBase::bitCheck(flag, kPOSubFlagCamShutter))
		{
			CPOBase::memWrite(cam_spec_ptr->m_shutter_mode, buffer_ptr, buffer_size);
			CPOBase::memWrite(cam_spec_ptr->m_jitter_time, buffer_ptr, buffer_size);
		}
	}

	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraStrobe(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}
	updateJobCamSetting(kCamSettingUpdateStrobe);

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	u8* buffer_ptr = NULL;
	i32 len, flag = packet_ptr->getReservedi32(0);
	CameraStrobe* cam_strobe_ptr = m_cam_param_ptr->getCameraStrobe();
	{
		anlock_guard_ptr(cam_strobe_ptr);

		len = cam_strobe_ptr->memSize();
		i32 buffer_size = len;
		if (len > 0)
		{
			pak->allocateBuffer(len, buffer_ptr);
			cam_strobe_ptr->memWrite(buffer_ptr, buffer_size);
		}
	}
	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraTrigger(Packet* packet_ptr, i32 code)
{
	if (!g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	if (!pak || !m_cam_param_ptr)
	{
		POSAFE_DELETE(pak);
		return kPOErrCmdFail;
	}

	u8* buffer_ptr = NULL;
	CameraTrigger* cam_trigger_ptr = m_cam_param_ptr->getCameraTrigger();

	if (cam_trigger_ptr)
	{
		i32 len = cam_trigger_ptr->memSize();
		i32 buffer_size = len;
		pak->allocateBuffer(len, buffer_ptr);

		cam_trigger_ptr->memWrite(buffer_ptr, buffer_size);
	}

	sendPacketToNet(pak, buffer_ptr);
	return kPOSuccess;
}

i32 CProcessor::responseCameraFocus(Packet* packet_ptr, i32 code)
{
	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::responseCameraHDR(Packet* packet_ptr, i32 code)
{
	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::responseCameraWhiteBlance(Packet* packet_ptr, i32 code)
{
	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::responseCameraMultiCapture(Packet* packet_ptr, i32 code)
{
	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataNone);
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CProcessor::onRequestRunProcess(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 mode = m_pu_mode;
	CPOBase::bitRemove(mode, kSCModeUserEdit);
	updatePUState(kSCStateRun, mode);
	
	Packet* pak = Packet::makeRespPacket(packet_ptr); 
	pak->setReservedi32(0, m_pu_state);
	pak->setReservedi32(1, m_pu_mode);
	sendPacketToNet(pak);

	g_sc_disk.writeAllSettings(g_main_app_ptr);
	return kPOSuccess;
}

i32 CProcessor::onRequestStopProcess(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 mode = m_pu_mode;
	CPOBase::bitRemove(mode, kSCModeUserEdit | kSCModeTest);
	updatePUState(kSCStateStop, mode);

	Packet* pak = Packet::makeRespPacket(packet_ptr);
	pak->setReservedi32(0, m_pu_state);
	pak->setReservedi32(1, m_pu_mode);
	sendPacketToNet(pak);

	g_sc_disk.writeAllSettings(g_main_app_ptr);
	return kPOSuccess;
}

i32 CProcessor::onRequestTestMode(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 mode = m_pu_mode;
	bool is_testmode = packet_ptr->getReservedb8(0);

	if (is_testmode)
	{
		//set freezen flag
		CPOBase::bitAdd(mode, kSCModeTest);
	}
	else
	{
		//remove freezen flag
		CPOBase::bitRemove(mode, kSCModeTest);
	}

	updatePUState(m_pu_state, mode);

	Packet* pak = Packet::makeRespPacket(packet_ptr);
	pak->setReservedi32(0, m_pu_state);
	pak->setReservedi32(1, m_pu_mode);
	sendPacketToNet(pak);

	g_sc_disk.writeAllSettings(g_main_app_ptr);
	return kPOSuccess;
}

i32 CProcessor::onRequestFreezeMode(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 mode = m_pu_mode;
	bool is_freezen = packet_ptr->getReservedb8(0);

	if (is_freezen)
	{
		//set freezen flag
		CPOBase::bitAdd(mode, kSCModeFrozen);
	}
	else
	{
		//remove freezen flag
		CPOBase::bitRemove(mode, kSCModeFrozen);
	}

	//update Processor state and mode
	updatePUState(m_pu_state, mode);

	//upload response
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	pak->setReservedi32(0, m_pu_state);
	pak->setReservedi32(1, m_pu_mode);
	sendPacketToNet(pak);

	g_sc_disk.writeAllSettings(g_main_app_ptr);
	return kPOSuccess;
}

i32 CProcessor::onRequestInteractiveMode(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !packet_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 mode = m_pu_mode;
	bool is_interactive = packet_ptr->getReservedb8(0);

	if (is_interactive)
	{
		//add interactive flag
		CPOBase::bitAdd(mode, kSCModeInteractive);
	}
	else
	{
		//remove interactive flag
		CPOBase::bitRemove(mode, kSCModeInteractive);
	}

	updatePUState(m_pu_state, mode);

	Packet* pak = Packet::makeRespPacket(packet_ptr);
	pak->setReservedi32(0, m_pu_state);
	pak->setReservedi32(1, m_pu_mode);
	sendPacketToNet(pak);
	return kPOSuccess;
}

void CProcessor::setProcessorCalibMode(bool is_calib)
{
	if (!isAdminProcessor())
	{
		return;
	}

	i32 mode = m_pu_mode;
	if (is_calib)
	{
		//add interactive flag
		CPOBase::bitAdd(mode, kSCModeCalib);
	}
	else
	{
		//remove interactive flag
		CPOBase::bitRemove(mode, kSCModeCalib);
	}

	updatePUState(m_pu_state, mode);
}

i32 CProcessor::onRequestManualTrigger(Packet* packet_ptr, i32 mode)
{
	//check camera param
	if (!m_cam_param_ptr)
	{
		printlog_lvs2(QString("The camera is not connected in Processor[%1]").arg(m_pu_id), LOG_SCOPE_APP);
		return kPOErrInvalidOper;
	}

	//check manual trigger condition
	switch (mode)
	{
		case kManualTriggerAdmin:
		{
			//if it is admin trigger mode...
			if (isAdminProcessor())
			{
				captureImage(kCamSnapManualEx);
			}
			break;
		}
		default:
		{
			if (isProcessorFrozenMode())
			{
				return kPOErrInvalidOper;
			}
			captureImage(kCamSnapManual);
			uploadEndPacket(packet_ptr);
			break;
		}
	}
	return kPOSuccess;
}

void CProcessor::onSyncCameraState()
{
	g_main_app_ptr->checkQtSignals(SC_SIGNAL_CAMSYNC, false);
	if (!isUsedProcessor())
	{
		printlog_lvs2("CameraSetting Uploading fail, invalid camera.", LOG_SCOPE_APP);
		return;
	}

	u8* buffer_ptr = NULL;
	CameraState* cam_state_ptr = m_cam_param_ptr->getCameraState();
	Packet* pak = po_new Packet(kSCCmdCameraSync, kPOPacketRespOK);

	if (cam_state_ptr)
	{
		//upload synchronize setting such as auto-camera-information and focus-index
		i32 len = cam_state_ptr->memSize();
		i32 buffer_size = len;
		if (len > 0)
		{
			pak->allocateBuffer(len, buffer_ptr);
			cam_state_ptr->memWrite(buffer_ptr, buffer_size);
		}
	}

	sendPacketToNet(pak, buffer_ptr);
}

const bool CProcessor::isCompatibilityJob(CJobUnit* job_ptr)
{
	//check parameter validation
	if (!m_cam_param_ptr || !job_ptr)
	{
		printlog_lvs2("Job CPT-Check fail, invalid camera or job.", LOG_SCOPE_APP);
		return false;
	}

	//check content and camera ID
	if (!job_ptr->hasJobData(kJobMeta) || job_ptr->getCamID() != m_cam_param_ptr->getCamID())
	{
		printlog_lvs2("Job CPT-Check fail, invalid content or camid.", LOG_SCOPE_APP);
		return false;
	}
	return true;
}

bool CProcessor::updateJobSelect(CJobUnit* job_ptr)
{
	//check user specified job compatibility
	if (!isCompatibilityJob(job_ptr))
	{
		return false;
	}

	singlelog_lv1(QString("Set current job[%1] by user").arg(job_ptr->getExternalID()));

	if (!job_ptr->isSameJob(m_job_manager_ptr->getCurJob(m_pu_id)))
	{
		updateOutputState(kPUUpdateRemoveTask);

		job_ptr->updateToolTags();
		m_job_manager_ptr->setCurJob(job_ptr, m_pu_id);

		updateCameraSetting(job_ptr->m_cam_setting);
		updateDOSetting(job_ptr->m_do_setting, false);
		updateModBusSetting(job_ptr->m_modbus_comm);
		updateFtpSetting(job_ptr->m_ftp_comm);
		updateOpcSetting(job_ptr);
		updateOutputState(kPUUpdateSetTask);
		
		m_scu_control.snap_level++;
		m_cam_capture_ptr->setCameraModeSelect(kCamCtrlSCUControl, m_scu_control.snap_level);
	}
	else
	{
		printlog_lv1(QString("The same job[%1] is selected already").arg(job_ptr->getExternalID()));
	}

	//update admin job selection
	if (isAdminProcessor())
	{
		m_job_manager_ptr->setCurJob(job_ptr);
	}
	return true;
}

bool CProcessor::updateCameraSetting(CameraSetting& cam_param_ptr)
{
	//set trigger mode
	if (!m_cam_capture_ptr->setCameraTriggerMode(cam_param_ptr.getCameraTrigger()->getValue()))
	{
		printlog_lvs2("CameraTriggerSetting is failed in UpdateCameraSetting.", LOG_SCOPE_APP);
		return false;
	}

	//set camera setting
	m_tmp_cam_param.setValue(m_cam_param_ptr, kCamSettingUpdateInfo);
	m_tmp_cam_param.setValue(&cam_param_ptr, kCamSettingUpdateAllCtrl);
	m_tmp_cam_param.updateValidation();

	i32 sflag = kPOSubFlagCamCtrl | kPOSubFlagCamInitFirst | kPOSubFlagCamClearFocus;
	if (onProcessCameraSetting(sflag, &m_tmp_cam_param) != kPOSuccess)
	{
		printlog_lvs2("CameraSetting is uncompatibility in UpdateCameraSetting.", LOG_SCOPE_APP);
	}

	//copy all camera setting from job
	m_cam_param_ptr->setValue(&cam_param_ptr, kCamSettingUpdateAllCtrl);
	m_sc_engine.setCalibParam(m_cam_param_ptr);
	return true;
}

void CProcessor::updateDOSetting(CIOOutput& ds, bool is_out)
{
	m_do_setting.setValue(ds);
	m_do_setting.updateMethod2Addr();

	if (is_out)
	{
		CJobResult* job_result_ptr = m_sc_engine.getJobResult();
		m_io_manager_ptr->output(&m_do_setting, &m_modbus_comm, job_result_ptr);
	}
}

void CProcessor::updateModBusSetting(CModBusComm& mb_comm, bool is_out)
{
	m_modbus_comm.setValue(mb_comm);

	if (is_out)
	{
		CJobResult* job_result_ptr = m_sc_engine.getJobResult();
		m_io_manager_ptr->output(&m_do_setting, &m_modbus_comm, job_result_ptr);
	}
}

void CProcessor::updateFtpSetting(CFtpComm& ftp_output_group)
{
	m_ftp_comm.setValue(ftp_output_group);
}

void CProcessor::updateOpcSetting(CJobUnit* job_ptr)
{
	if (!job_ptr)
	{
		return;
	}
	m_io_manager_ptr->updateJobOpcSetting(job_ptr);
}

void CProcessor::updateOutputState(i32 mode)
{
	if (mode == kPUUpdateNone)
	{
		return;
	}

	i32 cam_id = getCamID();
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	CJobResult* job_result_ptr = m_sc_engine.getJobResult();
	
	switch (mode)
	{
		case kPUUpdateRunStop:
		{
			m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutRun, cam_id, isProcessorRun());
			break;
		}
		case kPUUpdateFrozen:
		{
			m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutFreeze, cam_id, isProcessorFrozenMode());
			break;
		}
		case kPUUpdateSetTask:
		{
			//Job이 선택실행될때 IO출력선과 Modbus구역을 초기화 및 교정자료출력
			if (job_ptr && job_result_ptr)
			{
				m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutJobSelect, cam_id, true);
				job_result_ptr->initBuffer();
				m_tag_manager_ptr->addJob(job_ptr);
				m_tag_manager_ptr->addJobResult(job_ptr, job_result_ptr);

				m_io_manager_ptr->clearOutput(&m_do_setting, job_ptr); //must be init before addRemoveTask()
				m_io_manager_ptr->addRemoveTask(job_ptr, mode);
				m_io_manager_ptr->outputCalib(m_cam_param_ptr, 1);

				m_io_manager_ptr->output(&m_do_setting, &m_modbus_comm, job_result_ptr);
			}
			break;
		}
		case kPUUpdateRemoveTask:
		{
			//Job이 실행중지될때 IO출력선과 Modbus구역을 초기화한다.
			if (job_ptr && job_result_ptr)
			{
				m_tag_manager_ptr->removeJob(job_ptr);
				job_result_ptr->initBuffer();

				m_io_manager_ptr->addRemoveTask(job_ptr, mode);
				m_io_manager_ptr->clearOutput(&m_do_setting, job_ptr); //must be init after addRemoveTask()
			}
			break;
		}
		case kPUUpdateRunTask:
		{
			//Run상태로 이행할때마다 현재Job의 입력IO비트들을 초기화한다.
			if (job_ptr)
			{
				m_io_manager_ptr->clearInput(job_ptr);
			}
			break;
		}
	}
}

bool CProcessor::updateTriggerMode(CameraTrigger& cam_trigger)
{
	//update camera param and camera trigger setting
	if (!m_cam_capture_ptr || !m_cam_param_ptr)
	{
		printlog_lvs2("UpdateTriggerMode fail. invalid camera", LOG_SCOPE_APP);
		return false;
	}
	if (!m_cam_capture_ptr->setCameraTriggerMode(cam_trigger))
	{
		return false;
	}
	m_cam_param_ptr->setCameraTrigger(cam_trigger);
	return true;
}

bool CProcessor::updatePUState(i32 state, i32 mode)
{
	if (!m_cam_capture_ptr || (m_pu_state == state && m_pu_mode == mode))
	{
		return false;
	}
	singlelog_lvs3(QString("Update PUState(%1, mode:%2)").arg(state).arg(mode), LOG_SCOPE_APP);

	//update snap control
	m_scu_control.snap_level++;
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlSCUControl, m_scu_control.snap_level);
	m_cam_capture_ptr->removeImageFromQueue(m_scu_control);

	//update app state
	if (m_pu_state != state)
	{
		m_pu_state = state;
		updateOutputState(kPUUpdateRunStop);
		if (isProcessorRun())
		{
			CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
			if (job_ptr)
			{
				job_ptr->updateToolTags();
			}

			CJobResult* job_result_ptr = m_sc_engine.getJobResult();
			if (job_result_ptr)
			{
				m_tag_manager_ptr->clearJobResult(job_result_ptr);
				job_result_ptr->initBuffer();
			}
			updateOutputState(kPUUpdateRunTask);
		}
	}
	
	//update app mode
	if (m_pu_mode != mode)
	{
		m_pu_mode = mode;
		updateOutputState(kPUUpdateFrozen);
	}

	//update mainapp state
	if (isAdminProcessor())
	{
		i32 app_state = m_pu_state;
		i32 app_mode = m_pu_mode;
		CPOBase::bitRemove(app_mode, kSCModeCalib);
		CPOBase::bitRemove(app_mode, kSCModeInteractive);

		if (g_main_app_ptr->m_app_state != app_state || g_main_app_ptr->m_app_mode != app_mode)
		{
			g_main_app_ptr->m_app_state = app_state;
			g_main_app_ptr->m_app_mode = app_mode;

			Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
			pak->setSubCmd(kSCSubTypeDevState);
			pak->setReservedi32(0, m_pu_state);
			pak->setReservedi32(1, m_pu_mode);
			sendPacketToNet(pak);

			//update job to file
			if (isRunning() || isProcessorTestMode())
			{
				m_job_manager_ptr->writeJobToFile();
			}
		}
	}

	//update
	m_cam_capture_ptr->setCameraModeSelect(kCamCtrlStateMode, m_pu_state, m_pu_mode);
	return true;
}

void CProcessor::captureImage(i32 mode)
{
	printlog_lv1(QString("Capture image : capture mode[%1]").arg(mode));
	m_io_manager_ptr->outputIOState(&m_do_setting, kIOOutBusy, getCamID(), true);

	m_scu_control.snap_mode = mode;
	m_scu_control.time_stamp = kResultTimeCommon;
	m_scu_control.use_last_snap = m_general_param_ptr->useLastSnap();
	emit requireImage(m_scu_control);
}

i32 CProcessor::onRequestDeleteJob(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !isProcessorStop())
	{
		return kSCErrInvalidPUState;
	}

	i32 index;
	i32 job_id = packet_ptr->getReservedi32(0);
	CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id, index);
	if (!job_ptr || !isCompatibilityJob(job_ptr))
	{
		printlog_lvs2(QString("DeleteJob failed, invalid job, id:%1").arg(job_id), LOG_SCOPE_APP);
		return kSCErrInvalidJob;
	}

	//remove output state job 
	if (job_ptr == m_job_manager_ptr->getCurJob(m_pu_id))
	{
		updateOutputState(kPUUpdateRemoveTask);
	}

	//remove job information in runtime history
	updateRuntimeHistory(kRuntimeHistoryUpdateRemoveJob, job_ptr);

	//remove job ext-selection in SCDevice of SCIOManager
	if (m_io_manager_ptr->removeDevExtSel(job_ptr))
	{
		m_io_manager_ptr->uploadDevExtSel();
	}

	//delete and update job manager
	if (!m_job_manager_ptr->deleteJob(index))
	{
		return kPOErrInvalidOper;
	}
	m_job_manager_ptr->writeJobToFile();

	//upload response
	uploadEndPacket(packet_ptr);
	uploadRuntimeHistory(kSCCmdRuntimeSync);
	m_sc_db_client.removeJobConfigure(job_id);

	printlog_lv1(QString("Delete Job is OK, id:%1").arg(job_id));
	return kPOSuccess;
}

i32 CProcessor::onRequestSearchJobs(Packet* packet_ptr)
{
	JobVector job_vec;
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();

	switch (packet_ptr->getSubCmd())
	{
		case kSCSubTypeJobOperByIDArray:
		{
			i32 job_id = packet_ptr->getReservedi32(0);
			if (job_id < 0)
			{
				return kPOErrInvalidOper;
			}

			CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id);
			job_vec.push_back(job_ptr);
			break;
		}
		case kSCSubTypeJobOperByName:
		{
			powstring job_name;
			if (!buffer_ptr || buffer_size <= 0)
			{
				return kPOErrInvalidOper;
			}

			CPOBase::memRead(buffer_ptr, buffer_size, job_name);
			job_vec = m_job_manager_ptr->findJobByName(job_name);
			break;
		}
	}

	checkRead(buffer_ptr, packet_ptr);
	uploadJobMetaArray(packet_ptr, job_vec);
	return kPOSuccess;
}

i32 CProcessor::onRequestGetJobMeta(Packet* packet_ptr)
{
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidPacket;
	}

	i32 sub_cmd = packet_ptr->getSubCmd();
	switch (sub_cmd)
	{
		case kSCSubTypeJobOperByID:
		{
			CJobUnit* job_ptr = m_job_manager_ptr->findJob(packet_ptr->getReservedi32(0));
			if (!uploadJobMeta(packet_ptr, job_ptr))
			{
				return kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeJobOperByIDArray:
		{
			i32vector job_id_vec;
			CPOBase::memReadVector(job_id_vec, buffer_ptr, buffer_size);
			if (!uploadJobMetaByIDArray(packet_ptr, job_id_vec))
			{
				return kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeJobOperByIndexArray:
		{
			i32vector job_index_vec;
			CPOBase::memReadVector(job_index_vec, buffer_ptr, buffer_size);
			if (!uploadJobMetaByIndexArray(packet_ptr, job_index_vec))
			{
				return kPOErrInvalidOper;
			}
			break;
		}
		default:
		{
			return kPOErrInvalidOper;
		}
	}

	checkRead(buffer_ptr, packet_ptr);
	return kPOSuccess;
}

i32 CProcessor::onRequestSetJobMeta(Packet* packet_ptr)
{
	//check packet
	if (!packet_ptr)
	{
		return kPOErrInvalidPacket;
	}

	i32 sub_cmd = packet_ptr->getSubCmd();
	if (sub_cmd != kSCSubTypeJobOperByID)
	{
		return kPOErrInvalidOper;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidData;
	}

	//update job thumb
	i32 job_id = packet_ptr->getReservedi32(0);
	CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id);
	if (!job_ptr || !isCompatibilityJob(job_ptr))
	{
		printlog_lvs2("SetJobMeta failed, invalid job", LOG_SCOPE_APP);
		return kSCErrInvalidJob;
	}

	job_ptr->updateInfo(kJobEditUpdateThumb);
	job_ptr->setChanged(kJobMeta);
	job_ptr->memMetaRead(buffer_ptr, buffer_size);
	m_job_manager_ptr->writeJobToFile();

	checkRead(buffer_ptr, packet_ptr);
	
	//upload response
	if (buffer_size != 0)
	{
		return kPOErrInvalidData;
	}

	uploadJobMeta(packet_ptr, job_ptr);
	return kPOSuccess;
}

void CProcessor::sendSnapImage(i32 cmd, ImageData* img_data_ptr, bool is_last)
{
	if (!img_data_ptr || !img_data_ptr->isValid() || !isAdminProcessor())
	{
		return;
	}

	i32 len = img_data_ptr->memSize();
	if (len > 0)
	{
		u8* buffer_ptr;
		Packet* pak = po_new Packet(cmd, kPOPacketRespOK, is_last);
		{
			pak->setSubCmd(kSCSubTypeImageRaw);
			pak->allocateBuffer(len, buffer_ptr);
			img_data_ptr->memWrite(buffer_ptr, len);
		}
		sendPacketToNet(pak, buffer_ptr);
	}
}

bool CProcessor::uploadJobData(CJobUnit* job_ptr, i32 mode, const i32vector& tool_id_vec)
{
	if (!job_ptr || mode == kJobDataNone || !g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return false;
	}

	i32vector tool_index_vec;
	i32 i, index, count = (i32)tool_id_vec.size();
	if (!CPOBase::isPositive(count))
	{
		return true;
	}

	//convert to tool_index_vec from tool_id_vec 
	tool_index_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		index = job_ptr->getToolIndex(tool_id_vec[i]);
		if (index < 0)
		{
			return false;
		}
		tool_index_vec[i] = index;
	}

	//send selected information and tool data
	i32 len = 0;
	len += CPOBase::getVectorMemSize(tool_id_vec);
	len += job_ptr->memDataSize(mode, tool_index_vec);
	if (len <= 0)
	{
		return false;
	}

	u8* buffer_ptr;
	i32 buffer_size = len;
	Packet* pak = po_new Packet(kSCCmdJobDataSync, kPOPacketRespOK);
	pak->setSubCmd(mode);
	pak->setReservedi32(0, job_ptr->getJobID());
	pak->allocateBuffer(len, buffer_ptr);

	CPOBase::memWriteVector(tool_id_vec, buffer_ptr, buffer_size);
	job_ptr->memDataWrite(mode, tool_index_vec, buffer_ptr, buffer_size);
	sendPacketToNet(pak, buffer_ptr);
	return true;
}

bool CProcessor::uploadJobContent(i32 cmd, CJobUnit* job_ptr, bool is_last)
{
	if (!job_ptr)
	{
		return false;
	}

	i32 len = job_ptr->memSize();
	if (len <= 0)
	{
		return false;
	}

	u8* buffer_ptr;
	i32 buffer_size = len;
	Packet* pak = po_new Packet(cmd, kPOPacketRespOK, is_last);
	pak->setSubCmd(kSCSubTypeJobData);
	pak->setReservedi32(0, job_ptr->getJobID());
	pak->allocateBuffer(len, buffer_ptr);

	job_ptr->memWrite(buffer_ptr, buffer_size);
	sendPacketToNet(pak, buffer_ptr);
	return true;
}

bool CProcessor::uploadJobMetaByIDArray(Packet* packet_ptr, i32vector& job_id_vec)
{
	CJobUnit* job_ptr;
	JobVector job_ptr_vec;
	job_ptr_vec.clear();

	for (i32 i = 0; i < job_id_vec.size(); i++)
	{
		job_ptr = m_job_manager_ptr->findJob(job_id_vec[i]);
		if (!job_ptr)
		{
			continue;
		}
		job_ptr_vec.push_back(job_ptr);
	}
	return uploadJobMetaArray(packet_ptr, job_ptr_vec);
}

bool CProcessor::uploadJobMetaByIndexArray(Packet* packet_ptr, i32vector& job_index_vec)
{
	CJobUnit* job_ptr;
	JobVector job_ptr_vec;
	job_ptr_vec.clear();

	i32 i, count = (i32)job_index_vec.size();
	for (i = 0; i < count; i++)
	{
		job_ptr = m_job_manager_ptr->getJobUnit(job_index_vec[i]);
		if (!job_ptr || !isCompatibilityJob(job_ptr))
		{
			continue;
		}
		job_ptr_vec.push_back(job_ptr);
	}
	return uploadJobMetaArray(packet_ptr, job_ptr_vec);
}

bool CProcessor::uploadJobMeta(Packet* packet_ptr, CJobUnit* job_ptr)
{
	if (!job_ptr || !packet_ptr)
	{
		return false;
	}

	if (!job_ptr->hasJobData(kJobMeta))
	{
		if (!m_job_manager_ptr->loadJobData(job_ptr, kJobMeta))
		{
			printlog_lvs2(QString("UploadJobMeta is failed"), LOG_SCOPE_APP);
			return false;
		}
	}

	i32 len = job_ptr->memMetaSize();
	if (len > 0)
	{
		u8* buffer_ptr;
		i32 buffer_size = len;
		Packet* pak = Packet::makeRespPacket(packet_ptr);

		pak->allocateBuffer(len, buffer_ptr);
		job_ptr->memMetaWrite(buffer_ptr, buffer_size);
		sendPacketToNet(pak, buffer_ptr);
	}
	return true;
}

bool CProcessor::uploadJobMetaArray(Packet* packet_ptr, JobVector& job_vec)
{
	CJobUnit* job_ptr;
	i32 i, len = 0, count = 0;

	//calc length of thumblist
	for (i = 0; i < job_vec.size(); i++)
	{
		job_ptr = job_vec[i];
		if (!job_ptr->hasJobData(kJobMeta))
		{
			if (!m_job_manager_ptr->loadJobData(job_ptr, kJobMeta))
			{
				printlog_lvs2(QString("UploadJobMetaArray failed."), LOG_SCOPE_APP);
				continue;
			}
		}
		count++;
		len += job_ptr->memMetaSize();
	}
	len += sizeof(count);

	//send thumblist data
	u8* buffer_ptr;
	i32 buffer_size = len;
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	if (pak)
	{
		pak->allocateBuffer(len, buffer_ptr);
		CPOBase::memWrite(count, buffer_ptr, buffer_size);

		for (i = 0; i < job_vec.size(); i++)
		{
			job_ptr = job_vec[i];
			if (!job_ptr->hasJobData(kJobMeta))
			{
				continue;
			}

			job_ptr->memMetaWrite(buffer_ptr, buffer_size);
		}
		sendPacketToNet(pak, buffer_ptr);
	}
	return true;
}

void CProcessor::updateRuntimeHistory(CJobResult* job_result_ptr)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->updateRuntimeHistory(job_result_ptr);
	}
}

void CProcessor::updateRuntimeHistory(i32 mode, void* data_ptr)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->updateRuntimeHistory(mode, data_ptr);
	}
}

bool CProcessor::updateJobCamSetting(i32 mode)
{
	if (!m_cam_param_ptr)
	{
		return false;
	}

	//tune camera setting into current job
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
	if (job_ptr)
	{
		job_ptr->setCameraSetting(m_cam_param_ptr, mode);
		job_ptr->setChanged(kJobContent);
	}
	return true;
}

void CProcessor::sendPacketToNet(Packet* packet_ptr)
{
	if (!g_main_app_ptr || !packet_ptr)
	{
		POSAFE_DELETE(packet_ptr);
		return;
	}
	g_main_app_ptr->sendPacketToNet(packet_ptr);
}

void CProcessor::sendPacketToNet(Packet* packet_ptr, u8* buffer_ptr)
{
	if (!g_main_app_ptr || !packet_ptr)
	{
		POSAFE_DELETE(packet_ptr);
		return;
	}

	g_main_app_ptr->sendPacketToNet(packet_ptr, buffer_ptr);
}

void CProcessor::uploadRuntimeHistory(i32 cmd)
{
	if (!g_main_app_ptr)
	{
		return;
	}
	g_main_app_ptr->uploadRuntimeHistory(cmd);
}

void CProcessor::checkRead(u8* buffer_ptr, Packet* packet_ptr)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->checkPacket(packet_ptr, buffer_ptr);
	}
}

void CProcessor::uploadEndPacket(i32 cmd, i32 code, i32 sub_cmd)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->uploadEndPacket(cmd, code, sub_cmd);
	}
}

void CProcessor::uploadFailPacket(i32 cmd, i32 code, i32 subcmd)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->uploadFailPacket(cmd, code, subcmd);
	}
}

void CProcessor::uploadEndPacket(Packet* packet_ptr)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->uploadEndPacket(packet_ptr);
	}
}

void CProcessor::uploadFailPacket(Packet* packet_ptr, i32 code)
{
	if (g_main_app_ptr)
	{
		g_main_app_ptr->uploadFailPacket(packet_ptr, code);
	}
}

void CProcessor::uploadCalibExport(Packet* packet_ptr)
{
	CameraCalib cam_calib_param = m_cam_param_ptr->getCameraCalib()->getValue();
	i32 calib_size = cam_calib_param.memSize();
	i32 len = calib_size + sizeof(i32);

	u8* buffer_ptr;
	i32 buffer_size = len;
	Packet* pak = Packet::makeRespPacket(packet_ptr);

	if (pak->allocateBuffer(len, buffer_ptr))
	{
		CPOBase::memWrite(calib_size, buffer_ptr, buffer_size);
		cam_calib_param.memWrite(buffer_ptr, buffer_size);
	}
	else
	{
		printlog_lvs2("Can't allocate uploadCalibExport packet", LOG_SCOPE_APP);
	}
	sendPacketToNet(pak);
}

void CProcessor::uploadCalibResult(i32 cmd, i32 sub_cmd, bool is_last)
{
	CameraCalib cam_calib_param = m_sc_engine.getCalibResult();
	i32 len = cam_calib_param.memSize();
	if (len > 0)
	{
		u8* buffer_ptr;
		i32 buffer_size = len;
		Packet* pak = po_new Packet(cmd, kPOPacketRespOK, is_last);
		pak->setSubCmd(sub_cmd);
		if (pak->allocateBuffer(len, buffer_ptr))
		{
			cam_calib_param.memWrite(buffer_ptr, buffer_size);
			sendPacketToNet(pak, buffer_ptr);
			return;
		}
	}
	printlog_lvs2("Can't allocate uploadCalibResult packet", LOG_SCOPE_APP);
}

void CProcessor::uploadJobResult(i32 cmd, i32 mode, bool reset_stamp)
{
	CJobResult* job_result_ptr = m_sc_engine.getJobResult();
	if (!job_result_ptr || !g_main_app_ptr->checkAppDesc(kPODescHighLevel))
	{
		return;
	}

	//update result time stamp
	i64 bak_result_stamp = 0;
	if (reset_stamp)
	{
		bak_result_stamp = job_result_ptr->getTimeStamp();
		job_result_ptr->setTimeStamp(kResultTimeStampAlone);
	}

	i32 len = job_result_ptr->memSize(mode);
	if (len > 0)
	{
		u8* buffer_ptr;
		i32 buffer_size = len;
		Packet* pak = po_new Packet(cmd, kPOPacketRespOK);
		pak->setSubCmd(kSCSubTypeResultData);
		pak->allocateBuffer(len, buffer_ptr);
		pak->setReservedi64(0, job_result_ptr->getTimeStamp());
		//debug_log(QString("JobResult, timestamp:%1").arg(job_result_ptr->getTimeStamp()));

		job_result_ptr->memWrite(mode, buffer_ptr, buffer_size);
		sendPacketToNet(pak, buffer_ptr);
	}

	//restore result time stamp
	if (reset_stamp)
	{
		job_result_ptr->setTimeStamp(bak_result_stamp);
	}
}

void CProcessor::uploadToolResultVector(i32 cmd, CToolResultVector& result_vec)
{
	i32 i, count, mode = kResultWithAll;
	count = (i32)result_vec.size();

	//check result count
	if (!CPOBase::isCount(count))
	{
		return;
	}

	i32 len = 0;
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += result_vec[i]->memSize(mode);
	}
	if (len > 0)
	{
		u8* buffer_ptr;
		i32 buffer_size = len;
		Packet* pak = po_new Packet(cmd, kPOPacketRespOK, false);
		pak->setSubCmd(kSCSubTypeResultData);
		pak->allocateBuffer(len, buffer_ptr);

		CPOBase::memWrite(count, buffer_ptr, buffer_size);
		for (i = 0; i < count; i++)
		{
			result_vec[i]->memWrite(mode, buffer_ptr, buffer_size);
		}
		sendPacketToNet(pak, buffer_ptr);
	}
}

const bool CProcessor::isUsedProcessor()
{
	QMutexLocker l(&m_mutex_processor);
	if (!m_cam_param_ptr)
	{
		return false;
	}
	return (m_is_admin_used || m_is_thread_used);
}

const bool CProcessor::isAdminProcessor()
{
	QMutexLocker l(&m_mutex_processor);
	if (!m_cam_param_ptr)
	{
		return false;
	}
	return m_is_admin_used;
}

const bool CProcessor::isThreadProcessor()
{
	QMutexLocker l(&m_mutex_processor);
	if (!m_cam_param_ptr)
	{
		return false;
	}
	return m_is_thread_used;
}

i32 CProcessor::onInternalUsed(Packet* pak, i32 mode)
{
	if (!m_cam_param_ptr)
	{
		return kPOErrInvalidOper;
	}

	if (!m_is_admin_used && mode == kPUModeAdmin)
	{
		m_is_admin_used = true;
		updatePUState(g_main_app_ptr->m_app_state, g_main_app_ptr->m_app_mode);
		onInternalAdminSync(NULL);

		i32 app_desc = g_main_app_ptr->getAppDesc();
		if (CPOBase::bitCheck(app_desc, kPODescHighLevel))
		{
			//use h264 encoding for video streaming
			onInternalNetOpen(NULL);
		}
		printlog_lv1(QString("Amdin Processor%1 is set to use").arg(m_pu_id));
	}
	else if (mode == kPUModeThread)
	{
		m_is_thread_used = true;

		updatePUState(kSCStateRun, kSCModeNone);
		printlog_lv1(QString("Thread Processor%1 is set to use").arg(m_pu_id));
	}
	return kPOSuccess;
}

i32 CProcessor::onInternalIdle(Packet* pak, i32 mode)
{
	if (!isUsedProcessor())
	{
		return kPOSuccess;
	}
	
	if (m_is_thread_used && CPOBase::bitCheck(mode, kPUModeThread))
	{
		printlog_lv1(QString("Thread processor%1 is removed").arg(m_pu_id));
		m_is_thread_used = false;

		//remove thread job in current processor
		CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
		if (job_ptr && m_is_admin_used && !CPOBase::bitCheck(mode, kPUFlagKeepJob))
		{
			//send deselect command to high-level, for external device such as PLC
			m_job_manager_ptr->setCurJob(NULL);

			Packet* pak = po_new Packet(kSCCmdAdminDeSelectJob, kPOPacketRespOK);
			pak->setReservedi32(0, job_ptr->getJobID());
			sendPacketToNet(pak);
		}
	}
	if (m_is_admin_used && CPOBase::bitCheck(mode, kPUModeAdmin))
	{
		printlog_lv1(QString("Amdin processor%1 is removed").arg(m_pu_id));
		onInternalNetClose(NULL); //call before remove admin flag
		m_is_admin_used = false;

		if (!CPOBase::bitCheck(mode, kPUFlagKeepJob))
		{
			m_job_manager_ptr->setCurJob(NULL);
		}
		if (m_is_thread_used)
		{
			updatePUState(kSCStateRun, kSCModeNone);
		}
	}

	//현재의 Processor가 완전히 해방되였을때...
	if (!m_is_admin_used && !m_is_thread_used)
	{
		updatePUState(kSCStateStop, kSCModeNone);
		updateOutputState(kPUUpdateRemoveTask);
		m_job_manager_ptr->setCurJob(NULL, m_pu_id);

		//reset sc engine
		m_sc_engine.clearInstance();

		//reset camera capture
		if (m_cam_capture_ptr && !m_cam_capture_ptr->isReady())
		{
			m_cam_capture_ptr->exitInstance();

			QMutexLocker l(&m_mutex_processor);
			m_cam_param_ptr = NULL;
		}
	}
	return kPOSuccess;
}

i32 CProcessor::onInternalAdminSync(Packet* packet_ptr)
{
	if (!isAdminProcessor() || !isProcessorAvailable())
	{
		return kPOErrInvalidOper;
	}

	//send admin state with one packet
	i32 job_id = -1;
	bool is_cam_connected = false;

	i32 len = 0;
	len += sizeof(job_id);
	len += sizeof(m_pu_state);
	len += sizeof(m_pu_mode);
	len += sizeof(is_cam_connected);
	if (m_cam_param_ptr)
	{
		is_cam_connected = true;
		len += m_cam_param_ptr->memSize();
	}

	if (len > 0)
	{
		u8* buffer_ptr = NULL;
		i32 buffer_size = len;
		Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
		pak->setSubCmd(kSCSubTypeAdminCameraSync);
		pak->allocateBuffer(len, buffer_ptr);

		//collect admin state
		CPOBase::memWrite(m_pu_state, buffer_ptr, buffer_size);
		CPOBase::memWrite(m_pu_mode, buffer_ptr, buffer_size);

		//collect current camera setting
		CPOBase::memWrite(is_cam_connected, buffer_ptr, buffer_size);
		if (m_cam_param_ptr)
		{
			m_cam_param_ptr->memWrite(buffer_ptr, buffer_size);
		}

		//collect current selected job_id
		CJobUnit* job_ptr = m_job_manager_ptr->getCurJob(m_pu_id);
		if (job_ptr)
		{
			job_id = job_ptr->getJobID();
		}

		CPOBase::memWrite(job_id, buffer_ptr, buffer_size);
		sendPacketToNet(pak, buffer_ptr);
	}

	//output log message
	if (m_cam_param_ptr)
	{
		printlog_lv1(QString("Processor%1 cam%2 is syncronized").arg(m_pu_id).arg(m_cam_param_ptr->getCamID()));
	}
	else
	{
		printlog_lv1(QString("Processor%1 is syncronized, but camera param invalid.").arg(m_pu_id));
	}
	return kPOSuccess;
}

i32 CProcessor::onInternalExtTrigger(Packet* packet_ptr)
{
	if (!isProcessorRun() || !isUsedProcessor())
	{
		printlog_lv1(QString("PUstate:%1 invalid in onInternalExtTrigger").arg(m_pu_state));
		return kPOSuccess;
	}
	if (isProcessorFrozenMode())
	{
		printlog_lv1("Processer is frozen in onInternalExtTrigger");
		return kPOSuccess;
	}

	CameraTrigger* trigger_param_ptr = m_cam_param_ptr->getCameraTrigger();
	if (!trigger_param_ptr->isExtTrigger())
	{
		printlog_lv1(QString("Processor TriggerMode[%1] invalid in onInternalExtTrigger.")
						.arg(trigger_param_ptr->getTriggerMode()));
		return kPOSuccess;
	}

	//control delay time for capture frame in external trigger mode of internal I/O
	bool is_success = false;
	i32 from_device = packet_ptr->getReservedi32(0);

	switch (from_device)
	{
		case kPOIOSerial:
		{
			/* 지능카메라의 IO모듈로부터 트리거신호를 받은경우에는 신호의 올림/내림면검사를 진행한다. */
			if (trigger_param_ptr->getTriggerMode() == kCamTriggerIO)
			{
				is_success = true;
			}
			break;
		}
		case kPOIORawRS485:
		case kPOIOModbusRS:
		{
			/* 지능카메라가 콤포구로부터 트리거신호를 받은경우 */
			if (trigger_param_ptr->getTriggerMode() == kCamTriggerRS)
			{
				is_success = true;
			}
			break;
		}
		case kPOIORawTCP:
		case kPOIORawUDP:
		case kPOIOModbusTCP:
		case kPOIOModbusUDP:
		{
			/* 지능카메라가 망으로부터 트리거신호를 받은경우 */
			if (trigger_param_ptr->getTriggerMode() == kCamTriggerNetwork)
			{
				is_success = true;
			}
			break;
		}
		default:
		{
			break;
		}
	}

	if (is_success)
	{
		i32 delay_ms = trigger_param_ptr->getTriggerDelay();
		if (delay_ms > 0)
		{
			printlog_lv1(QString("External trigger is accepted. delay time:%1").arg(delay_ms));
			QTimer::singleShot(delay_ms, this, SLOT(onSnapTriggerTimer()));
		}
		else
		{
			onRequestManualTrigger(NULL, kManualTriggerExt);
		}
	}
	else
	{
		printlog_lv1(QString("SmartCamera is recived unknown trigger. dev[%1]").arg(from_device));
		return kPOErrInvalidOper;
	}
	return kPOSuccess;
}

void CProcessor::onSnapTriggerTimer()
{
	onRequestManualTrigger(NULL, kManualTriggerExt);
}

const i32 CProcessor::getProcessorID()
{
	QMutexLocker l(&m_mutex_processor);
	return m_pu_id;
}

const i32 CProcessor::getCamID()
{
	QMutexLocker l(&m_mutex_processor);
	if (m_cam_param_ptr)
	{
		return m_cam_param_ptr->getCamID();
	}
	return -1;
}

const i32 CProcessor::getResultCode()
{
	QMutexLocker l(&m_mutex_processor);
	return m_result_code;
}

void CProcessor::lock()
{
	if (m_pu_semaphore_ptr)
	{
		m_pu_semaphore_ptr->acquire();
	}
}

void CProcessor::unlock()
{
	if (m_pu_semaphore_ptr)
	{
		m_pu_semaphore_ptr->release();

		i32 available = m_pu_semaphore_ptr->available();
		if (available > 1)
		{
			printlog_lv1(QString("pu_semaphore available is %1").arg(available));
			assert(false);
		}
	}
}

void CProcessor::waitUnlock()
{
	if (m_pu_semaphore_ptr)
	{
		m_pu_semaphore_ptr->acquire();
		m_pu_semaphore_ptr->release();

		i32 available = m_pu_semaphore_ptr->available();
		if (available > 1)
		{
			printlog_lv1(QString("pu_semaphore available is %1").arg(available));
			assert(false);
		}
	}
}

const bool CProcessor::isProcessorRun()
{
	return m_pu_state == kSCStateRun;
}

const bool CProcessor::isProcessorStop()
{
	return m_pu_state == kSCStateStop;
}

const bool CProcessor::isProcessorAvailable()
{
	return CPOBase::checkIndex(m_pu_state, kSCStateNone, kSCStateCount);
}

const bool CProcessor::isProcessorRunning()
{
	if (isProcessorFrozenMode())
	{
		return false;
	}
	return (isProcessorRun() || isProcessorTestMode());
}

const bool CProcessor::isProcessorTestMode()
{
	return CPOBase::bitCheck(m_pu_mode, kSCModeTest);
}

const bool CProcessor::isProcessorFrozenMode()
{
	return CPOBase::bitCheck(m_pu_mode, kSCModeFrozen);
}

const bool CProcessor::isProcessorInteractiveMode()
{
	return CPOBase::bitCheck(m_pu_mode, kSCModeInteractive);
}

const bool CProcessor::isProcessorCalibMode()
{
	return CPOBase::bitCheck(m_pu_mode, kSCModeCalib);
}

void CProcessor::saveImageFrameForDebug(ImageData* img_data_ptr, i32 index)
{
	if (!img_data_ptr)
	{
		return;
	}

	char filename[PO_MAXPATH];
	if (index == 0)
	{
		//set auto frame index
		static i32 debug_frame = 0;
		po_sprintf(filename, PO_MAXPATH, PO_LOG_PATH"debug_%04d.bmp", debug_frame++);
	}
	else if (index < 0)
	{
		po_sprintf(filename, PO_MAXPATH, PO_LOG_PATH"static_debug.bmp");
	}
	else if (index > 0)
	{
		po_sprintf(filename, PO_MAXPATH, PO_LOG_PATH"debug_%d.bmp", index);
	}

	//write debug image to file
	CImageProc::saveImgOpenCV(filename, img_data_ptr);

	//write camera param to file
	m_cam_capture_ptr->saveCameraParamToFile();
}

const bool CProcessor::hasCameraConnection()
{
	//련결된 카메라아이디를 검사한다.
	i32 cam_id = -1;
	{
		QMutexLocker l(&m_mutex_processor);
		if (m_cam_param_ptr)
		{
			cam_id = m_cam_param_ptr->getCamID();
		}
	}
	if (cam_id < 0)
	{
		return false;
	}

	if (!m_cam_capture_ptr->isReady())
	{
		m_cam_capture_ptr->exitInstance();
		{
			QMutexLocker l(&m_mutex_processor);
			m_cam_param_ptr = NULL;
		}
		return false;
	}
	return true;
}