#include "region_info.h"
#include "base.h"

#if defined(POR_DEVICE)
#include "sc_define_internal.h"
#endif

const i32 kRegionMinRadius = 12;

//////////////////////////////////////////////////////////////////////////
CTransform::CTransform()
{
	init();
}

CTransform::CTransform(f32* tr0)
{
	init();
	if (tr0)
	{
		CPOBase::memCopy(m_t0, tr0, 6);
	}
}

CTransform::CTransform(f32* tr0, f32* tr1)
{
	init();
	if (tr0 != NULL && tr1 != NULL)
	{
		m_has_invert = true;
		CPOBase::memCopy(m_t0, tr0, 6);
		CPOBase::memCopy(m_t1, tr1, 6);
	}
}

CTransform::CTransform(const CTransform &robj)
{
	*this = robj;
}

void CTransform::init(f32* /*T*/)
{
	m_has_invert = false;
	CPOBase::init3x3(m_t0);
	CPOBase::init3x3(m_t1);
}

void CTransform::setValue(const CTransform& tr)
{
	memcpy(m_t0, tr.m_t0, sizeof(m_t0));
	memcpy(m_t1, tr.m_t1, sizeof(m_t1));
	m_has_invert = tr.m_has_invert;
}

void CTransform::setValue(CTransform* transform_ptr)
{
	if (transform_ptr)
	{
		memcpy(m_t0, transform_ptr->m_t0, sizeof(m_t0));
		memcpy(m_t1, transform_ptr->m_t1, sizeof(m_t1));
		m_has_invert = transform_ptr->m_has_invert;
	}
}

void CTransform::initStepwise(f32 angle, f32 cx, f32 cy, bool bT0)
{
	f32 costh = cosf(angle);
	f32 sinth = sinf(angle);
	initStepwise(costh, sinth, cx, cy, bT0);
}

void CTransform::initStepwise(f32 costh, f32 sinth, f32 cx, f32 cy, bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}

	T[0] = costh;	T[1] = -sinth;	T[2] = cx;
	T[3] = sinth;	T[4] = costh;	T[5] = cy;
}

void CTransform::initSetup(f32 angle, f32 cx, f32 cy, bool bT0)
{
	f32 costh = cosf(angle);
	f32 sinth = sinf(angle);
	initSetup(costh, sinth, cx, cy, bT0);
}

void CTransform::initSetup(f32 ax, f32 ay, f32 cx, f32 cy, bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}

	T[0] = ax;	T[1] = -ay; T[2] = -cx*ax + cy*ay;
	T[3] = ay; T[4] = ax;  T[5] = -cx*ay - ax*cy;
}

#if defined(POR_DEVICE)
void CTransform::applyTranslate(f32 tx, f32 ty, bool need_update)
{
	f32 m[9];
	CPOBase::init3x3(m, 0, 1.0, tx, ty);
	cv::Mat cv_trans(3, 3, CV_32FC1, m);
	cv::Mat cv_trans_forward(3, 3, CV_32FC1, m_t0);
	cv::Mat cv_cur_trans_forward(cv_trans_forward);
	cv_trans_forward = cv_trans*cv_cur_trans_forward;
	
	if (need_update)
	{
		update();
	}
}

void CTransform::applyRotation(f32 rot, bool need_update)
{
	f32 m[9];
	CPOBase::init3x3(m, rot);
	cv::Mat cv_rot(3, 3, CV_32FC1, m);
	cv::Mat cv_trans_forward(3, 3, CV_32FC1, m_t0);
	cv::Mat cv_cur_trans_forward(cv_trans_forward);
	cv_trans_forward = cv_rot*cv_cur_trans_forward;

	if (need_update)
	{
		update();
	}
}

void CTransform::applyScale(f32 scale, bool need_update)
{
	f32 m[9];
	CPOBase::init3x3(m, 0, scale);
	cv::Mat cv_scale(3, 3, CV_32FC1, m);
	cv::Mat cv_trans_forward(3, 3, CV_32FC1, m_t0);
	cv::Mat cv_cur_trans_forward(cv_trans_forward);
	cv_trans_forward = scale*cv_cur_trans_forward;

	if (need_update)
	{
		update();
	}
}

