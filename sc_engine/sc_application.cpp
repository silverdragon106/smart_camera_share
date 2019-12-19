#include <QSettings>
#include <QThreadPool>
#include <QFile>

#include "sc_application.h"
#include "sc_io_manager.h"
#include "sc_db_manager.h"
#include "sc_db_client.h"
#include "sc_ipc_manager.h"
#include "sc_define_internal.h"
#include "camera_capture.h"
#include "camera_stream.h"
#include "tag_manager.h"
#include "tag_ui_manager.h"
#include "processor.h"
#include "tool_category.h"
#include "tool_category_text.h"
#include "base.h"
#include "log_config.h"
#include "build_defs.h"
#include "test/test_code.h"
#include "os/os_support.h"

#include "cog_common/cog_common.h"
#include "cog_common/bc_common.h"

#if defined(POR_WITH_OVX)
#include "openvx/sc_graph_pool.h"
#endif
#if defined(POR_SMC3_ON_NVIDIA_TX2)
//extern "C"
//{
#include "smc3_middle_layer.h"
//}
#endif

//#define POR_TESTMODE
#define PU_QUEUE_SIZE		10

QSemaphore g_shp_manager(PO_UNIT_COUNT);
CSCApplication*	g_main_app_ptr = NULL;

CSCApplication::CSCApplication(i32 argc, char *argv[]) : CPOBaseApp(argc, argv)
{
	g_main_app_ptr = this;
	m_admin_processor_ptr = NULL;
	m_app_state = kSCStateRun;
	m_app_mode = kSCModeNone;
	m_opc_updating = false;
	m_modbus_updating = false;
	m_action_auth_fail = kPOTerminateApp;

#if defined(POR_WITH_GUI)
	m_quit_action_ptr = NULL;
	m_toggle_offline_ptr = NULL;
	m_systray_icon_ptr = NULL;
	m_systray_menu_ptr = NULL;

	m_action_ring0_ptr = NULL;
	m_action_app_ptr = NULL;
	m_action_module_ptr = NULL;
	m_action_eventflow_ptr = NULL;
	m_action_runtime_ptr = NULL;

	m_action_scope_app_ptr = NULL;
	m_action_scope_ipc_ptr = NULL;
	m_action_scope_net_ptr = NULL;
	m_action_scope_ivs_ptr = NULL;
	m_action_scope_cam_ptr = NULL;
	m_action_scope_encode_ptr = NULL;
	m_action_scope_io_ptr = NULL;
	m_action_scope_modbus_ptr = NULL;
	m_action_scope_opc_ptr = NULL;
	m_action_scope_db_ptr = NULL;
	m_action_scope_ftp_ptr = NULL;
	m_action_scope_tag_ptr = NULL;
	m_action_scope_ovx_ptr = NULL;

	m_action_scope_all_ptr = NULL;
	m_action_scope_clear_ptr = NULL;
#endif

	m_job_manager_ptr = NULL;
	m_tag_manager_ptr = NULL;
	m_tag_ui_manager_ptr = NULL;
	m_cam_manager_ptr = NULL;
	m_io_manager_ptr = NULL;
	m_db_manager_ptr = NULL;
	m_db_client_ptr = NULL;
	m_tool_categorys_ptr = NULL;
	m_global_var_ptr = NULL;

	for (i32 i = 0; i < SC_SIGNAL_COUNT; i++)
	{
		m_qtsig_reference[i] = 0;
	}

	i32 app_mode = kPODescAllComponents;
#if !defined(POR_WITH_IOMODULE)
	CPOBase::bitRemove(app_mode, kPODescIOManager);
#endif
#if !defined(POR_WITH_MYSQL) && !defined(POR_WITH_SQLITE)
	CPOBase::bitRemove(app_mode, kPODescDatabase);
#endif
#if !defined(POR_WITH_STREAM)
	CPOBase::bitRemove(app_mode, kPODescVideoStream);
#endif
#if !defined(POR_WITH_GUI)
	CPOBase::bitRemove(app_mode, kPODescGUIControl);
#endif
#if !defined(POR_WITH_LOG)
	CPOBase::bitRemove(app_mode, kPODescFileLog);
#endif
#if !defined(POR_WITH_DONGLE)
	CPOBase::bitRemove(app_mode, kPODescUsbDongle);
#endif

	m_app_inited = initApplication(app_mode, argc, argv);
}

CSCApplication::~CSCApplication()
{
	exitApplication();

	POSAFE_DELETE(m_cam_manager_ptr);
	POSAFE_DELETE(m_job_manager_ptr);
	POSAFE_DELETE(m_io_manager_ptr);
	POSAFE_DELETE(m_db_manager_ptr);
	POSAFE_DELETE(m_db_client_ptr);
	POSAFE_DELETE(m_tag_manager_ptr);
	POSAFE_DELETE(m_tag_ui_manager_ptr);
	POSAFE_DELETE(m_tool_categorys_ptr);
	POSAFE_DELETE(m_global_var_ptr);

	printlog_lv0("Quit Application, GoodBye!!!....");
	exitLogFiles();
}

bool CSCApplication::initApplication(i32 desc, i32 argc, char *argv[])
{
	if (!CPOBase::checkCount(PO_CAM_COUNT, PO_MAX_CAM_COUNT) ||
		!CPOBase::checkCount(PO_UNIT_COUNT, PO_MAX_CAM_COUNT))
	{
		return false;
	}

	m_cam_manager_ptr = po_new CCameraManager();
	m_job_manager_ptr = po_new CJobManager();
	m_tag_manager_ptr = po_new CTagManager();
	m_tag_ui_manager_ptr = po_new CTagUIManager();
	m_io_manager_ptr = po_new CSCIOManager();
	m_db_manager_ptr = po_new CSCDBManager();
	m_db_client_ptr = po_new CSCDBClient();
	m_tool_categorys_ptr = po_new CToolCategorys();
	m_global_var_ptr = po_new CGlobalVar();

	initLogFiles();
	singlelog_lv0(QString("SCApplication starting... (InstallDesc:[%1])").arg(getInstallDesc()));

	/* 체계자원할당 */
	registerDataTypes();
	registerToolCategorys();
	QThreadPool::globalInstance()->setMaxThreadCount(PO_UNIT_COUNT);

	initCognexEngine();

#if defined(POR_WITH_STREAM)
	CCaptureStream::registerAllCodec();
#endif
#if defined(POR_USE_HALCON)
	HalconCpp::HSystem::ResetObjDb(4096, 4096, 1);
	HalconCpp::HSystem::SetSystem("timer_mode", "performance_counter");
#endif
#if defined(POR_WITH_OVX)
	g_vx_context.create();
	g_vx_resource_pool.create(&g_vx_context, OvxResourcePool::kRPMaxQueueSize150);
	if (!g_vx_graph_pool.initInstance(&g_vx_context, &g_vx_resource_pool, OvxGraphPool::kGPMaxQueueSize40))
	{
		printlog_lv0("OpenVXGraphPool Init Failed.");
		return false;
	}
#endif
#if defined(POR_SMC3_ON_NVIDIA_TX2)
	if (smc3LayerInitialize() != kMLSuccess)
	{
		printlog_lv0("Smc3LayerInitialize() Failed.");
		return false;
	}
#endif

	/* 기초프로젝트초기화 및 자원창조 */
	if (!CPOBaseApp::initApplication(desc, argc, argv))
	{
		return false;
	}

	/* 디스크관리자를 창조하고 설정파일을 읽는다. */
	if (checkNeedDesc(kPODescDevice))
	{
		if (!g_sc_disk.initInstance(m_data_path.toStdTString()))
		{
			printlog_lv0("Device storage is not initialized.");
			alarmlog1(kPOErrDiskInit);
		}
		if (!initSettings())
		{
			printlog_lv0("InitDeviceSetting error!");
			return false;
		}
		addAppDesc(kPODescDevice);

		//output infomation
		m_device_info.build_date.setDateTime(BUILD_YEAR, BUILD_MONTH, BUILD_DAY,
											BUILD_HOUR, BUILD_MONTH, BUILD_SEC);
		DateTime build_date = m_device_info.getBuildDateTime();

		printlog_lv1("___________________________________");
		printlog_lv1(QString("DeviceID: %1").arg(m_device_info.getDeviceID()));
		printlog_lv1(QString("DeviceName: %1").arg(m_device_info.getDeviceName().c_str()));
		printlog_lv1(QString("DeviceVersion: %1").arg(m_device_info.getDeviceVersion().c_str()));
		printlog_lv0(QString("BuildDate: %1-%2-%3T%4:%5:%6Z")
					.arg(build_date.yy).arg(build_date.mm).arg(build_date.dd)
					.arg(build_date.h).arg(build_date.m).arg(build_date.s));
		printlog_lv1("___________________________________");

		//initialize tag-ui-manager module
		if (!m_tag_ui_manager_ptr->initInstance())
		{
			printlog_lv0("TagUI Manager InitInstance is failed.");
			return false;
		}
	}

	/* 하위기관리를 위한 GUI를 창조한다. */
#if defined(POR_WITH_GUI)
	if (checkNeedDesc(kPODescGUIControl))
	{
		createTrayGUI();
		updateTrayIcon(kPOTrayIconPrepare);
		addAppDesc(kPODescGUIControl);
	}
#endif

	/* 비데오전송을 위한 IPC공유기억관리자창조 */
	if (checkNeedDesc(kPODescIPCStream))
	{
		addAppDesc(kPODescIPCStream);
		m_ipc_manager_ptr = po_new CSCIPCManager();
		m_ipc_manager_ptr->initInstance(CIPCManager::kSideA, PO_IPC_SHMNAME, PO_IPC_SHMSIZE);
	}

	/* Processor초기화 */
	initProcessors();
	
	/* 자료기지를 열고 로그관리자를 초기화한다. */
	if (checkNeedDesc(kPODescDatabase))
	{
		/* 리력자료기지관리모듈을 초기화한다. */
		if (m_db_manager_ptr->initInstance(&m_db_config, true, false))
		{
			m_db_client_ptr->initInstance(&m_db_config);
			g_sc_logger.initInstance(&m_db_config);
			addAppDesc(kPODescDatabase);
		}
		else
		{
			printlog_lv0("MySQL Manager InitInstance is failed...");
			alarmlog1(kPOErrDBModuleInit);
		}
	}

	//register omits to cmd server
	i32vector omit_cmd_vec;
	omit_cmd_vec.push_back(kPOCmdPing);
	omit_cmd_vec.push_back(kSCCmdCameraImage);
	omit_cmd_vec.push_back(kSCCmdGetRumtimeHistory);
	omit_cmd_vec.push_back(kSCCmdCameraSync);
	omit_cmd_vec.push_back(kSCCmdRuntimeSync);
	m_cmd_server.registerOmitCmd(omit_cmd_vec);
	//omit_cmd_vec.push_back(kSCCmdSnapResult);  /* 결과자료는 화상-결과매칭을 위하여 부하제한을 하지 않는다. */
	
	//init event 
	initEvent();
	return true;
}

bool CSCApplication::exitApplication()
{
	singlelog_lv0("SCApplication Quitting Now...");

#if defined(POR_WITH_GUI)
	if (checkAppDesc(kPODescGUIControl))
	{
		deleteTrayGUI();
		removeAppDesc(kPODescGUIControl);
	}
#endif

	if (checkAppDesc(kPODescIPCStream))
	{
		if (m_ipc_manager_ptr)
		{
			m_ipc_manager_ptr->exitInstance();
		}
	}
	
	if (checkNeedDesc(kPODescDevice))
	{
		removeAppDesc(kPODescDevice);
		g_sc_disk.exitInstance();
		m_tag_ui_manager_ptr->exitInstance();
	}

	if (checkAppDesc(kPODescDatabase))
	{
		g_sc_logger.exitInstance();
		m_db_manager_ptr->exitInstance(true);
		m_db_client_ptr->exitInstance();
		removeAppDesc(kPODescDatabase);
	}
	
	exitProcessors();

#if defined(POR_WITH_OVX)
	g_vx_graph_pool.exitInstance();
	g_vx_resource_pool.destroy();
	g_vx_context.destroy();
#endif
#if defined(POR_SMC3_ON_NVIDIA_TX2)
	smc3LayerUninitialize();
#endif

	exitCognexEngine();
	return CPOBaseApp::exitApplication();
}

void CSCApplication::initLogFiles()
{
#if defined(POR_WITH_LOG)
	g_time_logger.initInstance(PO_TIMELOG_FILENAME);
	keep_uptime;
	name_chktime(SC_TIME_UPTIME, "UpTime");
	name_chktime(SC_TIME_PROCESS, "ProcessTime");
	name_chktime(SC_TIME_CAMCAPTURE, "CamCaptureTime");
	name_chktime(SC_TIME_OUTPUT, "OutputTime");
	name_chktime(SC_TIME_ONECYCLE, "OneCycleTime");

	g_debug_logger.initInstance(PO_LOG_FILENAME);
	printlog_lv0("___________________________________________________");
#endif
}

void CSCApplication::exitLogFiles()
{
#if defined(POR_WITH_LOG)
	g_time_logger.debugOutput();
	g_time_logger.exitInstance();
#endif
}

void CSCApplication::registerDataTypes()
{
	CPOBaseApp::registerDataTypes();
	qRegisterMetaType<ScuControl>("SyncControl");
}

void CSCApplication::initEvent()
{
	if (checkNeedDesc(kPODescIOManager))
	{
		//connect SCIOManager event
		m_io_manager_ptr->initEvent(this);
	}
	
#if defined(POR_WITH_GUI)
	if (checkNeedDesc(kPODescGUIControl))
	{
		//connect signal in GUIControl
		connect(m_action_scope_app_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeApp()));
		connect(m_action_scope_ipc_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeEvent()));
		connect(m_action_scope_net_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeNetwork()));
		connect(m_action_scope_ivs_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeIVS()));
		connect(m_action_scope_cam_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeCamera()));
		connect(m_action_scope_encode_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeEncode()));
		connect(m_action_scope_io_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeIOModule()));
		connect(m_action_scope_modbus_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeModbus()));
		connect(m_action_scope_opc_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeOpc()));
		connect(m_action_scope_db_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeDatabase()));
		connect(m_action_scope_ftp_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeFtp()));
		connect(m_action_scope_tag_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeTag()));
		connect(m_action_scope_ovx_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeOvx()));

		connect(m_action_scope_all_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeAll()));
		connect(m_action_scope_clear_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugScopeClear()));

		connect(m_action_ring0_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugLevelRing0()));
		connect(m_action_app_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugLevelApp()));
		connect(m_action_module_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugLevelModule()));
		connect(m_action_eventflow_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugLevelEventflow()));
		connect(m_action_runtime_ptr, SIGNAL(triggered()), this, SLOT(onChangeDebugLevelRuntime()));

		connect(m_toggle_offline_ptr, SIGNAL(triggered()), this, SLOT(onOnlineToggleControl()));
		connect(m_quit_action_ptr, SIGNAL(triggered()), qApp, SLOT(quit()));
	}
#endif

	if (checkNeedDesc(kPODescCamera))
	{
		m_cam_manager_ptr->initEvent(this);
	}
	CPOBaseApp::initEvent();
}

bool CSCApplication::initInstance()
{
	singlelog_lv0("SCApplication InitInstance");

	//태그관리자모듈을 맨 먼저 초기화한다. 리유는 모든 모듈들에서 태그관리자를 참조리용하기때문이다.
	if (!m_tag_manager_ptr->initInstance())
	{
		printlog_lv0("Tag Manager InitInstance is failed.");
		return false;
	}

	//initialize camera manager module
	if (checkNeedDesc(kPODescCamera))
	{
		if (!m_cam_manager_ptr->initInstance(&m_camera_param, &m_engine_param))
		{
			printlog_lv0("Camera Manager InitInstance is failed.");
			return false;
		}
		addAppDesc(kPODescCamera);
	}

	if (checkAppDesc(kPODescDevice))
	{
		//initialize job manager
		if (!m_job_manager_ptr->initInstance())
		{
			printlog_lv0("Template Manager InitInstance is failed.");
			return false;
		}
		if (m_db_manager_ptr)
		{
			m_db_manager_ptr->optimizeJobConfigure(m_job_manager_ptr->getJobVector());
		}

		//initialize communication module
		if (checkNeedDesc(kPODescIOManager))
		{
			if (!m_io_manager_ptr->initInstance(m_job_manager_ptr))
			{
				printlog_lv0("SCIOManager InitInstance is failed.");
				alarmlog1(kPOErrIOModuleOpen);
				return false;
			}
			addAppDesc(kPODescIOManager);
		}
	}
	
	//initialize else
	if (checkAppDesc(kPODescGUIControl))
	{
		updateTrayIcon(kPOTrayIconOnline);
	}

	//initialize processor unit
	i32 i, pu_count = (i32)m_pu_array.size();
	for (i = 0; i < pu_count; i++)
	{
		m_pu_array[i]->initInstance();
	}

	updateAppOpen();

	//initialize based application
	return CPOBaseApp::initInstance();
}

bool CSCApplication::exitInstance()
{
	singlelog_lv0("SCApplication ExitInstance");

	updateAppClose();
	g_sc_disk.writeAllSettings(this);
	g_sc_disk.writeDeviceINISettings(this);

	//initialize processor unit
	i32 i, pu_count = (i32)m_pu_array.size();
	for (i = 0; i < pu_count; i++)
	{
		m_pu_array[i]->exitInstance();
	}

	if (checkAppDesc(kPODescCamera))
	{
		m_cam_manager_ptr->exitInstance();
		removeAppDesc(kPODescCamera);
	}

	if (checkAppDesc(kPODescDevice))
	{
		if (checkAppDesc(kPODescIOManager))
		{
			m_io_manager_ptr->exitInstance();
			removeAppDesc(kPODescIOManager);
		}

		m_tag_manager_ptr->exitInstance();
		m_job_manager_ptr->exitInstance();
	}

	//exitinstance based application
	CPOBaseApp::exitInstance();
	return true;
}

bool CSCApplication::initProcessors()
{
	if (!exitProcessors())
	{
		return false;
	}
	if (!m_cam_manager_ptr->initCaptureUnits(dynamic_cast<CVirtualEncoder*>(m_ipc_manager_ptr)))
	{
		return false;
	}

	//create processor unit, set camera capture and template manager, MYSQL manager in each processor
	for (i32 i = 0; i < PO_UNIT_COUNT; i++)
	{
		CProcessor* processor_ptr = CPOBase::pushBackNew(m_pu_array);
		CCameraCapture* cam_capture_ptr = m_cam_manager_ptr->m_cam_capture_vec[i];
		if (!processor_ptr->initProcessor(cam_capture_ptr, i))
		{
			return false;
		}

		CSCEngine* sc_engine_ptr = processor_ptr->getSCEngine();
		cam_capture_ptr->presetInstance(sc_engine_ptr, NULL, NULL, NULL);
		printlog_lv1(QString("Processor[%1] is inited.").arg(processor_ptr->getProcessorID()));
	}

	initProcessorQueue();
	m_admin_processor_ptr = NULL;
	return true;
}

