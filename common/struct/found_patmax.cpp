#include "found_patmax.h"
#include "base.h"

CFoundPatMax::CFoundPatMax()
{
	init();
}

CFoundPatMax::~CFoundPatMax()
{
}

void CFoundPatMax::init()
{
	m_rotation = 0;
	m_scale = 1.0f;
	m_refer_point = vector2df();
	m_draw_pattern.clear();
}

void CFoundPatMax::freeBuffer()
{
	m_draw_pattern.clear();
}

void CFoundPatMax::setValue(CFoundPatMax* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	init();
	m_rotation = other_ptr->m_rotation;
	m_scale = other_ptr->m_scale;
	m_refer_point = other_ptr->m_refer_point;
	m_draw_pattern = other_ptr->m_draw_pattern;
}

i32 CFoundPatMax::memSize()
{
	i32 len = 0;
	len += sizeof(m_rotation);
	len += sizeof(m_scale);
	len += sizeof(m_refer_point);
	len += CPOBase::getVectorMemSize(m_draw_pattern);
	return len;
}

i32 CFoundPatMax::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_rotation, buffer_ptr, buffer_size);
	CPOBase::memRead(m_scale, buffer_ptr, buffer_size);
	CPOBase::memRead(m_refer_point, buffer_ptr, buffer_size);
	CPOBase::memReadVector(m_draw_pattern, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundPatMax::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_rotation, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_scale, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_refer_point, buffer_ptr, buffer_size);
	CPOBase::memWriteVector(m_draw_pattern, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundPatMax::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_rotation, fp);
	CPOBase::fileRead(m_scale, fp);
	CPOBase::fileRead(m_refer_point, fp);
	CPOBase::fileReadVector(m_draw_pattern, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundPatMax::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_rotation, fp);
	CPOBase::fileWrite(m_scale, fp);
	CPOBase::fileWrite(m_refer_point, fp);
	CPOBase::fileWriteVector(m_draw_pattern, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
