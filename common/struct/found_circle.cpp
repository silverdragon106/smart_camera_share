#include "found_circle.h"
#include "base.h"

CFoundCircle::CFoundCircle()
{
	init();
}

CFoundCircle::~CFoundCircle()
{
}

void CFoundCircle::init()
{
	m_match_score = 0;

	m_center_point = vector2df();
	m_radius = 0;
	m_st_angle = 0;
	m_ed_angle = PO_PI2;
}

void CFoundCircle::setResult(f32 match_score, vector2df& center, f32 radius, f32 st_angle, f32 ed_angle)
{
	m_match_score = match_score;
	
	m_center_point = center;
	m_radius = radius;
	m_st_angle = st_angle;
	m_ed_angle = ed_angle;
}

void CFoundCircle::setValue(CFoundCircle* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_match_score = other_ptr->m_match_score;

	m_center_point = other_ptr->m_center_point;
	m_radius = other_ptr->m_radius;
	m_st_angle = other_ptr->m_st_angle;
	m_ed_angle = other_ptr->m_ed_angle;
}

i32 CFoundCircle::memSize()
{
	i32 len = 0;
	len += sizeof(m_match_score);

	len += sizeof(m_center_point);
	len += sizeof(m_radius);
	len += sizeof(m_st_angle);
	len += sizeof(m_ed_angle);
	return len;
}

i32 CFoundCircle::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_match_score, buffer_ptr, buffer_size);
	
	CPOBase::memRead(m_center_point, buffer_ptr, buffer_size);
	CPOBase::memRead(m_radius, buffer_ptr, buffer_size);
	CPOBase::memRead(m_st_angle, buffer_ptr, buffer_size);
	CPOBase::memRead(m_ed_angle, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundCircle::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_match_score, buffer_ptr, buffer_size);

	CPOBase::memWrite(m_center_point, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_radius, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_st_angle, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_ed_angle, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundCircle::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_match_score, fp);

	CPOBase::fileRead(m_center_point, fp);
	CPOBase::fileRead(m_radius, fp);
	CPOBase::fileRead(m_st_angle, fp);
	CPOBase::fileRead(m_ed_angle, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundCircle::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_match_score, fp);

	CPOBase::fileWrite(m_center_point, fp);
	CPOBase::fileWrite(m_radius, fp);
	CPOBase::fileWrite(m_st_angle, fp);
	CPOBase::fileWrite(m_ed_angle, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
