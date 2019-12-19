#pragma once

#include "sc_struct.h"
#include "sc_engine.h"
#include "job_manager.h"
#include "output_result.h"
#include "sc_db_manager.h"
#include "network/packet.h"
#include "base.h"
#include <QThread>
#include <QSemaphore>
#include <QTimer>

class CSCApplication;
class CCameraCapture;
class CSCIOManager;
class CJobManager;
class CTagManager;

class CProcessor : public QThread,  public CLockGuard
{
	Q_OBJECT

public:
	CProcessor();
	virtual ~CProcessor();

	bool						initProcessor(CCameraCapture* cam_capture_ptr, i32 id);
	bool						exitProcessor();

	bool						initInstance();
	bool						exitInstance();

	void						initEvent();

	i32							onInternalNetOpen(Packet* packet_ptr);
	i32							onInternalNetClose(Packet* packet_ptr);
	i32							onInternalUsed(Packet* packet_ptr, i32 mode);
	i32							onInternalIdle(Packet* packet_ptr, i32 mode);
	i32							onInternalSelectCamera(Packet* packet_ptr, i32& blk_mode);
	i32							onInternalSelectProgram(Packet* packet_ptr);
	i32							onInternalExtTrigger(Packet* packet_ptr);
	i32							onInternalAdminSync(Packet* packet_ptr);
	i32							onInternalSelectJob(i32& cmd, Packet* packet_ptr);
	i32							onInternalJobSetting(Packet* packet_ptr);
	i32							onInternalOffline();
	i32							onInternalRunThread();
	i32							onInternalStopThread();

	i32							onRequestRunProcess(Packet* packet_ptr);
	i32							onRequestStopProcess(Packet* packet_ptr);
	i32							onRequestTestMode(Packet* packet_ptr);
	i32							onRequestFreezeMode(Packet* packet_ptr);
	i32							onRequestInteractiveMode(Packet* packet_ptr);
	i32							onRequestAddJob(Packet* packet_ptr);
	i32							onRequestSaveJob(Packet* packet_ptr);
	i32							onRequestGetJob(Packet* packet_ptr);
	i32							onRequestDeleteJob(Packet* packet_ptr);
	i32							onRequestSearchJobs(Packet* packet_ptr);
	i32							onRequestGetJobMeta(Packet* packet_ptr);
	i32							onRequestSetJobMeta(Packet* packet_ptr);
	i32							onRequestUpdateJobThumbnail(Packet* packet_ptr);
	i32							onRequestTeachJob(Packet* packet_ptr);
	i32							onRequestJobInteractive(Packet* packet_ptr);
	i32							onRequestOperation(Packet* packet_ptr);
	i32							onRequestCalib(Packet* packet_ptr);
	i32							onRequestCameraSetting(Packet* packet_ptr);
	i32							onRequestCameraTrigger(Packet* packet_ptr);
	i32							onRequestCameraFocus(Packet* packet_ptr);
	i32							onRequestCameraHDR(Packet* packet_ptr);
	i32							onRequestCameraMultiCapture(Packet* packet_ptr);
	i32							onRequestManualTrigger(Packet* packet_ptr, i32 mode = kManualTriggerNone);

	i32							responseCameraExposure(Packet* packet_ptr, i32 code);
	i32							responseCameraRange(Packet* packet_ptr, i32 code);
	i32							responseCameraColor(Packet* packet_ptr, i32 code);
	i32							responseCameraCorrection(Packet* packet_ptr, i32 code);
	i32							responseCameraShutter(Packet* packet_ptr, i32 code);
	i32							responseCameraStrobe(Packet* packet_ptr, i32 code);
	i32							responseCameraTrigger(Packet* packet_ptr, i32 code);
	i32							responseCameraFocus(Packet* packet_ptr, i32 code);
	i32							responseCameraHDR(Packet* packet_ptr, i32 code);
	i32							responseCameraWhiteBlance(Packet* packet_ptr, i32 code);
	i32							responseCameraMultiCapture(Packet* packet_ptr, i32 code);

	void						onProcessInteractive();
	void						onProcessTeachJob(ImageData* img_data_ptr);
	void						onProcessSnap(ImageData* img_data_ptr, i64 time_stamp, bool is_trigger);
	void						onPrepareSnap(ImageData* img_data_ptr, i64 time_stamp);
	void						onProcessSnapCalib(ImageData* img_data_ptr);
	i32							onProcessCameraCalib(u8*& buffer_ptr, i32& buffer_size);
	i32							onProcessCameraSetting(i32 sflag, CameraSetting* cam_param_ptr);
	i32							onRequestAddTool(Packet* packet_ptr, CJobUnit* job_ptr);
	i32							onRequestDeleteTool(Packet* packet_ptr, CJobUnit* job_ptr);
	i32							onRequestModifyTool(Packet* packet_ptr, CJobUnit* job_ptr, bool& is_tag_updated);
	i32							onRequestCheckMathExpression(Packet* packet_ptr);
	i32							onRequestCheckStringExpression(Packet* packet_ptr);
	i32							onRequestCheckLogicExpression(Packet* packet_ptr);

	bool						updateJobSelect(CJobUnit* job_ptr);
	bool						updatePUState(i32 state, i32 mode);
	bool						updateTriggerMode(CameraTrigger& cam_trigger);
	bool						updateCameraSetting(CameraSetting& cam_param);
	void						updateDOSetting(CIOOutput& ds, bool is_out = true);
	void						updateModBusSetting(CModBusComm& mb_comm, bool is_out = true);
	void						updateFtpSetting(CFtpComm& ftp_output_group);
	void						updateOpcSetting(CJobUnit* job_ptr);
	void						updateOutputState(i32 mode);
	void						updateRuntimeHistory(CJobResult* job_result_ptr);
	void						updateRuntimeHistory(i32 mode, void* data_ptr);
	bool						updateJobCamSetting(i32 mode);

