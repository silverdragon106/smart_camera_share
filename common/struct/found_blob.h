#pragma once
#include "struct.h"

class CFoundBlob
{
public:
	CFoundBlob();
	~CFoundBlob();

	void						init();
	void						setResult(f32 area, f32 rotation, vector2df& result_center_point);

	void						setValue(CFoundBlob* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	f32							m_area;				
	f32							m_rotation;		// 0 ~ 360degree
	vector2df					m_center;		// center point of founded blob
};
typedef std::vector<CFoundBlob>	FoundBlobVector;

struct SubBlob
{
	f32							cx;
	f32							cy;
	f32							ca[3];
	i32							pixel_count;
	i32							border_pixels;
	f32							angle;	//rad

	bool						is_valid;

public:
	SubBlob();
};
typedef std::vector<SubBlob>	SubBlobVector;
