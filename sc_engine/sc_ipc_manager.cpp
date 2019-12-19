#include "sc_ipc_manager.h"
#include "base.h"

CSCIPCManager::CSCIPCManager()
{
}

CSCIPCManager::~CSCIPCManager()
{
}

void* CSCIPCManager::onEncodedFrame(u8* data_ptr, i32 size, i64 pts, ImageData* img_data_ptr)
{
	if (!isPaired() || img_data_ptr == NULL)
	{
		return NULL;
	}

	i32 len = 0;
	i32 cam_id = size;

	len += sizeof(cam_id);
	len += sizeof(img_data_ptr->time_stamp);
	len += img_data_ptr->memImgSize();

	u8* buffer_ptr;
	i32 buffer_size = len;
	Packet* packet_ptr = po_new Packet(kSCCmdCameraImage, kPOPacketRespOK);
	packet_ptr->setSubCmd(kSCSubTypeImageRaw);
	packet_ptr->allocateBuffer(len, buffer_ptr);

	CPOBase::memWrite(cam_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(img_data_ptr->time_stamp, buffer_ptr, buffer_size);
	img_data_ptr->memImgWrite(buffer_ptr, buffer_size);
	return packet_ptr;
}

void CSCIPCManager::onSendFrame(void* send_void_ptr)
{
	if (!isPaired() || send_void_ptr == NULL)
	{
		return;
	}
	send(PacketRef((Packet*)send_void_ptr));
}