bool CSCApplication::exitProcessors()
{
	initProcessorQueue();
	if (m_cam_manager_ptr)
	{
		m_cam_manager_ptr->exitCaptureUnits();
	}

	m_admin_processor_ptr = NULL;
	POSAFE_CLEAR(m_pu_array);
	return true;
}

void CSCApplication::createTrayGUI()
{
#if defined(POR_WITH_GUI)
	m_action_scope_app_ptr = po_new QAction("Scope(app)", this);
	m_action_scope_ipc_ptr = po_new QAction("Scope(event)", this);
	m_action_scope_net_ptr = po_new QAction("Scope(net)", this);
	m_action_scope_ivs_ptr = po_new QAction("Scope(ivs)", this);
	m_action_scope_cam_ptr = po_new QAction("Scope(cam)", this);
	m_action_scope_encode_ptr = po_new QAction("Scope(encode)", this);
	m_action_scope_io_ptr = po_new QAction("Scope(io)", this);
	m_action_scope_modbus_ptr = po_new QAction("Scope(modbus)", this);
	m_action_scope_opc_ptr = po_new QAction("Scope(opc)", this);
	m_action_scope_db_ptr = po_new QAction("Scope(db)", this);
	m_action_scope_ftp_ptr = po_new QAction("Scope(ftp)", this);
	m_action_scope_tag_ptr = po_new QAction("Scope(tag)", this);
	m_action_scope_ovx_ptr = po_new QAction("Scope(ovx)", this);

	m_action_scope_app_ptr->setCheckable(true);
	m_action_scope_ipc_ptr->setCheckable(true);
	m_action_scope_net_ptr->setCheckable(true);
	m_action_scope_ivs_ptr->setCheckable(true);
	m_action_scope_cam_ptr->setCheckable(true);
	m_action_scope_encode_ptr->setCheckable(true);
	m_action_scope_io_ptr->setCheckable(true);
	m_action_scope_modbus_ptr->setCheckable(true);
	m_action_scope_opc_ptr->setCheckable(true);
	m_action_scope_db_ptr->setCheckable(true);
	m_action_scope_ftp_ptr->setCheckable(true);
	m_action_scope_tag_ptr->setCheckable(true);
	m_action_scope_ovx_ptr->setCheckable(true);

	m_action_scope_all_ptr = po_new QAction("Scope All", this);
	m_action_scope_clear_ptr = po_new QAction("Scope Clear", this);
	m_action_scope_all_ptr->setCheckable(true);
	m_action_scope_clear_ptr->setCheckable(true);
		
	m_action_ring0_ptr = po_new QAction("debug(ring0)", this);
	m_action_app_ptr = po_new QAction("debug(app)", this);
	m_action_module_ptr = po_new QAction("debug(module)", this);
	m_action_eventflow_ptr = po_new QAction("debug(eventflow)", this);
	m_action_runtime_ptr = po_new QAction("debug(runtime)", this);
	m_action_ring0_ptr->setCheckable(true);
	m_action_app_ptr->setCheckable(true);
	m_action_module_ptr->setCheckable(true);
	m_action_eventflow_ptr->setCheckable(true);
	m_action_runtime_ptr->setCheckable(true);

	m_toggle_offline_ptr = po_new QAction("Online/Offline", this);
	m_quit_action_ptr = po_new QAction("&Quit", this);
	
	m_systray_menu_ptr = po_new QMenu(NULL);
	
	m_systray_menu_ptr->addAction(m_action_scope_app_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_ipc_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_net_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_ivs_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_cam_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_encode_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_io_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_modbus_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_opc_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_db_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_ftp_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_tag_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_ovx_ptr);
	m_systray_menu_ptr->addSeparator();

	m_systray_menu_ptr->addAction(m_action_scope_all_ptr);
	m_systray_menu_ptr->addAction(m_action_scope_clear_ptr);
	m_systray_menu_ptr->addSeparator();

	m_systray_menu_ptr->addAction(m_action_ring0_ptr);
	m_systray_menu_ptr->addAction(m_action_app_ptr);
	m_systray_menu_ptr->addAction(m_action_module_ptr);
	m_systray_menu_ptr->addAction(m_action_eventflow_ptr);
	m_systray_menu_ptr->addAction(m_action_runtime_ptr);
	m_systray_menu_ptr->addSeparator();
	
	m_systray_menu_ptr->addAction(m_toggle_offline_ptr);
	m_systray_menu_ptr->addSeparator();

	m_systray_menu_ptr->addAction(m_quit_action_ptr);

	m_systray_icon_ptr = po_new QSystemTrayIcon(this);
	m_systray_icon_ptr->setContextMenu(m_systray_menu_ptr);

	updateDebugLevelGUI(get_loglevel());
	updateDebugScopeGUI(get_logscope());
#endif
}

void CSCApplication::deleteTrayGUI()
{
#if defined(POR_WITH_GUI)
	//hide trayicon, when exit first
	if (m_systray_icon_ptr)
	{
		m_systray_icon_ptr->hide();
	}

	POSAFE_DELETE(m_action_scope_app_ptr);
	POSAFE_DELETE(m_action_scope_ipc_ptr);
	POSAFE_DELETE(m_action_scope_net_ptr);
	POSAFE_DELETE(m_action_scope_ivs_ptr);
	POSAFE_DELETE(m_action_scope_cam_ptr);
	POSAFE_DELETE(m_action_scope_encode_ptr);
	POSAFE_DELETE(m_action_scope_io_ptr);
	POSAFE_DELETE(m_action_scope_modbus_ptr);
	POSAFE_DELETE(m_action_scope_opc_ptr);
	POSAFE_DELETE(m_action_scope_db_ptr);
	POSAFE_DELETE(m_action_scope_ftp_ptr);
	POSAFE_DELETE(m_action_scope_tag_ptr);
	POSAFE_DELETE(m_action_scope_ovx_ptr);

	POSAFE_DELETE(m_action_scope_all_ptr);
	POSAFE_DELETE(m_action_scope_clear_ptr);

	POSAFE_DELETE(m_action_ring0_ptr);
	POSAFE_DELETE(m_action_app_ptr);
	POSAFE_DELETE(m_action_module_ptr);
	POSAFE_DELETE(m_action_eventflow_ptr);
	POSAFE_DELETE(m_action_runtime_ptr);

	POSAFE_DELETE(m_toggle_offline_ptr);
	POSAFE_DELETE(m_quit_action_ptr);

	POSAFE_DELETE(m_systray_icon_ptr);
	POSAFE_DELETE(m_systray_menu_ptr);
#endif
}

void CSCApplication::updateTrayIcon(i32 mode)
{
#if defined(POR_WITH_GUI)
	if (!m_systray_icon_ptr)
	{
		return;
	}

	switch (mode)
	{
		case kPOTrayIconPrepare:
		{
			m_systray_icon_ptr->setIcon(QIcon(":/sc_device/icons/camera_prepare.png"));
			m_systray_icon_ptr->show();
			break;
		}
		case kPOTrayIconOnline:
		{
			m_systray_icon_ptr->setIcon(QIcon(":/sc_device/icons/camera_online.png"));
			break;
		}
		case kPOTrayIconOffline:
		{
			m_systray_icon_ptr->setIcon(QIcon(":/sc_device/icons/camera_offline.png"));
			break;
		}
		case kPOTrayIconDisconnect:
		{
			m_systray_icon_ptr->setIcon(QIcon(":/sc_device/icons/camera_disconnect.png"));
			break;
		}
	}
#endif
}

