#pragma once

#include "sc_define.h"
#include "sc_struct.h"
#include "po_atomic.h"
#include "struct/found_blob.h"
#include "struct/found_edge.h"
#include "struct/found_flaw.h"
#include "struct/found_object.h"
#include "struct/found_patmax.h"
#include "struct/found_circle.h"
#include "struct/found_defect.h"
#include "struct/found_barcode.h"
#include "struct/draw_objects.h"

class CRegionInfoEx : protected CRegionInfo, public CLockGuard
{
public:
	void					setValue(CRegionInfoEx* value_ptr);

	i32						getPlusRegionCount();

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CTransformEx : protected CTransform, public CLockGuard
{
public:
	void					init();

	CTransform				getValue();
	void					setValue(const CTransform& other);
	void					setValue(CTransformEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundObjectEx : protected CFoundObject, public CLockGuard
{
public:
	void					setValue(CFoundObjectEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundObjectVectorEx : protected FoundObjectVector, public CLockGuard
{
public:
	void					init();
	i32						size();

	FoundObjectVector&		getSharedVector();
	CFoundObject			getValue(i32 index);
	void					setValue(CFoundObjectVectorEx* other_ptr);
	void					setValue(const FoundObjectVector& found_vec);
	void					setValue(const CFoundObject& found_obj);
	void					addValue(const CFoundObject& found_obj);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundPatMaxVectorEx : protected FoundPatMaxVector, public CLockGuard
{
public:
	void					init();
	i32						size();

	FoundPatMaxVector&		getSharedVector();
	CFoundPatMax			getValue(i32 index);
	void					setValue(CFoundPatMaxVectorEx* other_ptr);
	void					setValue(const FoundPatMaxVector& found_vec);
	void					setValue(const FoundPatMaxPVector& found_ptr_vec);
	void					setValue(const CFoundPatMax& found_obj);
	void					addValue(const CFoundPatMax& found_obj);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundBlobEx : protected CFoundBlob, public CLockGuard
{
public:
	void					setValue(CFoundBlobEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundBlobVectorEx : protected FoundBlobVector, public CLockGuard
{
public:
	void					init();
	i32						size();
	
	FoundBlobVector&		getSharedVector();
	CFoundBlob				getValue(i32 index);
	void					setValue(const FoundBlobVector& found_vec);
	void					setValue(CFoundBlobVectorEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundEdgeEx : protected CFoundEdge, public CLockGuard
{
public:
	void					init();

	CFoundEdge				getValue();
	void					setValue(const CFoundEdge& edge);
	void					setValue(CFoundEdgeEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundEdgeVectorEx : protected FoundEdgeVector, public CLockGuard
{
public:
	void					init();
	i32						size();

	FoundEdgeVector&		getSharedVector();
	void					setValue(const FoundEdgeVector& found_vec);
	void					setValue(CFoundEdgeVectorEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundCircleEx : protected CFoundCircle, public CLockGuard
{
public:
	void					init();
	
	CFoundCircle			getValue();
	void					setValue(CFoundCircle& circle);
	void					setValue(CFoundCircleEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundFlawVectorEx : protected CFoundFlawVector, public CLockGuard
{
public:
	void					init();
	i32						size();
	CFoundFlawVector&		getSharedVector();
	CFoundFlaw				getValue(i32 index);
	void					setValue(const CFoundFlawVector& found_vec);
	void					setValue(CFoundFlawVectorEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundBeadDefectEx : protected CBeadDefect, public CLockGuard
{
public:
	void					init();
	i32						size();

	CBeadDefect&			getSharedVector();
	CBeadDefectItem			getValue(i32 index);
	void					setValue(const CBeadDefect& found_vec);
	void					setValue(CFoundBeadDefectEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CFoundVarTableEx : public CLockGuard
{
public:
	CFoundVarTableEx();
	~CFoundVarTableEx();

	void					init();
	void					reset();
	void					setValue(CFoundVarTableEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

	inline i32				getVarCount() { return m_var_count; };

public:
	i32						m_var_count; //변수의 개수
	bool					m_use_initial_value; //공정설정시 변수에 초기값을 설정하겠는가?
	postring				m_var_name[SC_MAX_VAR_COUNT];	//변수의 이름
	i32						m_var_type[SC_MAX_VAR_COUNT];	//변수의 형
	u8						m_var_index[SC_MAX_VAR_COUNT];	//변수의 인덱스
	
	bool					m_var_bool[SC_MAX_VAR_COUNT];	//변수의 초기값들
	i32						m_var_i32[SC_MAX_VAR_COUNT];	//초기값의 index는 대역변수의 index와 일치하다.
	f32						m_var_f32[SC_MAX_VAR_COUNT];
	postring				m_var_str[SC_MAX_VAR_COUNT];
};

class CFoundBarcodeVectorEx : protected FoundBarcodeVector, public CLockGuard
{
public:
	void					init();
	i32						size();

	FoundBarcodeVector		getValue();
	CFoundBarcode			getValue(i32 index);
	void					setValue(const FoundBarcodeVector& other);
	void					setValue(CFoundBarcodeVectorEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CDistanceDrawEx : protected CDistanceDraw, public CLockGuard
{
public:
	void					init();

	void					setValue(CDistanceDraw& draw_distance);
	void					setValue(CDistanceDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CDimensionDrawEx : protected CDimensionDraw, public CLockGuard
{
public:
	void					init();

	void					setValue(CDimensionDraw& draw_dimension);
	void					setValue(CDimensionDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CAngleDrawEx : protected CAngleDraw, public CLockGuard
{
public:
	void					init();

	void					setValue(CAngleDraw& draw_angle);
	void					setValue(CAngleDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CArcDrawEx : protected CPOArc, public CLockGuard
{
public:
	void					init();

	void					setValue(CPOArc& other);
	void					setValue(CArcDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CCircleDrawEx : protected CPOCircle, public CLockGuard
{
public:
	void					init();

	void					setValue(CPOCircle& other);
	void					setValue(CCircleDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CCrossDrawEx : protected CPOCross, public CLockGuard
{
public:
	void					init();

	void					setValue(CPOCross& draw_string);
	void					setValue(CCrossDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CLineDrawEx : protected CPOLine, public CLockGuard
{
public:
	void					init();

	void					setValue(CPOLine& other);
	void					setValue(CLineDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CRotatedRectDrawEx : protected CPORotatedRect, public CLockGuard
{
public:
	void					init();

	void					setValue(CPORotatedRect& other);
	void					setValue(CRotatedRectDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};

class CStringDrawEx : protected CStringDraw, public CLockGuard
{
public:
	void					init();

	void					setValue(CStringDraw& draw_string);
	void					setValue(CStringDrawEx* other_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};