void CTransform::applyRotationScale(f32 rot, f32 scale, bool need_update)
{
	f32 m[9];
	CPOBase::init3x3(m, rot, scale);
	cv::Mat rot_scale(3, 3, CV_32FC1, m);
	cv::Mat trans_forward(3, 3, CV_32FC1, m_t0);
	cv::Mat cur_trans_forward(trans_forward);
	trans_forward = rot_scale*cur_trans_forward;

	if (need_update)
	{
		update();
	}
}

void CTransform::applyTranslate(const vector2df& trans, bool need_update)
{
	applyTranslate(trans.x, trans.y, false);
	if (need_update)
	{
		update();
	}
}

void CTransform::applyRotationScaleTranslate(f32 rot, f32 scale, f32 tx, f32 ty, bool need_update)
{
	applyRotationScale(rot, scale, false);
	applyTranslate(tx, ty, false);

	if (need_update)
	{
		update();
	}
}

void CTransform::applyRotationScaleTranslate(f32 rot, f32 scale, vector2df& trans, bool need_update)
{
	applyRotationScale(rot, scale, false);
	applyTranslate(trans, false);

	if (need_update)
	{
		update();
	}
}

void CTransform::update()
{
	if (!m_has_invert)
	{
		cv::Mat trans_forward(3, 3, CV_32FC1, m_t0);
		cv::Mat trans_backward = trans_forward.inv();
		memcpy(m_t1, (f32*)trans_backward.data, sizeof(f32) * 9);

		m_has_invert = true;
	}
}

CTransform CTransform::inverse()
{
	if (!m_has_invert)
	{
		update();
	}
	return CTransform(this->m_t1, this->m_t0);
}

void CTransform::reverse()
{
	update();

	f32 dd[9];
	CPOBase::memCopy(dd, m_t0, 9);
	CPOBase::memCopy(m_t0, m_t1, 9);
	CPOBase::memCopy(m_t1, dd, 9);
}

CTransform CTransform::mul(CTransform& rhs)
{
	CTransform lhs;
	cv::Mat p0(3, 3, CV_32FC1, m_t0);
	cv::Mat p1(3, 3, CV_32FC1, rhs.m_t0);
	cv::Mat p2(3, 3, CV_32FC1, lhs.m_t0);
	p2 = p0*p1;
	return lhs;
}

CTransform CTransform::mul(CTransform* rhs_ptr)
{
	if (!rhs_ptr)
	{
		return *this;
	}

	CTransform lhs;
	cv::Mat p0(3, 3, CV_32FC1, m_t0);
	cv::Mat p1(3, 3, CV_32FC1, rhs_ptr->m_t0);
	cv::Mat p2(3, 3, CV_32FC1, lhs.m_t0);
	p2 = p0*p1;
	return lhs; 
}

CTransform CTransform::inverseMul(CTransform& rhs)
{
	if (!rhs.m_has_invert)
	{
		rhs.update();
	}

	CTransform lhs;
	cv::Mat p0(3, 3, CV_32FC1, m_t0);
	cv::Mat p1(3, 3, CV_32FC1, rhs.m_t1);
	cv::Mat p2(3, 3, CV_32FC1, lhs.m_t0);
	p2 = p0*p1;
	return lhs;
}

CTransform CTransform::inverseMul(CTransform* rhs_ptr)
{
	if (!rhs_ptr)
	{
		return *this;
	}
	if (!rhs_ptr->m_has_invert)
	{
		rhs_ptr->update();
	}

	CTransform lhs;
	cv::Mat p0(3, 3, CV_32FC1, m_t0);
	cv::Mat p1(3, 3, CV_32FC1, rhs_ptr->m_t1);
	cv::Mat p2(3, 3, CV_32FC1, lhs.m_t0);
	p2 = p0*p1;
	return lhs; 
}

void CTransform::transform(vector2df& point)
{
	CPOBase::trans2x3(m_t0, point);
}

void CTransform::transform(f32 x, f32 y, f32& px, f32& py)
{
	CPOBase::trans2x3(m_t0, x, y, px, py);
}

void CTransform::rotation(f32 x, f32 y, f32& px, f32& py)
{
	CPOBase::rotate2x3(m_t0, x, y, px, py);
}

void CTransform::invTransform(vector2df& point)
{
	if (!m_has_invert)
	{
		update();
	}
	CPOBase::trans2x3(m_t1, point);
}

void CTransform::invTransform(f32 x, f32 y, f32& px, f32& py)
{
	if (!m_has_invert)
	{
		update();
	}
	CPOBase::trans2x3(m_t1, x, y, px, py);
}

