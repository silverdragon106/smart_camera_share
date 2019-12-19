#pragma once
#include "sc_define.h"
#include "tool_core.h"

class CJobResultCore
{
public:
	CJobResultCore();
	virtual ~CJobResultCore();
	
	virtual void				initBuffer();
	virtual void				freeBuffer();

	virtual void				copyFrom(CJobResultCore &job_result_core);

	i32							memSize(i32 mode);
	bool 						memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size);
	bool						memRead(i32 mode, u8*& buffer_ptr, i32& buffer_size);

	virtual i32					memResultSize(i32 mode);
	virtual i32					memResultRead(i32 mode, u8*& buffer_ptr, i32& buffer_size);
	virtual i32					memResultWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size);

	inline ResultType			getResult() const { return m_result; };
	inline i32					getCamID() const { return m_cam_id; };
	inline i32					getJobID() const { return m_job_id; };
	inline i32					getJobSelID() const { return m_job_sel_id; };
	inline powstring			getJobName() const { return m_job_name; };
	
	inline i64					getTimeStamp() const { return m_time_stamp; };
	inline i32					getProcTime() const { return m_process_time; };
	inline i32					getImgProcTime() const { return m_image_proc_time; };
	inline i32					getToolResultCount() const { return (i32)m_tool_result_vec.size(); };

	inline bool					isResultValid() { return m_result != kResultTypeNone; };
	inline bool					isResultPassed() { return m_result == kResultTypePassed; };
	inline bool					isResultFailed() { return m_result == kResultTypeFailed; };

public:
	ResultType					m_result;
	i32							m_cam_id;
	i32							m_job_id;
	i32							m_job_sel_id;
	powstring					m_job_name;

	i64							m_time_stamp;			//ms, when image is triggered
	i64							m_trigger_time_stamp;	//ms, when trigger signal is accepted
	i32							m_process_time;			//ms
	i32							m_image_proc_time;		//ms

	CToolResultVector			m_tool_result_vec;
};
