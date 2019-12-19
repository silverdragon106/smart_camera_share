#pragma once
#include "sc_struct.h"
#include "tool_core.h"

enum JobEditType
{
	kJobEditNone = 0,
	kJobEditNew,
	kJobEditUpdate,
	kJobEditUpdateThumb
};

enum JobContentType
{
	kJobNone		= 0x00,
	kJobMeta		= 0x01,
	kJobBody		= 0x02,
	kJobNewFlag		= 0x80,

	kJobFree		= kJobBody,
	kJobContent		= (kJobMeta | kJobBody),
	kJobNew			= (kJobContent | kJobNewFlag)
};

enum JobDataTypes
{
	kJobDataNone			= 0x00,
	kJobDataOneTool			= 0x01,
	kJobDataAllTools		= 0x02,
	kJobDataUpdatedTools	= 0x04, 
	kJobDataCamSetting		= 0x08,
	kJobDataIOSetting		= 0x10
};

const i32 kJobToolMaxCount = 1024;
const i32 kJobRawSignCode = 0x2AF7;

class CJobCore
{
public:
	CJobCore();
	virtual ~CJobCore();

	virtual void				initBuffer();
	virtual void				freeBuffer();
	virtual void				copyFrom(CJobCore &job_core);
	virtual void				copyMeta(CJobCore &job_core);

	i32							memSize();
	bool						memRead(u8*& buffer_ptr, i32& buffer_size);
	bool						memWrite(u8*& buffer_ptr, i32& buffer_size);

	i32							memInfoSize();
	i32							memInfoRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memInfoWrite(u8*& buffer_ptr, i32& buffer_size);

	i32							memMetaSize();
	i32							memMetaRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memMetaWrite(u8*& buffer_ptr, i32& buffer_size);

	i32							memToolSize(i32 mode);
	i32							memToolRead(i32 mode, u8*& buffer_ptr, i32& buffer_size);
	i32							memToolWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size);

	i32							memDataSize(i32 mode, const i32vector& tool_index_vec);
	i32							memDataRead(i32 mode, const i32vector& tool_index_vec, u8*& buffer_ptr, i32& buffer_size);
	i32							memDataWrite(i32 mode, const i32vector& tool_index_vec, u8*& buffer_ptr, i32& buffer_size);

public: 
	//job thumbnail data
	i32							m_job_id;
	i32							m_cam_id;
	i32							m_select_id;
	DateTime					m_created_time;
	DateTime					m_updated_time;
	Img							m_thumbnail;

	powstring					m_job_name;

	//job data
	CameraSetting				m_cam_setting;
	CIOOutput					m_do_setting;
	CModBusComm					m_modbus_comm;
	CFtpComm					m_ftp_comm;
	COpcComm					m_opc_comm;

	i32							m_tool_count;
	CToolVector					m_tool_vec;
};
