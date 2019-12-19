#pragma once

#include "types.h"
#include "client.h"
#include "Packet.h"
#include "struct.h"
#include "tag_ui.h"

#include "sc_struct.h"
#include "struct/camera_setting.h"
#include "tool_category/tool_category.h"

#include "job_core.h"
#include "job_result_core.h"
#include "runtime_history.h"

#include "expl_struct.h"

class CSyncInfo
{
public:
	CSyncInfo();
	~CSyncInfo();

	void					initInstance();
	void					exitInstance();

	i32						memSize();
	bool					memSettingRead(u8* &buffer_ptr, i32 &buffer_size);
	bool					memJobRead(u8* &buffer_ptr, i32 &buffer_size);
	bool					memWrite(u8* &buffer_ptr, i32 &buffer_size);

	bool					isEmulator() {return m_device_info.is_emulator;}

public:
	postring				m_device_version;
	bool					m_is_emulator;
	bool					m_is_ivsmode;

	DeviceInfo				m_device_info;
	CameraSet				m_camera_set;
	CTagNode				m_cam_tag_node;
	CSecuritySetting		m_security_setting;
	CGeneralSetting			m_general_setting;
	CEngineParam			m_engine_param;
	CIPInfo					m_ip_info;
	DBConfig				m_db_config;
	IOReservedSetting		m_io_resv_setting;
	CTagNode				m_io_tag_node;
	CIOInput				m_io_input;
	std::vector<postring>	m_modbus_ports;
	std::vector<postring>	m_modbus_disp_ports;
	CIODevParam				m_io_dev;
	CModbusDev				m_modbus_dev;
	ModbusAllocation		m_modbus_alloc;
	CFTPDevGroup			m_fpt_dev_group;
	COpcDev					m_opc_dev;
	CTagUINode				m_tagui_node;
	CToolCategorys			m_tool_categorys;

	CRuntimeHistory			m_statistics;

	int						m_job_id;
	std::vector<CJobCore*>	m_jobcore_ptr_vector;
	CTagNode				m_job_tag_node;
};

