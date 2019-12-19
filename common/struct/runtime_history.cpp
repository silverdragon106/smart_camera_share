#include "runtime_history.h"
#include "base.h"

//////////////////////////////////////////////////////////////////////////
StatResult::StatResult()
{
	init();
}

StatResult::~StatResult()
{
}

void StatResult::init()
{
	memset(this, 0, sizeof(StatResult));
}

StatResult& StatResult::operator+=(const StatResult& other)
{
	this->pass_count += other.pass_count;
	this->ng_count += other.ng_count;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
JobStatResult::JobStatResult()
{
	snap_count = 0;
	cam_id = 0;
	job_sel_id = 0;
	job_id = 0;
	job_name.clear();
	job_result.init();

	cur_proc_time = 0;
	avg_proc_time = 0;
	min_proc_time = 0;
	max_proc_time = 0;
}

JobStatResult::JobStatResult(i32 cam_id, i32 job_id, i32 job_sel_id, const powstring& job_name, const StatResult& stat_result, f32 proc_time)
{
	this->cam_id = cam_id;
	this->job_id = job_id;
	this->job_sel_id = job_sel_id;
	this->job_name = job_name;
	this->job_result = stat_result;

	snap_count = 1;
	cur_proc_time = proc_time;
	avg_proc_time = proc_time;
	min_proc_time = proc_time;
	max_proc_time = proc_time;
}

void JobStatResult::update(StatResult& stat_result, f32 proc_time)
{
	job_result += stat_result;
	cur_proc_time = proc_time;
	max_proc_time = po::_max(proc_time, max_proc_time);
	min_proc_time = po::_min(proc_time, min_proc_time);
	avg_proc_time = (avg_proc_time*snap_count + proc_time) / (snap_count + 1);
	snap_count++;
}

i32 JobStatResult::memSize()
{
	i32 len = 0;
	len += sizeof(snap_count);
	len += sizeof(cam_id);
	len += sizeof(job_sel_id);
	len += sizeof(job_id);
	len += CPOBase::getStringMemSize(job_name);
	len += sizeof(job_result);
	len += sizeof(cur_proc_time);
	len += sizeof(avg_proc_time);
	len += sizeof(min_proc_time);
	len += sizeof(max_proc_time);
	return len;
}

i32 JobStatResult::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(snap_count, buffer_ptr, buffer_size);
	CPOBase::memRead(cam_id, buffer_ptr, buffer_size);
	CPOBase::memRead(job_sel_id, buffer_ptr, buffer_size);
	CPOBase::memRead(job_id, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, job_name);
	CPOBase::memRead(job_result, buffer_ptr, buffer_size);
	CPOBase::memRead(cur_proc_time, buffer_ptr, buffer_size);
	CPOBase::memRead(avg_proc_time, buffer_ptr, buffer_size);
	CPOBase::memRead(min_proc_time, buffer_ptr, buffer_size);
	CPOBase::memRead(max_proc_time, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 JobStatResult::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(snap_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(cam_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(job_sel_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(job_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, job_name);
	CPOBase::memWrite(job_result, buffer_ptr, buffer_size);
	CPOBase::memWrite(cur_proc_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(avg_proc_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(min_proc_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(max_proc_time, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
CRuntimeHistory::CRuntimeHistory()
{
	init();
}

CRuntimeHistory::~CRuntimeHistory()
{
}

void CRuntimeHistory::initRuntime()
{
	m_snap_count = 0;
	m_job_stat_map.clear();
}

void CRuntimeHistory::initRuntime(i32 cam_id)
{
	if (!CPOBase::checkIndex(cam_id, PO_CAM_COUNT))
	{
		return;
	}

	JobStatResultMap::iterator iter = m_job_stat_map.begin();
	while (iter != m_job_stat_map.end())
	{
		JobStatResult& stat_result = iter->second;
		if (stat_result.cam_id == cam_id)
		{
			iter = m_job_stat_map.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	update();
}

void CRuntimeHistory::init()
{
	m_snap_total = 0;
	m_boot_count = 0;
	m_acc_uptime = 0;
	m_bak_acc_uptime = 0;
	m_uptime = 0;

	initRuntime();
}

void CRuntimeHistory::update()
{
	i32 snap_count = 0;
	JobStatResultMap::iterator iter;
	for (iter = m_job_stat_map.begin(); iter != m_job_stat_map.end(); ++iter)
	{
		JobStatResult& stat_result = iter->second;
		snap_count += stat_result.snap_count;
	}

	m_snap_count = snap_count;
}

i32	CRuntimeHistory::memSize()
{
	i32 len = 0;
	len += sizeof(m_snap_total);
	len += sizeof(m_boot_count);
	len += sizeof(m_acc_uptime);

	len += sizeof(m_uptime);
	len += sizeof(m_snap_count);

	JobStatResultMap::iterator iter;
	for (iter = m_job_stat_map.begin(); iter != m_job_stat_map.end(); ++iter)
	{
		len += iter->second.memSize();
	}
	len += sizeof(i32);
	return len;
}

i32 CRuntimeHistory::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_snap_total, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_boot_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_acc_uptime, buffer_ptr, buffer_size);

	CPOBase::memWrite(m_uptime, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_snap_count, buffer_ptr, buffer_size);

	i32 count = (i32)m_job_stat_map.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	JobStatResultMap::iterator iter;
	for (iter = m_job_stat_map.begin(); iter != m_job_stat_map.end(); ++iter)
	{
		(iter->second).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CRuntimeHistory::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_snap_total, buffer_ptr, buffer_size);
	CPOBase::memRead(m_boot_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_acc_uptime, buffer_ptr, buffer_size);

	CPOBase::memRead(m_uptime, buffer_ptr, buffer_size);
	CPOBase::memRead(m_snap_count, buffer_ptr, buffer_size);

	i32 count = 0;
	m_job_stat_map.clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	for (i32 i = 0; i < count; i++)
	{
		JobStatResult job_state;
		job_state.memRead(buffer_ptr, buffer_size);
		m_job_stat_map[job_state.job_id] = job_state;
	}

	return buffer_ptr - buffer_pos;
}

bool CRuntimeHistory::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_snap_total, fp);
	CPOBase::fileRead(m_boot_count, fp);
	CPOBase::fileRead(m_bak_acc_uptime, fp);

	m_boot_count++;
	m_acc_uptime = m_bak_acc_uptime;
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CRuntimeHistory::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_snap_total, fp);
	CPOBase::fileWrite(m_boot_count, fp);
	CPOBase::fileWrite(m_acc_uptime, fp);

	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

JobStatResult* CRuntimeHistory::findJobHistory(i32 job_id)
{
	if (job_id < 0)
	{
		return NULL;
	}

	JobStatResultMap::iterator iter_job_result = m_job_stat_map.find(job_id);
	if (iter_job_result == m_job_stat_map.end())
	{
		return NULL;
	}
	return &(iter_job_result->second);
}
