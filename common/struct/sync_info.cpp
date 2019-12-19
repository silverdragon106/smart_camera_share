#include "sync_info.h"


CSyncInfo::CSyncInfo()
{
	m_is_ivsmode = false;
	m_is_emulator = false;
}


CSyncInfo::~CSyncInfo()
{
	exitInstance();
}

void CSyncInfo::initInstance()
{
}

void CSyncInfo::exitInstance()
{
	POSAFE_CLEAR(m_jobcore_ptr_vector);
}

bool CSyncInfo::memSettingRead(u8* &buffer_ptr, i32 &buffer_size)
{
	m_device_info.memRead(buffer_ptr, buffer_size);
	m_camera_set.memRead(buffer_ptr, buffer_size);
	m_cam_tag_node.memTagInfoRead(buffer_ptr, buffer_size);
	m_security_setting.memRead(buffer_ptr, buffer_size);
	m_general_setting.memRead(buffer_ptr, buffer_size);
	//m_engine_param.memRead(buffer_ptr, buffer_size);
	m_db_config.memRead(buffer_ptr, buffer_size);
	m_tool_categorys.memRead(buffer_ptr, buffer_size);
	m_io_resv_setting.memRead(buffer_ptr, buffer_size);
	m_io_tag_node.memTagInfoRead(buffer_ptr, buffer_size);
	CPOBase::memReadStrVector(m_modbus_ports, buffer_ptr, buffer_size);
	CPOBase::memReadStrVector(m_modbus_disp_ports, buffer_ptr, buffer_size);
	m_io_dev.memRead(buffer_ptr, buffer_size);
	m_io_input.memRead(buffer_ptr, buffer_size);
	m_io_input.memExtSelRead(buffer_ptr, buffer_size);
	m_modbus_dev.memRead(buffer_ptr, buffer_size);
	m_modbus_alloc.memRead(buffer_ptr, buffer_size);
	m_fpt_dev_group.memRead(buffer_ptr, buffer_size);
	m_opc_dev.memRead(buffer_ptr, buffer_size);
	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, PO_SIGN_CODE))
	{
		printlog_lv0("->[Error] Reading SyncSetting Error.");
		return false;

	}
	m_tagui_node.memRead(buffer_ptr, buffer_size);
	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, PO_SIGN_ENDCODE))
	{
		printlog_lv0("->[Error] Reading SyncSetting Error in Reading TagUINode.");
		return false;
	}

	return true;
}

bool CSyncInfo::memJobRead(u8* &buffer_ptr, i32 &buffer_size)
{
	exitInstance();
	m_job_id = -1;
	int size = 0;
	CPOBase::memRead(size, buffer_ptr, buffer_size);
	CPOBase::memRead(m_job_id, buffer_ptr, buffer_size);

	for (int i = 0; i < size; i++)
	{
		if (buffer_size <= 0)
		{
			return false;
		}

		CJobCore *job_ptr = new CJobCore();
		job_ptr->memInfoRead(buffer_ptr, buffer_size);

		m_jobcore_ptr_vector.push_back(job_ptr);
	}
	m_job_tag_node.memTagInfoRead(buffer_ptr, buffer_size);

	return true;
}
