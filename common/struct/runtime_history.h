#pragma once
#include "struct.h"
#include "lock_guide.h"

struct StatResult
{
	i64						pass_count;
	i64						ng_count;

public:
	StatResult();
	~StatResult();

	void					init();
	StatResult&				operator+=(const StatResult& other);
};

struct JobStatResult
{
	i64						snap_count;
	i32						cam_id;
	i32						job_sel_id;
	i32						job_id;
	powstring				job_name;
	StatResult				job_result;
	f32						cur_proc_time;	//s
	f32						avg_proc_time;	//s
	f32						min_proc_time;	//s
	f32						max_proc_time;	//s

public:
	JobStatResult();
    JobStatResult(i32 cam_id, i32 job_id, i32 job_sel_id, const powstring& job_name, const StatResult& stat_result, f32 proc_time);

	void					update(StatResult& stat_result, f32 proc_time);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);

	inline i32				getCamID() { return cam_id; };
};
typedef std::map<i32, JobStatResult> JobStatResultMap;

struct CRuntimeHistory : public CLockGuard
{
	// Accumulation history information
	i64						m_snap_total;
	i32						m_boot_count;
	f32						m_acc_uptime;		//s
	f32						m_bak_acc_uptime;	//s, temporary... but it's used to backup acctime in file

	// Runtime history
	f32						m_uptime;			//s
	i64						m_snap_count;
	JobStatResultMap		m_job_stat_map;

public:
	CRuntimeHistory();
	~CRuntimeHistory();

	void					init();
	void					initRuntime();
	void					initRuntime(i32 cam_id);
	JobStatResult*			findJobHistory(i32 job_id);
	void					update();

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};