bool CSCApplication::initCognexEngine()
{
	QFile file(":/sc_device/sc_engine/cognex/buffers.prm");
	if (!file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QByteArray data_array = file.readAll();
	u8* buffer_ptr = (u8*)(data_array.data());
	initialize_cognex();
	read_cog_buffers(buffer_ptr);
	read_bc_buffers(buffer_ptr);
	return true;
}

void CSCApplication::exitCognexEngine()
{
	exit_cog_buffers();
	exit_bc_buffers();
	uninitialize_cognex();
}

void CSCApplication::updateDebugLevelGUI(i32 level)
{
#if defined(POR_WITH_GUI)
	m_action_ring0_ptr->setChecked(level == LOG_LV0);
	m_action_app_ptr->setChecked(level == LOG_LV1);
	m_action_module_ptr->setChecked(level == LOG_LV2);
	m_action_eventflow_ptr->setChecked(level == LOG_LV3);
	m_action_runtime_ptr->setChecked(level == LOG_LV4);
#endif
}

void CSCApplication::updateDebugScopeGUI(i32 scope)
{
#if defined(POR_WITH_GUI)
	m_action_scope_app_ptr->setChecked(scope & LOG_SCOPE_APP);
	m_action_scope_ipc_ptr->setChecked(scope & LOG_SCOPE_IPC);
	m_action_scope_net_ptr->setChecked(scope & LOG_SCOPE_NET);
	m_action_scope_ivs_ptr->setChecked(scope & LOG_SCOPE_IVS);
	m_action_scope_cam_ptr->setChecked(scope & LOG_SCOPE_CAM);
	m_action_scope_encode_ptr->setChecked(scope & LOG_SCOPE_ENCODE);
	m_action_scope_io_ptr->setChecked(scope & LOG_SCOPE_IO);
	m_action_scope_modbus_ptr->setChecked(scope & LOG_SCOPE_COMM);
	m_action_scope_opc_ptr->setChecked(scope & LOG_SCOPE_OPC);
	m_action_scope_db_ptr->setChecked(scope & LOG_SCOPE_DB);
	m_action_scope_ftp_ptr->setChecked(scope & LOG_SCOPE_FTP);
	m_action_scope_tag_ptr->setChecked(scope & LOG_SCOPE_TAG);
	m_action_scope_ovx_ptr->setChecked(scope & LOG_SCOPE_OVX);

	m_action_scope_all_ptr->setChecked(scope & LOG_SCOPE_ALL);
	m_action_scope_clear_ptr->setChecked(scope == LOG_SCOPE_NONE);
#endif
}

void CSCApplication::updateAppOpen()
{
#if (1)
	{
		singlelog_lv0("TestMode is");

		CTestCode* test_code_ptr = po_new CTestCode();
		test_code_ptr->run();
		POSAFE_DELETE(test_code_ptr);
	}
#endif

	m_opc_updating = false;
	m_modbus_updating = false;
	m_io_manager_ptr->resetOutput();
	m_io_manager_ptr->outputCalib(m_camera_param.getCamSetting(0), PO_CAM_COUNT);

	//set default admin processor
 	Packet* pak = NULL;
	CJobUnit* job_ptr = m_job_manager_ptr->getCurJob();
	if (job_ptr)
	{
		if (setCameraProcessor(job_ptr->getCamID(), true, kNonHLCommand))
		{
			//send select model internal signal
			pak = po_new Packet(kSCCmdIntSelectJob, kPOPacketRequest);

			pak->setReservedi32(0, job_ptr->getJobID());
			pak->setReservedu8(4, kRespJobInfo);
			pak->setReservedu8(5, kOperationbyAdmin);

			if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
			{
				m_job_manager_ptr->setCurJob(NULL);
				printlog_lv1("Select Job Failed in AppOpen.");
			}
			return;
		}
	}
	m_job_manager_ptr->setCurJob(NULL);
	setAdminFirstCameraProcessor(true);
}

void CSCApplication::updateAppClose()
{
#if defined(POR_WITH_LOG)
	write_all_time;
#endif

	disconnectProcessorAll();

	if (checkAppDesc(kPODescIOManager))
	{
		m_io_manager_ptr->resetOutput();
	}
}

void CSCApplication::updateStateNetOpen()
{
	Packet* pak = po_new Packet(kSCCmdIntNetOpen, kPOPacketRequest);
	if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
	{
		printlog_lv1("Admin processor is busy in NetOpen");
	}
}

void CSCApplication::updateStateNetClose()
{
	Packet* pak = po_new Packet(kSCCmdIntNetClose, kPOPacketRequest);
	if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
	{
		printlog_lv1("Admin processor is busy in NetClose");
	}
}

bool CSCApplication::initSettings()
{
	keep_uptime;
	initAllSetting();

	if (g_sc_disk.loadAllSettings(this))
	{
		if (!g_sc_disk.loadDeviceINISettings(this))
		{
			g_sc_disk.writeAdminINISettings(this, m_device_info.getDeviceVersion());
			g_sc_disk.writeDeviceINISettings(this);
			return false;
		}
		printlog_lv0("Setting data loading OK.");
	}
	else
	{
		initAllSetting();
		if (!g_sc_disk.loadDeviceINISettings(this))
		{
			g_sc_disk.writeAdminINISettings(this, m_device_info.getDeviceVersion());
			g_sc_disk.writeDeviceINISettings(this);
		}
		g_sc_disk.writeAllSettings(this);
		printlog_lv0("Setting data is not compatibility with device.");
	}

	//update setting
	set_loglevel(m_general_param.getDebugLevel());
	COSBase::setHostName(m_device_info.getDeviceName());
	return true;
}

bool CSCApplication::loadDeviceINISettings(potstring& str_filename)
{
	QString qstr_filename = QString::fromTCharArray(str_filename.c_str());
	QSettings ini(qstr_filename, QSettings::IniFormat);
	if (ini.allKeys().count() <= 0)
	{
		return false;
	}

	//read device information such as device name, device model name, version from INI file.
	if (!m_use_ext_path)
	{
		m_device_info.device_id	= ini.value("DEVICE/DeviceID", m_device_info.device_id).toInt();
	}

	m_device_info.device_name		= ini.value("DEVICE/DeviceName", m_device_info.device_name.c_str()).toString().toStdString();
	m_device_info.device_version	= ini.value("DEVICE/Version", m_device_info.device_version.c_str()).toString().toStdString();
	m_device_info.model_name		= ini.value("DEVICE/ModelName", m_device_info.model_name.c_str()).toString().toStdString();
	m_device_info.is_hl_embedded	= ini.value("DEVICE/HLEmbedded", m_device_info.is_hl_embedded).toBool();
	m_device_info.is_auto_update	= ini.value("DEVICE/AutoUpdate", m_device_info.is_auto_update).toBool();
	m_device_info.comm_port			= ini.value("DEVICE/UDPPort", m_device_info.comm_port).toInt();
	
#if defined(POR_WITH_AUTHENTICATION)
	postring auth_id;
	postring auth_password = ini.value("DEVICE/AuthPassword", "").toString().toStdString();
	if (m_ip_info.m_netadapter_vec.size() > 0)
	{
		auth_id = m_ip_info.m_netadapter_vec[0].mac_address;
	}
	CPOBaseApp::setAuthIdPassword(auth_id, auth_password);
#endif

#if defined(POR_WITH_STREAM)
	postring encode_mode = ini.value("DEVICE/Encoder").toString().toStdString();
	CPOBase::toLower(encode_mode);

  #if defined(POR_SUPPORT_GSTREAMER)
	if (encode_mode == "gstreamer")
	{
		m_device_info.video_encoder = kPOEncoderGStreamerMJpeg;
	}
  #elif defined(POR_SUPPORT_FFMPEG)
	if (encode_mode == "ffmpeg")
	{
		m_device_info.video_encoder = kPOEncoderFFMpegMJpeg;
	}
  #endif

	//check valid encoder mode
	if (m_device_info.video_encoder == kPOEncoderNone)
	{
		m_device_info.video_encoder = kPOEncoderNetworkRaw;
	}
#endif

	loadCameraINISettings(ini);
	loadGeneralINISettings(ini);
	loadIOCommINISettings(ini);
	loadDBINISettigs(ini);
	return true;
}

void CSCApplication::loadCameraINISettings(QSettings& ini)
{
#if defined(POR_WITH_CAMERA)
	QString tag_port_name;
	anlock_guard(m_camera_param);

	for (i32 i = 0; i < PO_CAM_COUNT; i++)
	{
		tag_port_name = QString("CAMERA/Cam%1").arg(i);
		m_camera_param.m_cam_id[i] = ini.value(tag_port_name, m_camera_param.m_cam_id[i].c_str()).toString().toStdString();
	}
	m_camera_param.m_cam_available = ini.value("CAMERA/Available", kPOCamMindVision).toInt();
#endif
}

void CSCApplication::loadGeneralINISettings(QSettings& ini)
{
	anlock_guard(m_general_param);
	m_general_param.m_language_hl = ini.value("COMMON/Language", m_general_param.m_language_hl).toInt();
	m_general_param.m_history_setting = ini.value("COMMON/LogSetting", m_general_param.m_history_setting).toInt();
	m_general_param.m_decimal_disp = ini.value("COMMON/DecimalDisp", m_general_param.m_decimal_disp).toInt();
	m_general_param.m_debug_level = ini.value("COMMON/DebugLevel", m_general_param.m_debug_level).toInt();
	m_general_param.m_use_last_snap = ini.value("COMMON/UseLastSnap", m_general_param.m_use_last_snap).toBool();
	m_general_param.m_is_enable_remote = ini.value("COMMON/Remotable", m_general_param.m_is_enable_remote).toBool();
	m_general_param.m_is_sync_datetime = ini.value("COMMON/SyncDateTime", m_general_param.m_is_sync_datetime).toBool();
}

void CSCApplication::loadDBINISettigs(QSettings& ini)
{
#if defined(POR_WITH_SQLITE)
	{
		m_db_config.host_name = ini.value("DATABASE/DBFileName", PO_DATABASE_FILENAME).toString().toStdString();
	}
#elif defined(POR_WITH_MYSQL)
	{
		//read MYSQL Database information such as host, user, database, password 
		m_db_config.host_name = ini.value("DATABASE/HostName", m_db_config.host_name.c_str()).toString().toStdString();
		m_db_config.database_name = ini.value("DATABASE/DataBase", m_db_config.database_name.c_str()).toString().toStdString();
		m_db_config.username = ini.value("DATABASE/UserName", m_db_config.username.c_str()).toString().toStdString();
		m_db_config.password = ini.value("DATABASE/Password", m_db_config.password.c_str()).toString().toStdString();
		m_db_config.remote_username = ini.value("DATABASE/RemoteUserName", m_db_config.remote_username.c_str()).toString().toStdString();
		m_db_config.remote_password = ini.value("DATABASE/RemotePassword", m_db_config.remote_password.c_str()).toString().toStdString();
		m_db_config.db_port = ini.value("DATABASE/Port", m_db_config.db_port).toInt();
	}
#endif

	m_db_config.limit_log = ini.value("DATABASE/LimitLog", m_db_config.limit_log).toInt();
	m_db_config.limit_record = ini.value("DATABASE/LimitRecord", m_db_config.limit_record).toInt();
}

void CSCApplication::loadIOCommINISettings(QSettings& ini)
{
	//read I/O-Work information such as port number, bandwidth, parity and something as like that...
	CModbusDev* io_param_ptr = m_io_manager_ptr->getIODevParam();
	CModbusDev* modbus_param_ptr = m_io_manager_ptr->getModbusDevParam();
	CFtpDev* ftp_param0_ptr = m_io_manager_ptr->getFtpDevParam(0);
	CFtpDev* ftp_param1_ptr = m_io_manager_ptr->getFtpDevParam(1);
	COpcDev* opc_param_ptr = m_io_manager_ptr->getOpcDevParam();

	if (io_param_ptr)
	{
		anlock_guard_ptr(io_param_ptr);

		io_param_ptr->m_dev_address	= ini.value("IO/DevAddress", io_param_ptr->m_dev_address).toInt();
		io_param_ptr->m_port_name = ini.value("IO/Port", io_param_ptr->m_port_name.c_str()).toString().toStdString();
		io_param_ptr->m_rs_mode = ini.value("IO/RSMode", io_param_ptr->m_rs_mode).toInt();
		io_param_ptr->m_baud_rate = ini.value("IO/BaudBand", io_param_ptr->m_baud_rate).toInt();
		io_param_ptr->m_data_bits = ini.value("IO/DataBits", io_param_ptr->m_data_bits).toInt();
		io_param_ptr->m_parity = ini.value("IO/Parity", io_param_ptr->m_parity).toInt();
		io_param_ptr->m_stop_bits = ini.value("IO/StopBits", io_param_ptr->m_stop_bits).toInt();
		io_param_ptr->m_flow_control = ini.value("IO/FlowControls", io_param_ptr->m_flow_control).toInt();
	}

	if (modbus_param_ptr)
	{
		anlock_guard_ptr(modbus_param_ptr);

		modbus_param_ptr->m_dev_address = ini.value("MODBUS/DevAddress", modbus_param_ptr->m_dev_address).toInt();
		modbus_param_ptr->m_fmt_output_digits = ini.value("MODBUS/OutputDigits", modbus_param_ptr->m_fmt_output_digits).toInt();
		modbus_param_ptr->m_endian_mode = ini.value("MODBUS/EndianMode", modbus_param_ptr->m_endian_mode).toInt();
		modbus_param_ptr->m_tcp_port = ini.value("MODBUS/TCPPort", modbus_param_ptr->m_tcp_port).toInt();
		modbus_param_ptr->m_udp_port = ini.value("MODBUS/UDPPort", modbus_param_ptr->m_udp_port).toInt();
	
		modbus_param_ptr->m_port_name = ini.value("MODBUS/Port", modbus_param_ptr->m_port_name.c_str()).toString().toStdString();
		modbus_param_ptr->m_rs_mode = ini.value("MODBUS/RSMode", modbus_param_ptr->m_rs_mode).toInt();
		modbus_param_ptr->m_baud_rate = ini.value("MODBUS/BaudBand", modbus_param_ptr->m_baud_rate).toInt();
		modbus_param_ptr->m_data_bits = ini.value("MODBUS/DataBits", modbus_param_ptr->m_data_bits).toInt();
		modbus_param_ptr->m_parity = ini.value("MODBUS/Parity", modbus_param_ptr->m_parity).toInt();
		modbus_param_ptr->m_stop_bits = ini.value("MODBUS/StopBits", modbus_param_ptr->m_stop_bits).toInt();
		modbus_param_ptr->m_flow_control = ini.value("MODBUS/FlowControls", modbus_param_ptr->m_flow_control).toInt();
	}

	if (ftp_param0_ptr)
	{
		anlock_guard_ptr(ftp_param0_ptr);

		ftp_param0_ptr->m_ftp_hostname = ini.value("FTP1/HostName", ftp_param0_ptr->m_ftp_hostname.c_str()).toString().toStdString();
		ftp_param0_ptr->m_ftp_username = ini.value("FTP1/UserName", ftp_param0_ptr->m_ftp_username.c_str()).toString().toStdString();
		ftp_param0_ptr->m_ftp_password = ini.value("FTP1/Password", ftp_param0_ptr->m_ftp_password.c_str()).toString().toStdString();
		ftp_param0_ptr->m_ftp_port = ini.value("FTP1/Port", ftp_param0_ptr->m_ftp_port).toInt();
		ftp_param0_ptr->m_connection_retry = ini.value("FTP1/RetryCount", ftp_param0_ptr->m_connection_retry).toInt();
	}

	if (ftp_param1_ptr)
	{
		anlock_guard_ptr(ftp_param1_ptr);

		ftp_param1_ptr->m_ftp_hostname = ini.value("FTP2/HostName", ftp_param1_ptr->m_ftp_hostname.c_str()).toString().toStdString();
		ftp_param1_ptr->m_ftp_username = ini.value("FTP2/UserName", ftp_param1_ptr->m_ftp_username.c_str()).toString().toStdString();
		ftp_param1_ptr->m_ftp_password = ini.value("FTP2/Password", ftp_param1_ptr->m_ftp_password.c_str()).toString().toStdString();
		ftp_param1_ptr->m_ftp_port = ini.value("FTP2/Port", ftp_param1_ptr->m_ftp_port).toInt();
		ftp_param1_ptr->m_connection_retry = ini.value("FTP2/RetryCount", ftp_param1_ptr->m_connection_retry).toInt();
	}

	if (opc_param_ptr)
	{
		anlock_guard_ptr(opc_param_ptr);

		opc_param_ptr->m_port = ini.value("OPC/Port", opc_param_ptr->m_port).toInt();
		opc_param_ptr->m_interval = ini.value("OPC/Interval", opc_param_ptr->m_interval).toInt();
	}
}

void CSCApplication::setIniValue(QSettings& ini, const QString& strkey, const QVariant& value)
{
	//if (ini.contains(strkey))
	{
		ini.setValue(strkey, value);
	}
}

bool CSCApplication::writeAdminINISettings(potstring& ini_filename, const postring& dev_version)
{
	QString qstr_filename = QString::fromTCharArray(ini_filename.c_str());
	QSettings ini(qstr_filename, QSettings::IniFormat);

	setIniValue(ini, "DEVICE/DeviceID", m_device_info.device_id);
	setIniValue(ini, "DEVICE/DeviceName", m_device_info.device_name.c_str());
	setIniValue(ini, "DEVICE/ModelName", m_device_info.model_name.c_str());
	setIniValue(ini, "DEVICE/Version", dev_version.c_str());
	setIniValue(ini, "DEVICE/UDPPort", m_device_info.comm_port);
	setIniValue(ini, "DEVICE/AutoUpdate", m_device_info.is_auto_update);
	setIniValue(ini, "DEVICE/HLEmbedded", m_device_info.is_hl_embedded);

	writeCameraINISettings(ini);
	writeDBINISettings(ini);
	ini.sync();
	return true;
}

bool CSCApplication::writeDeviceINISettings(potstring& str_filename, i32 dev_mode)
{
	QString qstr_filename = QString::fromTCharArray(str_filename.c_str());
	QSettings ini(qstr_filename, QSettings::IniFormat);

	writeGeneralINISettings(ini);
	writeIOCommINISettings(ini, dev_mode);
	ini.sync();
	return true;
}

void CSCApplication::writeCameraINISettings(QSettings& ini)
{
#if defined(POR_WITH_CAMERA)
	QString tag_port_name;
	anlock_guard(m_camera_param);

	setIniValue(ini, "CAMERA/Available", m_camera_param.m_cam_available);
	for (i32 i = 0; i < PO_CAM_COUNT; i++)
	{
		tag_port_name = QString("CAMERA/Cam%1").arg(i);
		setIniValue(ini, tag_port_name, m_camera_param.m_cam_id[i].c_str());
	}
#endif
}

void CSCApplication::writeDBINISettings(QSettings& ini)
{
#if defined(POR_WITH_SQLITE)
	{
		setIniValue(ini, "DATABASE/DBFileName", m_db_config.host_name.c_str());
	}
#elif defined(POR_WITH_MYSQL)
	{
		setIniValue(ini, "DATABASE/HostName", m_db_config.host_name.c_str());
		setIniValue(ini, "DATABASE/DataBase", m_db_config.database_name.c_str());
		setIniValue(ini, "DATABASE/UserName", m_db_config.username.c_str());
		setIniValue(ini, "DATABASE/Password", m_db_config.password.c_str());
		setIniValue(ini, "DATABASE/RemoteUserName", m_db_config.remote_username.c_str());
		setIniValue(ini, "DATABASE/RemotePassword", m_db_config.remote_password.c_str());
		setIniValue(ini, "DATABASE/Port", m_db_config.db_port);
	}
#endif
	setIniValue(ini, "DATABASE/LimitLog", m_db_config.limit_log);
	setIniValue(ini, "DATABASE/LimitRecord", m_db_config.limit_record);
}

bool CSCApplication::writeGeneralINISettings(potstring& filename)
{
	QString qstr_filename = QString::fromTCharArray(filename.c_str());
	QSettings ini(qstr_filename, QSettings::IniFormat);

	writeGeneralINISettings(ini);
	ini.sync();
	return true;
}

bool CSCApplication::writeIOCommINISettings(potstring& filename, i32 dev_mode)
{
	QString qstr_filename = QString::fromTCharArray(filename.c_str());
	QSettings ini(qstr_filename, QSettings::IniFormat);

	writeIOCommINISettings(ini, dev_mode);
	ini.sync();
	return true;
}

void CSCApplication::writeGeneralINISettings(QSettings& ini)
{
	anlock_guard(m_general_param);
	setIniValue(ini, "COMMON/Language", m_general_param.m_language_hl);
	setIniValue(ini, "COMMON/LogSetting", m_general_param.m_history_setting);
	setIniValue(ini, "COMMON/DecimalDisp", m_general_param.m_decimal_disp);
	setIniValue(ini, "COMMON/DebugLevel", m_general_param.m_debug_level);
	setIniValue(ini, "COMMON/Remotable", m_general_param.m_is_enable_remote);
	setIniValue(ini, "COMMON/SyncDateTime", m_general_param.m_is_sync_datetime);
	setIniValue(ini, "COMMON/UseLastSnap", m_general_param.m_use_last_snap);
}

void CSCApplication::writeIOCommINISettings(QSettings& ini, i32 dev_mode)
{
	//write internal IOParam
	if (CPOBase::bitCheck(dev_mode, kPOIOSerial))
	{
		CModbusDev* io_param_ptr = m_io_manager_ptr->getIODevParam();
		CModbusDev io_dev_param = io_param_ptr->getValue(); //thread_safe

		setIniValue(ini, "IO/DevAddress", io_dev_param.m_dev_address);
		setIniValue(ini, "IO/Port", io_dev_param.m_port_name.c_str());
		setIniValue(ini, "IO/RSMode", io_dev_param.m_rs_mode);
		setIniValue(ini, "IO/BaudBand", io_dev_param.m_baud_rate);
		setIniValue(ini, "IO/DataBits", io_dev_param.m_data_bits);
		setIniValue(ini, "IO/Parity", io_dev_param.m_parity);
		setIniValue(ini, "IO/StopBits", io_dev_param.m_stop_bits);
		setIniValue(ini, "IO/FlowControls", io_dev_param.m_flow_control);
	}

	//write modbus param
	if (CPOBase::bitCheck(dev_mode, kPOIOModbusRS))
	{
		CModbusDev* modbus_param_ptr = m_io_manager_ptr->getModbusDevParam();
		CModbusDev modbus_dev_param = modbus_param_ptr->getValue(); //thread_safe

		setIniValue(ini, "MODBUS/DevAddress", modbus_dev_param.m_dev_address);
		setIniValue(ini, "MODBUS/OutputDigits", modbus_dev_param.m_fmt_output_digits);
		setIniValue(ini, "MODBUS/EndianMode", modbus_dev_param.m_endian_mode);
		setIniValue(ini, "MODBUS/Port", modbus_dev_param.m_port_name.c_str());
		setIniValue(ini, "MODBUS/RSMode", modbus_dev_param.m_rs_mode);
		setIniValue(ini, "MODBUS/BaudBand", modbus_dev_param.m_baud_rate);
		setIniValue(ini, "MODBUS/DataBits", modbus_dev_param.m_data_bits);
		setIniValue(ini, "MODBUS/Parity", modbus_dev_param.m_parity);
		setIniValue(ini, "MODBUS/StopBits", modbus_dev_param.m_stop_bits);
		setIniValue(ini, "MODBUS/FlowControls", modbus_dev_param.m_flow_control);
	}
	
	//write network param
	if (CPOBase::bitCheck(dev_mode, kPOIOModbusTCP))
	{
		CModbusDev* modbus_param_ptr = m_io_manager_ptr->getModbusDevParam();
		CModbusDev modbus_dev_param = modbus_param_ptr->getValue(); //thread_safe

		setIniValue(ini, "MODBUS/OutputDigits", modbus_dev_param.m_fmt_output_digits);
		setIniValue(ini, "MODBUS/EndianMode", modbus_dev_param.m_endian_mode);
		setIniValue(ini, "MODBUS/TCPPort", modbus_dev_param.m_tcp_port);
	}
	if (CPOBase::bitCheck(dev_mode, kPOIOModbusUDP))
	{
		CModbusDev* modbus_param_ptr = m_io_manager_ptr->getModbusDevParam();
		CModbusDev modbus_dev_param = modbus_param_ptr->getValue(); //thread_safe

		setIniValue(ini, "MODBUS/OutputDigits", modbus_dev_param.m_fmt_output_digits);
		setIniValue(ini, "MODBUS/EndianMode", modbus_dev_param.m_endian_mode);
		setIniValue(ini, "MODBUS/UDPPort", modbus_dev_param.m_udp_port);
	}

	//write ftp param
	if (CPOBase::bitCheck(dev_mode, kPOIOFtp))
	{
		CFtpDev* ftp_dev0_ptr = m_io_manager_ptr->getFtpDevParam(0);
		if (ftp_dev0_ptr)
		{
			CFtpDev ftp_dev = ftp_dev0_ptr->getValue(); //thread_safe
			setIniValue(ini, "FTP1/HostName", ftp_dev.m_ftp_hostname.c_str());
			setIniValue(ini, "FTP1/UserName", ftp_dev.m_ftp_username.c_str());
			setIniValue(ini, "FTP1/Password", ftp_dev.m_ftp_password.c_str());
			setIniValue(ini, "FTP1/Port", ftp_dev.m_ftp_port);
			setIniValue(ini, "FTP1/RetryCount", ftp_dev.m_connection_retry);
		}

		CFtpDev* ftp_dev1_ptr = m_io_manager_ptr->getFtpDevParam(1);
		if (ftp_dev1_ptr)
		{
			CFtpDev ftp_dev = ftp_dev1_ptr->getValue(); //thread_safe
			setIniValue(ini, "FTP2/HostName", ftp_dev.m_ftp_hostname.c_str());
			setIniValue(ini, "FTP2/UserName", ftp_dev.m_ftp_username.c_str());
			setIniValue(ini, "FTP2/Password", ftp_dev.m_ftp_password.c_str());
			setIniValue(ini, "FTP2/Port", ftp_dev.m_ftp_port);
			setIniValue(ini, "FTP2/RetryCount", ftp_dev.m_connection_retry);
		}
	}

	//write opc param
	if (CPOBase::bitCheck(dev_mode, kPOIOOpc))
	{
		COpcDev* opc_dev_ptr = m_io_manager_ptr->getOpcDevParam();
		if (opc_dev_ptr)
		{
			COpcDev opc_dev = opc_dev_ptr->getValue(); //thread_safe
			setIniValue(ini, "OPC/Port", opc_dev.m_port);
			setIniValue(ini, "OPC/Interval", opc_dev.m_interval);
		}
	}
}

void CSCApplication::initAllSetting()
{
	m_camera_param.init();
	m_security_param.init();
	m_general_param.init();
	m_runtime_history.init();
	m_engine_param.init();
	m_db_config.init();
	m_app_update.init();
	m_io_manager_ptr->initAllSettings();

	m_app_state = kSCStateRun;
	m_app_mode = kSCModeNone;
}

bool CSCApplication::loadAllSettings(FILE* fp)
{
	//check fileheader
	if (!CPOBase::fileSignRead(fp, PO_SETTING_BEGIN_CODE))
	{
		return false;
	}

	if (!m_device_info.fileRead(fp))
	{
		printlog_lv1("Can't load DeviceInfo in setting.dat");
		return false;
	}
	if (!m_camera_param.fileRead(fp))
	{
		printlog_lv1("Can't load CameraSet in setting.dat");
		return false;
	}
	if (!m_security_param.fileRead(fp))
	{
		printlog_lv1("Can't load SecuritySetting in setting.dat");
		return false;
	}
	if (!m_general_param.fileRead(fp))
	{
		printlog_lv1("Can't load GeneralSetting in setting.dat");
		return false;
	}
	if (!m_runtime_history.fileRead(fp))
	{
		printlog_lv1("Can't load RuntimeHistory in setting.dat");
		return false;
	}
	if (!m_engine_param.fileRead(fp))
	{
		printlog_lv1("Can't load EngineParam in setting.dat");
		return false;
	}
	if (!m_db_config.fileRead(fp))
	{
		printlog_lv1("Can't load DBConfig in setting.dat");
		return false;
	}
	if (!m_io_manager_ptr->loadAllSettings(fp))
	{
		printlog_lv1("Can't load SCIOManager in setting.dat");
		return false;
	}
	
	CPOBase::fileRead(m_app_state, fp);
	CPOBase::fileRead(m_app_mode, fp);
	return CPOBase::fileSignRead(fp, PO_SETTING_END_CODE);
}

bool CSCApplication::writeAllSettings(FILE* fp)
{
	//update
	updateUpTime();

	//write setting 
	CPOBase::fileSignWrite(fp, PO_SETTING_BEGIN_CODE);

	m_device_info.fileWrite(fp);
	m_camera_param.fileWrite(fp);
	m_security_param.fileWrite(fp);
	m_general_param.fileWrite(fp);
	m_runtime_history.fileWrite(fp);
	m_engine_param.fileWrite(fp);
	m_db_config.fileWrite(fp);
	m_io_manager_ptr->writeAllSettings(fp);

	CPOBase::fileWrite(m_app_state, fp);
	CPOBase::fileWrite(m_app_mode, fp);
	CPOBase::fileSignWrite(fp, PO_SETTING_END_CODE);
	printlog_lv1("Write all setting to setting.dat");
	return true;
}

bool CSCApplication::resetAllSettings()
{
	m_camera_param.init();
	m_security_param.init();
	m_general_param.init();
	m_runtime_history.init();
	m_engine_param.init();
	m_db_config.init();
	m_io_manager_ptr->initAllSettings();

	m_app_state = kSCStateRun;
	m_app_mode = kSCModeNone;
	m_app_update.init();

	//write all settings to disk
	g_sc_disk.deleteAllSettings();
	g_sc_disk.writeAllSettings(this);
	return g_sc_disk.writeDeviceINISettings(this, kPOIOAllDevice);
}

f32 CSCApplication::updateUpTime()
{
	exlock_guard(m_time_mutex);

	leave_uptime;
	f32 uptime = get_uptime_sec;
	m_runtime_history.m_uptime = uptime;
	m_runtime_history.m_acc_uptime = m_runtime_history.m_uptime + m_runtime_history.m_bak_acc_uptime;

	return uptime;
}

bool CSCApplication::getHeartBeat(HeartBeat& hb)
{
	if (!CPOBaseApp::getHeartBeat(hb))
	{
		return false;
	}

	hb.duration = updateUpTime();
	return true;
}

CProcessor* CSCApplication::getFreeProcessor()
{
	CProcessor* processor_ptr = NULL;
	CProcessor* unused_processor_ptr = NULL;
	i32 i, count = (i32)m_pu_array.size();
	for (i = 0; i < count; i++)
	{
 		processor_ptr = m_pu_array[i];
 		if (!processor_ptr->isUsedProcessor())
 		{
			if (!unused_processor_ptr)
			{
				unused_processor_ptr = processor_ptr;
			}
			if (processor_ptr->getCamID() < 0)
 			{
 				return processor_ptr;
 			}
 		}
	}
	return unused_processor_ptr;
}

CProcessor* CSCApplication::getUnusedProcessor()
{
	CProcessor* processor_ptr = NULL;
	i32 i, count = (i32)m_pu_array.size();
	for (i = 0; i < count; i++)
	{
		processor_ptr = m_pu_array[i];
		if (!processor_ptr->isUsedProcessor())
		{
			if (processor_ptr->getCamID() >= 0)
			{
				return processor_ptr;
			}
		}
	}
	return NULL;
}

CProcessor* CSCApplication::findProcessor(i32 cam_id)
{
	if (!CPOBase::checkIndex(cam_id, PO_CAM_COUNT))
	{
		return NULL;
	}

	CProcessor* processor_ptr;
	i32 i, count = (i32)m_pu_array.size();
	for (i = 0; i < count; i++)
	{
		processor_ptr = m_pu_array[i];
		if (processor_ptr->getCamID() == cam_id)
		{
			return processor_ptr;
		}
	}
	return NULL;
}
 
CProcessor* CSCApplication::setCameraProcessor(i32 cam_id, bool is_admin, i32 mode)
{
	Packet* pak = NULL;
	singlelog_lv1(QString("Camera%1 is selected, admin is %2").arg(cam_id).arg(is_admin));
 
	//find processor
	CProcessor* processor_ptr = findProcessor(cam_id);
	if (!processor_ptr)
	{
 		//find free processor
 		processor_ptr = getFreeProcessor();
 		if (!processor_ptr)
 		{
 			printlog_lv1("No FreeProcessor in setCameraProcessor.");
 			return NULL;
 		}
 		
 		CameraSetting* cam_setting_ptr = m_camera_param.getCamSetting(cam_id);
 		if (!cam_setting_ptr || !cam_setting_ptr->isUnusedReady())
 		{
 			printlog_lv1(QString("CameraSetting%1 is not exist in setCameraProcessor.").arg(cam_id));
 			return NULL;
 		}
 
 		//set camera setting in current processor
 		pak = po_new Packet(kSCCmdIntSelectCamera, kPOPacketRequest);
 		pak->setReservedi64(0, (i64)cam_setting_ptr);
		pak->setReservedi32(2, mode);
		cam_setting_ptr->setUsed();

		if (sendProcessorPacket(processor_ptr, pak, kPUCmdBlock) != kPOSuccess)
		{
			printlog_lv1(QString("kSCCmdIntSelectCamera[Cam:%1] is busy in setCameraProcessor").arg(cam_id));
			cam_setting_ptr->setUnused();
			return NULL;
		}
	}
 
	//processor activate
	if (is_admin)
	{
 		if (!processor_ptr->isAdminProcessor())
 		{
 			if (m_admin_processor_ptr)
 			{
 				printlog_lv1(QString("Previous AdminProcessor will be remove, pid is %1")
								.arg(m_admin_processor_ptr->getProcessorID()));

 				pak = po_new Packet(kSCCmdIntAdminIdle, kPOPacketRequest);
				if (sendProcessorPacket(m_admin_processor_ptr, pak, kPUCmdBlock) != kPOSuccess)
				{
					printlog_lv1(QString("kSCCmdIntAdminRemove[Cam:%1] is busy").arg(cam_id));
					return NULL;
				}
 			}
			if (!setAdminProcessor(processor_ptr, cam_id))
			{
				return NULL;
			}
 		}
	}
	else
	{
 		if (!processor_ptr->isThreadProcessor())
 		{
			pak = po_new Packet(kSCCmdIntUsed, kPOPacketRequest);
			if (sendProcessorPacket(processor_ptr, pak) != kPOSuccess)
			{
				printlog_lv1(QString("kSCCmdIntUsed[Cam:%1] is busy").arg(cam_id));
				return NULL;
			}
 		}
	}
 
	return processor_ptr;
}

bool CSCApplication::setAdminFirstCameraProcessor(bool is_force_mode)
{
	if (m_admin_processor_ptr)
	{
		return true;
	}
	if (!is_force_mode && !checkAppDesc(kPODescHighLevel))
	{
		return true;
	}

	//현재 사용중인 카메라를 검색하여 Admin카메라로 변경시킨다.
	CProcessor* processor_ptr;
	i32 i, count = (i32)m_pu_array.size();
	for (i = 0; i < count; i++)
	{
		processor_ptr = m_pu_array[i];
		if (processor_ptr->isUsedProcessor())
		{
			if (setAdminProcessor(processor_ptr, processor_ptr->getCamID()))
			{
				return true;
			}
		}
	}
	
	Packet* pak;
	processor_ptr = getUnusedProcessor();
	if (!processor_ptr)
	{
		processor_ptr = getFreeProcessor();
		if (!processor_ptr)
		{
			printlog_lv0("No Admin Processor in AppOpen/DeviceChange");
			return false;
		}
	}

	if (!processor_ptr->hasCameraConnection())
	{
		//사용하지 않는 카메라를 사용하지 않고있는 Processor에 대응시켜 Admin카메라로 기동시킨다.
		CameraSetting* first_cam_setting_ptr = m_camera_param.findFirstCamSetting();
		if (!first_cam_setting_ptr)
		{
			printlog_lv0("No Available Camera in AppOpen/DeviceChange");
			return false;
		}
		first_cam_setting_ptr->setUsed();

		pak = po_new Packet(kSCCmdIntSelectCamera, kPOPacketRequest);
		pak->setReservedi64(0, (i64)first_cam_setting_ptr);
		if (sendProcessorPacket(processor_ptr, pak, kPUCmdBlock) != kPOSuccess)
		{
			first_cam_setting_ptr->setUnused();
			return false;
		}
	}

	//카메라가 할당된 Proceesor에 대하여
	m_admin_processor_ptr = processor_ptr;
	pak = po_new Packet(kSCCmdIntAdminUsed, kPOPacketRequest);
	if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
	{
		m_admin_processor_ptr = NULL;
		return false;
	}
	return false;
}

bool CSCApplication::setAdminProcessor(CProcessor* processor_ptr, i32 cam_id)
{
	if (!processor_ptr)
	{
		return false;
	}

	m_admin_processor_ptr = processor_ptr;
	Packet* pak = po_new Packet(kSCCmdIntAdminUsed, kPOPacketRequest);
	if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
	{
		m_admin_processor_ptr = NULL;
		printlog_lv1(QString("kSCCmdIntAdminUsed[Cam:%1] is busy").arg(cam_id));
		return false;
	}

	i32 pid = processor_ptr->getProcessorID();
	printlog_lv1(QString("New AdminProcessor is activate, pid %1, cid %2").arg(pid).arg(cam_id));
	return true;
}

bool CSCApplication::freeCameraProcessor(i32 cam_id, i32 cmd)
{
	CProcessor* processor_ptr = findProcessor(cam_id);
	if (processor_ptr)
	{
		i32 mode;
		bool is_admin = processor_ptr->isAdminProcessor();

		//preprocessing
		switch (cmd)
		{
			case kSCCmdIntDisconnect:
			{
				mode = kPUCmdBlock;
				break;
			}
			default:
			{
				mode = kPUCmdNonBlock;
				break;
			}
		}

		//send free camera processor command
		Packet* pak = po_new Packet(cmd, kPOPacketRequest);
		if (sendProcessorPacket(processor_ptr, pak, mode) != kPOSuccess)
		{
			printlog_lv1(QString("freeCamProcessor[Cam:%1] is busy").arg(cam_id));
			return false;
		}

		//after processing
		switch (cmd)
		{
			case kSCCmdIntDisconnect:
			{
				m_admin_processor_ptr = NULL;
				break;
			}
		}
	}
	else
	{
		printlog_lv1(QString("Specified Processor is not exist in app, cam %1").arg(cam_id));
	}
	return true;
}

i32 CSCApplication::sendProcessorPacket(CProcessor* processor_ptr, Packet* pak, i32 mode)
{
	if (processor_ptr && pak)
	{
		//send event packet to specified processor
		if (mode == kPUCmdBlock)
		{
			processor_ptr->lock();
			QMetaObject::invokeMethod(processor_ptr, "onReadPacket", Qt::AutoConnection,
							Q_ARG(Packet*, pak), Q_ARG(i32, mode), Q_ARG(i32, kPOConnAdmin));
			processor_ptr->waitUnlock();
			return processor_ptr->getResultCode();
		}
		else
		{
			i32 puid = processor_ptr->getProcessorID();
			if (!incProcessorQueue(puid))
			{
				POSAFE_DELETE(pak);
				return kPOErrBusy;
			}
			QMetaObject::invokeMethod(processor_ptr, "onReadPacket", Qt::QueuedConnection,
							Q_ARG(Packet*, pak), Q_ARG(i32, mode), Q_ARG(i32, kPOConnAdmin));
			return kPOSuccess;
		}
	}
	POSAFE_DELETE(pak);
	return kPOErrInvalidOper;
}

void CSCApplication::onAcceptExtCamera(i32 method, i32 dev_mode, i32 cam_id)
{
	//find processor unit for current camera info
	CProcessor* processor_ptr = findProcessor(cam_id);
	if (!processor_ptr)
	{
		printlog_lv1(QString("The empty processor for selected camera is not exist, cam %1").arg(cam_id));
		return;
	}

	//send external trigger signal to processor
	Packet* pak = NULL;
	switch (method)
	{
		case kIOInTrigger:
		{
			pak = po_new Packet(kSCCmdIntExtTrigger, kPOPacketRequest);
			pak->setReservedi32(0, dev_mode);
			break;
		}
		case kIOInJobTeach:
		{
			pak = po_new Packet(kSCCmdIntExtTeaching, kPOPacketRequest);
			break;
		}
	}
	
	if (pak)
	{
		if (sendProcessorPacket(processor_ptr, pak) != kPOSuccess)
		{
			printlog_lv1(QString("kIOInTrigger/kIOInJobTeach[%1] is busy").arg(method));
			return;
		}
	}
}

void CSCApplication::onAcceptExtProgram(i32 method, i32 dev, i32 program)
{
	//find model with program id
	CJobUnit* job_ptr = NULL;
	switch (dev)
	{
		case kPOIOSerial:
		{
			job_ptr = m_job_manager_ptr->findJob(m_io_manager_ptr->getProgram(program));
			break;
		}
		case kPOIOModbusRS:
		case kPOIOModbusTCP:
		case kPOIOModbusUDP:
		{
			job_ptr = m_job_manager_ptr->findJobByExternalID(program);
			break;
		}
	}

	if (!job_ptr)
	{
		printlog_lv1(QString("Selected Program is not exist, program %1").arg(program));
		return;
	}

	switch (method)
	{
		case kIOInJobSelect:
		{
			CProcessor* processor_ptr = setCameraProcessor(job_ptr->getCamID(), false, kNonHLCommand);
			if (processor_ptr)
			{
				//send select model internal signal
				Packet* pak = po_new Packet();

				if (processor_ptr->isAdminProcessor())
				{
					pak->setCmd(kSCCmdIntSelectJob);

					//set sender device information in reserved u5
					pak->setReservedu8(4, (u8)kRespJobNone);
					pak->setReservedu8(5, (u8)kOperationbyController);
				}
				else
				{
					pak->setCmd(kSCCmdIntSelectProgram);
				}

				pak->setReservedi32(0, job_ptr->getJobID());
				if (sendProcessorPacket(processor_ptr, pak) != kPOSuccess)
				{
					printlog_lv1("kIOInJobSelect is busy.");
					return;
				}
			}
			else
			{
				printlog_lv1("kIOInJobSelect is error. empty processor nothing.");
			}
			break;
		}
		case kIOInJobDeselect:
		{
			freeCameraProcessor(job_ptr->getCamID(), kSCCmdIntIdle);
			break;
		}
	}
}

void CSCApplication::onAcceptExtSignal(i32 dev, i32 method, i32 param1, i32 param2)
{
	checkQtSignals(SC_SIGNAL_EXTCONTROL, false);

 	switch (method)
 	{
		case kIOInTrigger:
		case kIOInJobTeach:
		{
			//shift 8bit, becuase cam_id is high byte of param
			i32 cam_id = param1 >> 8;
			printlog_lvs2(QString("AcceptExtSignal CamOperation. method:%1, dev:%2, cam:%3")
							.arg(method).arg(dev).arg(param1), LOG_SCOPE_APP);

			onAcceptExtCamera(method, dev, cam_id);
			break;
		}
 		case kIOInJobSelect:
		case kIOInJobDeselect:
		{
			printlog_lvs2(QString("AcceptExtSignal JobOperation. method %1, dev %2, id %3 ")
							.arg(method).arg(dev).arg(param1), LOG_SCOPE_APP);

			onAcceptExtProgram(method, dev, param1);
			break;
		}
 		case kIOInClearError:
		{
			break;
		}
		case kIOInRunStop:
		{
			//shift 8bit, becuase cam_id is high byte of param
			i32 cam_id = param1 >> 8;
			bool is_run_mode = (param1 & 0xFF) != 0;
			printlog_lvs2(QString("AcceptExtSignal Run/Stop camera. dev:%1, cam_id:%2, run_mode:%3")
							.arg(dev).arg(cam_id).arg(is_run_mode), LOG_SCOPE_APP);

			onAcceptExtRunStopThread(cam_id, is_run_mode);
			break;
		}
		case kIOInRunStopAll:
		{
			bool is_run_mode = (param1 & 0xFF) != 0;
			printlog_lvs2(QString("AcceptExtSignal Run/Stop all camera. dev:%1, run_mode:%2")
							.arg(dev).arg(is_run_mode), LOG_SCOPE_APP);

			onAcceptExtRunStopThreadAll(is_run_mode);
			break;
		}
		case kIOInResetCounter:
		{
			//shift 8bit, becuase cam_id is high byte of param
			i32 cam_id = param1 >> 8;
			printlog_lvs2(QString("Runtime history one camera. dev:%1, cam_id:%2").arg(dev).arg(cam_id), LOG_SCOPE_APP);

			updateRuntimeHistory(kRuntimeHistoryClearCamera, (void*)&cam_id);
			uploadRuntimeHistory(kSCCmdRuntimeSync);
			break;
		}
		case kIOInResetAllCounter:
		{
			printlog_lvs2(QString("Runtime history all camera. dev:%1").arg(dev), LOG_SCOPE_APP);

			updateRuntimeHistory(kRuntimeHistoryClearAll, NULL);
			uploadRuntimeHistory(kSCCmdRuntimeSync);
			break;
		}
		case kIOInDebugLevel:
		{
			printlog_lvs2(QString("debug level change to %1").arg(param1), LOG_SCOPE_APP);

			if (m_general_param.getDebugLevel() != param1)
			{
				m_general_param.setDebugLevel(param1);
				g_sc_disk.writeDeviceINISettings(this, 0);
				set_loglevel(param1);
			}
			break;
		}
 	}
}

void CSCApplication::onAcceptDevExtSel()
{
	if (!checkAppDesc(kPODescHighLevel))
	{
		return;
	}

	u8* buffer_ptr;
	i32 len = m_io_manager_ptr->memExtSelSize();
	Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
	{
		pak->setSubCmd(kSCSubTypeUpdatedExtSel);
		pak->allocateBuffer(len, buffer_ptr);
		m_io_manager_ptr->memExtSelWrite(buffer_ptr, len);
	}

	printlog_lv1("Send Device ExtSelection Data");
	sendPacketToNet(pak, buffer_ptr);
}

void CSCApplication::onAcceptDevIOStatus(i32 sub_cmd, i32 dev_mode, u8* data_ptr, u8 count)
{
	checkQtSignals(SC_SIGNAL_RAWIOSYNC, false);
	if (!checkAppDesc(kPODescHighLevel) || !data_ptr || count <= 0)
	{
		return;
	}

	u8* buffer_ptr;
	i32 len = sizeof(dev_mode) + sizeof(count) + count;
	Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
	{
		i32 buffer_size = len;
		pak->setSubCmd(sub_cmd);
		pak->allocateBuffer(len, buffer_ptr);

		CPOBase::memWrite(dev_mode, buffer_ptr, buffer_size);
		CPOBase::memWrite(count, buffer_ptr, buffer_size);
		CPOBase::memWrite(data_ptr, count, buffer_ptr, buffer_size);
	}

	printlog_lvs3(QString("Send RawIOStatus. cmd:%1, sender:%2").arg(sub_cmd).arg(dev_mode), LOG_SCOPE_APP);
	sendPacketToNet(pak, buffer_ptr);
}

void CSCApplication::onDeviceError(i32 subdev, i32 index, i32 errcode)
{
	checkQtSignals(SC_SIGNAL_DEVERRSYNC, false);
	if (!checkAppDesc(kPODescHighLevel))
	{
		return;
	}

	Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
	{
		pak->setSubCmd(kSCSubTypeDevError);
		pak->setReservedi32(0, subdev);	//POSubDevType
		pak->setReservedi32(1, index);
		pak->setReservedi32(2, errcode);
	}
	sendPacketToNet(pak);
	printlog_lv1(QString("SCDevice[%1] error[%2] is rised.").arg(subdev).arg(errcode));
}

void CSCApplication::onDevicePluged(i32 subdev, i32 index)
{
	checkQtSignals(SC_SIGNAL_DEVMANAGER, false);
	printlog_lv1(QString("SCDevice[%1] index[%2] is pluged.").arg(subdev).arg(index));

	switch (subdev)
	{
		case kPOSubDevCamera:
		{
			setAdminFirstCameraProcessor();
			break;
		}
	}

	if (checkAppDesc(kPODescHighLevel))
	{
		Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
		{
			pak->setSubCmd(kSCSubTypeDevPlugged);
			pak->setReservedi32(0, subdev);
			pak->setReservedi32(1, index);
		}
		sendPacketToNet(pak);
	}
}

void CSCApplication::onDeviceUnPluged(i32 subdev, i32 index)
{
	checkQtSignals(SC_SIGNAL_DEVMANAGER, false);
	printlog_lv1(QString("SCDevice[%1] index[%2] is unpluged.").arg(subdev).arg(index));

	switch (subdev)
	{
		case kPOSubDevCamera:
		{
			CProcessor* processor_ptr = findProcessor(index);
			if (processor_ptr)
			{
				bool is_admin = processor_ptr->isAdminProcessor();
				freeCameraProcessor(index, kSCCmdIntDisconnect);
				if (!setAdminFirstCameraProcessor())
				{
					uploadNoneCameraState();
					printlog_lv1(QString("No connected camera after plugout camera%1").arg(index));
				}
			}
			break;
		}
	}

	if (checkAppDesc(kPODescHighLevel))
	{
		Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
		{
			pak->setSubCmd(kSCSubTypeDevUnPlugged);
			pak->setReservedi32(0, subdev);
			pak->setReservedi32(1, index);
		}
		sendPacketToNet(pak);
	}
}

void CSCApplication::onChangeDebugLevelRing0()
{
	set_loglevel(LOG_LV0);
	updateDebugLevelGUI(LOG_LV0);
}

void CSCApplication::onChangeDebugLevelApp()
{
	set_loglevel(LOG_LV1);
	updateDebugLevelGUI(LOG_LV1);
}

void CSCApplication::onChangeDebugLevelModule()
{
	set_loglevel(LOG_LV2);
	updateDebugLevelGUI(LOG_LV2);
}

void CSCApplication::onChangeDebugLevelEventflow()
{
	set_loglevel(LOG_LV3);
	updateDebugLevelGUI(LOG_LV3);
}

void CSCApplication::onChangeDebugLevelRuntime()
{
	set_loglevel(LOG_LV4);
	updateDebugLevelGUI(LOG_LV4);
}

void CSCApplication::onChangeDebugScopeApp()
{
	if (chk_logscope(LOG_SCOPE_APP))
	{
		remove_logscope(LOG_SCOPE_APP);
	}
	else
	{
		add_logscope(LOG_SCOPE_APP);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeEvent()
{
	if (chk_logscope(LOG_SCOPE_APP))
	{
		remove_logscope(LOG_SCOPE_APP);
	}
	else
	{
		add_logscope(LOG_SCOPE_APP);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeNetwork()
{
	if (chk_logscope(LOG_SCOPE_NET))
	{
		remove_logscope(LOG_SCOPE_NET); 
	}
	else
	{
		add_logscope(LOG_SCOPE_NET);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeIVS()
{
	if (chk_logscope(LOG_SCOPE_IVS))
	{
		remove_logscope(LOG_SCOPE_IVS);
	}
	else
	{
		add_logscope(LOG_SCOPE_IVS);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeCamera()
{
	if (chk_logscope(LOG_SCOPE_CAM))
	{
		remove_logscope(LOG_SCOPE_CAM);
	}
	else
	{
		add_logscope(LOG_SCOPE_CAM);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeEncode()
{
	if (chk_logscope(LOG_SCOPE_ENCODE))
	{
		remove_logscope(LOG_SCOPE_ENCODE);
	}
	else
	{
		add_logscope(LOG_SCOPE_ENCODE);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeIOModule()
{
	if (chk_logscope(LOG_SCOPE_IO))
	{
		remove_logscope(LOG_SCOPE_IO);
	}
	else
	{
		add_logscope(LOG_SCOPE_IO);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeModbus()
{
	if (chk_logscope(LOG_SCOPE_COMM))
	{
		remove_logscope(LOG_SCOPE_COMM);
	}
	else
	{
		add_logscope(LOG_SCOPE_COMM);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeOpc()
{
	if (chk_logscope(LOG_SCOPE_OPC))
	{
		remove_logscope(LOG_SCOPE_OPC);
	}
	else
	{
		add_logscope(LOG_SCOPE_OPC);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeOvx()
{
	if (chk_logscope(LOG_SCOPE_OVX))
	{
		remove_logscope(LOG_SCOPE_OVX);
	}
	else
	{
		add_logscope(LOG_SCOPE_OVX);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeDatabase()
{
	if (chk_logscope(LOG_SCOPE_DB))
	{
		remove_logscope(LOG_SCOPE_DB);
	}
	else
	{
		add_logscope(LOG_SCOPE_DB);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeFtp()
{
	if (chk_logscope(LOG_SCOPE_FTP))
	{
		remove_logscope(LOG_SCOPE_FTP);
	}
	else
	{
		add_logscope(LOG_SCOPE_FTP);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeTag()
{
	if (chk_logscope(LOG_SCOPE_TAG))
	{
		remove_logscope(LOG_SCOPE_TAG);
	}
	else
	{
		add_logscope(LOG_SCOPE_TAG);
	}
	updateDebugScopeGUI(get_logscope());
}

void CSCApplication::onChangeDebugScopeAll()
{
	set_logscope(LOG_SCOPE_ALL);
	updateDebugScopeGUI(LOG_SCOPE_ALL);
}

void CSCApplication::onChangeDebugScopeClear()
{
	set_logscope(LOG_SCOPE_NONE);
	updateDebugScopeGUI(LOG_SCOPE_NONE);
}

void CSCApplication::onReadCmdPacket(Packet* packet_ptr, i32 conn_mode)
{
	if (!packet_ptr)
	{
		return;
	}

	i32 nid = packet_ptr->getID();
	i32 cmd = packet_ptr->getCmd();
	i32 code = kPOErrCmdFail;

	switch (cmd)
	{
		case kPOCmdAuth:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdAuth is");
				code = onRequestPermissionAuth(packet_ptr);
			}
			break;
		}
		case kPOCmdChangeIP:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdChangeIP is");
				code = onRequestChangeIP(packet_ptr);
			}
			break;
		}
		case kPOCmdChangePwd:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdChangePwd is");
				code = onRequestChangePassword(packet_ptr);
			}
			break;
		}

		case kPOCmdSync:
		{
			singlelog_lv1("kPOCmdSync is");
			code = onRequestSync(packet_ptr);
			break;
		}
		case kPOCmdUnSync:
		{
			singlelog_lv1("kPOCmdUnSync is");
			code = onRequestUnSync(packet_ptr);
			break;
		}
		case kPOCmdWake:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdWake is");
				code = onRequestOnline(packet_ptr);
			}
			break;
		}
		case kPOCmdSleep:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdSleep is");
				code = onRequestOffline(packet_ptr);
			}
			break;
		}
		case kPOCmdDevImport:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdDevImport is");
				code = onRequestSettingImport(packet_ptr);
			}
			break;
		}
		case kPOCmdDevExport:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdDevExport is");
				code = onRequestSettingExport(packet_ptr);
			}
			break;
		}
		case kPOCmdDevUpdate:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdDevUpdate is");
				code = onRequestDeviceUpdate(packet_ptr);
			}
			break;
		}
		case kPOCmdDevFactoryReset:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kPOCmdDevFactoryReset is");
				code = onRequestResetFactory(packet_ptr);
			}
			break;
		}
		case kPOCmdEmulator:
		{
#if defined(POR_EMULATOR)
			if (conn_mode >= kPOConnAdmin && checkAppDesc(kPODescEmulator))
			{
				singlelog_lv1("kSCCmdEmulator is");
				code = onRequestEmulator(packet_ptr);
			}
#endif
			break;
		}

		case kSCCmdGetRumtimeHistory:
		{
			singlelog_lv1("kSCCmdGetRumtimeHistory is");
			code = onRequestGetRuntimeHistory(packet_ptr);
			break;
		}
		case kSCCmdResetRuntimeHistory:
		{
			singlelog_lv1("kSCCmdResetRuntimeHistory is");
			code = onRequestResetRuntimeHistory(packet_ptr);
			break;
		}
		case kSCCmdSetting:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kSCCmdSetting is");
				code = onRequestSetting(packet_ptr);
			}
			break;
		}
		case kSCCmdDevSetting:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kSCCmdDevSetting is");
				code = onRequestDevSetting(packet_ptr);
			}
			break;
		}
		case kSCCmdIOSetting:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kSCCmdIOSetting is");
				code = onRequestIOSetting(packet_ptr);
			}
			break;
		}
		case kSCCmdCommSetting:
		{
			if (conn_mode >= kPOConnAdmin)
			{
				singlelog_lv1("kSCCmdCommSetting is");
				code = onRequestCommSetting(packet_ptr);
			}
			break;
		}
		case kSCCmdIORawCmd:
		{
			singlelog_lv1("kSCCmdIORawCmd is");
			code = onRequestIORawCommand(packet_ptr);
			break;
		}
		case kSCCmdSelectCamera:
		{
			singlelog_lv1("kSCCmdSelectCamera is");
			code = onRequestCameraSelect(packet_ptr);
			break;
		}
		case kSCCmdSelectJob:
		{
			singlelog_lv1("kSCCmdSelectJob is");
			code = onRequestSelectJob(packet_ptr);
			break;
		}
		case kSCCmdDBOperation:
		{
			singlelog_lv1("kSCCmdDBOperation is");
			code = onRequestDBOperation(packet_ptr);
			break;
		}
		default:
		{
			//send packet to admin processor
			if (m_admin_processor_ptr)
			{
				i32 puid = m_admin_processor_ptr->getProcessorID();

				if (incProcessorQueue(puid))
				{
					printlog_lv1(QString("Send packet to processor, cmd:%1, puid:%2").arg(cmd).arg(puid));
					QMetaObject::invokeMethod(m_admin_processor_ptr, "onReadPacket", Qt::QueuedConnection,
									Q_ARG(Packet*, packet_ptr), Q_ARG(i32, kPUCmdNonBlock), Q_ARG(i32, conn_mode));
					return;
				}
				else
				{
					printlog_lv1(QString("Admin Command Skip, BusyNow, cmd:%1, puid:%2").arg(cmd).arg(puid));
				}
				code = kPOErrBusy;
			}
		}
	}

	if (code != kPOSuccess)
	{
		uploadFailPacket(packet_ptr, code);
		printlog_lv1(QString("[Command Fail********************] cmd%1, code%2").arg(cmd).arg(code));
	}
	POSAFE_DELETE(packet_ptr);
}

i32 CSCApplication::onRequestSync(Packet* packet_ptr)
{
	if (packet_ptr == NULL || checkAppDesc(kPODescHighLevel))
	{
		return kPOErrCmdFail;
	}

	addAppDesc(kPODescHighLevel);
	updateHLParam(packet_ptr);
	updateSyncDateTime(packet_ptr);
	updateIPInfo();

	Packet* pak;
	i32 len = 0;
	i32 netid = packet_ptr->getID();
	postring dev_version = m_device_info.getDeviceVersion();

	//기본설정동기화
	len += CPOBase::getStringMemSize(dev_version);
	len += m_device_info.memSize();
	len += m_camera_param.memSize();
	len += m_camera_param.memTagInfoSize();
	len += m_security_param.memSize();
	len += m_general_param.memSize();
	len += m_db_config.memSize();
	len += m_tool_categorys_ptr->memSize();
	len += m_io_manager_ptr->memEnvSize();
	len += m_io_manager_ptr->memSize();
	len += m_tag_ui_manager_ptr->memSize();
	
	if (len > 0)
	{
		u8* buffer_ptr;
		i32 buffer_size = len;
		pak = po_new Packet(kPOCmdSync, kPOPacketRespOK, false, netid);
		pak->setSubCmd(kSCSubTypeSyncSetting);
		pak->allocateBuffer(len, buffer_ptr);

		CPOBase::memWrite(buffer_ptr, buffer_size, dev_version);
		m_device_info.memWrite(buffer_ptr, buffer_size);
		m_camera_param.memWrite(buffer_ptr, buffer_size);
		m_camera_param.memTagInfoWrite(buffer_ptr, buffer_size);
		m_security_param.memWrite(buffer_ptr, buffer_size);
		m_general_param.memWrite(buffer_ptr, buffer_size);
		m_db_config.memWrite(buffer_ptr, buffer_size);
		m_tool_categorys_ptr->memWrite(buffer_ptr, buffer_size);
		m_io_manager_ptr->memEnvWrite(buffer_ptr, buffer_size);
		m_io_manager_ptr->memWrite(buffer_ptr, buffer_size);
		m_tag_ui_manager_ptr->memWrite(buffer_ptr, buffer_size);
		
		sendPacketToNet(pak, buffer_ptr);
	}

	//실시간리력자료의 동기화
	u8* buffer_ptr = NULL;
	pak = po_new Packet(kPOCmdSync, kPOPacketRespOK, false, netid);
	pak->setSubCmd(kSCSubTypeSyncRTHistory);

	len = m_runtime_history.memSize();
	if (len > 0)
	{
		i32 buffer_size = len;
		pak->allocateBuffer(len, buffer_ptr);
		m_runtime_history.memWrite(buffer_ptr, buffer_size);
	}
	sendPacketToNet(pak, buffer_ptr);

	//공정자료기지및 공정결과메타자료에 대한 동기화
	CJobResult tmpl_job_result;
	buffer_ptr = NULL;
	pak = po_new Packet(kPOCmdSync, kPOPacketRespOK, true, netid);
	pak->setSubCmd(kSCSubTypeSyncJobInfo);

	len = m_job_manager_ptr->memJobsInfoSize();
	len += tmpl_job_result.memTagInfoSize();
	if (len > 0)
	{
		i32 buffer_size = len;
		pak->allocateBuffer(len, buffer_ptr);
		m_job_manager_ptr->memJobsInfoWrite(buffer_ptr, buffer_size);
		tmpl_job_result.memTagInfoWrite(buffer_ptr, buffer_size);
	}
	sendPacketToNet(pak, buffer_ptr);

	//카메라선택통보전송
	setAdminFirstCameraProcessor();

	pak = po_new Packet(kSCCmdIntAdminSync, kPOPacketRequest);
	if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
	{
		printlog_lv2("SyncCmd's admin sync busy/failed.");
		uploadFailPacket(kSCCmdDevState, kPOErrCmdFail, kSCSubTypeAdminCameraSync);
		return kPOSuccess;
	}

	//update
	updateStateNetOpen();
	updateTrayIcon(kPOTrayIconOnline);
	m_ivs_server.updateState(kIVSStateBusy, m_ip_info.getHighID());
	return kPOSuccess;
}

i32 CSCApplication::onRequestUnSync(Packet* packet_ptr)
{
	if (checkAppDesc(kPODescHighLevel))
	{
		removeAppDesc(kPODescHighLevel);
		uploadEndPacket(packet_ptr);

		updateStateNetClose();
		m_job_manager_ptr->writeJobToFile(); //if user changed current model, it's need

		i32 state = packet_ptr ? kIVSStateBusy : kIVSStateNetFree;
		if (isDeviceOffline())
		{
			state = kIVSStateNone;
		}
		m_ivs_server.updateState(state);
	}
	return kPOSuccess;
}

i32 CSCApplication::onRequestOnline(Packet* packet_ptr)
{
	if (!isDeviceOffline())
	{
		return kSCErrIsNotOffline;
	}

	initInstance();
	updateTrayIcon(kPOTrayIconOnline);
	m_ivs_server.updateState(kIVSStateNetFree);
	return kPOSuccess;
}

i32 CSCApplication::onRequestOffline(Packet* packet_ptr)
{
	if (isDeviceOffline())
	{
		return kSCErrIsNotOffline;
	}

	exitInstance();
	updateTrayIcon(kPOTrayIconOffline);
	m_ivs_server.updateState(kIVSStateNone);
	return kPOSuccess;
}

i32 CSCApplication::onRequestStop(Packet* packet_ptr)
{
	stopProcessorAll();
	return kPOSuccess;
}

i32 CSCApplication::onRequestPermissionAuth(Packet* packet_ptr)
{
	if (!packet_ptr || !checkAppDesc(kPODescHighLevel))
	{
		return kPOErrInvalidOper;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidPacket;
	}

	postring password;
	CPOBase::memRead(buffer_ptr, buffer_size, password);
	if (!checkPassword(password))
	{
		printlog_lv0("Invalid authentication password");
		return kPOErrInvalidPassword;
	}

	checkPacket(packet_ptr, buffer_ptr);
	uploadEndPacket(packet_ptr);
	return kPOSuccess;
}

bool CSCApplication::isAppInited()
{
	return m_app_inited;
}

bool CSCApplication::isLocalConnection()
{
	if (!checkAppDesc(kPODescHighLevel))
	{
		return false;
	}
	return m_cmd_server.isLocalConnection();
}

bool CSCApplication::isAvailableConnect(i32 conn, i32 conn_mode)
{
	bool is_remote = m_general_param.isRemoteConntable();
	return m_cmd_server.isAvailableConnect(conn, conn_mode, is_remote);
}

i32 CSCApplication::onRequestPreConnect(Packet* packet_ptr, i32 conn)
{
	if (!packet_ptr)
	{
		printlog_lv1("Preconnect Packet is invalid.");
		return kPOErrInvalidConnect;
	}

	i32 conn_mode = packet_ptr->getReservedu8(0);
	if (!CPOBase::checkIndex(conn_mode, kPOConnNone, kPOConnCount))
	{
		printlog_lv1(QString("Preconnect Packet is invalid. connection mode is not valid [%1]").arg(conn_mode));
		return kPOErrInvalidConnect;
	}

	Packet* pak = po_new Packet(kPOCmdPreConnect, kPOPacketRespOK);
	pak->setSubCmd(kPOSubTypeNone);
	pak->setReservedb8(0, m_security_param.getPermission(kPermissionNetConnect));
	pak->setReservedb8(1, isAvailableConnect(conn, conn_mode));
	m_cmd_server.sendPacket(conn, pak);

	printlog_lv0(QString("Preconnect:%1 is OK").arg(conn));
	return kPOSuccess;
}

i32 CSCApplication::onRequestConnect(Packet* packet_ptr, i32 conn)
{
	if (!packet_ptr)
	{
		printlog_lv1("Connect Packet is invalid.");
		return kPOErrInvalidOper;
	}

	i32 conn_mode = packet_ptr->getReservedu8(0);
	if (!CPOBase::checkIndex(conn_mode, kPOConnNone, kPOConnCount))
	{
		printlog_lv1(QString("Connect Packet is invalid. connection mode is not valid [%1]").arg(conn_mode));
		return kPOErrInvalidConnect;
	}

	if (!isAvailableConnect(conn, conn_mode))
	{
		printlog_lv1(QString("The remote user can't use current device. remote id is %1, mode %2").arg(conn).arg(conn_mode));
		return kPOErrInvalidOper;
	}

	//read password for connection
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		printlog_lv1("Connect: packet's buffer is invalid");
		return kPOErrInvalidPacket;
	}

	postring password_str;
	CPOBase::memRead(buffer_ptr, buffer_size, password_str);
	checkPacket(packet_ptr, buffer_ptr);

	if (m_security_param.getPermission(kPermissionNetConnect))
	{
		if (!checkPassword(password_str))
		{
			printlog_lv1("The user was used invalid password for connect device.");
			return kPOErrInvalidPassword;
		}
	}

	printlog_lv0(QString("Connect:%1 is OK").arg(conn));
	return kPOSuccess;
}