void CTransform::invRotation(f32 x, f32 y, f32& px, f32& py)
{
	if (!m_has_invert)
	{
		update();
	}
	CPOBase::rotate2x3(m_t1, x, y, px, py);
}
#endif

void CTransform::getInvTransform(f32& tx, f32& ty, f32& angle)
{
	if (!m_has_invert)
	{
#if defined(POR_DEVICE)
		update();
#else
		tx = 0; ty = 0; angle = 0;
		return;
#endif
	}

	angle = 0;
	tx = m_t1[2];
	ty = m_t1[5];

	f32 scale = sqrt(m_t1[0] * m_t1[0] + m_t1[1] * m_t1[1]);
	if (scale < PO_EPSILON)
	{
		return;
	}
	angle = acos((m_t1[0] + m_t1[4]) / (2 * scale));
}

void CTransform::invertT0()
{
	m_t0[0] = -m_t0[0]; m_t0[1] = -m_t0[1]; m_t0[2] = -m_t0[2];
	m_t0[3] = -m_t0[3]; m_t0[4] = -m_t0[4]; m_t0[5] = -m_t0[5];
}

void CTransform::invertT1()
{
	m_t1[0] = -m_t1[0]; m_t1[1] = -m_t1[1]; m_t1[2] = -m_t1[2];
	m_t1[3] = -m_t1[3]; m_t1[4] = -m_t1[4]; m_t1[5] = -m_t1[5];
}

i32 CTransform::memSize()
{
	i32 nSize = 0;
	nSize += sizeof(m_t0);
	nSize += sizeof(m_t1);
	nSize += sizeof(m_has_invert);

	return nSize;
}

i32 CTransform::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_t0, 9, buffer_ptr, buffer_size);
	CPOBase::memRead(m_t1, 9, buffer_ptr, buffer_size);
	CPOBase::memRead(m_has_invert, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

i32 CTransform::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_t0, 9, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_t1, 9, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_has_invert, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

bool CTransform::fileRead(FILE *fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_t0, 9, fp);
	CPOBase::fileRead(m_t1, 9, fp);
	CPOBase::fileRead(m_has_invert, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CTransform::fileWrite(FILE *fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_t0, 9, fp);
	CPOBase::fileWrite(m_t1, 9, fp);
	CPOBase::fileWrite(m_has_invert, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

f32 CTransform::getRotation(bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}
	
	return CPOBase::getUnitVectorAngle(T[0], T[3]);
}

vector2df CTransform::getTranslation(bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}

	return vector2df(T[2], T[5]);
}

vector2df CTransform::getUnitVector(bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}

	return vector2df(T[0], T[3]);
}

void CTransform::setTranslation(f32 tx, f32 ty, bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}

	T[2] = tx;
	T[5] = ty;
}

void CTransform::setTransform(const CTransform& other)
{
	*this = other;
}

CTransform CTransform::getTransform()
{
	return *this;
}

void CTransform::getTransform(f32& tx, f32& ty, f32& angle, bool bT0)
{
	f32 *T = m_t0;
	if (!bT0)
	{
		T = m_t1;
	}

	tx = T[2];
	ty = T[5];
	angle = 0;

	f32 scale = sqrt(T[0] * T[0] + T[1] * T[1]);
	if (scale < PO_EPSILON)
	{
		return;
	}

	f32 cos_al = (T[0] + T[4]) / (2 * scale);
	f32 sin_al = (T[3] - T[1]) / (2 * scale);
	if (sin_al >= 0)
	{
		angle = acosf(cos_al); //part-1,2
	}
	else
	{
		angle = -acosf(cos_al); //part-3,4
	}
}

//////////////////////////////////////////////////////////////////////////
/// RegionBase
CSubRegion::CSubRegion()
{
	init();
}

CSubRegion::CSubRegion(RegionShape shape_type)
{
	init();
	m_region_shape = shape_type;
}

CSubRegion::CSubRegion(const CSubRegion &robj)
{
	*this = robj;
}

CSubRegion& CSubRegion::setValue(CSubRegion* sub_region_ptr, bool copy_poly_vec)
{
	if (sub_region_ptr)
	{
		m_region_shape = sub_region_ptr->m_region_shape;
		m_region_oper = sub_region_ptr->m_region_oper;
		m_box = sub_region_ptr->m_box;
		CTransform::setValue(sub_region_ptr);

		memcpy(&u, &sub_region_ptr->u, sizeof(u));
		if (copy_poly_vec)
		{
			m_poly_point_vec = sub_region_ptr->m_poly_point_vec;
		}
	}
	return *this;
}

