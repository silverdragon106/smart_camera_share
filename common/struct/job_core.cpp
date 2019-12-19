#include "job_core.h"
#include "base.h"
#include "tool_core.h"

#if defined(POR_DEVICE)
#include "tool_data.h"
#endif

CJobCore::CJobCore()
{
}

CJobCore::~CJobCore()
{
	freeBuffer();
}

void CJobCore::initBuffer()
{
	m_job_id = -1;
	m_cam_id = -1;
	m_select_id = -1;
	memset(&m_created_time, 0, sizeof(DateTime));
	memset(&m_updated_time, 0, sizeof(DateTime));
	
	m_job_name = L"";
	m_cam_setting.init();
	m_do_setting.init();
	m_modbus_comm.init();
	m_ftp_comm.init();
	m_opc_comm.init();

	CJobCore::freeBuffer();
}

void CJobCore::freeBuffer()
{
	m_tool_count = 0;
	for (i32 i = 0; i < m_tool_vec.size(); i++)
	{
		POSAFE_DELETE(m_tool_vec[i]);
	}

	m_tool_vec.clear();
}

void CJobCore::copyFrom(CJobCore &job_core)
{
#if defined(POR_EXPLORER)
	freeBuffer();

	m_job_id = job_core.m_job_id;
	m_cam_id = job_core.m_cam_id;
	m_select_id = job_core.m_select_id;
	m_created_time = job_core.m_created_time;
	m_updated_time = job_core.m_updated_time;

	Img &thumbnail = job_core.m_thumbnail;
	m_thumbnail.copyImage(thumbnail);
	m_job_name = job_core.m_job_name;
	
	m_cam_setting.setValue(&job_core.m_cam_setting, kCamSettingUpdateAll);
	m_do_setting = job_core.m_do_setting;
	m_modbus_comm = job_core.m_modbus_comm;
	m_ftp_comm = job_core.m_ftp_comm;
	m_opc_comm = job_core.m_opc_comm;
	
	m_tool_count = job_core.m_tool_count;
	CToolVector &tool_vec = job_core.m_tool_vec;
	CToolVector::iterator iter;
	for (iter = tool_vec.begin(); iter != tool_vec.end(); ++iter)
	{
		CToolCore *tool_core_ptr = po_new CToolCore;
		tool_core_ptr->copyFrom(*(*iter));

		m_tool_vec.push_back(tool_core_ptr);
	}
#endif
}

void CJobCore::copyMeta(CJobCore &job_core)
{
#if defined(POR_EXPLORER)
	m_job_id = job_core.m_job_id;
	m_cam_id = job_core.m_cam_id;
	m_select_id = job_core.m_select_id;
	m_created_time = job_core.m_created_time;
	m_updated_time = job_core.m_updated_time;

	Img &thumbnail = job_core.m_thumbnail;
	m_thumbnail.copyImage(thumbnail);
	m_job_name = job_core.m_job_name;

	m_tool_count = job_core.m_tool_count;
#endif
}

i32 CJobCore::memInfoSize()
{
	i32 len = 0;

#if defined(POR_DEVICE)
	len += sizeof(m_job_id);
	len += sizeof(m_cam_id);
	len += sizeof(m_select_id);
#endif
	len += CPOBase::getStringMemSize(m_job_name);

	return len;
}

i32 CJobCore::memInfoWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

#if defined(POR_DEVICE)
	CPOBase::memWrite(m_job_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_cam_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_select_id, buffer_ptr, buffer_size);
#endif
	CPOBase::memWrite(buffer_ptr, buffer_size, m_job_name);
	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memInfoRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

#if !defined(POR_DEVICE)
	CPOBase::memRead(m_job_id, buffer_ptr, buffer_size);
	CPOBase::memRead(m_cam_id, buffer_ptr, buffer_size);
	CPOBase::memRead(m_select_id, buffer_ptr, buffer_size);
#endif
	CPOBase::memRead(buffer_ptr, buffer_size, m_job_name);
	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memMetaSize()
{
	i32 len = memInfoSize();
#if defined(POR_DEVICE)
	len += sizeof(m_tool_count);
	len += sizeof(m_created_time);
	len += sizeof(m_updated_time);
	len += m_thumbnail.memSize();
#endif
	return len;
}

i32 CJobCore::memMetaWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	memInfoWrite(buffer_ptr, buffer_size);