i32 CSCApplication::onRequestChangeIP(Packet* packet_ptr)
{
	if (!packet_ptr || !checkAppDesc(kPODescNetConnected))
	{
		return kPOErrInvalidOper;
	}

	i32 ip = packet_ptr->getReservedi32(0);
	i32 subnet = packet_ptr->getReservedi32(1);
	i32 ipconfType = packet_ptr->getReservedi32(2);

	uploadEndPacket(packet_ptr);
	return kPOSuccess;
}

i32 CSCApplication::onRequestChangePassword(Packet* packet_ptr)
{
	if (!packet_ptr || !checkAppDesc(kPODescHighLevel))
	{
		return kPOErrInvalidOper;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidOper;
	}

	postring cur_password;
	postring new_password;
	CPOBase::memRead(buffer_ptr, buffer_size, cur_password);
	CPOBase::memRead(buffer_ptr, buffer_size, new_password);
	checkPacket(packet_ptr, buffer_ptr);

	if (setPassword(cur_password, new_password))
	{
		uploadEndPacket(packet_ptr);
		return kPOSuccess;
	}
	return kPOErrInvalidPassword;
}

i32 CSCApplication::onRequestSettingImport(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidPacket;
	}

	if (!deviceImport(packet_ptr))
	{
		printlog_lv1(QString("The device can't import. cmd:%1").arg(packet_ptr->getSubCmd()));
		return kPOErrInvalidOper;
	}

	uploadEndPacket(packet_ptr);
	return kPOSuccess;
}

