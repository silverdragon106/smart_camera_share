#include "draw_objects.h"
#include "base.h"

CDistanceDraw::CDistanceDraw()
{
	init();
}

CDistanceDraw::~CDistanceDraw()
{
}

void CDistanceDraw::init()
{
	m_has_modpoint = false;
	m_point_1st = vector2df();
	m_point_2nd = vector2df();
	m_point_2nd_mod = vector2df();
}

void CDistanceDraw::setValue(CDistanceDraw* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}
	
	m_has_modpoint = other_ptr->m_has_modpoint;

	m_point_1st = other_ptr->m_point_1st;
	m_point_2nd = other_ptr->m_point_2nd;
	m_point_2nd_mod = other_ptr->m_point_2nd_mod;
}

i32 CDistanceDraw::memSize()
{
	i32 len = 0;

	len += sizeof(m_has_modpoint);
	len += sizeof(m_point_1st);
	len += sizeof(m_point_2nd);
	len += sizeof(m_point_2nd_mod);
	return len;
}

i32 CDistanceDraw::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(m_has_modpoint, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd_mod, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CDistanceDraw::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(m_has_modpoint, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd_mod, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CDistanceDraw::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_has_modpoint, fp);
	CPOBase::fileRead(m_point_1st, fp);
	CPOBase::fileRead(m_point_2nd, fp);
	CPOBase::fileRead(m_point_2nd_mod, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CDistanceDraw::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	CPOBase::fileWrite(m_has_modpoint, fp);
	CPOBase::fileWrite(m_point_1st, fp);
	CPOBase::fileWrite(m_point_2nd, fp);
	CPOBase::fileWrite(m_point_2nd_mod, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CDimensionDraw::CDimensionDraw()
{
	init();
}

CDimensionDraw::~CDimensionDraw()
{
}

void CDimensionDraw::init()
{
	m_point_1st = vector2df();
	m_point_2nd = vector2df();
	m_point_1st_mod = vector2df();
	m_point_2nd_mod = vector2df();
	m_point_1st_end = vector2df();
	m_point_2nd_end = vector2df();
}

void CDimensionDraw::setValue(CDimensionDraw* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_point_1st = other_ptr->m_point_1st;
	m_point_2nd = other_ptr->m_point_2nd;
	m_point_1st_mod = other_ptr->m_point_1st_mod;
	m_point_2nd_mod = other_ptr->m_point_2nd_mod;
	m_point_1st_end = other_ptr->m_point_1st_end;
	m_point_2nd_end = other_ptr->m_point_2nd_end;
}

i32 CDimensionDraw::memSize()
{
	i32 len = 0;

	len += sizeof(m_point_1st);
	len += sizeof(m_point_2nd);
	len += sizeof(m_point_1st_mod);
	len += sizeof(m_point_2nd_mod);
	len += sizeof(m_point_1st_end);
	len += sizeof(m_point_2nd_end);
	return len;
}

i32 CDimensionDraw::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_1st_mod, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd_mod, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_1st_end, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd_end, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CDimensionDraw::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_1st_mod, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd_mod, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_1st_end, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd_end, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CDimensionDraw::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_point_1st, fp);
	CPOBase::fileRead(m_point_2nd, fp);
	CPOBase::fileRead(m_point_1st_mod, fp);
	CPOBase::fileRead(m_point_2nd_mod, fp);
	CPOBase::fileRead(m_point_1st_end, fp);
	CPOBase::fileRead(m_point_2nd_end, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CDimensionDraw::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_point_1st, fp);
	CPOBase::fileWrite(m_point_2nd, fp);
	CPOBase::fileWrite(m_point_1st_mod, fp);
	CPOBase::fileWrite(m_point_2nd_mod, fp);
	CPOBase::fileWrite(m_point_1st_end, fp);
	CPOBase::fileWrite(m_point_2nd_end, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CAngleDraw::CAngleDraw()
{
	init();
}

CAngleDraw::~CAngleDraw()
{
}

void CAngleDraw::init()
{
	m_angle = 0;

	m_point_1st = vector2df();
	m_point_2nd = vector2df();
	m_point_cross = vector2df();
}

void CAngleDraw::setValue(CAngleDraw* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_angle = other_ptr->m_angle;

	m_point_1st = other_ptr->m_point_1st;
	m_point_2nd = other_ptr->m_point_2nd;
	m_point_cross = other_ptr->m_point_cross;
}

i32 CAngleDraw::memSize()
{
	i32 len = 0;
	len += sizeof(m_angle);
	len += sizeof(m_point_1st);
	len += sizeof(m_point_2nd);
	len += sizeof(m_point_cross);
	return len;
}

i32 CAngleDraw::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(m_angle, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_2nd, buffer_ptr, buffer_size);
	CPOBase::memRead(m_point_cross, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CAngleDraw::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(m_angle, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_1st, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_2nd, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_point_cross, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CAngleDraw::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_angle, fp);
	CPOBase::fileRead(m_point_1st, fp);
	CPOBase::fileRead(m_point_2nd, fp);
	CPOBase::fileRead(m_point_cross, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CAngleDraw::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	CPOBase::fileWrite(m_angle, fp);
	CPOBase::fileWrite(m_point_1st, fp);
	CPOBase::fileWrite(m_point_2nd, fp);
	CPOBase::fileWrite(m_point_cross, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CStringDraw::CStringDraw()
{
	init();
}

CStringDraw::~CStringDraw()
{
}

void CStringDraw::init()
{
	m_point = vector2df(0, 0);
	m_string = "";
}

void CStringDraw::setValue(CStringDraw* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_point = other_ptr->m_point;
	m_string = other_ptr->m_string;
}

i32 CStringDraw::memSize()
{
	i32 len = 0;
	len += sizeof(m_point);
	len += CPOBase::getStringMemSize(m_string);
	return len;
}

i32 CStringDraw::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(m_point, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, m_string);
	return buffer_ptr - buffer_pos;
}

i32 CStringDraw::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(m_point, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, m_string);
	return buffer_ptr - buffer_pos;
}

bool CStringDraw::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_point, fp);
	CPOBase::fileRead(fp, m_string);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CStringDraw::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_point, fp);
	CPOBase::fileWrite(fp, m_string);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
