#pragma once
#include "struct.h"

class CFoundCircle
{
public:
	CFoundCircle();
	~CFoundCircle();

	void						init();
	void						setResult(f32 match_score, vector2df& center, f32 radius, f32 st_angle = 0, f32 ed_angle = PO_PI2);

	void						setValue(CFoundCircle* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

	inline vector2df			getCenterPoint()  { return m_center_point; };

public:
	f32							m_match_score;				
	
	vector2df					m_center_point;
	f32							m_radius;
	f32							m_st_angle;
	f32							m_ed_angle;
};
typedef std::vector<CFoundCircle>	FoundCircleVector;
