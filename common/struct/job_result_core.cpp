#include "job_result_core.h"
#include "base.h"
#include "tool_core.h"

CJobResultCore::CJobResultCore()
{
	initBuffer();
}

CJobResultCore::~CJobResultCore()
{
	freeBuffer();
}

void CJobResultCore::initBuffer()
{
	m_result = kResultTypeNone;
	m_cam_id = -1;
	m_job_id = -1;
	m_job_sel_id = -1;
	m_job_name = L"";

	m_time_stamp = 0;
	m_trigger_time_stamp = 0;
	m_process_time = 0;
	m_image_proc_time = 0;

	freeBuffer();
}

void CJobResultCore::freeBuffer()
{
	POSAFE_CLEAR(m_tool_result_vec);
}

void CJobResultCore::copyFrom(CJobResultCore &job_result_core)
{
#if defined(POR_EXPLORER)
	m_result = job_result_core.m_result;
	m_cam_id = job_result_core.m_cam_id;
	m_job_id = job_result_core.m_job_id;
	m_job_sel_id = job_result_core.m_job_sel_id;
	m_job_name = job_result_core.m_job_name;

	m_time_stamp = job_result_core.m_time_stamp;
	m_trigger_time_stamp = job_result_core.m_trigger_time_stamp;
	m_process_time = job_result_core.m_process_time;
	m_image_proc_time = job_result_core.m_image_proc_time;

	POSAFE_CLEAR(m_tool_result_vec);
	CToolResultVector &tool_result_vec = job_result_core.m_tool_result_vec;
	CToolResultVector::iterator iter;
	for (iter = tool_result_vec.begin(); iter != tool_result_vec.end(); ++iter)
	{
		CToolResultCore *tool_result_ptr = po_new CToolResultCore;
		tool_result_ptr->copyFrom(*(*iter));

		m_tool_result_vec.push_back(tool_result_ptr);
	}
#endif
}

i32 CJobResultCore::memResultSize(i32 mode)
{
	i32 len = 0;
	i32 i, count = (i32)m_tool_result_vec.size();
	for (i = 0; i < count; i++)
	{
		len += m_tool_result_vec[i]->memSize(mode);
	}
	len += sizeof(count);
	return len;
}

i32 CJobResultCore::memResultRead(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	POSAFE_CLEAR(m_tool_result_vec);

	i32 i, count = -1;
	if (!CPOBase::memRead(count, buffer_ptr, buffer_size) || !CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	m_tool_result_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		m_tool_result_vec[i] = po_new CToolResultCore();
		m_tool_result_vec[i]->memRead(mode, buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CJobResultCore::memResultWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 i, count = (i32)m_tool_result_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_tool_result_vec[i]->memWrite(mode, buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CJobResultCore::memSize(i32 mode)
{
	i32 len = 0;

	len += sizeof(i32) * 2;
	len += sizeof(m_result);
	len += sizeof(m_cam_id);
	len += sizeof(m_job_id);
	len += sizeof(m_job_sel_id);
	len += CPOBase::getStringMemSize(m_job_name);

	len += sizeof(m_time_stamp);
	len += sizeof(m_trigger_time_stamp);
	len += sizeof(m_process_time);
	len += sizeof(m_image_proc_time);

	len += memResultSize(mode);
	return len;
}

bool CJobResultCore::memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	CPOBase::memSignWrite(buffer_ptr, buffer_size, PO_SIGN_CODE);
	CPOBase::memWrite(m_result, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_cam_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_job_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_job_sel_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, m_job_name);

	CPOBase::memWrite(m_time_stamp, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_trigger_time_stamp, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_process_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_image_proc_time, buffer_ptr, buffer_size);

	memResultWrite(mode, buffer_ptr, buffer_size);
	CPOBase::memSignWrite(buffer_ptr, buffer_size, PO_SIGN_ENDCODE);
	return true;
}

bool CJobResultCore::memRead(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::memRead(m_result, buffer_ptr, buffer_size);
	CPOBase::memRead(m_cam_id, buffer_ptr, buffer_size);
	CPOBase::memRead(m_job_id, buffer_ptr, buffer_size);
	CPOBase::memRead(m_job_sel_id, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, m_job_name);

	CPOBase::memRead(m_time_stamp, buffer_ptr, buffer_size);
	CPOBase::memRead(m_trigger_time_stamp, buffer_ptr, buffer_size);
	CPOBase::memRead(m_process_time, buffer_ptr, buffer_size);
	CPOBase::memRead(m_image_proc_time, buffer_ptr, buffer_size);

	memResultRead(mode, buffer_ptr, buffer_size);
	return CPOBase::memSignRead(buffer_ptr, buffer_size, PO_SIGN_ENDCODE);
}