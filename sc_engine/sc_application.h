#pragma once
#include "config.h"

#include "app/base_app.h"
#include "sc_define.h"
#include "sc_struct.h"
#include "sc_disk.h"
#include "sc_camera_set.h"

#if defined(POR_WITH_GUI)
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#endif

#include <QSettings>
#include <QSemaphore>

Q_DECLARE_METATYPE(ScuControl);
Q_DECLARE_METATYPE(CFtpDev);

class CJobResult;
class CJobManager;
class CSCDBClient;
class CSCDBManager;
class CSCIOManager;
class CCameraManager;
class CTagManager;
class CTagUIManager;
class CProcessor;
class CToolCategorys;

class CSCApplication : public CPOBaseApp, public CLockGuard
{
	Q_OBJECT

public:
	CSCApplication(i32 argc, char *argv[]);
	~CSCApplication();

	virtual bool				initApplication(i32 desc, i32 argc, char *argv[]);
	virtual bool				exitApplication();

	virtual bool				initInstance();
	virtual bool				exitInstance();

	bool						initProcessors();
	bool						exitProcessors();

	void						initProcessorQueue();
	bool						incProcessorQueue(i32 puid);
	bool						decProcessorQueue(i32 puid);

	void						initEvent();
	bool						initSettings();
	void						initAllSetting();
	void						initLogFiles();
	void						exitLogFiles();
	bool						initCognexEngine();
	void						exitCognexEngine();
	void						registerToolCategorys();

	bool						resetAllSettings();
	bool						loadAllSettings(FILE* fp);
	bool						writeAllSettings(FILE* fp);
	bool						loadDeviceINISettings(potstring& filename);
	bool						writeDeviceINISettings(potstring& filename, i32 dev_mode = kPOIOAllDevice);
	bool						writeGeneralINISettings(potstring& filename);
	bool						writeIOCommINISettings(potstring& filename, i32 dev_mode = kPOIOAllDevice);
	bool						writeAdminINISettings(potstring& filename, const postring& dev_version);
	void						loadCameraINISettings(QSettings& ini);
	void						loadGeneralINISettings(QSettings& ini);
	void						loadIOCommINISettings(QSettings& ini);
	void						loadDBINISettigs(QSettings& ini);
	void						writeCameraINISettings(QSettings& ini);
	void						writeDBINISettings(QSettings& ini);
	void						writeGeneralINISettings(QSettings& ini);
	void						writeIOCommINISettings(QSettings& ini, i32 dev_mode);
	void						setIniValue(QSettings& ini, const QString& strkey, const QVariant& value);
    void						registerDataTypes();

	bool						isAppInited();
	bool						isAvailableConnect(i32 conn, i32 conn_mode);
	bool						isLocalConnection();

	void						stopProcessorAll();
	void						disconnectProcessorAll();

	bool						deviceImport(Packet* packet_ptr);
	bool						deviceImportStart(Packet* packet_ptr);
	bool						deviceImportFinish(Packet* packet_ptr);
	bool						deviceImportSetting(Packet* packet_ptr);
	bool						deviceImportJobManager(Packet* packet_ptr);

	bool						deviceExport(PacketPVector& pak_vec);
	Packet*						deviceExportStart();
	Packet*						deviceExportFinish();
	Packet*						deviceExportSetting();
	Packet*						deviceExportJobManager();

	f32							updateUpTime();
	i32							sendProcessorPacket(CProcessor* processor_ptr, Packet* pak, i32 mode = kPUCmdNonBlock);
	void						uploadRuntimeHistory(i32 cmd);
	void						uploadNoneCameraState();
	void						uploadWarningMessage(const powstring& str_en, const powstring& str_ch, const powstring& str_ko);
	bool						setPassword(postring& cur_password, postring& new_password);
	bool						getHeartBeat(HeartBeat& hb);
	bool						checkPassword(postring& password);
	bool						executeSQLQuery(QString& strquery);
	bool						appendToFile(const char* filename, u8* buffer_ptr, i32 len);
	bool						updateDeviceINISettings(QString& lowlevel_path);
	bool						updateDeviceOffline();
	bool						checkQtSignals(i32 signal, bool is_push);
	void						outputRuntimeHistory(i32 job_id = -1);

	CCameraManager*				getCameraManager();
	CJobManager*				getJobManager();
	CSCIOManager*				getIOManager();
	CSCDBManager*				getDBManager();
	CTagManager*				getTagManager();