i32 CSCApplication::onRequestSettingExport(Packet* packet_ptr)
{
	PacketPVector pak_vec;
	deviceExport(pak_vec);

	i32 i, count = (i32)pak_vec.size();
	if (!CPOBase::isPositive(count))
	{
		printlog_lv1("The device is failed to export.");
		return kPOErrInvalidOper;
	}

	//set last packet
	Packet* pak = pak_vec[count - 1];
	if (pak)
	{
		pak->setLastPacket(true);
	}

	for (i = 0; i < count; i++)
	{
		pak = pak_vec[i];
		if (!pak)
		{
			return kPOErrInvalidOper;
		}

		pak_vec[i]->setCmd(kPOCmdDevExport);
		pak_vec[i]->setSubCmd(kPOSubTypeDevExportData);
		sendPacketToNet(pak_vec[i]);
	}
	return kPOSuccess;
}

i32 CSCApplication::onRequestResetFactory(Packet* packet_ptr)
{
	uploadEndPacket(packet_ptr);
	QThread::msleep(1000);

	disconnectProcessorAll();
	
	m_runtime_history.init();
	m_job_manager_ptr->deleteAllJobs();
	m_db_manager_ptr->deleteAllHistory();
	resetAllSettings();
	onRequestOffline(NULL);

	initSettings();
	onRequestOnline(NULL);
	return kPOSuccess;
}