	i32							selectJob(i32 job_id);
	void						captureImage(i32 mode);
	void						checkRead(u8* buffer_ptr, Packet* packet_ptr);
	void						sendPacketToNet(Packet* packet_ptr);
	void						sendPacketToNet(Packet* packet_ptr, u8* buffer_ptr);
	void						sendSnapImage(i32 cmd, ImageData* img_data_ptr, bool is_last);
	void						saveImageFrameForDebug(ImageData* img_data_ptr, i32 index = 0);

	void						uploadCalibResult(i32 cmd, i32 sub_cmd, bool is_last);
	void						uploadCalibExport(Packet* packet_ptr);
	void						uploadJobResult(i32 cmd, i32 mode, bool reset_stamp = false);
	void						uploadRuntimeHistory(i32 cmd);
	void						uploadToolResultVector(i32 cmd, CToolResultVector& tool_result_vec);
	bool						uploadJobData(CJobUnit* job_ptr, i32 mode, const i32vector& tool_id_vec);
	bool						uploadJobContent(i32 cmd, CJobUnit* job_ptr, bool is_last);
	bool						uploadJobMeta(Packet* packet_ptr, CJobUnit* job_ptr);
	bool						uploadJobMetaByIDArray(Packet* packet_ptr, i32vector& job_id_vec);
	bool						uploadJobMetaByIndexArray(Packet* packet_ptr, i32vector& job_id_vec);
	bool						uploadJobMetaArray(Packet* packet_ptr, JobVector& job_vec);
	
	void						uploadEndPacket(Packet* packet_ptr);
	void						uploadFailPacket(Packet* packet_ptr, i32 code = kPOErrCmdFail);
	void						uploadEndPacket(i32 cmd, i32 code = kPOSuccess, i32 sub_cmd = kPOSubTypeNone);
	void						uploadFailPacket(i32 cmd, i32 code = kPOErrCmdFail, i32 sub_cmd = kPOSubTypeNone);

	void						lock();
	void						unlock();
	void						waitUnlock();

	const bool					isProcessorRun();
	const bool					isProcessorStop();
	const bool					isProcessorAvailable();
	const bool					isCompatibilityJob(CJobUnit* job_ptr);
	const bool					isAcceptableSnap(const ScuControl& sync);
	const bool					hasCameraConnection();

	const bool					isProcessorRunning();
	const bool					isProcessorTestMode();
	const bool					isProcessorFrozenMode();
	const bool					isProcessorInteractiveMode();
	const bool					isProcessorCalibMode();
	void						setProcessorCalibMode(bool is_calib);

	const i32					getCamID();
	const i32					getResultCode();
	const i32					getProcessorID();
	const inline i32			getProcessorState() { return m_pu_state; };
	inline CameraSetting*		getCameraSetting() { return m_cam_param_ptr; };
	inline CSCEngine*			getSCEngine() { return &m_sc_engine; };

	const bool					isUsedProcessor();
	const bool					isAdminProcessor();
	const bool					isThreadProcessor();

private:
	i32							onProcessCamColorSetting(i32 sflag, CameraSetting* cam_param_ptr);
	i32							onProcessCamShutterSetting(i32 sflag, CameraSetting* cam_param_ptr);
	i32							onProcessCamRangeSetting(i32 sflag, CameraSetting* cam_param_ptr);
	i32							onProcessCamExposureSetting(i32 sflag, CameraSetting* cam_param_ptr);
	i32							onProcessCamCorrectionSetting(i32 sflag, CameraSetting* cam_param_ptr);
	i32							onProcessCamStrobeSetting(i32 sflag, CameraSetting* cam_param_ptr);
	
	void						onUpdatingTool(CJobUnit* job_ptr);

protected:
	void						run() Q_DECL_OVERRIDE;

signals:
	void						requireImage(ScuControl sync);

public slots:
	void						onSnapImage();
	void						onSyncCameraState();
	void						onSnapTriggerTimer();

	void						onReadPacket(Packet* packet_ptr, i32 mode, i32 conn_mode);
	void						onBroadcastPacket(i32 cmd, i32 mode);

	void						onRequestCommand(i32 cmd, i32 data);
	void						onUpdatingToolFromOpc();
	void						onUpdatingToolFromModbus(i32 addr, i32 size);

public:
	CCameraCapture*				m_cam_capture_ptr;
	CSCIOManager*				m_io_manager_ptr;
	CJobManager*				m_job_manager_ptr;
	CTagManager*				m_tag_manager_ptr;

	CIOOutput					m_do_setting;
	CModBusComm					m_modbus_comm;
	CFtpComm					m_ftp_comm;
	DeviceInfo*					m_device_param_ptr;
	CGeneralSetting*			m_general_param_ptr;
	CEngineParam*				m_engine_param_ptr;
	CameraSetting*				m_cam_param_ptr;

	bool						m_is_inited;
	std::atomic<bool>			m_is_admin_used;
	std::atomic<bool>			m_is_thread_used;
	CSCEngine					m_sc_engine;
	CSCDBManager				m_sc_db_client;
	COutputManager				m_sc_output_manager;

	i32							m_pu_id;
	i32							m_pu_state;
	i32							m_pu_mode;
	ScuControl					m_scu_control;
	
	i32							m_result_code;
	QMutex						m_mutex_processor;
	QSemaphore*					m_pu_semaphore_ptr;
	QTimer						m_pu_idle_timer;

	CameraSetting				m_tmp_cam_param; //used temporary
};

typedef std::vector<CProcessor*> PUArray;