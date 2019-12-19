#pragma once
#include "struct.h"

class CDistanceDraw
{
public:
	CDistanceDraw();
	~CDistanceDraw();

	void						init();
	void						setValue(CDistanceDraw* other_ptr);
	
	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	bool						m_has_modpoint;

	vector2df					m_point_1st;
	vector2df					m_point_2nd;
	vector2df					m_point_2nd_mod;
};

class CDimensionDraw
{
public:
	CDimensionDraw();
	~CDimensionDraw();

	void						init();
	void						setValue(CDimensionDraw* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	vector2df					m_point_1st;
	vector2df					m_point_2nd;
	vector2df					m_point_1st_mod;
	vector2df					m_point_2nd_mod;
	vector2df					m_point_1st_end;
	vector2df					m_point_2nd_end;
};

class CAngleDraw
{
public:
	CAngleDraw();
	~CAngleDraw();

	void						init();
	void						setValue(CAngleDraw* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	f32							m_angle;

	vector2df					m_point_1st;
	vector2df					m_point_2nd;
	vector2df					m_point_cross;
};

class CStringDraw
{
public:
	CStringDraw();
	~CStringDraw();

	void						init();
	void						setValue(CStringDraw* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	vector2df					m_point;
	postring					m_string;
};