void CSubRegion::init()
{
	CTransform::init();
	memset(&u, 0, sizeof(u));

	m_region_shape = kRegionShapeRect;
	m_region_oper = kRegionOperPlus;
}

i32 CSubRegion::memSize()
{
	i32 nSize = 0;
	
	nSize += CTransform::memSize();
	nSize += sizeof(m_box);
	nSize += sizeof(m_region_shape);
	nSize += sizeof(m_region_oper);
	nSize += sizeof(u);
	nSize += CPOBase::getVectorMemSize(m_poly_point_vec);
	return nSize;
}

i32 CSubRegion::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CTransform::memRead(buffer_ptr, buffer_size);

	CPOBase::memRead(m_box, buffer_ptr, buffer_size);
	CPOBase::memRead(m_region_shape, buffer_ptr, buffer_size);
	CPOBase::memRead(m_region_oper, buffer_ptr, buffer_size);
	CPOBase::memRead(u, buffer_ptr, buffer_size);
	CPOBase::memReadVector(m_poly_point_vec, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

i32 CSubRegion::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	
	CTransform::memWrite(buffer_ptr, buffer_size);
	
	CPOBase::memWrite(m_box, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_region_shape, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_region_oper, buffer_ptr, buffer_size);
	CPOBase::memWrite(u, buffer_ptr, buffer_size);
	CPOBase::memWriteVector(m_poly_point_vec, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

bool CSubRegion::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}
	
	if (!CTransform::fileRead(fp))
	{
		return false;
	}
	CPOBase::fileRead(m_box, fp);
	CPOBase::fileRead(m_region_shape, fp);
	CPOBase::fileRead(m_region_oper, fp);
	CPOBase::fileRead(u, fp);
	CPOBase::fileReadVector(m_poly_point_vec, fp);

	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CSubRegion::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	if (!CTransform::fileWrite(fp))
	{
		return false;
	}

	CPOBase::fileWrite(m_box, fp);
	CPOBase::fileWrite(m_region_shape, fp);
	CPOBase::fileWrite(m_region_oper, fp);
	CPOBase::fileWrite(u, fp);
	CPOBase::fileWriteVector(m_poly_point_vec, fp);

	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

vector2df CSubRegion::getCenter()
{
	f32 cx,cy;
	m_box.getCenterPoint<f32>(cx, cy);
	return vector2df(cx, cy);
}

void CSubRegion::getCenterPoint(f32& cx, f32& cy)
{
	m_box.getCenterPoint<f32>(cx, cy);
}

f32 CSubRegion::getWidth()
{
	return m_box.getWidth();
}

f32 CSubRegion::getHeight()
{
	return m_box.getHeight();
}

f32 CSubRegion::getLength()
{
	switch (m_region_shape)
	{
		case kRegionShapeRect:
		{
			return m_box.getWidth();
		}
		case kRegionShapeEllipse:
		{
			f32 rw = m_box.getWidth() / 2;
			f32 rh = m_box.getHeight() / 2;
			return PO_PI2* sqrtf((rw*rw + rh*rh) / 2);
		}
		case kRegionShapeCircle:
		{
			return PO_PI2 * u.circle.m_outer_radius;
		}
		case kRegionShapeRing:
		{
			return fabs(u.ring.m_angle_length) * (u.ring.m_inner_radius + u.ring.m_outer_radius) / 2;
		}
		case kRegionShapePolygon:
		{
			i32 i, ni;
			i32 count = (i32)m_poly_point_vec.size();
			f32 len = 0;
			for (i = 0; i < count; i++)
			{
				ni = (i + 1) % count;
				len += CPOBase::distance(m_poly_point_vec[i], m_poly_point_vec[ni]);
			}
			return len;
		}
	}
	return 0;
}

f32 CSubRegion::getThickness()
{
	f32 thickness = 0;
	switch (m_region_shape)
	{
		case kRegionShapeRect:
		{
			thickness = m_box.getHeight();
			break;
		}
		case kRegionShapeRing:
		{
			thickness = u.ring.m_outer_radius - u.ring.m_inner_radius;
			break;
		}
	}
	return thickness;
}

void CSubRegion::getAngleRange(f32& st_angle, f32& ed_angle)
{
	if (m_region_shape != kRegionShapeRing)
	{
		st_angle = 0;
		ed_angle = PO_PI2;
		return;
	}

	//calc st angle
	st_angle = CPOBase::getAbsAngle(u.ring.m_start_angle);
	ed_angle = st_angle + u.ring.m_angle_length;
	if (u.ring.m_angle_length < 0)
	{
		CPOBase::swap(st_angle, ed_angle);
	}

	//calc ed angle
	st_angle = CPOBase::getAbsAngle(st_angle);
	while (ed_angle <= st_angle)
	{
		ed_angle += PO_PI2;
	}
}

f32 CSubRegion::getAngleStart()
{
	if (m_region_shape != kRegionShapeRing)
	{
		return 0;
	}
	return u.ring.m_start_angle;
}

f32 CSubRegion::getAngleLength()
{
	if (m_region_shape != kRegionShapeRing)
	{
		return PO_PI2;
	}
	return u.ring.m_angle_length;
}

f32 CSubRegion::getInnerCircleRadius()
{
	f32 radius = 0;
	switch (m_region_shape)
	{
		case kRegionShapeRing:
		{
			radius = u.ring.m_inner_radius;
			break;
		}
		default:
		{
			radius = po::_min(u.circle.m_outer_radius * 0.25f, kRegionMinRadius);
			break;
		}
	}
	return radius;
}

f32 CSubRegion::getOuterCircleRadius()
{
	f32 radius = 0;
	switch (m_region_shape)
	{
		case kRegionShapeCircle:
		{
			radius = u.circle.m_outer_radius;
			break;
		}
		case kRegionShapeEllipse:
		{
			f32 r1 = u.ellipse.m_x2 - u.ellipse.m_x1;
			f32 r2 = u.ellipse.m_y2 - u.ellipse.m_y1;
			radius = po::_min(r1, r2) / 2;
			break;
		}
		case kRegionShapeRing:
		{
			radius = u.ring.m_outer_radius;
			break;
		}
		default:
		{
			radius = po::_min(m_box.getWidth(), m_box.getHeight()) / 2;
			break;
		}
	}
	return radius;
}

f32 CSubRegion::getInnerCircleLength()
{
	return fabs(getAngleLength()) * getInnerCircleRadius();
}

f32 CSubRegion::getOuterCircleLength()
{
	return fabs(getAngleLength()) * getOuterCircleRadius();
}

void CSubRegion::setRectangle(Rectf region)
{
	init();

	m_box = region;
	m_region_oper = kRegionOperPlus;
	m_region_shape = kRegionShapeRect;
	u.rect.m_x1 = region.x1;
	u.rect.m_y1 = region.y1;
	u.rect.m_x2 = region.x2;
	u.rect.m_y2 = region.y2;
}

Rectf CSubRegion::getRectangle()
{
	Rectf rt;
	if (m_region_shape == kRegionShapeRect)
	{
		rt.x1 = u.rect.m_x1;
		rt.y1 = u.rect.m_y1;
		rt.x2 = u.rect.m_x2;
		rt.y2 = u.rect.m_y2;
	}
	return rt;
}

void CSubRegion::setCircle(Rectf region)
{
	init();

	f32 cx, cy, r;
	region.getCenterPoint(cx, cy);
	r = po::_min(region.getWidth(), region.getHeight()) / 2;

	m_box = region;
	m_region_oper = kRegionOperPlus;
	m_region_shape = kRegionShapeCircle;
	u.circle.m_cx = cx;
	u.circle.m_cy = cy;
	u.circle.m_inner_radius = 0;
	u.circle.m_outer_radius = r;
}

void CSubRegion::setEllipse(Rectf region)
{
	init();

	m_box = region;
	m_region_oper = kRegionOperPlus;
	m_region_shape = kRegionShapeEllipse;
	u.ellipse.m_x1 = region.x1;
	u.ellipse.m_y1 = region.y1;
	u.ellipse.m_x2 = region.x2;
	u.ellipse.m_y2 = region.y2;
}

//////////////////////////////////////////////////////////////////////////
/// CRegionInfo
CRegionInfo::CRegionInfo()
{
	init();
}

void CRegionInfo::init()
{
	free();
	CTransform::init();
}

void CRegionInfo::free()
{
	m_box.reset();
	m_region_vec.clear();
	m_plus_region_count = 0;
}

CRegionInfo& CRegionInfo::setValue(CRegionInfo* rgn_info_ptr)
{
	if (rgn_info_ptr)
	{
		free();

		CTransform::setValue(rgn_info_ptr);

		m_box = rgn_info_ptr->m_box;
		m_plus_region_count = rgn_info_ptr->m_plus_region_count;

		i32 i, region_count = (i32)rgn_info_ptr->m_region_vec.size();
		CSubRegion* sub_region_ptr = rgn_info_ptr->m_region_vec.data();
		m_region_vec.resize(region_count);

		for (i = 0; i < region_count; i++)
		{
			m_region_vec[i].setValue(sub_region_ptr + i);
		}
	}
	return *this;
}

i32 CRegionInfo::memSize()
{
	i32 len = 0;

	len += CTransform::memSize();
	len += sizeof(m_box);
	len += sizeof(m_plus_region_count);
	len += sizeof(i32);
	for (i32 i = 0; i < m_region_vec.size(); i++)
	{
		len += m_region_vec[i].memSize();
	}

	return len;
}

i32 CRegionInfo::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	free();

	u8* buffer_pos = buffer_ptr;

	i32 count = 0;
	CTransform::memRead(buffer_ptr, buffer_size);
	CPOBase::memRead(m_box, buffer_ptr, buffer_size);
	CPOBase::memRead(m_plus_region_count, buffer_ptr, buffer_size);

	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(count))
	{
		m_region_vec.resize(count);
		for (i32 i = 0; i < count; i++)
		{
			m_region_vec[i].memRead(buffer_ptr, buffer_size);
		}
	}
	return buffer_ptr - buffer_pos;
}