i32 CSCApplication::onRequestDeviceUpdate(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidPacket;
	}
	
	i32 code = kPOErrInvalidOper;
	i32 sub_cmd = packet_ptr->getSubCmd();
	i32 conn = m_cmd_server.getConnection();

	switch (sub_cmd)
	{
		case kPOSubTypeAppUpdate:
		{
			u8* buffer_ptr = packet_ptr->getData();
			i32 buffer_size = packet_ptr->getDataLen();
			if (!buffer_ptr || buffer_size <= 0)
			{
				return kPOErrInvalidPacket;
			}

			i32 pid = packet_ptr->getReservedi32(0);
			i32 packet_count = packet_ptr->getReservedi32(1);
			code = deviceUpdateStream(conn, pid, buffer_ptr, buffer_size);

			if (code == kPOSuccess)
			{
				if (checkUpdateReady(conn))
				{
					Packet* pak = Packet::makeRespPacket(packet_ptr);
					pak->setReservedi32(1, packet_count);
					pak->setReservedi32(2, PO_UPDATE_DELAYTIME);
					sendPacketToNet(pak);

					deviceUpdateConfirm(PO_UPDATE_DELAYTIME);
				}
				else
				{
					uploadEndPacket(packet_ptr);
				}
			}
			break;
		}
		case kPOSubTypeAppUpdateCancel:
		{
			if (m_app_update.isUpdate())
			{
				printlog_lvs2(QString("Update canceled, conn[%1]").arg(conn), LOG_SCOPE_APP);
				deviceUpdateCancel();
				uploadEndPacket(packet_ptr);
			}
			break;
		}
	}
	return code;
}

i32 CSCApplication::onRequestEmulator(Packet* packet_ptr)
{
#if defined(POR_EMULATOR)
	if (CPOBaseApp::onRequestEmulator(packet_ptr) != kPOSuccess)
	{
		return kPOErrInvalidOper;
	}
#endif
	return kPOSuccess;
}

bool CSCApplication::deviceImport(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return false;
	}
	
	i32 sub_cmd = packet_ptr->getReservedi32(0);
	switch (sub_cmd)
	{
		case kPODevInfoImportStart:
		{
			return deviceImportStart(packet_ptr);
		}
		case kPODevInfoImportFinish:
		{
			return deviceImportFinish(packet_ptr);
		}
		case kPODevInfoSettingData:
		{
			return deviceImportSetting(packet_ptr);
		}
		case kPODevInfoJobDBData:
		{
			return deviceImportJobManager(packet_ptr);
		}
	}
	return false;
}

bool CSCApplication::deviceImportStart(Packet* packet_ptr)
{
	if (!m_app_update.isUpdate())
	{
		m_app_update.is_update = true;
		disconnectProcessorAll();
	}
	return true;
}

bool CSCApplication::deviceImportFinish(Packet* packet_ptr)
{
	if (m_app_update.isUpdate())
	{
		onRequestOffline(NULL);

		initSettings();
		onRequestOnline(NULL);
	}
	return true;
}

bool CSCApplication::deviceImportSetting(Packet* packet_ptr)
{
	if (!packet_ptr || !m_app_update.isUpdate())
	{
		printlog_lv0("Can't ImportSetting, Invalid Packet or UpdateInfomation.");
		return false;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();

	i32 len = 0;
	CPOBase::memRead(len, buffer_ptr, buffer_size);
	if (len != buffer_size)
	{
		printlog_lv0("Can't ImportSetting, Invalid DataLength.");
		return false;
	}

	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, PO_SIGN_CODE))
	{
		printlog_lv0("Can't ImportSetting, Invalid PreSignCode.");
		return false;
	}

	//import device settings
	DeviceInfo tmp_device_param;
	tmp_device_param.memRead(buffer_ptr, buffer_size);
	if (!m_device_info.isCompatibility(tmp_device_param))
	{
		printlog_lv0("Can't ImportSetting, Invalid Compatiblity.");
		return false;
	}
	m_device_info.import(tmp_device_param);

	//import camera settings
	CameraSet tmp_cam_param;
	tmp_cam_param.memRead(buffer_ptr, buffer_size);
	m_camera_param.import(tmp_cam_param);

	//import else settings
	m_security_param.memRead(buffer_ptr, buffer_size);
	m_general_param.memRead(buffer_ptr, buffer_size);
	m_engine_param.memRead(buffer_ptr, buffer_size);
	m_io_manager_ptr->memImport(buffer_ptr, buffer_size);

	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, PO_SIGN_ENDCODE))
	{
		printlog_lv0("Can't ImportSetting, Invalid PostSignCode.");
		return false;
	}

	m_runtime_history.init();
	g_sc_disk.writeAllSettings(this);
	g_sc_disk.writeDeviceINISettings(this, kPOIOAllDevice);
	return true;
}

bool CSCApplication::deviceImportJobManager(Packet* packet_ptr)
{
	if (!packet_ptr || !m_app_update.isUpdate())
	{
		printlog_lv0("Can't ImportJobManager, Invalid Packet or UpdateInfomation.");
		return false;
	}

	//reset history
	m_runtime_history.init();
	m_db_manager_ptr->deleteAllHistory();
	
	//import job_db 
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	i32 len = 0;
	CPOBase::memRead(len, buffer_ptr, buffer_size);
	if (len != buffer_size)
	{
		printlog_lv0("Can't ImportJobManager, Invalid DataLength.");
		return false;
	}
	return m_job_manager_ptr->memImportJobDB(buffer_ptr, buffer_size);
}

bool CSCApplication::deviceExport(PacketPVector& pak_vec)
{
	pak_vec.push_back(deviceExportStart());
	pak_vec.push_back(deviceExportSetting());
	pak_vec.push_back(deviceExportJobManager());
	pak_vec.push_back(deviceExportFinish());
	return true;
}

Packet* CSCApplication::deviceExportStart()
{
	stopProcessorAll();
	
	Packet* pak = po_new Packet(kPOCmdDevExport, kPOPacketRespOK);
	pak->setReservedi32(0, kPODevInfoExportStart);

	return pak;
}

Packet* CSCApplication::deviceExportFinish()
{
	Packet* pak = po_new Packet(kPOCmdDevExport, kPOPacketRespOK);
	pak->setReservedi32(0, kPODevInfoExportFinish);
	return pak;
}

Packet* CSCApplication::deviceExportSetting()
{
	i32 len = 0;
	len += m_device_info.memSize();
	len += m_camera_param.memSize();
	len += m_security_param.memSize();
	len += m_general_param.memSize();
	len += m_engine_param.memSize();
	len += m_io_manager_ptr->memSize();
	len += 2 * sizeof(i32); //two sign-code
	
	u8* buffer_ptr;
	i32 buffer_size = len + sizeof(len);
	Packet* pak = po_new Packet(kPOCmdDevExport, kPOPacketRespOK);
	pak->allocateBuffer(buffer_size, buffer_ptr);
	pak->setReservedi32(0, kPODevInfoSettingData);

	CPOBase::memWrite(len, buffer_ptr, buffer_size);
	CPOBase::memSignWrite(buffer_ptr, buffer_size, PO_SIGN_CODE);

	m_device_info.memWrite(buffer_ptr, buffer_size);
	m_camera_param.memWrite(buffer_ptr, buffer_size);
	m_security_param.memWrite(buffer_ptr, buffer_size);
	m_general_param.memWrite(buffer_ptr, buffer_size);
	m_engine_param.memWrite(buffer_ptr, buffer_size);
	m_io_manager_ptr->memWrite(buffer_ptr, buffer_size);

	CPOBase::memSignWrite(buffer_ptr, buffer_size, PO_SIGN_ENDCODE);
	return pak;
}

Packet* CSCApplication::deviceExportJobManager()
{
	i32 len = m_job_manager_ptr->memJobDBSize();

	u8* buffer_ptr;
	i32 buffer_size = len + sizeof(len);
	Packet* pak = po_new Packet(kPOCmdDevExport, kPOPacketRespOK);
	pak->allocateBuffer(buffer_size, buffer_ptr);
	pak->setReservedi32(0, kPODevInfoJobDBData);

	CPOBase::memWrite(len, buffer_ptr, buffer_size);
	m_job_manager_ptr->memExportJobDB(buffer_ptr, buffer_size);
	return pak;
}

void CSCApplication::updateIPInfo()
{
	anlock_guard(m_ip_info);
	m_ip_info.m_cmd_address = m_cmd_server.getConnection();
}

void CSCApplication::updateHLParam(Packet* packet_ptr)
{
	if (packet_ptr == NULL)
	{
		return;
	}

	{
		anlock_guard(m_ip_info);
		m_ip_info.m_high_address = packet_ptr->getReservedi32(0);
		m_ip_info.m_data_port = packet_ptr->getReservedu16(2);
		m_ip_info.m_high_adapter = packet_ptr->getReservedu8(6);
		m_ip_info.m_high_id = packet_ptr->getReservedi64(1);

		printlog_lv0(QString("HL address is %1, UDPStream port is %2, LL netadapter is %3, HL id is %4")
						.arg(m_ip_info.m_high_address)
						.arg(m_ip_info.m_data_port)
						.arg(m_ip_info.m_high_adapter)
						.arg(m_ip_info.m_high_id));
	}

	//update high-level datapath for device update
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	bool is_embedded = m_cmd_server.isLocalConnection() && m_device_info.is_hl_embedded;

	if (buffer_ptr && buffer_size > 0 && is_embedded)
	{
		postring hl_data_path;
		CPOBase::memRead(buffer_ptr, buffer_size, hl_data_path);
		m_data_path_hl = hl_data_path.c_str();
		if (m_data_path_hl != "")
		{
			is_embedded = true;
		}

		printlog_lv1(QString("HighLevel DataPath is %1").arg(hl_data_path.c_str()));
		checkPacket(packet_ptr, buffer_ptr);
	}
	m_ivs_server.setHLEmbedded(is_embedded, m_device_info.is_hl_embedded == is_embedded);
}

void CSCApplication::updateSyncDateTime(Packet* packet_ptr)
{
	if (!packet_ptr || !m_general_param.isAutoSyncDateTime())
	{
		return;
	}

	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	if (!buffer_ptr || buffer_size <= 0)
	{
		printlog_lvs2("DateTimeSync Failed.", LOG_SCOPE_APP);
		return;
	}

	DateTime dtm;
	CPOBase::memRead(dtm, buffer_ptr, buffer_size);
	if (!COSBase::setSystemDateTime(dtm))
	{
		printlog_lvs2("DateTimeSync set failed.", LOG_SCOPE_APP);
		return;
	}
}

bool CSCApplication::isSingleInstance(const char* uuidstr)
{
	if (m_device_info.is_emulator)
	{
		return true;
	}

	return CPOBaseApp::isSingleInstance(uuidstr);
}

bool CSCApplication::isOpcUpdating()
{
	return m_opc_updating;
}

bool CSCApplication::isModbusUpdating()
{
	return m_modbus_updating;
}

void CSCApplication::setOpcUpdating(bool is_update)
{
	m_opc_updating = is_update;
}

void CSCApplication::setModbusUpdating(bool is_update)
{
	m_modbus_updating = is_update;
}

Packet* CSCApplication::getRuntimeHistory(i32 cmd)
{
	Packet* pak = NULL;

	m_runtime_history.lock();
	i32 len = m_runtime_history.memSize();
	if (len > 0)
	{
		u8* buffer_ptr;
		i32 buffer_size = len;
		pak = po_new Packet(cmd, kPOPacketRespOK);
		pak->setSubCmd(kSCSubTypeSyncRTHistory);
		pak->allocateBuffer(len, buffer_ptr);

		m_runtime_history.memWrite(buffer_ptr, buffer_size);
		checkPacket(pak, buffer_ptr);
	}
	m_runtime_history.unlock();
	return pak;
}

void CSCApplication::uploadRuntimeHistory(i32 cmd)
{
	if (checkAppDesc(kPODescHighLevel))
	{
		sendPacketToNet(getRuntimeHistory(cmd));
	}
}

void CSCApplication::updateRuntimeHistory(i32 mode, void* data_ptr)
{
	i32 cam_id = -1;
	JobStatResultMap& job_result_map = m_runtime_history.m_job_stat_map;
	m_runtime_history.lock();
	{
		switch (mode)
		{
			case kRuntimeHistoryUpdateRemoveJob:
			{
				CJobUnit* job_ptr = (CJobUnit*)data_ptr;
				cam_id = job_ptr->getCamID();
				job_result_map.erase(job_ptr->getJobID());
				m_runtime_history.update();
				break;
			}
			case kRuntimeHistoryClearCamera:
			{
				cam_id = *((i32*)data_ptr);
				m_runtime_history.initRuntime(cam_id);
				break;
			}
			case kRuntimeHistoryClearAll:
			{
				m_runtime_history.initRuntime();
				break;
			}
		}
	}
	m_runtime_history.unlock();

	//output
	if (m_io_manager_ptr)
	{
		switch (mode)
		{
			case kRuntimeHistoryUpdateRemoveJob:
			case kRuntimeHistoryClearCamera:
			{
				m_io_manager_ptr->outputRuntimeHistory(cam_id, NULL);
				break;
			}
			case kRuntimeHistoryClearAll:
			{
				for (i32 i = 0; i < PO_CAM_COUNT; i++)
				{
					m_io_manager_ptr->outputRuntimeHistory(i, NULL);
				}
				break;
			}
		}
	}
}

void CSCApplication::updateRuntimeHistory(CJobResult* proc_result_ptr)
{
	if (!proc_result_ptr)
	{
		return;
	}

	i32 job_id;
	f32 proc_time = (f32)proc_result_ptr->getProcTime() / 1000;
	JobStatResultMap::iterator iter_job_stat;
	StatResult ob_per_job;
	JobStatResult job_stat;
		
	m_runtime_history.lock();
	{
		m_runtime_history.m_snap_count++;
		m_runtime_history.m_snap_total++;
		JobStatResultMap& job_stat_map = m_runtime_history.m_job_stat_map;

		switch (proc_result_ptr->getResult())
		{
			case kResultTypePassed:
			{
				ob_per_job.pass_count++;
				break;
			}
			case kResultTypeFailed:
			{
				ob_per_job.ng_count++;
				break;
			}
		}

		//collect measure result per model
		job_id = proc_result_ptr->getJobID();
		if (job_id != -1)
		{
			iter_job_stat = job_stat_map.find(job_id);
			if (iter_job_stat == job_stat_map.end())
			{
				i32 cam_id = proc_result_ptr->getCamID();
				i32 job_sel_id = proc_result_ptr->getJobSelID();
				powstring job_name = proc_result_ptr->getJobName();
				job_stat_map[job_id] = JobStatResult(cam_id, job_id, job_sel_id, job_name, ob_per_job, proc_time);
				job_stat = job_stat_map[job_id];
			}
			else
			{
				iter_job_stat->second.update(ob_per_job, proc_time);
				job_stat = iter_job_stat->second;
			}
		}

		updateUpTime();
	}
	m_runtime_history.unlock();

	//output
	if (m_io_manager_ptr)
	{
		m_io_manager_ptr->outputRuntimeHistory(job_stat.cam_id, &job_stat);
	}
}