#if defined(POR_DEVICE)
	CPOBase::memWrite(m_tool_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_created_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_updated_time, buffer_ptr, buffer_size);
	m_thumbnail.memWrite(buffer_ptr, buffer_size);
#endif
	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memMetaRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	memInfoRead(buffer_ptr, buffer_size);

#if !defined(POR_DEVICE)
	CPOBase::memRead(m_tool_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_created_time, buffer_ptr, buffer_size);
	CPOBase::memRead(m_updated_time, buffer_ptr, buffer_size);
	m_thumbnail.memRead(buffer_ptr, buffer_size);
#endif
	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memSize()
{
	i32 len = 0;

	len += sizeof(i32) * 2;
	len += memMetaSize();

	len += m_cam_setting.memSize();
	len += m_do_setting.memSize();
	len += m_modbus_comm.memSize();
	len += m_ftp_comm.memSize();
	len += m_opc_comm.memSize();
	len += memToolSize(kToolWithAll);
	return len;
}

bool CJobCore::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	CPOBase::memSignWrite(buffer_ptr, buffer_size, kJobRawSignCode);

	//write job thumb
 	memMetaWrite(buffer_ptr, buffer_size);
 
 	//write job content
 	m_cam_setting.memWrite(buffer_ptr, buffer_size);
 	m_do_setting.memWrite(buffer_ptr, buffer_size);
 	m_modbus_comm.memWrite(buffer_ptr, buffer_size);
 	m_ftp_comm.memWrite(buffer_ptr, buffer_size);
	m_opc_comm.memWrite(buffer_ptr, buffer_size);
	memToolWrite(kToolWithAll, buffer_ptr, buffer_size);

	CPOBase::memSignWrite(buffer_ptr, buffer_size, kJobRawSignCode);
	return true;
}

bool CJobCore::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, kJobRawSignCode))
	{
		return false;
	}

	//read job thumb
	memMetaRead(buffer_ptr, buffer_size);

	//read job content
	m_cam_setting.memRead(buffer_ptr, buffer_size);
	m_do_setting.memRead(buffer_ptr, buffer_size);
	m_modbus_comm.memRead(buffer_ptr, buffer_size);
	m_ftp_comm.memRead(buffer_ptr, buffer_size);
	m_opc_comm.memRead(buffer_ptr, buffer_size);
	memToolRead(kToolWithAll, buffer_ptr, buffer_size);

	bool ret = CPOBase::memSignRead(buffer_ptr, buffer_size, kJobRawSignCode);
	return ret;
}

i32 CJobCore::memToolSize(i32 mode)
{
	i32 len = 0;
	i32 i, count = (i32)m_tool_vec.size();
	for (i = 0; i < count; i++)
	{
		len += m_tool_vec[i]->memSize(mode);
	}

	len += sizeof(count);
	return len;
}

