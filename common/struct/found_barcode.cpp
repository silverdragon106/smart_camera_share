#include "found_barcode.h"
#include "sc_define.h"
#include "base.h"

CFoundBarcode::CFoundBarcode()
{
	init();
}

CFoundBarcode::CFoundBarcode(const postring& id_string, const postring& str_symbology,
						f32 cx, f32 cy, f32 angle, i32 grade)
{
	init();
	
	m_string = id_string;
	m_symbology = str_symbology;
	m_grade = grade;

	m_center_x = cx;
	m_center_y = cy;
	m_angle_deg = angle;
}

CFoundBarcode::~CFoundBarcode()
{
}

void CFoundBarcode::init()
{
	m_string = "";
	m_symbology = "";
	m_grade = kBarcodeGradeNone;

	m_center_x = 0;
	m_center_y = 0;
	m_angle_deg = 0;
}

void CFoundBarcode::setValue(CFoundBarcode* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}
	*this = *other_ptr;
}

i32 CFoundBarcode::memSize()
{
	i32 len = 0;

	len += CPOBase::getStringMemSize(m_string);
	len += CPOBase::getStringMemSize(m_symbology);
	len += sizeof(m_grade);

	len += sizeof(m_center_x);
	len += sizeof(m_center_y);
	len += sizeof(m_angle_deg);
	return len;
}

i32 CFoundBarcode::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	init();

	CPOBase::memRead(buffer_ptr, buffer_size, m_string);
	CPOBase::memRead(buffer_ptr, buffer_size, m_symbology);
	CPOBase::memRead(m_grade, buffer_ptr, buffer_size);

	CPOBase::memRead(m_center_x, buffer_ptr, buffer_size);
	CPOBase::memRead(m_center_y, buffer_ptr, buffer_size);
	CPOBase::memRead(m_angle_deg, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CFoundBarcode::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(buffer_ptr, buffer_size, m_string);
	CPOBase::memWrite(buffer_ptr, buffer_size, m_symbology);
	CPOBase::memWrite(m_grade, buffer_ptr, buffer_size);

	CPOBase::memWrite(m_center_x, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_center_y, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_angle_deg, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CFoundBarcode::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	init();

	CPOBase::fileRead(fp, m_string);
	CPOBase::fileRead(fp, m_symbology);
	CPOBase::fileRead(m_grade, fp);

	CPOBase::fileRead(m_center_x, fp);
	CPOBase::fileRead(m_center_y, fp);
	CPOBase::fileRead(m_angle_deg, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundBarcode::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	
	CPOBase::fileWrite(fp, m_string);
	CPOBase::fileWrite(fp, m_symbology);
	CPOBase::fileWrite(m_grade, fp);

	CPOBase::fileWrite(m_center_x, fp);
	CPOBase::fileWrite(m_center_y, fp);
	CPOBase::fileWrite(m_angle_deg, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