void CSCApplication::stopProcessorAll()
{
	if (m_pu_array.size() != PO_UNIT_COUNT)
	{
		return;
	}

	g_shp_manager.acquire(PO_UNIT_COUNT);
	
	emit broadcastPacket(kSCCmdIntStopSystem, kPUCmdBlock);
	
	g_shp_manager.acquire(PO_UNIT_COUNT);
	g_shp_manager.release(PO_UNIT_COUNT);

	i32 available = g_shp_manager.available();
	if (available > PO_UNIT_COUNT)
	{
		printlog_lv1(QString("g_shp_manager available is %1").arg(available));
		assert(false);
	}
}

void CSCApplication::onAcceptExtRunStopThread(i32 cam_id, bool is_run_mode)
{
	if (!CPOBase::checkIndex(cam_id, PO_CAM_COUNT))
	{
		return;
	}

	CProcessor* processor_ptr = findProcessor(cam_id);
	if (processor_ptr)
	{
		i32 cmd = is_run_mode ? kSCCmdIntRunThread : kSCCmdIntStopThread;
		Packet* pak = po_new Packet(cmd, kPOPacketRequest);
		if (sendProcessorPacket(processor_ptr, pak) != kPOSuccess)
		{
			printlog_lv1(QString("RunStopThread[Cam:%1] is busy").arg(cam_id));
			return;
		}
	}
}

void CSCApplication::onAcceptExtRunStopThreadAll(bool is_run_mode)
{
	if (m_pu_array.size() != PO_UNIT_COUNT)
	{
		return;
	}

	i32 cmd = is_run_mode ? kSCCmdIntRunThread : kSCCmdIntStopThread;
	emit broadcastPacket(cmd, kPUCmdNonBlock);
}

void CSCApplication::disconnectProcessorAll()
{
	if (m_pu_array.size() != PO_UNIT_COUNT)
	{
		return;
	}

	g_shp_manager.acquire(PO_UNIT_COUNT);
	{
		printlog_lv1(QString("SCApplication acquire global shemaphore[%1].").arg(g_shp_manager.available()));
		emit broadcastPacket(kSCCmdIntDisconnect, kPUCmdBlock);
	}
	g_shp_manager.acquire(PO_UNIT_COUNT);
	g_shp_manager.release(PO_UNIT_COUNT);

	i32 available = g_shp_manager.available();
	if (available > PO_UNIT_COUNT)
	{
		printlog_lv1(QString("g_shp_manager available is %1").arg(available));
		assert(false);
	}
	m_admin_processor_ptr = NULL;
}

i32 CSCApplication::onRequestSelectJob(Packet* packet_ptr)
{
	i32 job_id = packet_ptr->getReservedi32(0);
	CJobUnit* job_ptr = m_job_manager_ptr->findJob(job_id);
	if (!job_ptr) 
	{
		printlog_lv1(QString("The specified job[%1] is not exist").arg(job_id));
		return kPOErrInvalidPacket;
	}
	printlog_lv1(QString("The model[%1] selection by user").arg(job_ptr->getExternalID()));

	//find current model and check compatibility
	if (!m_admin_processor_ptr || m_admin_processor_ptr->getCamID() != job_ptr->getCamID())
	{
		if (!m_admin_processor_ptr)
		{
			printlog_lv1("Invalid admin processor in kPOCmdSelectJob.");
		}
		else
		{
			printlog_lv1(QString("Select model is failed. cam of admin processor[%1], cam of model[%2]")
				.arg(m_admin_processor_ptr->getCamID()).arg(job_ptr->getCamID()));
		}
		return kPOErrInvalidOper;
	}

	//send select model internal signal
	Packet* pak = po_new Packet(packet_ptr);
	pak->setCmd(kSCCmdIntSelectJob);

	pak->setReservedi32(0, job_id);
	pak->setReservedu8(4, kRespJobInfo);
	pak->setReservedu8(5, kOperationbyAdmin);

	return sendProcessorPacket(m_admin_processor_ptr, pak);
}

i32 CSCApplication::onRequestGetRuntimeHistory(Packet* packet_ptr)
{
	uploadRuntimeHistory(kSCCmdGetRumtimeHistory);
	return kPOSuccess;
}

i32 CSCApplication::onRequestResetRuntimeHistory(Packet* packet_ptr)
{
	m_runtime_history.initRuntime();

	updateUpTime();
	uploadRuntimeHistory(kSCCmdResetRuntimeHistory);
	outputRuntimeHistory();
	return kPOSuccess;
}

i32 CSCApplication::onRequestIORawCommand(Packet* packet_ptr)
{
	i32 cmd = packet_ptr->getSubCmd();
	m_io_manager_ptr->setIORawCommand(cmd);
	
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CSCApplication::onRequestSetting(Packet* packet_ptr)
{
	if (!packet_ptr || !packet_ptr->getData())
	{
		return kPOErrInvalidPacket;
	}

	i32 code = kPOErrInvalidOper;
	i32 sub_cmd = packet_ptr->getSubCmd();

	switch (sub_cmd)
	{
		case kSCSubTypeGeneralSetting:
		{
			code = onRequestGeneralSetting(packet_ptr);
			break;
		}
		default:
		{
			break;
		}
	}

	if (code == kPOSuccess)
	{
		g_sc_disk.writeAllSettings(this);
		g_sc_disk.writeGeneralINISettings(this);
	}
	else
	{
		printlog_lv1("Update data is not exist in setting page by user.");
	}

	Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataMove);
	sendPacketToNet(pak); //ok and fail response
	return kPOSuccess;
}

i32 CSCApplication::onRequestDevSetting(Packet* packet_ptr)
{
	if (packet_ptr == NULL || packet_ptr->getData() == NULL)
	{
		return kPOErrInvalidPacket;
	}

	i32 code = kPOErrInvalidOper;
	i32 sub_cmd = packet_ptr->getSubCmd();
	switch (sub_cmd)
	{
		case kSCSubTypeDateTimeSetting:
		{
			code = onRequestDateTimeSetting(packet_ptr);
			break;
		}
		case kSCSubTypeNetworkSetting:
		{
			code = onRequestNetworkSetting(packet_ptr);
			break;
		}
		default:
		{
			printlog_lvs2(QString("RequestDevSetting Failed. cmd:%1").arg(sub_cmd), LOG_SCOPE_APP);
			break;
		}
	}
	return code;
}

i32 CSCApplication::onRequestIOSetting(Packet* packet_ptr)
{
	if (packet_ptr == NULL || packet_ptr->getData() == NULL)
	{
		return kPOErrInvalidPacket;
	}
	if (!m_io_manager_ptr)
	{
		return kPOErrInvalidOper;
	}

	i32 code = kPOSuccess;
	i32 sub_cmd = packet_ptr->getSubCmd();
	i32 sub_mode = packet_ptr->getReservedi32(0);
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();

	switch (sub_cmd)
	{
		case kSCSubTypeIOInput:
		{
			//kSCSubTypeIOSetting와 처리는 같음.
			sub_mode = kSCSubFlagIOInput;
		}
		case kSCSubTypeIOSetting:
		{
			printlog_lv1("onRequestIOSetting [sub_cmd]kSCSubTypeIOSetting/kSCSubTypeIOInput");
			if (m_io_manager_ptr->memParamRead(buffer_ptr, buffer_size, kPOIOSerial, sub_mode))
			{
				g_sc_disk.writeIOManagerINISettings(this, kPOIOSerial);
			}
			g_sc_disk.writeAllSettings(this);

			//update io manager
			m_io_manager_ptr->updateIOManager(sub_cmd, sub_mode);
			break;
		}
		case kSCSubTypeIOOutput:
		{
			printlog_lv1("onRequestIOSetting [sub_cmd]kSCSubTypeIOOutput");

			Packet* pak = po_new Packet(packet_ptr);
			pak->setCmd(kSCCmdIntJobSetting);
			pak->setSubCmd(kSCSubTypeIOOutput);
			pak->copyData(packet_ptr);
			sendProcessorPacket(m_admin_processor_ptr, pak);
			break;
		}
		default:
		{
			code = kPOErrInvalidOper;
			break;
		}
	}

	if (code == kPOSuccess)
	{
		//command response
		Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataMove);
		sendPacketToNet(pak);
	}
	else
	{
		printlog_lv1(QString("onRequestIOSetting Error:%1").arg(code));
	}
	return code;
}