i32 CRegionInfo::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 count = (i32)m_region_vec.size();
	CTransform::memWrite(buffer_ptr, buffer_size);
	CPOBase::memWrite(m_box, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_plus_region_count, buffer_ptr, buffer_size);

	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i32 i = 0; i < count; i++)
	{
		m_region_vec[i].memWrite(buffer_ptr, buffer_size);
	}

	return buffer_ptr - buffer_pos;
}

bool CRegionInfo::fileRead(FILE* fp)
{
	free();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	if (!CTransform::fileRead(fp))
	{
		return false;
	}

	i32 count = 0;
	CPOBase::fileRead(m_box, fp);
	CPOBase::fileRead(m_plus_region_count, fp);

	CPOBase::fileRead(count, fp);
	if (CPOBase::isCount(count))
	{
		m_region_vec.resize(count);
		for (i32 i = 0; i < count; i++)
		{
			if (!m_region_vec[i].fileRead(fp))
			{
				return false;
			}
		}
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CRegionInfo::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	if (!CTransform::fileWrite(fp))
	{
		return false;
	}

	i32 count = (i32)m_region_vec.size();
	CPOBase::fileWrite(m_box, fp);
	CPOBase::fileWrite(m_plus_region_count, fp);

	CPOBase::fileWrite(count, fp);
	for (i32 i = 0; i < count; i++)
	{
		if (!m_region_vec[i].fileWrite(fp))
		{
			return false;
		}
	}
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

i32 CRegionInfo::calcPlusCount()
{
	i32 plus_count = 0;
	SubRegionVector::iterator iter;
	for (iter = m_region_vec.begin(); iter != m_region_vec.end(); ++iter)
	{
		if ((*iter).m_region_oper == kRegionOperPlus)
		{
			plus_count++;
		}
	}

	return plus_count;
}
bool CRegionInfo::addSubRegion(CSubRegion rgn)
{
	m_region_vec.push_back(rgn);
	m_plus_region_count = calcPlusCount();
	return true;
}

bool CRegionInfo::deleteSubRegion(i32 idx)
{
	if (idx < 0 || idx >= m_region_vec.size())
	{
		return false;
	}
	
	i32 i = 0;
	SubRegionVector::iterator iter;
	for (iter = m_region_vec.begin(); iter != m_region_vec.end(); ++iter, i++)
	{
		if (i == idx)
		{
			iter = m_region_vec.erase(iter);
			break;
		}
	}
	
	m_plus_region_count = calcPlusCount();
	return true;
}

void CRegionInfo::setRectangle(Recti range)
{
	free();

	m_box = Rectf(range);
	m_plus_region_count = 1;

	CSubRegion sub_region;
	sub_region.setRectangle(m_box);
	m_region_vec.push_back(sub_region);
}

void CRegionInfo::setCircle(Recti range)
{
	free();

	m_box = Rectf(range);
	m_plus_region_count = 1;

	CSubRegion sub_region;
	sub_region.setCircle(m_box);
	m_region_vec.push_back(sub_region);
}

void CRegionInfo::setEllipse(Recti range)
{
	free();

	m_box = Rectf(range);
	m_plus_region_count = 1;

	CSubRegion sub_region;
	sub_region.setEllipse(m_box);
	m_region_vec.push_back(sub_region);
}

CSubRegion* CRegionInfo::getSubRegion(i32 index)
{
	if (!CPOBase::checkIndex(index, (i32)m_region_vec.size()))
	{
		return NULL;
	}
	return m_region_vec.data() + index;
}

CSubRegion CRegionInfo::getPlusSubRegion(i32 index, bool is_cpoy_poly)
{
	CSubRegion sub_region;
	i32 i, ni = 0, count = (i32)m_region_vec.size();
	if (!CPOBase::checkIndex(index, m_plus_region_count))
	{
		return sub_region;
	}

	CSubRegion* tmp_sub_region_ptr;
	CSubRegion* sub_region_ptr = m_region_vec.data();
	for (i = 0; i < count; i++)
	{
		tmp_sub_region_ptr = sub_region_ptr + i;
		if (tmp_sub_region_ptr->isPlusRegion())
		{
			if (index == ni)
			{
				sub_region.setValue(tmp_sub_region_ptr, is_cpoy_poly);
				return sub_region;
			}
			ni++;
		}
	}
	return sub_region;
}

CSubRegion CRegionInfo::getMinusSubRegion(i32 index, bool is_copy_poly)
{
	CSubRegion sub_region;
	i32 i, ni = 0, count = (i32)m_region_vec.size();
	if (!CPOBase::checkIndex(index, count - m_plus_region_count))
	{
		return sub_region;
	}

	CSubRegion* tmp_sub_region_ptr;
	CSubRegion* sub_region_ptr = m_region_vec.data();
	for (i = 0; i < count; i++)
	{
		tmp_sub_region_ptr = sub_region_ptr + i;
		if (tmp_sub_region_ptr->isMinusRegion())
		{
			if (index == ni)
			{
				sub_region.setValue(tmp_sub_region_ptr, is_copy_poly);
				return sub_region;
			}
			ni++;
		}
	}
	return sub_region;
}

#if defined(POR_DEVICE)
CSubRegion CRegionInfo::getSubRegion(i32 index, CTransform& parent_tr, i32 sub_type, bool is_copy_poly)
{
	CSubRegion sub_region;
	CTransform tmp_transform;
	
	switch (sub_type)
	{
		case kSubRegionPlus:
		{
			sub_region = getPlusSubRegion(index, is_copy_poly);
			break;
		}
		case kSubRegionMinus:
		{
			sub_region = getMinusSubRegion(index, is_copy_poly);
			break;
		}
		default:
		{
			sub_region.setValue(getSubRegion(index), is_copy_poly);
			break;
		}
	}

	tmp_transform.setValue(dynamic_cast<CTransform*>(this)); //this's transform: t0:world->local
	tmp_transform = tmp_transform.inverseMul(parent_tr); //parent_transfor: t0:local->world
	tmp_transform = dynamic_cast<CTransform*>(&sub_region)->mul(tmp_transform); //sub transform: t0:world->local
	tmp_transform.update();

	sub_region.setTransform(tmp_transform);
	return sub_region;
}
#endif

void CSubRegion::translate(f32 dx, f32 dy)
{
	m_box.translate(dx, dy);
	switch (m_region_shape)
	{
		case kRegionShapeRect:
		case kRegionShapeEllipse:
		{
			u.rect.m_x1 += dx;
			u.rect.m_y1 += dy;
			u.rect.m_x2 += dx;
			u.rect.m_y2 += dy;
			break;
		}
		case kRegionShapeCircle:
		{
			u.circle.m_cx += dx;
			u.circle.m_cy += dy;
			break;
		}
		case kRegionShapeRing:
		{
			u.ring.m_cx += dx;
			u.ring.m_cy += dy;
			break;
		}
		case kRegionShapePolygon:
		{
			for (i32 i = 0; i < m_poly_point_vec.size(); i++)
			{
				m_poly_point_vec[i] += vector2df(dx, dy);
			}
			break;
		}
	}
}
