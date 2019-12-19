#include "found_object.h"
#include "base.h"

CFoundObject::CFoundObject()
{
	init();
}

CFoundObject::~CFoundObject()
{
}

void CFoundObject::init()
{
	m_match_score = 0;
	m_rotation = 0;
	m_scale = 1.0f;
	m_refer_point = vector2df();
	m_transform.init();
}

void CFoundObject::setResult(f32 match_score, f32 rotation, f32 scale, const vector2df& result_refer_point)
{
	m_match_score = po::_max(po::_min(CPOBase::percent(match_score), 100), 0);
	m_rotation = CPOBase::radToDeg(rotation);
	m_scale = scale;
	m_refer_point = result_refer_point;
}

#if defined(POR_DEVICE)
void CFoundObject::setTransform(CTransform& transform)
{
	m_transform.setValue(&transform);
	m_transform.update();
}
#endif

void CFoundObject::setValue(CFoundObject* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_match_score = other_ptr->m_match_score;
	m_rotation = other_ptr->m_rotation;
	m_scale = other_ptr->m_scale;
	m_refer_point = other_ptr->m_refer_point;
	m_transform.setValue(&(other_ptr->m_transform));
}

i32 CFoundObject::memSize()
{
	i32 len = 0;
	len += sizeof(m_match_score);
	len += sizeof(m_rotation);
	len += sizeof(m_scale);
	len += sizeof(m_refer_point);
	len += m_transform.memSize();
	return len;
}

i32 CFoundObject::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_match_score, buffer_ptr, buffer_size);
	CPOBase::memRead(m_rotation, buffer_ptr, buffer_size);
	CPOBase::memRead(m_scale, buffer_ptr, buffer_size);
	CPOBase::memRead(m_refer_point, buffer_ptr, buffer_size);
	m_transform.memRead(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundObject::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_match_score, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_rotation, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_scale, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_refer_point, buffer_ptr, buffer_size);
	m_transform.memWrite(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundObject::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_match_score, fp);
	CPOBase::fileRead(m_rotation, fp);
	CPOBase::fileRead(m_scale, fp);
	CPOBase::fileRead(m_refer_point, fp);
	m_transform.fileRead(fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundObject::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_match_score, fp);
	CPOBase::fileWrite(m_rotation, fp);
	CPOBase::fileWrite(m_scale, fp);
	CPOBase::fileWrite(m_refer_point, fp);
	m_transform.fileWrite(fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
