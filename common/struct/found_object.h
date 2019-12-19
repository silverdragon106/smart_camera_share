#pragma once
#include "struct/region_info.h"

class CFoundObject
{
public:
	CFoundObject();
	~CFoundObject();

	void						init();
	void						setResult(f32 match_score, f32 rotation, f32 scale, const vector2df& result_refer_point);
	void						setTransform(CTransform& transform);

	void						setValue(CFoundObject* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	f32							m_match_score;		// 0 ~ 100
	f32							m_rotation;			// 0 ~ 360deg
	f32							m_scale;			// 0.5 ~ 1.5
	vector2df					m_refer_point;		// founded point of model referpoint
	CTransform					m_transform;		// world -> found fixture
};
typedef std::vector<CFoundObject>	FoundObjectVector;