	DeviceInfo*					getDeviceInfo();
	CIPInfo*					getIPInfo();
	QString						getLowLevelName();
	QString						getHighLevelName();
	Packet*						getRuntimeHistory(i32 cmd);
	CJobUnit*					getProcessorCurJob(i32 pu_id);
	CGlobalVar*					getGlobalVar();

	CProcessor*					getFreeProcessor();		//cam(x), used(x), if nothing cam(o), used(x): processor
	CProcessor*					getUnusedProcessor();	//cam(o), used(x): processor
	CProcessor*					findProcessor(i32 cam_id);
	CProcessor* 				setCameraProcessor(i32 cam_id, bool is_admin, i32 mode);
	bool						setAdminProcessor(CProcessor* processor_ptr, i32 cam_id);
	bool						setAdminFirstCameraProcessor(bool is_force = false);
	bool						freeCameraProcessor(i32 cam_id, i32 cmd);

	void						createTrayGUI();
	void						deleteTrayGUI();

	bool						isOpcUpdating();
	bool						isModbusUpdating();
	bool						isSingleInstance(const char* uuid_str);

	void						setOpcUpdating(bool is_update);
	void						setModbusUpdating(bool is_update);

	void						updateAppOpen();
	void						updateAppClose();
	void						updateStateNetOpen();
	void						updateStateNetClose();
	void						updateIPInfo();
	void						updateRuntimeHistory(i32 mode, void* data_ptr);
	void						updateRuntimeHistory(CJobResult* job_result_ptr);
	void						updateHLParam(Packet* packet_ptr);
	void						updateSyncDateTime(Packet* packet_ptr);
	void						updateTrayIcon(i32 mode);
	void						updateDebugLevelGUI(i32 level);
	void						updateDebugScopeGUI(i32 scope);

	i32							onRequestPreConnect(Packet* packet_ptr, i32 conn);
	i32							onRequestConnect(Packet* packet_ptr, i32 conn);
	void						onReadCmdPacket(Packet* packet_ptr, i32 conn_mode);
	void						onAcceptExtCamera(i32 method, i32 dev, i32 cam);
	void						onAcceptExtProgram(i32 method, i32 dev, i32 program);
	void						onAcceptExtRunStopThread(i32 cam_id, bool is_run_mode);
	void						onAcceptExtRunStopThreadAll(bool is_run_mode);

	i32							onRequestSync(Packet* packet_ptr);
	i32							onRequestUnSync(Packet* packet_ptr);
	i32							onRequestOnline(Packet* packet_ptr);
	i32							onRequestOffline(Packet* packet_ptr);
	i32							onRequestStop(Packet* packet_ptr);
	i32							onRequestPermissionAuth(Packet* packet_ptr);
	i32							onRequestChangeIP(Packet* packet_ptr);
	i32							onRequestChangePassword(Packet* packet_ptr);
	i32							onRequestSelectJob(Packet* packet_ptr);
	i32							onRequestCameraSelect(Packet* packet_ptr);
	i32							onRequestIORawCommand(Packet* packet_ptr);
	i32							onRequestSetting(Packet* packet_ptr);
	i32							onRequestDevSetting(Packet* packet_ptr);
	i32							onRequestIOSetting(Packet* packet_ptr);
	i32							onRequestCommSetting(Packet* packet_ptr);
	i32							onRequestGeneralSetting(Packet* packet_ptr);
	i32							onRequestDateTimeSetting(Packet* packet_ptr);
	i32							onRequestNetworkSetting(Packet* packet_ptr);
	i32							onRequestGetRuntimeHistory(Packet* packet_ptr);
	i32							onRequestResetRuntimeHistory(Packet* packet_ptr);
	i32							onRequestSettingImport(Packet* packet_ptr);
	i32							onRequestSettingExport(Packet* packet_ptr);
	i32							onRequestDeviceUpdate(Packet* packet_ptr);
	i32							onRequestResetFactory(Packet* packet_ptr);
	i32							onRequestDBOperation(Packet* packet_ptr);
	i32							onRequestEmulator(Packet* packet_ptr);

signals:
	void						broadcastPacket(i32 cmd, i32 mode);

public slots:
	void						onChangeDebugScopeApp();
	void						onChangeDebugScopeEvent();
	void						onChangeDebugScopeNetwork();
	void						onChangeDebugScopeIVS();
	void						onChangeDebugScopeCamera();
	void						onChangeDebugScopeEncode();
	void						onChangeDebugScopeIOModule();
	void						onChangeDebugScopeModbus();
	void						onChangeDebugScopeOpc();
	void						onChangeDebugScopeOvx();
	void						onChangeDebugScopeDatabase();
	void						onChangeDebugScopeFtp();
	void						onChangeDebugScopeTag();

