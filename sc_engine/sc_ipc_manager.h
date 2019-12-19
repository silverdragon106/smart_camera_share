#pragma once

#include "sc_define.h"
#include "streaming/ipc_manager.h"

class CSCIPCManager : public CIPCManager
{
	Q_OBJECT

public:
	CSCIPCManager();
	~CSCIPCManager();
	
	virtual void*			onEncodedFrame(u8* buffer_ptr, i32 size, i64 pts, ImageData* img_data_ptr);
	virtual void			onSendFrame(void* send_void_ptr);
};