i32 CSCApplication::onRequestCommSetting(Packet* packet_ptr)
{
	if (packet_ptr == NULL || packet_ptr->getData() == NULL)
	{
		return kPOErrInvalidPacket;
	}

	i32 code = kPOSuccess;
	i32 sub_cmd = packet_ptr->getSubCmd();
	i32 sub_mode = packet_ptr->getReservedi32(0);
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();

	switch (sub_cmd)
	{
		case kSCSubTypeModbusSetting:
		{
			printlog_lv1("onRequestCommSetting [sub_cmd]kSCSubTypeModbusSetting");
			if (m_io_manager_ptr->memParamRead(buffer_ptr, buffer_size, kPOIOModbus, sub_mode))
			{
				g_sc_disk.writeIOManagerINISettings(this, kPOIOModbus);
			}
			g_sc_disk.writeAllSettings(this);
			m_io_manager_ptr->updateIOManager(sub_cmd, sub_mode);
			break;
		}
		case kSCSubTypeFtpSetting:
		{
			printlog_lv1("onRequestCommSetting [sub_cmd]kSCSubTypeFtpSetting");
			if (m_io_manager_ptr->memParamRead(buffer_ptr, buffer_size, kPOIOFtp, sub_mode))
			{
				g_sc_disk.writeIOManagerINISettings(this, kPOIOFtp);
				g_sc_disk.writeAllSettings(this);
				m_io_manager_ptr->updateIOManager(sub_cmd, sub_mode);
			}
			else
			{
				code = kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeOpcSetting:
		{
			printlog_lv1("onRequestCommSetting [sub_cmd]kSCSubTypeOpcSetting");
			if (m_io_manager_ptr->memParamRead(buffer_ptr, buffer_size, kPOIOOpc, sub_mode))
			{
				g_sc_disk.writeIOManagerINISettings(this, kPOIOOpc);
				g_sc_disk.writeAllSettings(this);
				m_io_manager_ptr->updateIOManager(sub_cmd, sub_mode);
			}
			else
			{
				code = kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeModbusComm:
		{
			printlog_lv1("onRequestCommSetting [sub_cmd]kSCSubTypeModbusComm");

			Packet* pak = po_new Packet(packet_ptr);
			pak->setCmd(kSCCmdIntJobSetting);
			pak->setSubCmd(kSCSubTypeModbusComm);
			pak->copyData(packet_ptr);

			if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
			{
				code = kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeFtpComm:
		{
			printlog_lv1("onRequestCommSetting [sub_cmd]kSCSubTypeFtpComm");

			Packet* pak = po_new Packet(packet_ptr);
			pak->setCmd(kSCCmdIntJobSetting);
			pak->setSubCmd(kSCSubTypeFtpComm);
			pak->copyData(packet_ptr);

			if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
			{
				code = kPOErrInvalidOper;
			}
			break;
		}
		case kSCSubTypeOpcComm:
		{
			printlog_lv1("onRequestCommSetting [sub_cmd]kSCSubTypeOpcComm");

			Packet* pak = po_new Packet(packet_ptr);
			pak->setCmd(kSCCmdIntJobSetting);
			pak->setSubCmd(kSCSubTypeOpcComm);
			pak->copyData(packet_ptr);

			if (sendProcessorPacket(m_admin_processor_ptr, pak) != kPOSuccess)
			{
				code = kPOErrInvalidOper;
			}
			break;
		}
		default:
		{
			code = kPOErrInvalidOper;
			break;
		}
	}

	if (code == kPOSuccess)
	{
		//command response
		Packet* pak = Packet::makeRespPacket(packet_ptr, code, kPacketDataMove);
		sendPacketToNet(pak);
	}
	else
	{
		printlog_lv1(QString("onRequestCommSetting Error:%1").arg(code));
	}
	return code;
}

i32 CSCApplication::onRequestGeneralSetting(Packet* packet_ptr)
{
	bool is_updated = false;
	i32 mode = packet_ptr->getReservedi32(0);
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();

	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidOper;
	}

	CGeneralSetting tmp_gs_setting; //thread safe
	tmp_gs_setting.memRead(buffer_ptr, buffer_size);
	printlog_lv1(QString("GeneralSetting mode:%1.").arg(mode));

	m_general_param.lock();
	if (CPOBase::bitCheck(mode, kSCSubFlagGeneralLanguage))
	{
		if (CPOBase::checkIndex(tmp_gs_setting.m_language_hl, kPOLangCount) &&
			tmp_gs_setting.m_language_hl != m_general_param.m_language_hl)
		{
			is_updated = true;
			m_general_param.m_language_hl = tmp_gs_setting.m_language_hl;
		}
	}
	if (CPOBase::bitCheck(mode, kSCSubFlagGeneralHistory))
	{
		if (CPOBase::checkIndex(tmp_gs_setting.m_history_setting, kResultOperTypeCount) &&
			tmp_gs_setting.m_history_setting != m_general_param.m_history_setting)
		{
			is_updated = true;
			m_general_param.m_history_setting = tmp_gs_setting.m_history_setting;
		}
	}
	if (CPOBase::bitCheck(mode, kSCSubFlagGeneralDispFmt))
	{
		if (CPOBase::checkRange(tmp_gs_setting.m_decimal_disp, 10) &&
			tmp_gs_setting.m_decimal_disp != m_general_param.m_decimal_disp)
		{
			is_updated = true;
			m_general_param.m_decimal_disp = tmp_gs_setting.m_decimal_disp;
		}
	}
	if (CPOBase::bitCheck(mode, kSCSubFlagGeneralDebug))
	{
		if (CPOBase::checkRange(tmp_gs_setting.m_debug_level, LOG_LV4) &&
			tmp_gs_setting.m_debug_level != m_general_param.m_debug_level)
		{
			is_updated = true;
			m_general_param.m_debug_level = tmp_gs_setting.m_debug_level;
			set_loglevel(m_general_param.m_debug_level);
		}
	}
	m_general_param.unlock();

	if (!is_updated)
	{
		return kPOErrInvalidOper;
	}
	return kPOSuccess;
}

i32 CSCApplication::onRequestDateTimeSetting(Packet* packet_ptr)
{
	DateTime dtm;
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	i32 mode = packet_ptr->getReservedi32(0);

	if (CPOBase::bitCheck(mode, kDevRequestDateTimeFlag))
	{
		CPOBase::getNowTime(dtm);
	}
	else if (CPOBase::bitCheck(mode, kDevSetDateTimeFlag))
	{
		if (!buffer_ptr || buffer_size <= 0)
		{
			return kPOErrInvalidOper;
		}

		bool is_sync_datetime = false;
		CPOBase::memRead(dtm, buffer_ptr, buffer_size);
		CPOBase::memRead(is_sync_datetime, buffer_ptr, buffer_size);

		bool is_update = false;
		m_general_param.lock();
		if (m_general_param.m_is_sync_datetime != is_sync_datetime)
		{
			is_update = true;
			m_general_param.m_is_sync_datetime = is_sync_datetime;
		}
		m_general_param.unlock();

		if (is_update)
		{
			g_sc_disk.writeAllSettings(this);
			g_sc_disk.writeGeneralINISettings(this);
		}
		if (!COSBase::setSystemDateTime(dtm))
		{
			printlog_lvs2("DateTimeSetting is failed.", LOG_SCOPE_APP);
			return kPOErrInvalidOper;
		}
	}

	//response packet
	buffer_size = sizeof(dtm) + sizeof(bool);
	Packet* pak = Packet::makeRespPacket(packet_ptr);
	pak->allocateBuffer(buffer_size, buffer_ptr);
	CPOBase::memWrite(m_general_param.isAutoSyncDateTime(), buffer_ptr, buffer_size);
	CPOBase::memWrite(dtm, buffer_ptr, buffer_size);
	sendPacketToNet(pak);
	return kPOSuccess;
}

i32 CSCApplication::onRequestNetworkSetting(Packet* packet_ptr)
{
	u8* buffer_ptr = packet_ptr->getData();
	i32 buffer_size = packet_ptr->getDataLen();
	i32 mode = packet_ptr->getReservedi32(0);

	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidOper;
	}

	//장치이름설정
	if (CPOBase::bitCheck(mode, kDevSetNameFlag))
	{
		postring dev_name;
		CPOBase::memRead(buffer_ptr, buffer_size, dev_name);
		if (dev_name != m_device_info.getDeviceName())
		{
			m_device_info.device_name = dev_name;

			COSBase::setHostName(dev_name);
			g_sc_disk.writeAllSettings(this);
			g_sc_disk.writeAdminINISettings(this, m_device_info.getDeviceVersion());
		}

		Packet* pak = Packet::makeRespPacket(packet_ptr, kPOSuccess, kPacketDataMove);
		sendPacketToNet(pak);
		return kPOSuccess;
	}
	//망주소설정
	if (CPOBase::bitCheck(mode, kDevSetNetAddressFlag))
	{
		i32 conn = m_cmd_server.getConnection();
		i32 ip_address = 0, ip_submask = 0, ip_gateway = 0, ip_dns_server = 0;
		bool is_conf_dhcp = false;

		CPOBase::memRead(is_conf_dhcp, buffer_ptr, buffer_size);
		CPOBase::memRead(ip_address, buffer_ptr, buffer_size);
		CPOBase::memRead(ip_submask, buffer_ptr, buffer_size);
		CPOBase::memRead(ip_gateway, buffer_ptr, buffer_size);
		CPOBase::memRead(ip_dns_server, buffer_ptr, buffer_size);

		disconnectProcessorAll();
		onRequestOffline(NULL);
				
		COSBase::setNetworkAddress(conn, is_conf_dhcp, ip_address, ip_submask, ip_gateway, ip_dns_server);
		onRequestOnline(NULL);
		return kPOSuccess;
	}
	return kPOErrInvalidOper;
}

i32 CSCApplication::onRequestCameraSelect(Packet* packet_ptr)
{
	if (!packet_ptr)
	{
		return kPOErrInvalidPacket;
	}

	i32 cam_id = packet_ptr->getReservedi32(0);
	singlelog_lv1(QString("Camera%1 selection").arg(cam_id));

	CProcessor* processor_ptr = setCameraProcessor(cam_id, true, kHLCommand);
	if (!processor_ptr)
	{
		return kPOErrInvalidOper;
	}
	return kPOSuccess;
}

CCameraManager* CSCApplication::getCameraManager()
{
	return m_cam_manager_ptr;
}

CJobManager* CSCApplication::getJobManager()
{
	return m_job_manager_ptr;
}

CSCIOManager* CSCApplication::getIOManager()
{
	return m_io_manager_ptr;
}

CSCDBManager* CSCApplication::getDBManager()
{
	return m_db_manager_ptr;
}

CTagManager* CSCApplication::getTagManager()
{
	return m_tag_manager_ptr;
}

DeviceInfo* CSCApplication::getDeviceInfo()
{
	return (DeviceInfo*)&m_device_info;
}

CIPInfo* CSCApplication::getIPInfo()
{
	return (CIPInfo*)&m_ip_info;
}

QString CSCApplication::getLowLevelName()
{
	return QString(PO_LOWLEVEL_NAME);
}

QString CSCApplication::getHighLevelName()
{
	return QString(PO_HIGHLEVEL_NAME);
}

bool CSCApplication::setPassword(postring& strCurPassword, postring& strNewPassword)
{
	if (strNewPassword.size() < PO_PASSWORD_MINLEN)
	{
		printlog_lv1(QString("New password len is very short, [%1]").arg(strNewPassword.c_str()));
		return false;
	}
	if (!checkPassword(strCurPassword))
	{
		printlog_lv1("Invalid password used");
		return false;
	}

	m_security_param.setPassword(strNewPassword);
	g_sc_disk.writeAllSettings(this);
	return true;
}

bool CSCApplication::checkPassword(postring& strPassword)
{
	QThread::msleep(100);
	return m_security_param.checkPermissionAuth(strPassword);
}

bool CSCApplication::executeSQLQuery(QString& strquery)
{
	if (!m_db_manager_ptr)
	{
		return false;
	}
	m_db_manager_ptr->executeQuery(&m_db_config, strquery);
	return true;
}

bool CSCApplication::updateDeviceOffline()
{
	g_sc_disk.writeAllSettings(this);
	g_sc_disk.writeDeviceINISettings(this);
	onRequestOffline(NULL);
	return true;
}

bool CSCApplication::updateDeviceINISettings(QString& strLLPath)
{
	potstring filename;
	filename = strLLPath.toStdTString() + CPOBase::stringToTString(PO_DEVICE_INIFILE);
	if (!writeAdminINISettings(filename, m_app_update.update_version))
	{
		printlog_lv1("Device Admin.INI writting is failed");
		return false;
	}
	if (!writeDeviceINISettings(filename, kPOIOAllDevice))
	{
		printlog_lv1("Device INI writting is failed");
		return false;
	}
	return true;
}

bool CSCApplication::appendToFile(const char* filename, u8* buffer_ptr, i32 len)
{
	return g_sc_disk.writeAppend(filename, buffer_ptr, len);
}

void CSCApplication::initProcessorQueue()
{
	for (i32 i = 0; i < PO_UNIT_COUNT; i++)
	{
		m_pu_queue[i] = 0;
	}
}

bool CSCApplication::incProcessorQueue(i32 puid)
{
	if (!CPOBase::checkIndex(puid, PO_UNIT_COUNT))
	{
		return false;
	}

	if (m_pu_queue[puid] >= PU_QUEUE_SIZE)
	{
		return false;
	}
	m_pu_queue[puid]++;
	return true;
}

bool CSCApplication::decProcessorQueue(i32 puid)
{
	if (!CPOBase::checkIndex(puid, PO_UNIT_COUNT))
	{
		return false;
	}

	m_pu_queue[puid] = po::_max(m_pu_queue[puid] - 1, 0);
	return true;
}

CJobUnit* CSCApplication::getProcessorCurJob(i32 puid)
{
	if (!CPOBase::checkIndex(puid, PO_UNIT_COUNT))
	{
		return NULL;
	}
	return m_job_manager_ptr->getCurJob(puid);
}

CGlobalVar* CSCApplication::getGlobalVar()
{
	return m_global_var_ptr;
}

i32 CSCApplication::onRequestDBOperation(Packet* packet_ptr)
{
	if (!packet_ptr || !m_db_client_ptr)
	{
		return kPOErrInvalidPacket;
	}

	i32 sub_cmd = packet_ptr->getSubCmd();
	i32 buffer_size = packet_ptr->getDataLen();
	u8* buffer_ptr = packet_ptr->getData();
	if (!buffer_ptr || buffer_size <= 0)
	{
		return kPOErrInvalidData;
	}

	switch (sub_cmd)
	{
		case kSCSubTypeExecQuery:
		case kSCSubTypeFilterHistory:
		case kSCSubTypeFilterHistoryCount:
		case kSCSubTypeGetThumbs:
		case kSCSubTypeGetCurHistory:
		case kSCSubTypeGetHistoryVector:
		case kSCSubTypeClearHistory:
		{
			if (!m_db_client_ptr->sendOperation(packet_ptr))
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
	return kPOSuccess;
}

void CSCApplication::uploadNoneCameraState()
{
	//련결되지 않은, 즉 카메라가 없는 상태에서의 CameraAdminSync명령응답을 올려보낸다.
	u8* buffer_ptr = NULL;
	i32 buffer_size = 0;
	bool is_cam_connected = false;
	i32 job_id = -1;
	CameraSetting tmp_cam_setting;

	buffer_size += sizeof(m_app_state);
	buffer_size += sizeof(m_app_mode);
	buffer_size += sizeof(is_cam_connected);
	buffer_size += tmp_cam_setting.memSize();
	buffer_size += sizeof(job_id);

	Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
	pak->setSubCmd(kSCSubTypeAdminCameraSync);
	pak->allocateBuffer(buffer_size, buffer_ptr);

	CPOBase::memWrite(m_app_state, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_app_mode, buffer_ptr, buffer_size);
	CPOBase::memWrite(is_cam_connected, buffer_ptr, buffer_size);
	tmp_cam_setting.memWrite(buffer_ptr, buffer_size);
	CPOBase::memWrite(job_id, buffer_ptr, buffer_size);
	sendPacketToNet(pak, buffer_ptr);
}

void CSCApplication::uploadWarningMessage(const powstring& str_en, const powstring& str_ch, const powstring& str_ko)
{
	UIString disp_str;
	disp_str.ui_string[kPOLangEnglish] = str_en;
	disp_str.ui_string[kPOLangChinese] = str_ch;
	disp_str.ui_string[kPOLangKorean] = str_ko;

	i32 len = disp_str.memSize();
	u8* buffer_ptr;
	i32 buffer_size = len;

	Packet* pak = po_new Packet(kSCCmdDevState, kPOPacketRespOK);
	pak->setSubCmd(kSCSubTypeDevWarningMessage);
	pak->allocateBuffer(len, buffer_ptr);
	disp_str.memWrite(buffer_ptr, buffer_size);

	sendPacketToNet(pak);
}

bool CSCApplication::checkQtSignals(i32 signal, bool is_push)
{
	exlock_guard(m_qtsig_mutex);
	if (!CPOBase::checkIndex(signal, SC_SIGNAL_COUNT))
	{
		return false;
	}

	if (is_push)
	{
		if (m_qtsig_reference[signal] > SC_SIGNAL_MAXOVERLAP)
		{
			return false;
		}
		//increase qtsignal reference
		m_qtsig_reference[signal]++;
	}
	else
	{
		//decrease qtsignal reference
		m_qtsig_reference[signal] = po::_max(m_qtsig_reference[signal] - 1, 0);
	}
	return true;
}

void CSCApplication::outputRuntimeHistory(i32 job_id)
{
	JobStatResult* job_stat_ptr = NULL;
	if (job_id >= 0)
	{
		job_stat_ptr = m_runtime_history.findJobHistory(job_id);
	}

	if (job_stat_ptr)
	{
		m_io_manager_ptr->outputRuntimeHistory(job_stat_ptr->getCamID(), job_stat_ptr);
	}
	else
	{
		for (i32 i = 0; i < PO_CAM_COUNT; i++)
		{
			m_io_manager_ptr->outputRuntimeHistory(i, NULL);
		}
	}
}

void CSCApplication::registerToolCategorys()
{
	m_tool_categorys_ptr->init();

	//Locator Tools
	CToolCategory* tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCLocatorTool, kToolCategoryLocatorText);
	tool_category_ptr->addToolDescriptor(kSCLocatorToolShapeMax, kToolFlagColorAny,
							kToolRegionFlagAll & (~(kToolRegionFlagEllipse | kToolRegionFlagRing)),
							kToolRegionFlagAll & (~(kToolRegionFlagEllipse | kToolRegionFlagRing)),
							kToolLocatorShapeMaxText);
	tool_category_ptr->addToolDescriptor(kSCLocatorToolShapeBased,kToolFlagColorAny,
							kToolRegionFlagAll, kToolRegionFlagAll,
							kToolLocatorShapeBasedText);
	//tool_category_ptr->addToolDescriptor(kSCLocatorToolContentBased, kToolFlagSupportAnyColor,
	//						kToolRegionFlagAll, kToolRegionFlagAll,
	//						kToolLocatorContentBasedText);
	
	tool_category_ptr->addToolDescriptor(kSCLocatorToolEdge, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll & (~kToolRegionFlagMaskable),
							kToolLocatorEdgeText);
	tool_category_ptr->addToolDescriptor(kSCLocatorToolBlob, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolLocatorBlobText);
	tool_category_ptr->addToolDescriptor(kSCLocatorToolBlobs, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolLocatorBlobsText);
	tool_category_ptr->addToolDescriptor(kSCLocatorToolCircle, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagCircle | kToolRegionFlagRing,
							kToolLocatorCircleText);
	tool_category_ptr->addToolDescriptor(kSCLocatorToolComputeFixture, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolLocatorComputeFixtureText);
	
	//Presence Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCPresenceTool, kToolCategoryPresenceText);
	tool_category_ptr->addToolDescriptor(kSCPresenceToolBrightness, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolPresenceBrightnessText);
	tool_category_ptr->addToolDescriptor(kSCPresenceToolContrast, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolPresenceContrastText);
	tool_category_ptr->addToolDescriptor(kSCPresenceToolSharpness, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolPresenceSharpnessText);
	tool_category_ptr->addToolDescriptor(kSCPresenceToolEdge, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagEllipse | kToolRegionFlagRotatable,
							kToolPresenceEdgeText);
	tool_category_ptr->addToolDescriptor(kSCPresenceToolCircle, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagCircle | kToolRegionFlagRing,
							kToolPresenceCircleText);

	//Measurement Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCMeasurementTool, kToolCategoryMeasurementText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolDistance, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Objects,
							kToolMeasurementDistanceText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolAngle, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Edges,
							kToolMeasurementAngleText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolBlobArea, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolMeasurementBlobAreaText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolBlobAreas, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolMeasurementBlobAreasText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolCircleDiameter, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagCircle | kToolRegionFlagRing,
							kToolMeasurementCircleDiameterText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolCircleConcentricity, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Circles,
							kToolMeasurementCircleConcentricityText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolCircleRadius, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagRing,
							kToolMeasurementCircleRadiusText);
	tool_category_ptr->addToolDescriptor(kSCMeasureToolMinMaxPoints, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagRect | kToolRegionFlagRing |
							kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolMeasurementMinMaxPointsText);

	//Counting Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCCountingTool, kToolCategoryCountingText);
	tool_category_ptr->addToolDescriptor(kSCCountToolBlobs, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolCountingBlobsText);
	tool_category_ptr->addToolDescriptor(kSCCountToolEdges, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll & (~kToolRegionFlagMaskable),
							kToolCountingEdgesText);
	tool_category_ptr->addToolDescriptor(kSCCountToolEdgePairs, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll & (~kToolRegionFlagMaskable),
							kToolCountingEdgePairsText);
	tool_category_ptr->addToolDescriptor(kSCCountToolShapeMaxLocator, kToolFlagColorAny,
							kToolRegionFlagAll & (~(kToolRegionFlagEllipse | kToolRegionFlagRing)),
							kToolRegionFlagAll & (~(kToolRegionFlagEllipse | kToolRegionFlagRing)),
							kToolCountingShapeMaxLocatorText);
	tool_category_ptr->addToolDescriptor(kSCCountToolShapeLocator, kToolFlagColorAny,
							kToolRegionFlagAll, kToolRegionFlagAll,
							kToolCountingShapeLocatorText);
	tool_category_ptr->addToolDescriptor(kSCCountToolPixelCounter, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolCountingPixelCounterText);

	//Geometry Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCGeometryTool, kToolCategoryGeometryText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolP2PLine, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Points,
							kToolGeometryP2PLineText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolP2PMidPoint, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Points,
							kToolGeometryP2PMidPointText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolP2PDimension, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2PointsAndEdge,
							kToolGeometryP2PDimensionText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolPerpendicularLine, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPickEdge,
							kToolGeometryPerpendicularLineText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolLineIntersection, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Edges,
							kToolGeometryLineIntersectionText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolBisetAngle, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPick2Edges,
							kToolGeometryBisetAngleText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolFitLineNPoints, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPickMultiPoints,
							kToolGeometryFitLineNPointsText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolFitCircleNPoints, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPickMultiPoints,
							kToolGeometryFitCircleNPointsText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolCircleLineIntersection, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagPickCircleAndEdge,
							kToolGeometryCircleLineIntersectionText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolLineFit, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolGeometryLineFitText);
	tool_category_ptr->addToolDescriptor(kSCGeometryToolCircleFit, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRing | kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolGeometryCircleFitText);

	//Identification Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCIdentifyTool, kToolCategoryIdentifyText);
	tool_category_ptr->addToolDescriptor(kSCIDTool1DCode, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagRegularCreate, 
							kToolIdentification1DCodeText);
	tool_category_ptr->addToolDescriptor(kSCIDTool1DCodes, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagRegularCreate, 
							kToolIdentification1DCodesText);
	tool_category_ptr->addToolDescriptor(kSCIDTool2DCode, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagRegularCreate,
							kToolIdentification2DCodeText);
	tool_category_ptr->addToolDescriptor(kSCIDTool2DCodes, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagRegularCreate, 
							kToolIdentification2DCodesText);
	tool_category_ptr->addToolDescriptor(kSCIDToolPostalCode, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagRegularCreate,
							kToolIdentificationPostalCodeText);
	tool_category_ptr->addToolDescriptor(kSCIDToolOCRMax, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagRotatable | kToolRegionFlagRegularCreate,
							kToolIdentificationOCRMaxText);

	//ImageFilter Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCImageFilterTool, kToolCategoryImageFilterText);
	tool_category_ptr->addToolDescriptor(kSCImageFilterToolFilter, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagRotatable,
							kToolImageFilterFilterText);
	tool_category_ptr->addToolDescriptor(kSCImageFilterToolColor2Gray, kToolFlagColorRGB,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRotatable, 
							kToolImageFilterColor2GrayText);

	//Math Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCMathTool, kToolCategoryMathText);
	tool_category_ptr->addToolDescriptor(kSCMathToolMath, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolMathMathText);
	tool_category_ptr->addToolDescriptor(kSCMathToolString, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolMathStringText);
	tool_category_ptr->addToolDescriptor(kSCMathToolLogic, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolMathLogicText);
	tool_category_ptr->addToolDescriptor(kSCMathToolVariables, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolMathVariablesText);

	//Plot Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCPlotTool, kToolCategoryPlotText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolArc, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotArcText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolCircle, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotCircleText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolCross, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotCrossText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolLine, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotLineText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolPoint, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotPointText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolRegion, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotRegionText);
	tool_category_ptr->addToolDescriptor(kSCPlotToolString, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagNone,
							kToolPlotStringText);

	//Defect Detection Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCDefectDetectionTool, kToolCategoryDefectDetectionText);
	tool_category_ptr->addToolDescriptor(kSCDefectToolSurfaceFlaw, kToolFlagColorAny,
							kToolRegionFlagNone, kToolRegionFlagAll,
							kToolDefectSurfaceFlawText);
	tool_category_ptr->addToolDescriptor(kSCDefectToolEdge, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagRing | kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolDefectEdgeText);
	tool_category_ptr->addToolDescriptor(kSCDefectToolEdgePair, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRing | kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolDefectEdgePairText);
	tool_category_ptr->addToolDescriptor(kSCDefectToolEdgeWidth, kToolFlagColorAny,
							kToolRegionFlagNone,
							kToolRegionFlagRect | kToolRegionFlagRing | kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolDefectEdgeWidthText);
	tool_category_ptr->addToolDescriptor(kSCDefectToolBeadFinder, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRing | kToolRegionFlagRotatable | kToolRegionFlagMaskable,
							kToolDefectBeadFinderText);
	tool_category_ptr->addToolDescriptor(kSCDefectToolBeadTracker, kToolFlagColorAny,
							kToolRegionFlagNone, 
							kToolRegionFlagRect | kToolRegionFlagRing | kToolRegionFlagRotatable | kToolRegionFlagMaskable, 
							kToolDefectBeadTrackerText);

	//Calibration Tools
	tool_category_ptr = m_tool_categorys_ptr->addToolCategory(kSCCalibrationTool, kToolCategoryCalibrationText);
}

void CSCApplication::onRequestCommand(i32 cmd, i32 cam_id)
{
	CProcessor* processor_ptr = findProcessor(cam_id);
	if (processor_ptr)
	{
		i32 data = 0;
		QMetaObject::invokeMethod(processor_ptr, "onRequestCommand", Qt::QueuedConnection,
							Q_ARG(i32, cmd), Q_ARG(i32, data));
	}
}

void CSCApplication::onUpdatingToolFromOpc(i32 cam_id)
{
	CProcessor* processor_ptr = findProcessor(cam_id);
	if (processor_ptr)
	{
		QMetaObject::invokeMethod(processor_ptr, "onUpdatingToolFromOpc", Qt::QueuedConnection);
	}
}

void CSCApplication::onUpdatingToolFromModbus(i32 cam_id, i32 addr, i32 size)
{
	CProcessor* processor_ptr = findProcessor(cam_id);
	if (processor_ptr)
	{
		QMetaObject::invokeMethod(processor_ptr, "onUpdatingToolFromModbus", Qt::QueuedConnection,
							Q_ARG(i32, addr), Q_ARG(i32, size));
	}
}