	void						onChangeDebugScopeAll();
	void						onChangeDebugScopeClear();

	void						onChangeDebugLevelRing0();
	void						onChangeDebugLevelApp();
	void						onChangeDebugLevelModule();
	void						onChangeDebugLevelEventflow();
	void						onChangeDebugLevelRuntime();

	void						onAcceptExtSignal(i32 dev, i32 method, i32 param1, i32 param2);
	void						onAcceptDevIOStatus(i32 sub_cmd, i32 dev_mode, u8* data_ptr, u8 num);
	void						onAcceptDevExtSel();

	void						onDeviceError(i32 subdev, i32 index, i32 errcode);
	void						onDevicePluged(i32 subdev, i32 index);
	void						onDeviceUnPluged(i32 subdev, i32 index);

	void						onRequestCommand(i32 cmd, i32 cam_id);
	void						onUpdatingToolFromOpc(i32 cam_id);
	void						onUpdatingToolFromModbus(i32 cam_id, i32 addr, i32 size);

public:
	CJobManager*				m_job_manager_ptr;
	CTagManager*				m_tag_manager_ptr;
	CTagUIManager*				m_tag_ui_manager_ptr;
	CCameraManager*				m_cam_manager_ptr;
	CSCIOManager*				m_io_manager_ptr;
	CSCDBManager*				m_db_manager_ptr;
	CSCDBClient*				m_db_client_ptr;

	DeviceInfo					m_device_info;
	CIPInfo						m_ip_info;
	DBConfig					m_db_config;
	CSecuritySetting			m_security_param;
	CGeneralSetting				m_general_param;
	CEngineParam				m_engine_param;
	CameraSetEx					m_camera_param;
	CRuntimeHistory				m_runtime_history;
	CToolCategorys*				m_tool_categorys_ptr;
	CGlobalVar*					m_global_var_ptr;

	std::vector<CProcessor*>	m_pu_array;
	std::atomic<u16>			m_pu_queue[PO_UNIT_COUNT];
	CProcessor*					m_admin_processor_ptr;

	bool						m_app_inited;
	std::atomic<i32>			m_app_state;
	std::atomic<i32>			m_app_mode;
	std::atomic<bool>			m_opc_updating;
	std::atomic<bool>			m_modbus_updating;
	i32							m_qtsig_reference[SC_SIGNAL_COUNT];
	POMutex						m_qtsig_mutex;
	POMutex						m_time_mutex;
	
#if defined(POR_WITH_GUI)
	QMenu*						m_systray_menu_ptr;
	QSystemTrayIcon*			m_systray_icon_ptr;
	
	QAction*					m_action_scope_app_ptr;
	QAction*					m_action_scope_ipc_ptr;
	QAction*					m_action_scope_net_ptr;
	QAction*					m_action_scope_ivs_ptr;
	QAction*					m_action_scope_cam_ptr;
	QAction*					m_action_scope_encode_ptr;
	QAction*					m_action_scope_io_ptr;
	QAction*					m_action_scope_modbus_ptr;
	QAction*					m_action_scope_opc_ptr;
	QAction*					m_action_scope_db_ptr;
	QAction*					m_action_scope_ftp_ptr;
	QAction*					m_action_scope_tag_ptr;
	QAction*					m_action_scope_ovx_ptr;

	QAction*					m_action_scope_all_ptr;
	QAction*					m_action_scope_clear_ptr;

	QAction*					m_action_ring0_ptr;
	QAction*					m_action_app_ptr;
	QAction*					m_action_module_ptr;
	QAction*					m_action_eventflow_ptr;
	QAction*					m_action_runtime_ptr;

	QAction*					m_toggle_offline_ptr;
	QAction*					m_quit_action_ptr;
#endif
};

extern QSemaphore				g_shp_manager;
extern CSCApplication*			g_main_app_ptr;
