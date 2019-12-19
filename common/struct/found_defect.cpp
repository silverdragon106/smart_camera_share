#include "found_defect.h"
#include "base.h"

CBeadDefectItem::CBeadDefectItem()
{
	init();
}

CBeadDefectItem::CBeadDefectItem(u8 defect_type, f32 defect_size)
{
	setResult(defect_type, defect_size);
}

CBeadDefectItem::~CBeadDefectItem()
{
}

void CBeadDefectItem::init()
{
	m_defect_type = kDefectNone;
	m_defect_count = 0;
	m_defect_percent = -1;
	m_smallest_size = -1;
	m_largest_size = -1;
	m_acc_size = -1;
}

void CBeadDefectItem::addResult(u8 defect_type, f32 defect_size, f32 size)
{
	if (size < 0)
	{
		return;
	}
	if (defect_type != m_defect_type)
	{
		setResult(defect_type, defect_size, size);
		return;
	}

	m_defect_count++;
	m_defect_percent = po::_min(100, m_defect_percent + defect_size * 100 / size);
	m_smallest_size = po::_min(m_smallest_size, defect_size);
	m_largest_size = po::_max(m_largest_size, defect_size);
	m_acc_size += defect_size;
}

void CBeadDefectItem::setResult(u8 defect_type, f32 defect_size, f32 size)
{
	m_defect_type = defect_type;
	m_defect_count = 1;
	m_defect_percent = po::_min(100, defect_size * 100 / size);
	m_smallest_size = defect_size;
	m_largest_size = defect_size;
	m_acc_size = defect_size;
}

void CBeadDefectItem::setValue(CBeadDefectItem* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	m_defect_type = other_ptr->m_defect_type;
	m_defect_count = other_ptr->m_defect_count;
	m_defect_percent = other_ptr->m_defect_percent;
	m_smallest_size = other_ptr->m_smallest_size;
	m_largest_size = other_ptr->m_largest_size;
	m_acc_size = other_ptr->m_acc_size;
}

void CBeadDefectItem::setScale(f32 scale)
{
	m_smallest_size *= scale;
	m_largest_size *= scale;
	m_acc_size *= scale;
}

i32 CBeadDefectItem::memSize()
{
	i32 len = 0;
	len += sizeof(m_defect_type);
	len += sizeof(m_defect_count);
	len += sizeof(m_defect_percent);
	len += sizeof(m_smallest_size);
	len += sizeof(m_largest_size);
	len += sizeof(m_acc_size);
	return len;
}

i32 CBeadDefectItem::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_defect_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_defect_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_defect_percent, buffer_ptr, buffer_size);
	CPOBase::memRead(m_smallest_size, buffer_ptr, buffer_size);
	CPOBase::memRead(m_largest_size, buffer_ptr, buffer_size);
	CPOBase::memRead(m_acc_size, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CBeadDefectItem::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_defect_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_defect_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_defect_percent, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_smallest_size, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_largest_size, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_acc_size, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CBeadDefectItem::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_defect_type, fp);
	CPOBase::fileRead(m_defect_count, fp);
	CPOBase::fileRead(m_defect_percent, fp);
	CPOBase::fileRead(m_smallest_size, fp);
	CPOBase::fileRead(m_largest_size, fp);
	CPOBase::fileRead(m_acc_size, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CBeadDefectItem::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_defect_type, fp);
	CPOBase::fileWrite(m_defect_count, fp);
	CPOBase::fileWrite(m_defect_percent, fp);
	CPOBase::fileWrite(m_smallest_size, fp);
	CPOBase::fileWrite(m_largest_size, fp);
	CPOBase::fileWrite(m_acc_size, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

CBeadDefect::CBeadDefect()
{
	m_over_all_length = -1.0f;
}

CBeadDefect::~CBeadDefect()
{
}

void CBeadDefect::init()
{
	clear();
	m_over_all_length = -1.0f;
}

void CBeadDefect::setScale(f32 scale)
{
	m_over_all_length *= scale;
	i32 i, count = (i32)size();
	for (i = 0; i < count; i++)
	{
		at(i).setScale(scale);
	}
}

void CBeadDefect::setOverAllLength(f32 length)
{
	m_over_all_length = length;
}

void CBeadDefect::addDefect(u8 defect_type, f32 defect_size)
{
	if (defect_size <= 0)
	{
		return;
	}

	i32 i, count = (i32)size();
	for (i = 0; i < count; i++)
	{
		if (at(i).m_defect_type == defect_type)
		{
			at(i).addResult(defect_type, defect_size, m_over_all_length);
			return;
		}
	}

	CBeadDefectItem tmp;
	tmp.setResult(defect_type, defect_size, m_over_all_length);
	push_back(tmp);
}

bool CBeadDefect::isValid()
{
	return (size() > 0) ? true : false;
}

i32 CBeadDefect::getDefectCount(i32 defect_type)
{
	i32 i, count = (i32)size();
	for (i = 0; i < count; i++)
	{
		if (at(i).m_defect_type == defect_type)
		{
			return at(i).m_defect_count;
		}
	}
	return 0;
}
