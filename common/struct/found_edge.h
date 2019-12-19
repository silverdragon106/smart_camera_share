#pragma once
#include "struct.h"

class CFoundEdge
{
public:
	CFoundEdge();
	~CFoundEdge();

	void						init();
	void						setResult(f32 match_score, vector2df& point_1st, vector2df& point_2nd);

	void						setValue(CFoundEdge* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

	inline vector2df			getCenterPoint()  { return (m_point_1st + m_point_2nd) / 2; };

public:
	f32							m_match_score;				
	vector2df					m_point_1st;
	vector2df					m_point_2nd;
};
typedef std::vector<CFoundEdge>	FoundEdgeVector;
