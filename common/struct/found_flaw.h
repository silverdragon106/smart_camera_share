#pragma once
#include "types.h"

class CFoundFlaw
{
public:
	CFoundFlaw();
	~CFoundFlaw();

	void						init();
	void						setResult(f32 x,f32 y, f32 area, f32 perimeter, f32 roundedness, f32 intensity);

	void						setValue(CFoundFlaw* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	f32							m_x1, m_y1;			// Top-Left coordinates of Flaw Region
	f32							m_x2, m_y2;			// Bottom-Right coordinates of Flaw Region
	f32							m_width, m_height;	// W, H of Flaw Region
	f32							m_area;				// A number of pixels of Flaw
	f32							m_perimeter;
	f32							m_roundedness;		// Roundness
	f32							m_intensity;		// Intensity
};
typedef std::vector<CFoundFlaw>	CFoundFlawVector;
