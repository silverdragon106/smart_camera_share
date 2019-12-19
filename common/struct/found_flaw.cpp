#include "found_flaw.h"
#include "base.h"

CFoundFlaw::CFoundFlaw()
{
	init();
}

CFoundFlaw::~CFoundFlaw()
{
}

void CFoundFlaw::init()
{
	m_x1 = m_y1 = 0;
	m_area = 0;
	m_perimeter = 0;
	m_roundedness = 0;
	m_intensity = 0;
}

void CFoundFlaw::setResult(f32 x,f32 y, f32 area, f32 perimeter, f32 roundedness, f32 intensity)
{
	m_x1 = x;
	m_y1 = y;
	m_area = area;
	m_perimeter = perimeter;
	m_roundedness = roundedness;
	m_intensity = intensity;
}

void CFoundFlaw::setValue(CFoundFlaw* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_x1 = other_ptr->m_x1;
	m_y1 = other_ptr->m_y1;
	m_area = other_ptr->m_area;
	m_perimeter = other_ptr->m_perimeter;
	m_roundedness = other_ptr->m_roundedness;
	m_intensity = other_ptr->m_intensity;
}

i32 CFoundFlaw::memSize()
{
	i32 len = 0;
	len += sizeof(m_x1);
	len += sizeof(m_y1);
	len += sizeof(m_width);
	len += sizeof(m_height);
	len += sizeof(m_area);
	len += sizeof(m_perimeter);
	len += sizeof(m_roundedness);
	len += sizeof(m_intensity);
	return len;
}

i32 CFoundFlaw::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_x1, buffer_ptr, buffer_size);
	CPOBase::memRead(m_y1, buffer_ptr, buffer_size);
	CPOBase::memRead(m_width, buffer_ptr, buffer_size);
	CPOBase::memRead(m_height, buffer_ptr, buffer_size);
	CPOBase::memRead(m_area, buffer_ptr, buffer_size);
	CPOBase::memRead(m_perimeter, buffer_ptr, buffer_size);
	CPOBase::memRead(m_roundedness, buffer_ptr, buffer_size);
	CPOBase::memRead(m_intensity, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundFlaw::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_x1, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_y1, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_width, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_height, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_area, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_perimeter, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_roundedness, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_intensity, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundFlaw::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_x1, fp);
	CPOBase::fileRead(m_y1, fp);
	CPOBase::fileRead(m_width, fp);
	CPOBase::fileRead(m_height, fp);
	CPOBase::fileRead(m_area, fp);
	CPOBase::fileRead(m_perimeter, fp);
	CPOBase::fileRead(m_roundedness, fp);
	CPOBase::fileRead(m_intensity, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundFlaw::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_x1, fp);
	CPOBase::fileWrite(m_y1, fp);
	CPOBase::fileWrite(m_width, fp);
	CPOBase::fileWrite(m_height, fp);
	CPOBase::fileWrite(m_area, fp);
	CPOBase::fileWrite(m_perimeter, fp);
	CPOBase::fileWrite(m_roundedness, fp);
	CPOBase::fileWrite(m_intensity, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