i32 CJobCore::memToolRead(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1, tool_count = 0;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(count))
	{
		CToolCore* tool_ptr;
		m_tool_vec.resize(count, NULL);
		for (i = 0; i < count; i++)
		{
#if defined(POR_DEVICE)
			tool_ptr = CBaseTool::newMemRead(mode, buffer_ptr, buffer_size);
#else
			tool_ptr = po_new CToolCore();
			tool_ptr->memRead(mode, buffer_ptr, buffer_size);
#endif
			if (!tool_ptr)
			{
				continue;
			}

			m_tool_vec[tool_count] = tool_ptr;
			tool_count++;
		}
		m_tool_vec.resize(tool_count);
	}
	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memToolWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 i, count = (i32)m_tool_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_tool_vec[i]->memWrite(mode, buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memDataSize(i32 mode, const i32vector& tool_index_vec)
{
	i32 len = 0;
	len += memInfoSize();
	if (CPOBase::bitCheck(mode, kJobDataOneTool))
	{
		if (tool_index_vec.size() <= 0)
		{
			return 0;
		}

		i32 index = tool_index_vec[0];
		if (CPOBase::checkIndex(index, (i32)m_tool_vec.size()))
		{
			len += m_tool_vec[index]->memSize(kToolWithAll);
		}
	}
	else if (CPOBase::bitCheck(mode, kJobDataAllTools))
	{
		len += memToolSize(kToolWithAll);
	}
	else if (CPOBase::bitCheck(mode, kJobDataUpdatedTools))
	{
		i32 i, index, count = (i32)tool_index_vec.size();
		if (count <= 0)
		{
			return 0;
		}

		for (i = 0; i < count; i++)
		{
			index = tool_index_vec[i];
			if (CPOBase::checkIndex(index, (i32)m_tool_vec.size()))
			{
				len += m_tool_vec[index]->memSize(kToolWithAll);
			}
		}
	}

	if (CPOBase::bitCheck(mode, kJobDataCamSetting))
	{
		len += m_cam_setting.memSize();
	}
	if (CPOBase::bitCheck(mode, kJobDataIOSetting))
	{
		len += m_do_setting.memSize();
		len += m_modbus_comm.memSize();
		len += m_ftp_comm.memSize();
		len += m_opc_comm.memSize();
	}
	return len;
}

i32 CJobCore::memDataRead(i32 mode, const i32vector& tool_index_vec, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	memInfoRead(buffer_ptr, buffer_size);
	if (CPOBase::bitCheck(mode, kJobDataOneTool))
	{
		if (tool_index_vec.size() > 0)
		{
			i32 index = tool_index_vec[0];
			if (CPOBase::checkIndex(index, (i32)m_tool_vec.size()))
			{
				m_tool_vec[index]->memRead(kToolWithAll, buffer_ptr, buffer_size);
			}
		}
	}
	else if (CPOBase::bitCheck(mode, kJobDataAllTools))
	{
		memToolRead(kToolWithAll, buffer_ptr, buffer_size);
	}
	else if (CPOBase::bitCheck(mode, kJobDataUpdatedTools))
	{
		i32 i, index;
		i32 count = (i32)tool_index_vec.size();
		for (i = 0; i < count; i++)
		{
			index = tool_index_vec[i];
			if (CPOBase::checkIndex(index, (i32)m_tool_vec.size()))
			{
				m_tool_vec[index]->memRead(kToolWithAll, buffer_ptr, buffer_size);
			}
		}
	}

	if (CPOBase::bitCheck(mode, kJobDataCamSetting))
	{
		m_cam_setting.memRead(buffer_ptr, buffer_size);
	}
	if (CPOBase::bitCheck(mode, kJobDataIOSetting))
	{
		m_do_setting.memRead(buffer_ptr, buffer_size);
		m_modbus_comm.memRead(buffer_ptr, buffer_size);
		m_ftp_comm.memRead(buffer_ptr, buffer_size);
		m_opc_comm.memRead(buffer_ptr, buffer_size);
	}

	return buffer_ptr - buffer_pos;
}

i32 CJobCore::memDataWrite(i32 mode, const i32vector& tool_index_vec, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	memInfoWrite(buffer_ptr, buffer_size);
	if (CPOBase::bitCheck(mode, kJobDataOneTool))
	{
		if (tool_index_vec.size() > 0)
		{
			i32 index = tool_index_vec[0];
			if (CPOBase::checkIndex(index, (i32)m_tool_vec.size()))
			{
				m_tool_vec[index]->memWrite(kToolWithAll, buffer_ptr, buffer_size);
			}
		}
	}
	else if (CPOBase::bitCheck(mode, kJobDataAllTools))
	{
		memToolWrite(kToolWithAll, buffer_ptr, buffer_size);
	}
	else if (CPOBase::bitCheck(mode, kJobDataUpdatedTools))
	{
		i32 i, index, count = (i32)tool_index_vec.size();
		for (i = 0; i < count; i++)
		{
			index = tool_index_vec[i];
			if (CPOBase::checkIndex(index, (i32)m_tool_vec.size()))
			{
				m_tool_vec[index]->memWrite(kToolWithAll, buffer_ptr, buffer_size);
			}
		}
	}

	if (CPOBase::bitCheck(mode, kJobDataCamSetting))
	{
		m_cam_setting.memWrite(buffer_ptr, buffer_size);
	}
	if (CPOBase::bitCheck(mode, kJobDataIOSetting))
	{
		m_do_setting.memWrite(buffer_ptr, buffer_size);
		m_modbus_comm.memWrite(buffer_ptr, buffer_size);
		m_ftp_comm.memWrite(buffer_ptr, buffer_size);
		m_opc_comm.memWrite(buffer_ptr, buffer_size);
	}

	return buffer_ptr - buffer_pos;
}