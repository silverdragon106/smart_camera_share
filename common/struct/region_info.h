#pragma once
#include "sc_define.h"
#include "struct.h"

enum RegionShape
{
	kRegionShapeNone,
	kRegionShapeRect,
	kRegionShapeEllipse,
	kRegionShapeCircle,
	kRegionShapeRing,
	kRegionShapePolygon,

	kRegionShapeLine,
	kRegionShapePick = kRegionShapeLine,
};

enum RegionOperator
{
	kRegionOperNone,
	kRegionOperPlus,
	kRegionOperMinus,
	kRegionOperAnd,
	kRegionOperXor,
};

enum SubRegionTypes
{
	kSubRegionPlus = 0,
	kSubRegionMinus,
	kSubRegionAny
};

class CTransform
{
public:
	CTransform();
	CTransform(f32* tr0);
	CTransform(f32* tr0, f32* tr1);
	CTransform(const CTransform &robj);

	void					init(f32* tr = NULL);
	void					initStepwise(f32 angle, f32 cx, f32 cy, bool bT0 = true);
	void					initStepwise(f32 costh, f32 sinth, f32 cx, f32 cy, bool bT0 = true);
	void					initSetup(f32 angle, f32 cx, f32 cy, bool bT0 = true);
	void					initSetup(f32 ax, f32 ay, f32 cx, f32 cy, bool bT0 = true);

	void					invertT0();
	void					invertT1();

#if defined(POR_DEVICE)
	void					applyRotation(f32 rot, bool need_update = false);
	void					applyScale(f32 scale, bool need_update = false);
	void					applyRotationScale(f32 rot, f32 scale, bool need_update = false);
	void					applyTranslate(f32 tx, f32 ty, bool need_update = false);
    void					applyTranslate(const vector2df& trans, bool need_update = false);
	void					applyRotationScaleTranslate(f32 rot, f32 scale, f32 tx, f32 ty, bool need_update = false);
	void					applyRotationScaleTranslate(f32 rot, f32 scale, vector2df& trans, bool need_update = false);

	void					update();
	void					reverse();
	CTransform				inverse();
	CTransform				mul(CTransform& rhs);
	CTransform				mul(CTransform* rhs_ptr);
	CTransform				inverseMul(CTransform& rhs);
	CTransform				inverseMul(CTransform* rhs_ptr);

	void					transform(vector2df& point);
	void					transform(f32 x, f32 y, f32& px, f32& py);
	void					rotation(f32 x, f32 y, f32& px, f32& py);
	void					invTransform(vector2df& point);
	void					invTransform(f32 x, f32 y, f32& px, f32& py);
	void					invRotation(f32 x, f32 y, f32& px, f32& py);

#endif

	void					setValue(const CTransform& tr);
	void					setValue(CTransform* transform_ptr);
	
	f32						getRotation(bool bT0 = true);
	vector2df				getTranslation(bool bT0 = true);
	vector2df				getUnitVector(bool bT0 = true);
	CTransform				getTransform();
	void					getTransform(f32& tx, f32& ty, f32& angle, bool bT0 = true);
	void					getInvTransform(f32& tx, f32& ty, f32& angle);
	void					setTranslation(f32 tx, f32 ty, bool bT0 = true);
	void					setTransform(const CTransform& other);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

public:
	f32						m_t0[9];	//world(rotation box) -> local(normal rectangle)
	f32						m_t1[9];	//local(normal rectangle) -> world(rotation box)
	bool					m_has_invert;
};

struct RectInfo
{
	f32						m_x1, m_y1;      // Top-Left Corner
	f32						m_x2, m_y2;      // Bottom-Right Corner
};

struct CircleInfo
{
	f32						m_cx, m_cy;
	f32						m_inner_radius;
	f32						m_outer_radius;
};

