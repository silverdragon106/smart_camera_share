#include "sc_struct.h"
#include "base.h"
#include "proc/find_caliper.h"

//////////////////////////////////////////////////////////////////////////
CSecuritySetting::CSecuritySetting()
{
	reset();
}

void CSecuritySetting::reset()
{
	//init permission array
	m_permission_count = kPermissionCount;
	memset(m_permission_enabled, 0, sizeof(bool)*PO_MAX_PERMISSION);

	//set default password
	m_cur_password = PO_INIT_PASSWORD;
}

void CSecuritySetting::init()
{
	lock_guard();
	reset();
}

CSecuritySetting CSecuritySetting::getValue()
{
	lock_guard();
	return *this;
}

void CSecuritySetting::setValue(CSecuritySetting& other)
{
	lock_guard();
	anlock_guard(other);
	*this = other;
}

i32 CSecuritySetting::memSize()
{
	lock_guard();

	i32 len = 0;
	len += sizeof(m_permission_count);
	len += sizeof(bool)*m_permission_count;
	return len;
}

i32 CSecuritySetting::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_permission_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_permission_enabled, m_permission_count, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CSecuritySetting::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_permission_count, buffer_ptr, buffer_size);
	if (CPOBase::checkRange(m_permission_count, PO_MAX_PERMISSION))
	{
		CPOBase::memRead(m_permission_enabled, m_permission_count, buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CSecuritySetting::fileRead(FILE* fp)
{
	lock_guard();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_permission_count, fp);
	if (!CPOBase::isCount(m_permission_count))
	{
		return false;
	}

	CPOBase::fileRead(m_permission_enabled, PO_MAX_PERMISSION, fp);
	if (!CPOBase::fileRead(fp, m_cur_password))
	{
		return false;
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CSecuritySetting::fileWrite(FILE* fp)
{
	lock_guard();

	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	CPOBase::fileWrite(m_permission_count, fp);
	CPOBase::fileWrite(m_permission_enabled, PO_MAX_PERMISSION, fp);
	CPOBase::fileWrite(fp, m_cur_password);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

void CSecuritySetting::setPassword(postring& new_password)
{
	lock_guard();
	m_cur_password = new_password;
}

bool CSecuritySetting::checkPermissionAuth(postring& chk_password)
{
	lock_guard();
	return (chk_password == m_cur_password);
}

bool CSecuritySetting::getPermission(i32 cmd)
{
	lock_guard();
	if (!CPOBase::checkIndex(cmd, m_permission_count))
	{
		return false;
	}
	return m_permission_enabled[cmd];
}

//////////////////////////////////////////////////////////////////////////
CGeneralSetting::CGeneralSetting()
{
	reset();
}

void CGeneralSetting::reset()
{
	m_angle_unit = 0;
	m_language_hl = kPOLangEnglish;
	m_history_setting = kResultOperTypeFailed;
	m_decimal_disp = 2;

	m_debug_level = LOG_LV2;
	m_use_last_snap = true;
	m_is_enable_remote = true;
	m_is_sync_datetime = false;
}

void CGeneralSetting::init()
{
	lock_guard();
	reset();
}

CGeneralSetting CGeneralSetting::getValue()
{
	lock_guard();
	return *this;
}

void CGeneralSetting::setValue(CGeneralSetting& other)
{
	lock_guard();
	anlock_guard(other);
	*this = other;
}

void CGeneralSetting::setDebugLevel(i32 level)
{
	lock_guard();
	m_debug_level = level;
}

i32 CGeneralSetting::memSize()
{
	lock_guard();
	i32 len = 0;

	len += sizeof(m_language_hl);
	len += sizeof(m_history_setting);
	len += sizeof(m_decimal_disp);
	len += sizeof(m_debug_level);
	len += sizeof(m_use_last_snap);
	len += sizeof(m_is_enable_remote);
	len += sizeof(m_is_sync_datetime);
	return len;
}

i32 CGeneralSetting::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_language_hl, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_history_setting, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_decimal_disp, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_debug_level, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_use_last_snap, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_is_enable_remote, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_is_sync_datetime, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CGeneralSetting::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_language_hl, buffer_ptr, buffer_size);
	CPOBase::memRead(m_history_setting, buffer_ptr, buffer_size);
	CPOBase::memRead(m_decimal_disp, buffer_ptr, buffer_size);
	CPOBase::memRead(m_debug_level, buffer_ptr, buffer_size);
	CPOBase::memRead(m_use_last_snap, buffer_ptr, buffer_size);
	CPOBase::memRead(m_is_enable_remote, buffer_ptr, buffer_size);
	CPOBase::memRead(m_is_sync_datetime, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CGeneralSetting::fileRead(FILE* fp)
{
	lock_guard();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CGeneralSetting::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void LocatorParam::init()
{
	default_edge_strength = TOOLCONST_LOC_EDGE_STRENGTH;
	default_search_accuarcy = kLocAccuracyNormal;
	min_match_score = TOOLCONST_LOC_MATCH_MIN;
	rotation_tolerance = TOOLCONST_LOC_ROTATION;
	scale_tolerance = TOOLCONST_LOC_SCALE;

	min_contour_pixel_count = 10;
	min_roi_contribution = 0.97f;
	max_pyramid_level_density = 0.40f;
	min_pyramid_level_inherit = 0.70f;
	min_pyramid_basic_size_ght = 48;
	min_pyramid_basic_size_rpt = 32;
	min_basic_integral = 0.5f;
	min_sub_match_rate = 0.5f;
	min_sub_count = 3;

	ght_cell_interval = 3;
	ght_angle_count = 90;
	ght_rtable_cell = 30;
	ght_angle_step_inv = 1000;
	ght_pixel_step = 2;
	ght_contour_min_size = 5;
}

//////////////////////////////////////////////////////////////////////////
void BlobParam::init()
{
	threshold_mode = kBlobThresholdAuto;
	threshold = TOOLCONST_BLOB_THRESHOLD;
	blob_color_mode = kBlobColorEither;
	allow_boundary = true;
	blur_size = 3;
}

//////////////////////////////////////////////////////////////////////////
void EdgeParam::init()
{
	default_edge_strength = TOOLCONST_EDGE_STRENGTH;
	default_edge_width = TOOLCONST_EDGE_WIDTH;
	max_gap_size = TOOLCONST_EDGE_UNION_GAPSIZE;
	max_angle_diff = TOOLCONST_EDGE_UNION_ANGLE;
	min_size = TOOLCONST_EDGE_LEN_MIN;
	max_size = TOOLCONST_EDGE_LEN_MAX;
}

//////////////////////////////////////////////////////////////////////////
void SurfaceFlawParam::init()
{
	// Mask
	use_mask = false;
	mask_method = kSurfaceFlawEdgeMask;
	mask_smooth_factor = TOOLCONST_SURFLAW_MASK_SMOOTH_FACTOR;
	mask_edge_contrast = TOOLCONST_SURFLAW_MASK_CONTRAST;
	mask_segment_size = TOOLCONST_SURFLAW_MASK_SEGMENT_SIZE;
	mask_hole_fill_size = TOOLCONST_SURFLAW_MASK_HOLE_FILL_SIZE;
	mask_median_factor = TOOLCONST_SURFLAW_MASK_MEDIAN_FACTOR;
	mask_erode_factor = TOOLCONST_SURFLAW_MASK_ERODE_FACTOR;

	// Flaw
	detection_type = kSurfaceFlawFastMethod;
	display_type = kSurfaceFlawDisplayFlaws;
	sampling_factor = TOOLCONST_SURFLAW_SAMPLING_FACTOR;
	auto_smoothing = false;
	smooth_factor = TOOLCONST_SURFLAW_SMOOTH_FACTOR;
	detection_axis = kSurfaceFlawDetectXY;
	detection_size = TOOLCONST_SURFLAW_DETECTION_SIZE;
	min_contrast = TOOLCONST_SURFLAW_EDGE_CONTRAST;
	min_flaw_area = TOOLCONST_SURFLAW_MIN_AREA;
	max_flaw_area = TOOLCONST_SURFLAW_MAX_AREA;
}

//////////////////////////////////////////////////////////////////////////
void DefectEdgeParam::init()
{
	caliper_size = TOOLCONST_DEFECTEDGE_CALIPER_SIZE;
	caliper_space = TOOLCONST_DEFECTEDGE_CALIPER_SPACE;
	caliper_threshold = TOOLCONST_DEFECTEDGE_THRESHOLD;

	filter_size = TOOLCONST_DEFECTEDGE_FILTER_SIZE;
	min_contrast = TOOLCONST_DEFECTEDGE_CONTRAST;
	max_contrast = TOOLCONST_DEFECTEDGE_UPPER_CONTRAST;
	edge_pair_size = TOOLCONST_DEFECTEDGE_PAIR_SIZE;
	
	fit_distance_filter = TOOLCONST_DEFECTEDGE_FITFILTER;
	fit_distance_furthest = TOOLCONST_DEFECTEDGE_FITFURTHEST;

	min_distance = TOOLCONST_DEFECTEDGE_MIN_DISTANCE;
	max_distance = TOOLCONST_DEFECTEDGE_MAX_DISTANCE;
	defect_min_size = TOOLCONST_DEFECTEDGE_MIN_SIZE;
	defect_max_size = TOOLCONST_DEFECTEDGE_MAX_SIZE;
	defect_min_area = TOOLCONST_DEFECTEDGE_MIN_AREA;
	defect_max_area = TOOLCONST_DEFECTEDGE_MAX_AREA;
	
	gap_min_size = TOOLCONST_DEFECTEDGE_GAP_MIN_SIZE;
	gap_max_size = TOOLCONST_DEFECTEDGE_GAP_MAX_SIZE;
}

//////////////////////////////////////////////////////////////////////////
void BeadParam::init()
{
	default_width = TOOLCONST_BEAD_WIDTH;
	default_width_tolerance = TOOLCONST_BEAD_WIDTH_TOLERANCE;
	default_position_tolerance = TOOLCONST_BEAD_POSITION_TOLERANCE;
	default_edge_strength = TOOLCONST_EDGE_STRENGTH;
	clip_end_points = TOOLCONST_BEAD_CLIPEDGE;

	bead_caliper_size = TOOLCONST_BEAD_CALIPER_SIZE;
	bead_caliper_space = 0;
	bead_pair_size = TOOLCONST_BEAD_PAIR_SIZE;
	bead_threshold = TOOLCONST_BEAD_THRESHOLD;

	bead_edge_height = TOOLCONST_BEAD_EDGE_HEIGHT;
	bead_edge_caliper_size = TOOLCONST_BEAD_EDGE_CALIPER_SIZE;
	bead_edge_caliper_space = 0;
	bead_edge_pair_size = TOOLCONST_BEAD_EDGE_PAIR_SIZE;
	bead_edge_threshold = TOOLCONST_BEAD_EDGE_THRESHOLD;

	bead_filter_size = TOOLCONST_BEAD_FILTER_SIZE;
	bead_min_contrast = TOOLCONST_BEAD_CONTRAST;
	bead_max_contrast = TOOLCONST_BEAD_UPPER_CONTRAST;
	bead_edge_direction = kBeadEdgeDirectionOutsideToIn;
	bead_edge0_transition = kEdgeEither;
	bead_edge1_transition = kEdgeEither;

	bead_min_width = TOOLCONST_BEAD_DEFECT_MIN_WIDTH;
	bead_max_width = TOOLCONST_BEAD_DEFECT_MAX_WIDTH;
	bead_defect_min_size = TOOLCONST_BEAD_DEFECT_MIN_SIZE;
	bead_defect_max_size = TOOLCONST_BEAD_DEFECT_MAX_SIZE;
	bead_defect_min_area = TOOLCONST_BEAD_DEFECT_MIN_AREA;
	bead_defect_max_area = TOOLCONST_BEAD_DEFECT_MAX_AREA;

	bead_gap_min_size = TOOLCONST_BEAD_GAP_MIN_SIZE;
	bead_gap_max_size = TOOLCONST_BEAD_GAP_MAX_SIZE;

	bead_position_threshold = TOOLCONST_BEAD_POSITION_THRESHOLD;
	bead_position_min_size = TOOLCONST_BEAD_POSITION_MIN_SIZE;
	bead_position_max_size = TOOLCONST_BEAD_POSITION_MAX_SIZE;
	bead_position_min_area = TOOLCONST_BEAD_POSITION_MIN_AREA;
	bead_position_max_area = TOOLCONST_BEAD_POSITION_MAX_AREA;
}

//////////////////////////////////////////////////////////////////////////
void CircleParam::init()
{
	default_edge_strength = TOOLCONST_EDGE_STRENGTH;
	default_edge_width = TOOLCONST_EDGE_WIDTH;
	max_arc_angle_diff = TOOLCONST_CIRCLE_ANGLE_DIFF_MAX;
	max_arc_overlap = TOOLCONST_CIRCLE_ARC_OVERLAP;
	max_tangent_angle = TOOLCONST_CIRCLE_ANGLE_TANGENT_MAX;
	max_dist = TOOLCONST_CIRCLE_DISTANCE_MAX;
	max_radius_diff = TOOLCONST_CIRCLE_RADIUS_DIFF_MAX;
	max_center_dist = TOOLCONST_CIRCLE_CENTER_DIFF_MAX;
	min_circle_line = 17;
	max_circle_line = 80000;
}

//////////////////////////////////////////////////////////////////////////
void CountingParam::init()
{
	default_threshold_mode = kImgProcThresholdAuto;
	default_low_threshold = TOOLCONST_IMGPROC_LOW_THRESHOLD;
	default_high_threshold = TOOLCONST_IMGPROC_HIGH_THRESHOLD;
	counting_blur_size = 3;
}

//////////////////////////////////////////////////////////////////////////
void FilterParam::init()
{
	default_filter = kFilterBinarize;
	default_kernel_size = TOOLCONST_FILTER_KERNEL_SIZE;
	default_threshold_mode = kFilterAutoMode;
	default_threshold = TOOLCONST_FILTER_THRESHOLD;
	default_clip_min = TOOLCONST_FILTER_THRESHOLD;
	default_clip_max = TOOLCONST_FILTER_THRESHOLD;
	default_kernel_gain = TOOLCONST_FILTER_GAIN_MIN;
}

//////////////////////////////////////////////////////////////////////////
CEngineParam::CEngineParam()
{
	reset();
}

void CEngineParam::init()
{
	lock_guard();
	reset();
}

void CEngineParam::reset()
{
	m_locator_param.init();
	m_blob_param.init();
	m_edge_param.init();
	m_circle_param.init();
	m_counting_param.init();
	m_filter_param.init();
	m_surflaw_param.init();
	m_defect_edge_param.init();
	m_bead_param.init();
	m_th_min_focus = 10;
}

CEngineParam CEngineParam::getValue()
{
	lock_guard();
	return *this;
}

void CEngineParam::setValue(CEngineParam& other)
{
	lock_guard();
	anlock_guard(other);
	*this = other;
}

i32 CEngineParam::memSize()
{
	lock_guard();
	i32 len = 0;

	len += sizeof(m_locator_param);
	len += sizeof(m_blob_param);
	len += sizeof(m_edge_param);
	len += sizeof(m_circle_param);
	len += sizeof(m_counting_param);
	len += sizeof(m_filter_param);
	len += sizeof(m_defect_edge_param);
	len += sizeof(m_bead_param);
	len += sizeof(m_th_min_focus);
	return len;
}

i32 CEngineParam::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_locator_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_blob_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_edge_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_circle_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_counting_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_filter_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_defect_edge_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_bead_param, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_th_min_focus, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CEngineParam::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_locator_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_blob_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_edge_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_circle_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_counting_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_filter_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_defect_edge_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_bead_param, buffer_ptr, buffer_size);
	CPOBase::memRead(m_th_min_focus, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CEngineParam::fileRead(FILE* fp)
{
	lock_guard();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_locator_param, fp);
	CPOBase::fileRead(m_blob_param, fp);
	CPOBase::fileRead(m_edge_param, fp);
	CPOBase::fileRead(m_circle_param, fp);
	CPOBase::fileRead(m_counting_param, fp);
	CPOBase::fileRead(m_filter_param, fp);
	CPOBase::fileRead(m_defect_edge_param, fp);
	CPOBase::fileRead(m_bead_param, fp);
	CPOBase::fileRead(m_th_min_focus, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CEngineParam::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_locator_param, fp);
	CPOBase::fileWrite(m_blob_param, fp);
	CPOBase::fileWrite(m_edge_param, fp);
	CPOBase::fileWrite(m_circle_param, fp);
	CPOBase::fileWrite(m_counting_param, fp);
	CPOBase::fileWrite(m_filter_param, fp);
	CPOBase::fileWrite(m_defect_edge_param, fp);
	CPOBase::fileWrite(m_bead_param, fp);
	CPOBase::fileWrite(m_th_min_focus, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}
//////////////////////////////////////////////////////////////////////////
OpcItem::OpcItem()
{
	opc_tag_name = "";
	opc_access_mode = 0;
	opc_data_type = 0;
}

i32 OpcItem::memSize()
{
	i32 len = 0;

	len += CPOBase::getStringMemSize(opc_tag_name);
	len += sizeof(opc_access_mode);
	len += sizeof(opc_data_type);
	return len;
}

i32 OpcItem::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(buffer_ptr, buffer_size, opc_tag_name);
	CPOBase::memWrite(opc_access_mode, buffer_ptr, buffer_size);
	CPOBase::memWrite(opc_data_type, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 OpcItem::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(buffer_ptr, buffer_size, opc_tag_name);
	CPOBase::memRead(opc_access_mode, buffer_ptr, buffer_size);
	CPOBase::memRead(opc_data_type, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool OpcItem::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(fp, opc_tag_name);
	CPOBase::fileRead(opc_access_mode, fp);
	CPOBase::fileRead(opc_data_type, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool OpcItem::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(fp, opc_tag_name);
	CPOBase::fileWrite(opc_access_mode, fp);
	CPOBase::fileWrite(opc_data_type, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
COpcComm::COpcComm()
{
	reset();
}

COpcComm::~COpcComm()
{
	reset();
}

void COpcComm::reset()
{
	m_opc_items.clear();
}

void COpcComm::init()
{
	lock_guard();
	reset();
}

COpcComm COpcComm::getValue()
{
	lock_guard();
	return *this;
}

void COpcComm::setValue(COpcComm& value)
{
	lock_guard();
	anlock_guard(value);
	*this = value;
}

i32 COpcComm::memSize()
{
	lock_guard();

	i32 len = 0;
	i32 i, count = (i32)m_opc_items.size();
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += m_opc_items[i].memSize();
	}
	return len;
}

i32 COpcComm::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = (i32)m_opc_items.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_opc_items[i].memWrite(buffer_ptr, buffer_size);
	}

	return buffer_ptr - buffer_pos;
}

i32 COpcComm::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	reset();

	i32 i, count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	m_opc_items.resize(count);
	for (i = 0; i < count; i++)
	{
		m_opc_items[i].memRead(buffer_ptr, buffer_size);
	}

	return buffer_ptr - buffer_pos;
}

bool COpcComm::fileRead(FILE* fp)
{
	lock_guard();

	reset();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	//file read opc input
	i32 i, count = -1;
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	m_opc_items.resize(count);
	for (i = 0; i < count; i++)
	{
		m_opc_items[i].fileRead(fp);
	}

	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool COpcComm::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	//file write opc input
	i32 i, count = (i32)m_opc_items.size();
	CPOBase::fileWrite(count, fp);
	for (i = 0; i < count; i++)
	{
		m_opc_items[i].fileWrite(fp);
	}

	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
ModBusItem::ModBusItem()
{
	modbus_tag_name = "";
	modbus_address = 0;
	modbus_data_type = 0;
	modbus_size = 0;
}

i32 ModBusItem::memSize()
{
	i32 len = 0;

	len += CPOBase::getStringMemSize(modbus_tag_name);
	len += sizeof(modbus_address);
	len += sizeof(modbus_data_type);
	len += sizeof(modbus_size);
	return len;
}

i32 ModBusItem::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(buffer_ptr, buffer_size, modbus_tag_name);
	CPOBase::memWrite(modbus_address, buffer_ptr, buffer_size);
	CPOBase::memWrite(modbus_data_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(modbus_size, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 ModBusItem::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(buffer_ptr, buffer_size, modbus_tag_name);
	CPOBase::memRead(modbus_address, buffer_ptr, buffer_size);
	CPOBase::memRead(modbus_data_type, buffer_ptr, buffer_size);
	CPOBase::memRead(modbus_size, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool ModBusItem::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(fp, modbus_tag_name);
	CPOBase::fileRead(modbus_address, fp);
	CPOBase::fileRead(modbus_data_type, fp);
	CPOBase::fileRead(modbus_size, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool ModBusItem::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(fp, modbus_tag_name);
	CPOBase::fileWrite(modbus_address, fp);
	CPOBase::fileWrite(modbus_data_type, fp);
	CPOBase::fileWrite(modbus_size, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CModBusComm::CModBusComm()
{
	reset();
}

CModBusComm::~CModBusComm()
{
	reset();
}

void CModBusComm::reset()
{
	m_modbus_input.clear();
	m_modbus_output.clear();
}

void CModBusComm::init()
{
	lock_guard();
	reset();
}

CModBusComm CModBusComm::getValue()
{
	lock_guard();
	return *this;
}

void CModBusComm::setValue(CModBusComm& value)
{
	lock_guard();
	anlock_guard(value);
	*this = value;
}

i32 CModBusComm::memSize()
{
	lock_guard();

	i32 len = 0;
	i32 i, count = (i32)m_modbus_input.size();
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += m_modbus_input[i].memSize();
	}
	
	count = (i32)m_modbus_output.size();
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += m_modbus_output[i].memSize();
	}
	return len;
}

i32 CModBusComm::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = (i32)m_modbus_input.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_modbus_input[i].memWrite(buffer_ptr, buffer_size);
	}

	count = (i32)m_modbus_output.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_modbus_output[i].memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CModBusComm::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	reset();

	i32 i, count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	m_modbus_input.resize(count);
	for (i = 0; i < count; i++)
	{
		m_modbus_input[i].memRead(buffer_ptr, buffer_size);
	}

	count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	m_modbus_output.resize(count);
	for (i = 0; i < count; i++)
	{
		m_modbus_output[i].memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CModBusComm::fileRead(FILE* fp)
{
	lock_guard();

	reset();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	//file read modbus input
	i32 i, count = -1;
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	m_modbus_input.resize(count);
	for (i = 0; i < count; i++)
	{
		m_modbus_input[i].fileRead(fp);
	}

	//file read modbus output
	count = -1;
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	m_modbus_output.resize(count);
	for (i = 0; i < count; i++)
	{
		m_modbus_output[i].fileRead(fp);
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CModBusComm::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	
	//file write modbus input
	i32 i, count = (i32)m_modbus_input.size();
	CPOBase::fileWrite(count, fp);
	for (i = 0; i < count; i++)
	{
		m_modbus_input[i].fileWrite(fp);
	}

	//file write modbus output
	count = (i32)m_modbus_output.size();
	CPOBase::fileWrite(count, fp);
	for (i = 0; i < count; i++)
	{
		m_modbus_output[i].fileWrite(fp);
	}
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
ModbusAllocation::ModbusAllocation()
{
	init();
}

void ModbusAllocation::init()
{
	m_cam_count = PO_CAM_COUNT;
	for (i32 i = 0; i < PO_CAM_COUNT; i++)
	{
		m_input_address[i] = SCADDR_VARBLOCK_START + i*SCADDR_CAMVAR_SIZE;
		m_input_size[i] = SCADDR_CAMVAR_SIZE;

		m_output_address[i] = SCADDR_TOOLRESULT_START + i*SCADDR_CAMRESULT_SIZE;
		m_output_size[i] = SCADDR_CAMRESULT_SIZE;
	}
}

i32 ModbusAllocation::memSize()
{
	i32 len = 0;

	len += sizeof(m_cam_count);
	len += sizeof(m_input_address);
	len += sizeof(m_input_size);
	len += sizeof(m_output_address);
	len += sizeof(m_output_size);
	return len;
}

i32 ModbusAllocation::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_input_address, m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_input_size, m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_output_address, m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_output_size, m_cam_count, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 ModbusAllocation::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_cam_count, buffer_ptr, buffer_size);
	if (!CPOBase::checkRange(m_cam_count, PO_CAM_COUNT))
	{
		return buffer_ptr - buffer_pos;
	}

	CPOBase::memRead(m_input_address, m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_input_size, m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_output_address, m_cam_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_output_size, m_cam_count, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool ModbusAllocation::checkModbusAllocation(i32 cam_id, CModBusComm& modbus_comm)
{
	if (!CPOBase::checkIndex(cam_id, m_cam_count))
	{
		return false;
	}

	i32 in_address_start = m_input_address[cam_id];
	i32 in_address_end = m_input_address[cam_id] + m_input_size[cam_id];

	i32 out_address_start = m_output_address[cam_id];
	i32 out_address_end = m_output_address[cam_id] + m_output_size[cam_id];

	ModBusItemVector& mb_in_vec = modbus_comm.m_modbus_input;
	i32 i, count = (i32)mb_in_vec.size();
	i32 addr_start, addr_end;

	for (i = 0; i < count; i++)
	{
		addr_start = mb_in_vec[i].modbus_address;
		addr_end = addr_start + mb_in_vec[i].modbus_size - 1;
		if (!CPOBase::checkIndex(addr_start, in_address_start, in_address_end))
		{
			return false;
		}
		if (!CPOBase::checkIndex(addr_end, in_address_start, in_address_end))
		{
			return false;
		}
	}

	ModBusItemVector& mb_out_vec = modbus_comm.m_modbus_output;
	count = (i32)mb_out_vec.size();

	for (i = 0; i < count; i++)
	{
		addr_start = mb_out_vec[i].modbus_address;
		addr_end = addr_start + mb_out_vec[i].modbus_size - 1;
		if (!CPOBase::checkIndex(addr_start, out_address_start, out_address_end))
		{
			return false;
		}
		if (!CPOBase::checkIndex(addr_end, out_address_start, out_address_end))
		{
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
FtpOutput::FtpOutput()
{
	init();
}

FtpOutput::~FtpOutput()
{
}

void FtpOutput::init()
{
	is_enabled = true;
	ftp_name = L"";
	dev_index = 0;
	cond_tag_name = "";
	file_format = kPOSaveFormatBmp;
	image_size = kPOImageSizeFull;
	default_filename = L"image";
	add_include_graphics = false;
	add_time_stamp = true;
	limit_count = POFTP_MAX_IMAGE;
}

i32 FtpOutput::memSize()
{
	i32 len = 0;
	len += sizeof(is_enabled);
	len += CPOBase::getStringMemSize(ftp_name);
	len += sizeof(dev_index);
	len += CPOBase::getStringMemSize(cond_tag_name);
	len += sizeof(file_format);
	len += sizeof(image_size);
	len += CPOBase::getStringMemSize(default_filename);
	len += sizeof(add_include_graphics);
	len += sizeof(add_time_stamp);
	len += sizeof(limit_count);
	return len;
}

i32 FtpOutput::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(is_enabled, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, ftp_name);
	CPOBase::memWrite(dev_index, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, cond_tag_name);
	CPOBase::memWrite(file_format, buffer_ptr, buffer_size);
	CPOBase::memWrite(image_size, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, default_filename);
	CPOBase::memWrite(add_include_graphics, buffer_ptr, buffer_size);
	CPOBase::memWrite(add_time_stamp, buffer_ptr, buffer_size);
	CPOBase::memWrite(limit_count, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 FtpOutput::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(is_enabled, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, ftp_name);
	CPOBase::memRead(dev_index, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, cond_tag_name);
	CPOBase::memRead(file_format, buffer_ptr, buffer_size);
	CPOBase::memRead(image_size, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, default_filename);
	CPOBase::memRead(add_include_graphics, buffer_ptr, buffer_size);
	CPOBase::memRead(add_time_stamp, buffer_ptr, buffer_size);
	CPOBase::memRead(limit_count, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

bool FtpOutput::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(is_enabled, fp);
	CPOBase::fileRead(fp, ftp_name);
	CPOBase::fileRead(dev_index, fp);
	CPOBase::fileRead(fp, cond_tag_name);
	CPOBase::fileRead(file_format, fp);
	CPOBase::fileRead(image_size, fp);
	CPOBase::fileRead(fp, default_filename);
	CPOBase::fileRead(add_include_graphics, fp);
	CPOBase::fileRead(add_time_stamp, fp);
	CPOBase::fileRead(limit_count, fp);

	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool FtpOutput::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(is_enabled, fp);
	CPOBase::fileWrite(fp, ftp_name);
	CPOBase::fileWrite(dev_index, fp);
	CPOBase::fileWrite(fp, cond_tag_name);
	CPOBase::fileWrite(file_format, fp);
	CPOBase::fileWrite(image_size, fp);
	CPOBase::fileWrite(fp, default_filename);
	CPOBase::fileWrite(add_include_graphics, fp);
	CPOBase::fileWrite(add_time_stamp, fp);
	CPOBase::fileWrite(limit_count, fp);

	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CFtpComm::CFtpComm()
{
	reset();
}

CFtpComm::~CFtpComm()
{
}

void CFtpComm::reset()
{
	m_output_vec.clear();
}

void CFtpComm::init()
{
	lock_guard();
	reset();
}

CFtpComm CFtpComm::getValue()
{
	lock_guard();
	return *this;
}

void CFtpComm::setValue(CFtpComm& ftp_comm)
{
	lock_guard();
	*this = ftp_comm;
}

i32 CFtpComm::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)m_output_vec.size();

	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += m_output_vec[i].memSize();
	}
	return len;
}

i32 CFtpComm::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = (i32)m_output_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_output_vec[i].memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFtpComm::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	reset();

	i32 i, count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::checkRange(count, 255))
	{
		return buffer_ptr - buffer_pos;
	}

	m_output_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		m_output_vec[i].memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFtpComm::fileRead(FILE* fp)
{
	lock_guard();
	reset();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	i32 i, count = -1;
	CPOBase::fileRead(count, fp);
	if (!CPOBase::checkRange(count, 255))
	{
		return false;
	}

	m_output_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		m_output_vec[i].fileRead(fp);
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFtpComm::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	i32 i, count = (i32)m_output_vec.size();
	CPOBase::fileWrite(count, fp);

	for (i = 0; i < count; i++)
	{
		m_output_vec[i].fileWrite(fp);
	}
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
i32 DBFilterHistory::memSize()
{
	i32 len = 0;
	len += sizeof(cam_id);
	len += sizeof(job_id);
	len += sizeof(start_date);
	len += sizeof(end_date);
	len += sizeof(filter_mode);
	len += sizeof(offset);
	len += sizeof(limit_count);
	return len;
}

i32 DBFilterHistory::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(cam_id, buffer_ptr, buffer_size);
	CPOBase::memRead(job_id, buffer_ptr, buffer_size);
	CPOBase::memRead(start_date, buffer_ptr, buffer_size);
	CPOBase::memRead(end_date, buffer_ptr, buffer_size);
	CPOBase::memRead(filter_mode, buffer_ptr, buffer_size);
	CPOBase::memRead(offset, buffer_ptr, buffer_size);
	CPOBase::memRead(limit_count, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 DBFilterHistory::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(cam_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(job_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(start_date, buffer_ptr, buffer_size);
	CPOBase::memWrite(end_date, buffer_ptr, buffer_size);
	CPOBase::memWrite(filter_mode, buffer_ptr, buffer_size);
	CPOBase::memWrite(offset, buffer_ptr, buffer_size);
	CPOBase::memWrite(limit_count, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
i32 DBHistoryThumb::memSize()
{
	return CPOBase::getVectorMemSize(record_id_vec);
}

i32 DBHistoryThumb::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memReadVector(record_id_vec, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 DBHistoryThumb::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWriteVector(record_id_vec, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
DBJobResult::DBJobResult()
{
	record_id = 0;
	cam_id = 0;
	job_id = 0;
	job_name.clear();
	tool_result = 0;
	tool_count = 0;
	proc_time = 0;
	memset(&created_time, 0, sizeof(DateTime));
	is_calib = false;
	pixel_per_mm = 0;
	image_format = kPOSaveFormatNone;
}

i32 DBJobResult::memSize()
{
	i32 len = 0;
	len += sizeof(record_id);
	len += sizeof(cam_id);
	len += sizeof(job_id);
	len += CPOBase::getStringMemSize(job_name);
	len += sizeof(tool_result);
	len += sizeof(tool_count);
	len += sizeof(proc_time);
	len += sizeof(created_time);
	len += sizeof(is_calib);
	len += sizeof(pixel_per_mm);
	len += sizeof(image_format);
	return len;
}

i32 DBJobResult::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(record_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(cam_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(job_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, job_name);

	CPOBase::memWrite(tool_result, buffer_ptr, buffer_size);
	CPOBase::memWrite(tool_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(proc_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(created_time, buffer_ptr, buffer_size);
	CPOBase::memWrite(is_calib, buffer_ptr, buffer_size);
	CPOBase::memWrite(pixel_per_mm, buffer_ptr, buffer_size);
	CPOBase::memWrite(image_format, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 DBJobResult::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(record_id, buffer_ptr, buffer_size);
	CPOBase::memRead(cam_id, buffer_ptr, buffer_size);
	CPOBase::memRead(job_id, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, job_name);
	
	CPOBase::memRead(tool_result, buffer_ptr, buffer_size);
	CPOBase::memRead(tool_count, buffer_ptr, buffer_size);
	CPOBase::memRead(proc_time, buffer_ptr, buffer_size);
	CPOBase::memRead(created_time, buffer_ptr, buffer_size);
	CPOBase::memRead(is_calib, buffer_ptr, buffer_size);
	CPOBase::memRead(pixel_per_mm, buffer_ptr, buffer_size);
	CPOBase::memRead(image_format, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
CIODevParam::CIODevParam()
{
	m_base_in_regindex = 0;
	m_base_out_regindex = 0;
	m_pulse_time_ms = 40;
}

CModbusDev* CIODevParam::getModbusDev()
{
	return dynamic_cast<CModbusDev*>(this);
}

void CIODevParam::init()
{
	lock_guard();

	m_base_in_regindex = 0;
	m_base_out_regindex = 0;
	m_pulse_time_ms = 40;
	CModbusDev::init();
}

void CIODevParam::setUsedDevice(i32 used_device)
{
	lock_guard();
	m_used_device = used_device;
}

i32	CIODevParam::memSize()
{
	lock_guard();
	i32 len = 0;
	len += sizeof(m_base_in_regindex);
	len += sizeof(m_base_out_regindex);
	len += sizeof(m_pulse_time_ms);
	len += CModbusDev::memSize();
	return len;
}

i32	CIODevParam::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(m_base_in_regindex, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_base_out_regindex, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_pulse_time_ms, buffer_ptr, buffer_size);
	CModbusDev::memWrite(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32	CIODevParam::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(m_base_in_regindex, buffer_ptr, buffer_size);
	CPOBase::memRead(m_base_out_regindex, buffer_ptr, buffer_size);
	CPOBase::memRead(m_pulse_time_ms, buffer_ptr, buffer_size);
	CModbusDev::memRead(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32	CIODevParam::memImport(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(m_base_in_regindex, buffer_ptr, buffer_size);
	CPOBase::memRead(m_base_out_regindex, buffer_ptr, buffer_size);
	CPOBase::memRead(m_pulse_time_ms, buffer_ptr, buffer_size);
	CModbusDev::memImport(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CIODevParam::fileRead(FILE* fp)
{
	lock_guard();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_base_in_regindex, fp);
	CPOBase::fileRead(m_base_out_regindex, fp);
	CPOBase::fileRead(m_pulse_time_ms, fp);
	if (!CModbusDev::fileRead(fp))
	{
		return false;
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CIODevParam::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_base_in_regindex, fp);
	CPOBase::fileWrite(m_base_out_regindex, fp);
	CPOBase::fileWrite(m_pulse_time_ms, fp);
	CModbusDev::fileWrite(fp);

	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CIOInput::CIOInput()
{
	reset();
}

void CIOInput::reset()
{
	m_in_count = SCIO_MAXIN;

	memset(m_in_method, kIOInNone, sizeof(i32)*SCIO_MAXIN);
	memset(m_in_edge, kSCIOEdgeNone, SCIO_MAXIN);
	memset(m_in_force, kSCIOForceNone, SCIO_MAXIN);
	memset(m_in_mode, kSCIONormalMode, SCIO_MAXIN);

	for (i32 i = 0; i < SCIO_MAXIN; i++)
	{
		m_in_name[i] = QString("InputLine%1").arg(i + 1).toStdWString();
		m_in_job_include[i].clear();
	}

	m_use_program_bits = false;
	memset(m_job_select_table, 0xFF, sizeof(i32)*SCIO_MAXPROGRAM); //fill with (-1)
}

void CIOInput::init()
{
	lock_guard();
	reset();
}

i32 CIOInput::memSize()
{
	lock_guard();

	i32 len = 0;
	len += sizeof(m_in_count);
	len += sizeof(m_in_method[0])*m_in_count;
	len += sizeof(m_in_edge[0])*m_in_count;
	len += sizeof(m_in_force[0])*m_in_count;
	len += sizeof(m_in_mode[0])*m_in_count;

	for (i32 i = 0; i < m_in_count; i++)
	{
		len += CPOBase::getStringMemSize(m_in_name[i]);
		len += CPOBase::getStringMemSize(m_in_job_include[i]);
	}
	return len;
}

i32 CIOInput::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_in_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_in_method, m_in_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_in_edge, m_in_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_in_force, m_in_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_in_mode, m_in_count, buffer_ptr, buffer_size);

	for (i32 i = 0; i < m_in_count; i++)
	{
		CPOBase::memWrite(buffer_ptr, buffer_size, m_in_name[i]);
		CPOBase::memWrite(buffer_ptr, buffer_size, m_in_job_include[i]);
	}
	return buffer_ptr - buffer_pos;
}

i32 CIOInput::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 tmp_count = -1;
	CPOBase::memRead(tmp_count, buffer_ptr, buffer_size);
	if (CPOBase::checkRange(tmp_count, SCIO_MAXIN))
	{
		m_in_count = tmp_count;
		CPOBase::memRead(m_in_method, m_in_count, buffer_ptr, buffer_size);
		CPOBase::memRead(m_in_edge, m_in_count, buffer_ptr, buffer_size);
		CPOBase::memRead(m_in_force, m_in_count, buffer_ptr, buffer_size);
		CPOBase::memRead(m_in_mode, m_in_count, buffer_ptr, buffer_size);

		for (i32 i = 0; i < m_in_count; i++)
		{
			CPOBase::memRead(buffer_ptr, buffer_size, m_in_name[i]);
			CPOBase::memRead(buffer_ptr, buffer_size, m_in_job_include[i]);
		}
	}
	return buffer_ptr - buffer_pos;
}

i32 CIOInput::memExtSelSize()
{
	lock_guard();
	i32 len = 0;

	len += sizeof(m_use_program_bits);
	len += sizeof(m_job_select_table);
	return len;
}

i32 CIOInput::memExtSelWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_use_program_bits, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_job_select_table, SCIO_MAXPROGRAM, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CIOInput::memExtSelRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_use_program_bits, buffer_ptr, buffer_size);
	CPOBase::memRead(m_job_select_table, SCIO_MAXPROGRAM, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

bool CIOInput::fileRead(FILE* fp)
{
	lock_guard();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_in_count, fp);
	if (!CPOBase::checkRange(m_in_count, SCIO_MAXIN))
	{
		return false;
	}

	CPOBase::fileRead(m_in_method, SCIO_MAXIN, fp);
	CPOBase::fileRead(m_in_edge, SCIO_MAXIN, fp);
	CPOBase::fileRead(m_in_force, SCIO_MAXIN, fp);
	CPOBase::fileRead(m_in_mode, SCIO_MAXIN, fp);

	for (i32 i = 0; i < SCIO_MAXIN; i++)
	{
		CPOBase::fileRead(fp, m_in_name[i]);
		CPOBase::fileRead(fp, m_in_job_include[i]);
	}

	CPOBase::fileRead(m_use_program_bits, fp);
	CPOBase::fileRead(m_job_select_table, SCIO_MAXPROGRAM, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CIOInput::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_in_count, fp);
	CPOBase::fileWrite(m_in_method, SCIO_MAXIN, fp);
	CPOBase::fileWrite(m_in_edge, SCIO_MAXIN, fp);
	CPOBase::fileWrite(m_in_force, SCIO_MAXIN, fp);
	CPOBase::fileWrite(m_in_mode, SCIO_MAXIN, fp);

	for (i32 i = 0; i < SCIO_MAXIN; i++)
	{
		CPOBase::fileWrite(fp, m_in_name[i]);
		CPOBase::fileWrite(fp, m_in_job_include[i]);
	}

	CPOBase::fileWrite(m_use_program_bits, fp);
	CPOBase::fileWrite(m_job_select_table, SCIO_MAXPROGRAM, fp);
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

void CIOInput::resetProgram(i32 cam_pos, i32 job_id)
{
	lock_guard();
	i32 prog_count = 1 << SCIO_PROGBITS;
	if (job_id >= 0)
	{
		prog_count = po::_min(cam_pos + prog_count, SCIO_MAXPROGRAM);
		for (i32 i = cam_pos; i < prog_count; i++)
		{
			if (m_job_select_table[i] == job_id)
			{
				m_job_select_table[i] = -1;
				break;
			}
		}
	}
	else
	{
		if (cam_pos + prog_count <= SCIO_MAXPROGRAM)
		{
			memset(m_job_select_table + cam_pos, 0xFF, sizeof(i32)*prog_count);
		}
	}
}

i32 CIOInput::getProgram(i32 index)
{
	lock_guard();
	if (!CPOBase::checkIndex(index, SCIO_MAXPROGRAM))
	{
		return -1;
	}
	return m_job_select_table[index];
}

postring CIOInput::getJobIncludeTagName(i32 index)
{
	lock_guard();
	if (!CPOBase::checkIndex(index, SCIO_MAXIN))
	{
		return postring();
	}
	return m_in_job_include[index];
}

bool CIOInput::isUsedExtProgram()
{
	lock_guard();
	return m_use_program_bits;
}

//////////////////////////////////////////////////////////////////////////
CIOOutput::CIOOutput()
{
	reset();
}

CIOOutput::~CIOOutput()
{
}

void CIOOutput::reset()
{
	m_out_count = SCIO_MAXOUT;

	memset(m_out_method, kIOOutNone, sizeof(i32)*SCIO_MAXOUT);
	memset(m_out_force, kSCIOForceNone, SCIO_MAXOUT);
	memset(m_out_mode, kSCIONormalMode, SCIO_MAXOUT);
	memset(m_out_address, 0xFF, sizeof(i16)*kIOOutCount);

	for (i32 i = 0; i < SCIO_MAXOUT; i++)
	{
		m_out_name[i] = QString("OutputLine%1").arg(i + 1).toStdWString();
		m_out_job_result[i].clear();
	}
}

void CIOOutput::init()
{
	lock_guard();
	reset();
}

CIOOutput CIOOutput::getValue()
{
	lock_guard();
	CIOOutput tmp;
	tmp.m_out_count = m_out_count;
	CPOBase::memCopy(tmp.m_out_method, m_out_method, m_out_count);
	CPOBase::memCopy(tmp.m_out_force, m_out_force, m_out_count);
	CPOBase::memCopy(tmp.m_out_mode, m_out_mode, m_out_count);
	CPOBase::memCopy(tmp.m_out_address, m_out_address, kIOOutCount);

	for (i32 i = 0; i < m_out_count; i++)
	{
		tmp.m_out_name[i] = m_out_name[i];
		tmp.m_out_job_result[i] = m_out_job_result[i];
	}
	return tmp;
}

void CIOOutput::setValue(CIOOutput& value)
{
	lock_guard();
	anlock_guard(value);
	m_out_count = value.m_out_count;
	CPOBase::memCopy(m_out_method, value.m_out_method, m_out_count);
	CPOBase::memCopy(m_out_force, value.m_out_force, m_out_count);
	CPOBase::memCopy(m_out_mode, value.m_out_mode, m_out_count);
	CPOBase::memCopy(m_out_address, value.m_out_address, kIOOutCount);

	for (i32 i = 0; i < m_out_count; i++)
	{
		m_out_name[i] = value.m_out_name[i];
		m_out_job_result[i] = value.m_out_job_result[i];
	}
	updateMethod2Addr();
}

i32 CIOOutput::memSize()
{
	lock_guard();

	i32 len = 0;
	len += sizeof(m_out_count);
	len += sizeof(m_out_method[0])*m_out_count;
	len += sizeof(m_out_force[0])*m_out_count;
	len += sizeof(m_out_mode[0])*m_out_count;

	for (i32 i = 0; i < m_out_count; i++)
	{
		len += CPOBase::getStringMemSize(m_out_name[i]);
		len += CPOBase::getStringMemSize(m_out_job_result[i]);
	}
	return len;
}

i32 CIOOutput::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_out_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_out_method, m_out_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_out_force, m_out_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_out_mode, m_out_count, buffer_ptr, buffer_size);

	for (i32 i = 0; i < m_out_count; i++)
	{
		CPOBase::memWrite(buffer_ptr, buffer_size, m_out_name[i]);
		CPOBase::memWrite(buffer_ptr, buffer_size, m_out_job_result[i]);
	}
	return buffer_ptr - buffer_pos;
}

i32 CIOOutput::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 tmp_count = -1;
	CPOBase::memRead(tmp_count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(tmp_count))
	{
		m_out_count = tmp_count;
		CPOBase::memRead(m_out_method, m_out_count, buffer_ptr, buffer_size);
		CPOBase::memRead(m_out_force, m_out_count, buffer_ptr, buffer_size);
		CPOBase::memRead(m_out_mode, m_out_count, buffer_ptr, buffer_size);

		for (i32 i = 0; i < m_out_count; i++)
		{
			CPOBase::memRead(buffer_ptr, buffer_size, m_out_name[i]);
			CPOBase::memRead(buffer_ptr, buffer_size, m_out_job_result[i]);
		}
	}

	updateMethod2Addr();
	return buffer_ptr - buffer_pos;
}

bool CIOOutput::fileRead(FILE* fp)
{
	lock_guard();
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_out_count, fp);
	if (!CPOBase::isCount(m_out_count))
	{
		return false;
	}

	CPOBase::fileRead(m_out_method, SCIO_MAXOUT, fp);
	CPOBase::fileRead(m_out_force, SCIO_MAXOUT, fp);
	CPOBase::fileRead(m_out_mode, SCIO_MAXOUT, fp);

	for (i32 i = 0; i < SCIO_MAXOUT; i++)
	{
		CPOBase::fileRead(fp, m_out_name[i]);
		CPOBase::fileRead(fp, m_out_job_result[i]);
	}
	updateMethod2Addr();
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CIOOutput::fileWrite(FILE* fp)
{
	lock_guard();
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	CPOBase::fileWrite(m_out_count, fp);
	CPOBase::fileWrite(m_out_method, SCIO_MAXOUT, fp);
	CPOBase::fileWrite(m_out_force, SCIO_MAXOUT, fp);
	CPOBase::fileWrite(m_out_mode, SCIO_MAXOUT, fp);

	for (i32 i = 0; i < SCIO_MAXOUT; i++)
	{
		CPOBase::fileWrite(fp, m_out_name[i]);
		CPOBase::fileWrite(fp, m_out_job_result[i]);
	}
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

void CIOOutput::updateMethod2Addr()
{
	lock_guard();
	memset(m_out_address, 0xFF, sizeof(i16)*kIOOutCount);
	for (i32 i = 0; i < m_out_count; i++)
	{
		if (!CPOBase::checkIndex(m_out_method[i], kIOOutNone, kIOOutCount))
		{
			continue;
		}
		m_out_address[m_out_method[i]] = i;
	}
}

//////////////////////////////////////////////////////////////////////////
IOSignal::IOSignal()
{
	signal_type = 0;
	for (i32 i = 0; i < kPOLangCount; i++)
	{
		signal_disp_name[i] = L"";
	}

	is_allow_edgetype = false;
	is_allow_jobdata = false;
	is_allow_force = true;
	is_allow_mode = true;
}

IOSignal::IOSignal(i32 index, 
					const powstring& desc_en_str, const powstring& desc_cn_str, const powstring& desc_ko_str,
					bool allow_edgetype, bool allow_jobdata, bool allow_force, bool allow_invert)
{
	signal_type = index;
	signal_disp_name[kPOLangEnglish] = desc_en_str;
	signal_disp_name[kPOLangChinese] = desc_cn_str;
	signal_disp_name[kPOLangKorean] = desc_ko_str;

	is_allow_edgetype = allow_edgetype;
	is_allow_jobdata = allow_jobdata;
	is_allow_force = allow_force;
	is_allow_mode = allow_invert;
}

i32 IOSignal::memSize()
{
	i32 len = 0;
	i32 i, count = kPOLangCount;
	len += sizeof(signal_type);
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += CPOBase::getStringMemSize(signal_disp_name[i]);
	}

	len += sizeof(is_allow_edgetype);
	len += sizeof(is_allow_jobdata);
	len += sizeof(is_allow_force);
	len += sizeof(is_allow_mode);
	return len;
}

i32 IOSignal::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 i, count = kPOLangCount;
	CPOBase::memRead(signal_type, buffer_ptr, buffer_size);
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(count))
	{
		for (i = 0; i < count; i++)
		{
			CPOBase::memRead(buffer_ptr, buffer_size, signal_disp_name[i]);
		}
	}

	CPOBase::memRead(is_allow_edgetype, buffer_ptr, buffer_size);
	CPOBase::memRead(is_allow_jobdata, buffer_ptr, buffer_size);
	CPOBase::memRead(is_allow_force, buffer_ptr, buffer_size);
	CPOBase::memRead(is_allow_mode, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 IOSignal::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 i, count = kPOLangCount;
	CPOBase::memWrite(signal_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		CPOBase::memWrite(buffer_ptr, buffer_size, signal_disp_name[i]);
	}

	CPOBase::memWrite(is_allow_edgetype, buffer_ptr, buffer_size);
	CPOBase::memWrite(is_allow_jobdata, buffer_ptr, buffer_size);
	CPOBase::memWrite(is_allow_force, buffer_ptr, buffer_size);
	CPOBase::memWrite(is_allow_mode, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

