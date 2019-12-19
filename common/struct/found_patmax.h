#pragma once
#include "sc_struct.h"

class CFoundPatMax
{
public:
	CFoundPatMax();
	~CFoundPatMax();

	void						init();
	void						freeBuffer();
	void						setValue(CFoundPatMax* other_ptr);
	
	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	f32							m_rotation;		//0 ~ 360deg
	f32							m_scale;		//0 ~ 1
	vector2df					m_refer_point;	//px
	std::vector<Line<f32>>		m_draw_pattern;
};
typedef std::vector<CFoundPatMax>	FoundPatMaxVector;
typedef std::vector<CFoundPatMax*>	FoundPatMaxPVector;