struct RingInfo
{
	f32						m_cx, m_cy;
	f32						m_inner_radius;
	f32						m_outer_radius;
	f32						m_start_angle;      // 0~360 degree in Cartesian coordinate.
	f32						m_angle_length;     // Angle length in degree.
};

class CSubRegion : public CTransform
{
public:
	CSubRegion();
	CSubRegion(RegionShape shape_type);
	CSubRegion(const CSubRegion &robj);

	void					init();
	CSubRegion&				setValue(CSubRegion* sub_region_ptr, bool copy_poly_vec = true);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE *fp);

	void					setRectangle(Rectf region);
	void					setCircle(Rectf region);
	void					setEllipse(Rectf region);
	void					translate(f32 dx, f32 dy);

	vector2df				getCenter();
	void					getCenterPoint(f32& cx, f32& cy);
	void					getAngleRange(f32& st_angle, f32& ed_angle);
	f32						getAngleStart();
	f32						getAngleLength();
	f32						getInnerCircleRadius();
	f32						getOuterCircleRadius();
	f32						getInnerCircleLength();
	f32						getOuterCircleLength();
	f32						getWidth();
	f32						getHeight();
	f32						getLength();
	f32						getThickness();
	Rectf					getRectangle();

	inline f32				getXRadius()	{ return m_box.getWidth() / 2; };
	inline f32				getYRadius()	{ return m_box.getHeight() / 2; };
	inline f32				getRadius()		{ return po::_min(m_box.getWidth(), m_box.getHeight()) / 2; };
	inline Rectf			getBoundingRect() { return m_box; };

	inline vector2df*		getPolyPoints()	{ return m_poly_point_vec.data(); };
	inline i32				getPolyPtCount(){ return (i32)m_poly_point_vec.size(); };

	inline i32				getRegionType() { return m_region_shape; };
	inline i32				getRegionOper() { return m_region_oper; };
	inline bool				isPlusRegion() { return m_region_oper == kRegionOperPlus; };
	inline bool				isMinusRegion() { return m_region_oper == kRegionOperMinus; };

	inline bool				isRectangleRegion() { return m_region_shape == kRegionShapeRect; };
	inline bool				isCircleRegion() { return m_region_shape == kRegionShapeCircle; };
	inline bool				isEllipseRegion() { return m_region_shape == kRegionShapeEllipse; };

public:
	RegionShape				m_region_shape;
	RegionOperator			m_region_oper;	// Plus, Minus, Xor, Nor
	Rectf					m_box;			// Unrotated original box

	union
	{
		RectInfo rect, ellipse;
		CircleInfo circle;
		RingInfo ring;
	} u;
	ptvector2df			m_poly_point_vec;	// Just for Polygon
};

typedef std::list<CSubRegion>	SubRegionList;
typedef std::vector<CSubRegion>	SubRegionVector;

class CRegionInfo : public CTransform
{
public:
	CRegionInfo();

	void					init();
	void					free();
	CRegionInfo&			setValue(CRegionInfo* region_info_ptr);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

	CSubRegion*				getSubRegion(i32 index);
	CSubRegion				getPlusSubRegion(i32 index, bool is_copy_poly = false);
	CSubRegion				getMinusSubRegion(i32 index, bool is_copy_poly = false);
	CSubRegion				getSubRegion(i32 index, CTransform& parent_tr, i32 sub_type = kSubRegionAny, bool is_copy_poly = false);
	void					setRectangle(Recti range);
	void					setCircle(Recti range);
	void					setEllipse(Recti range);

	i32						calcPlusCount();
	bool					addSubRegion(CSubRegion rgn);
	bool					deleteSubRegion(i32 idx);

	inline Rectf&			getBoundingRect() { return m_box; };
	inline i32				getPlusRegionCount() { return m_plus_region_count; };
	inline i32				getSubRegionCount() { return (i32)m_region_vec.size(); };

public:
	Rectf					m_box;				// Bounding Box including minus Region
	i32						m_plus_region_count;
	SubRegionVector			m_region_vec;
};
