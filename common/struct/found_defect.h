#pragma once
#include "struct.h"

enum DefectTypes
{
	kDefectNone = 0,
	kDefectCommon,
	kDefectThickDefect,
	kDefectThinDefect,
	kDefectGap,
	kDefectInvalidPosition
};

class CBeadDefectItem
{
public:
	CBeadDefectItem();
	CBeadDefectItem(u8 defect_type, f32 defect_size);
	~CBeadDefectItem();

	void						init();
	void						setResult(u8 defect_type, f32 defect_size, f32 size = -1.0f);
	void						addResult(u8 defect_type, f32 defect_size, f32 size = -1.0f);
	void						setValue(CBeadDefectItem* other_ptr);
	void						setScale(f32 scale);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	u8							m_defect_type;
	i32							m_defect_count;
	f32							m_defect_percent;
	f32							m_smallest_size;
	f32							m_largest_size;
	f32							m_acc_size;
};

class CBeadDefect : public std::vector<CBeadDefectItem>
{
public:
	CBeadDefect();
	~CBeadDefect();

	void						init();

	void						setScale(f32 scale);
	void						setOverAllLength(f32 length);
	void						addDefect(u8 defect_type, f32 defect_size);

	bool						isValid();
	i32							getDefectCount(i32 defect_type);

public:
	f32							m_over_all_length;
};

