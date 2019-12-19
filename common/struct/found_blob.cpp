#include "found_blob.h"
#include "base.h"

CFoundBlob::CFoundBlob()
{
	init();
}

CFoundBlob::~CFoundBlob()
{
}

void CFoundBlob::init()
{
	m_area = 0;
	m_rotation = 0;
	m_center = vector2df();
}

void CFoundBlob::setResult(f32 area, f32 rotation, vector2df& result_center_point)
{
	m_area = area;
	m_rotation = CPOBase::radToDeg(rotation);
	m_center = result_center_point;
}

void CFoundBlob::setValue(CFoundBlob* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_area = other_ptr->m_area;
	m_rotation = other_ptr->m_rotation;
	m_center = other_ptr->m_center;
}

i32 CFoundBlob::memSize()
{
	i32 len = 0;
	len += sizeof(m_area);
	len += sizeof(m_rotation);
	len += sizeof(m_center);
	return len;
}

i32 CFoundBlob::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_area, buffer_ptr, buffer_size);
	CPOBase::memRead(m_rotation, buffer_ptr, buffer_size);
	CPOBase::memRead(m_center, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundBlob::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_area, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_rotation, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_center, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundBlob::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_area, fp);
	CPOBase::fileRead(m_rotation, fp);
	CPOBase::fileRead(m_center, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundBlob::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_area, fp);
	CPOBase::fileWrite(m_rotation, fp);
	CPOBase::fileWrite(m_center, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
SubBlob::SubBlob()
{
	memset(this, 0, sizeof(SubBlob));
}