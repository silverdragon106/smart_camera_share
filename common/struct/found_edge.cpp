#include "found_edge.h"
#include "base.h"

CFoundEdge::CFoundEdge()
{
	init();
}

CFoundEdge::~CFoundEdge()
{
}

void CFoundEdge::init()
{
	m_match_score = 0;
	m_point_1st = vector2df();
	m_point_2nd = vector2df();
}

void CFoundEdge::setResult(f32 match_score, vector2df& point_1st, vector2df& point_2nd)
{
	m_match_score = match_score;
	m_point_1st = point_1st;
	m_point_2nd = point_2nd;
}

void CFoundEdge::setValue(CFoundEdge* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_match_score = other_ptr->m_match_score;
	m_point_1st = other_ptr->m_point_1st;
	m_point_2nd = other_ptr->m_point_2nd;
}

i32 CFoundEdge::memSize()
{
	i32 len = 0;
	len += sizeof(m_match_score);
	len += sizeof(m_point_1st);
	len += sizeof(m_point_2nd);
	return len;
}

i32 CFoundEdge::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_match_score, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundEdge::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_match_score, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundEdge::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_match_score, fp);
	CPOBase::fileRead(m_point_1st, fp);
	CPOBase::fileRead(m_point_2nd, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundEdge::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_match_score, fp);
	CPOBase::fileWrite(m_point_1st, fp);
	CPOBase::fileWrite(m_point_2nd, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
