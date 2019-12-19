#pragma once
#include "sc_define.h"
#include "struct.h"
#include "struct/run_table.h"
#include "tag_node.h"
#include "region_info.h"
#include "runtime_history.h"
#ifdef POR_DEVICE
# include "sc_camera_set.h"
#else
#include "camera_setting.h"
#endif

#pragma pack(push, 4)

class CSecuritySetting : public CLockGuard
{
public:
	CSecuritySetting();

	void					init();
	void					reset();

	CSecuritySetting		getValue();
	void					setValue(CSecuritySetting& other);
	
	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

	bool					getPermission(i32 cmd);
	bool					checkPermissionAuth(postring& chk_password);
	void					setPassword(postring& new_password);

public:
	i32						m_permission_count;	//하위기에 설정가능한 보안설정의 개수
	bool					m_permission_enabled[PO_MAX_PERMISSION];	//보안설정류형별로 보안이 필요한가를 설정하는 기발
	postring				m_cur_password;		//보안설정에 리용되는 하위기사용자암호
};

class CGeneralSetting : public CLockGuard
{
public:
	CGeneralSetting();

	void					init();
	void					reset();

	CGeneralSetting			getValue();
	void					setValue(CGeneralSetting& other);
	void					setDebugLevel(i32 level);

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

	inline i32				getLanguageHL()		{ lock_guard(); return m_language_hl; };
	inline i32				getHistorySetting() { lock_guard(); return m_history_setting; };
	inline i32				getDecimalDisp()	{ lock_guard(); return m_decimal_disp; };
	inline i32				getDebugLevel()		{ lock_guard(); return m_debug_level; };
	inline i32				useLastSnap()		{ lock_guard(); return m_use_last_snap; };

	inline bool				isRemoteConntable() { lock_guard(); return m_is_enable_remote; };
	inline bool				isAutoSyncDateTime() { lock_guard(); return m_is_sync_datetime; };

public:
	i32						m_angle_unit;
	i32						m_language_hl;		//상위기 언어설정
	i32						m_history_setting;	//하위기 리력설정
	i32						m_decimal_disp;		//상위기 측정자료의 출력자리수
	i32						m_debug_level;		//하위기 검사로그준위
	bool					m_use_last_snap;	//하위기가 Teach, JobAdd시에 마지막프레임을 사용하겠는가 아니면 새로운 프레임을?
	bool					m_is_enable_remote;	//상위기가 하위기에 다른콤퓨터에서 접속가능?
	bool					m_is_sync_datetime;	//상위기가 접속할때마다 하위기는 시간을 재설정한다.
};

struct LocatorParam
{
	u8						default_edge_strength;
	i32						default_search_accuarcy;
	f32						min_match_score;
	i32						rotation_tolerance;
	i32						scale_tolerance;

	i32						min_contour_pixel_count;
	i32 					min_pyramid_basic_size_ght;
	i32 					min_pyramid_basic_size_rpt;
	f32						max_pyramid_level_density;
	f32						min_pyramid_level_inherit;
	f32						min_roi_contribution;
	f32						min_basic_integral;
	f32						min_sub_match_rate;
	f32						min_sub_count;

	i32						ght_cell_interval;
	i32						ght_angle_count;
	i32						ght_rtable_cell;
	i32						ght_angle_step_inv;
	i32						ght_pixel_step;
	i32						ght_contour_min_size;

public:
	void					init();
};

struct BlobParam
{
	i32						threshold_mode;
	u8						threshold;
	i32						blob_color_mode;
	bool					allow_boundary;
	i32						blur_size;

public:
	void					init();
};

struct EdgeParam
{
	u8						default_edge_strength;
	u8						default_edge_width;
	i32						max_gap_size;
	i32						max_angle_rate;
	i32						max_angle_diff;
	i32						min_size;
	i32						max_size;

public:
	void					init();
};

struct CircleParam
{
	u8						default_edge_strength;
	u8						default_edge_width;
	i32						max_arc_angle_diff;
	i32						max_arc_overlap;
	i32						max_tangent_angle;
	i32						max_dist;
	i32						max_radius_diff;
	i32						max_center_dist;
	i32						min_circle_line;
	i32						max_circle_line;

public:
	void					init();
};

struct CountingParam
{
	u8						default_threshold_mode;
	u8						default_low_threshold;
	u8						default_high_threshold;
	u8						counting_blur_size;

public:
	void					init();
};

struct FilterParam
{
	u8						default_filter;
	u8						default_kernel_size;
	u8						default_threshold_mode;
	u8						default_threshold;
	u8						default_clip_min;
	u8						default_clip_max;
	u8						default_kernel_gain;

public:
	void					init();
};

struct SurfaceFlawParam
{
	bool					use_mask;
	u8						mask_method;
	i32						mask_segment_size;
	i32						mask_smooth_factor;
	u8						mask_edge_contrast;
	i32						mask_hole_fill_size;
	i32						mask_median_factor;
	i32						mask_erode_factor;

	u8						detection_type;
	u8						display_type;
	i32						sampling_factor;
	bool					auto_smoothing;
	i32						smooth_factor;
	u8						detection_axis;
	i32						detection_size;
	u8						min_contrast;
	i32						min_flaw_area;
	i32						max_flaw_area;

public:
	void					init();
};

struct BeadParam
{
	u8						default_width;
	u8						default_width_tolerance;
	u8						default_position_tolerance;
	u8						default_edge_strength;
	u8						clip_end_points;

	u16						bead_caliper_size;
	u16						bead_caliper_space;
	u16						bead_pair_size;
	u16						bead_threshold;

	u16						bead_edge_height;
	u16						bead_edge_caliper_size;
	u16						bead_edge_caliper_space;
	u16						bead_edge_pair_size;
	u16						bead_edge_threshold;

	u16						bead_filter_size;
	u16						bead_min_contrast;
	u16						bead_max_contrast;
	u16						bead_edge_direction;
	u8						bead_edge0_transition;
	u8						bead_edge1_transition;

	f32						bead_min_width;
	f32						bead_max_width;
	f32						bead_defect_min_size;
	f32						bead_defect_max_size;
	f32						bead_defect_min_area;
	f32						bead_defect_max_area;

	f32						bead_gap_min_size;
	f32						bead_gap_max_size;

	f32						bead_position_threshold;
	f32						bead_position_min_size;
	f32						bead_position_max_size;
	f32						bead_position_min_area;
	f32						bead_position_max_area;

public:
	void					init();
};

struct DefectEdgeParam
{
	u16						caliper_size;
	u16						caliper_space;
	u16						caliper_threshold;

	u16						filter_size;
	u16						min_contrast;
	u16						max_contrast;
	u16						edge_pair_size;

	//fit parameter
	f32						fit_distance_filter;
	f32						fit_distance_furthest;

	//defect parameter
	f32						min_distance;
	f32						max_distance;
	f32						defect_min_size;
	f32						defect_max_size;
	f32						defect_min_area;
	f32						defect_max_area;

	//gap parameter
	f32						gap_min_size;
	f32						gap_max_size;
	
public:
	void					init();
};

class CEngineParam : public CLockGuard
{
public:
	CEngineParam();

	void					init();
	void					reset();

	CEngineParam			getValue();
	void					setValue(CEngineParam& other);

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	
	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

	inline LocatorParam		getLocatorParam()	{ lock_guard(); return m_locator_param; };
	inline BlobParam		getBlobParam()		{ lock_guard(); return m_blob_param; };
	inline CountingParam	getCountingParam()	{ lock_guard(); return m_counting_param; };
	inline FilterParam		getFilterParam()	{ lock_guard(); return m_filter_param; };
	inline i32				getFocusThreshold()	{ lock_guard(); return m_th_min_focus; };
	
public:
	LocatorParam			m_locator_param;
	BlobParam				m_blob_param;
	EdgeParam				m_edge_param;
	CircleParam				m_circle_param;
	FilterParam				m_filter_param;
	CountingParam			m_counting_param;
	SurfaceFlawParam		m_surflaw_param;
	DefectEdgeParam			m_defect_edge_param;
	BeadParam				m_bead_param;
	i32						m_th_min_focus;
};

class CIODevParam : public CModbusDev
{
public:
	CIODevParam();

	void					init();

	CModbusDev*				getModbusDev();
	void					setUsedDevice(i32 used_device);

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memImport(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
	
	inline i32				getUsedDevice() { lock_guard();  return m_used_device; };
	inline u16				getPulseTimeMs() { lock_guard();  return m_pulse_time_ms; };

public:
	i32						m_base_in_regindex;	 //RS232IO/Modbus통신에서 사용하는 입구등록기의 기초인덱스
	i32						m_base_out_regindex; //RS232IO/Modbus통신에서 사용하는 출구등록기의 기초인덱스
	u16						m_pulse_time_ms;	 //임플스너비는 InternalIO에서만 유용하다.
};

class CIOInput : public CLockGuard
{
public:
	CIOInput();

	void					init();
	void					reset();
	void					resetProgram(i32 index, i32 model_id = -1);

	CIOInput				getValue();

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	i32						memExtSelSize();
	i32						memExtSelWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memExtSelRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

	i32						getProgram(i32 index);
	postring				getJobIncludeTagName(i32 index);
	bool					isUsedExtProgram();

public:
	//device input pin -> functions pairs(method, mode)
	i32						m_in_count;						//입력선의 개수
	powstring				m_in_name[SCIO_MAXIN];			//입력선이름
	i32						m_in_method[SCIO_MAXIN];		//입력선신호종류 Online/Offline/Run/ClearError ...
	u8						m_in_edge[SCIO_MAXIN];			//입력선경계
	u8						m_in_force[SCIO_MAXIN];			//입력선강제방식/ForceON/ForceOFF
	u8						m_in_mode[SCIO_MAXIN];			//입력선방식 Invert/NCMode
	postring				m_in_job_include[SCIO_MAXIN];	//입력선에 할당된 도구태그이름

	//Camera Bit3~Bit0 Program Bit3~Bit0 -> program_index
	//job_select_table[program_index] = Job ID
	bool					m_use_program_bits;						//입력선에 의한 모형선택기발
	i32						m_job_select_table[SCIO_MAXPROGRAM];	//입력선에 의한 모형선택표
};

class CIOOutput : public CLockGuard
{
public:
	CIOOutput();
	~CIOOutput();

	void					init();
	void					reset();
	void					updateMethod2Addr();

	CIOOutput				getValue();
	void					setValue(CIOOutput& value);

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

public:
	i32						m_out_count;					// 출력선의 개수
	powstring				m_out_name[SCIO_MAXOUT];		// 출력선이름
	i32						m_out_method[SCIO_MAXOUT];		// 출력선신호종류
	u8						m_out_force[SCIO_MAXOUT];		// 출력선강제방식
	u8						m_out_mode[SCIO_MAXOUT];		// 출력선방식
	postring				m_out_job_result[SCIO_MAXOUT];	// 출력선에 할당된 도구결과태그이름
	
	i16						m_out_address[kIOOutCount];
};

struct IOSignal
{
	i32						signal_type;
	powstring				signal_disp_name[kPOLangCount];
	bool					is_allow_edgetype;
	bool					is_allow_jobdata;
	bool					is_allow_force;
	bool					is_allow_mode;

public:
	IOSignal();
	IOSignal(i32 index,
			const powstring& desc_en_str, const powstring& desc_cn_str,
			const powstring& desc_ko_str, bool allow_edgetype = false,
			bool allow_jobdata = false, bool allow_force = true, bool allow_invert = true);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
};
typedef std::vector<IOSignal> IOSignalResourceVec;

struct ModBusItem 
{
	postring				modbus_tag_name;	//Modbus통신에 리용되는 태그이름
	u16						modbus_address;		//시작주소(대역적)
	i32						modbus_data_type;	//Modbus의 자료형(상위기용)
	u16						modbus_size;		//내용의 크기(단위:u16)

public:
	ModBusItem();

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};
typedef std::vector<ModBusItem> ModBusItemVector;

class CModBusComm : public CLockGuard
{
public:
	CModBusComm();
	~CModBusComm();

	void					init();
	void					reset();

	CModBusComm				getValue();
	void					setValue(CModBusComm& value);

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

public:
	ModBusItemVector		m_modbus_input;			//Modbus통신입력에서 리용되는 목록
	ModBusItemVector		m_modbus_output;		//Modbus통신출력에서 리용되는 목록
};

struct ModbusAllocation
{
	u16						m_cam_count;

	//input register
	u16						m_input_address[PO_CAM_COUNT];		//카메라별 입력파라메터공간 시작주소 
	u16						m_input_size[PO_CAM_COUNT];			//카메라별 입력파라메터공간 크기

	//output register
	u16						m_output_address[PO_CAM_COUNT];		//카메라별 출력파라메터공간 시작주소
	u16						m_output_size[PO_CAM_COUNT];		//카메라별 출력파라메터공간 크기

public:
	ModbusAllocation();

	void					init();

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					checkModbusAllocation(i32 cam_id, CModBusComm& modbus_comm);
};

struct FtpOutput
{
	bool					is_enabled;
	powstring				ftp_name;
	u8						dev_index;
	postring				cond_tag_name;
	u8						file_format;
	u8						image_size;
	powstring				default_filename;
	bool					add_include_graphics;
	bool					add_time_stamp;
	i32						limit_count;

public:
	FtpOutput();
	~FtpOutput();

	void					init();

	i32						memSize();
	i32 					memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32 					memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};
typedef std::vector<FtpOutput>	FtpOutputVector;

class CFtpComm : public CLockGuard
{
public:
	CFtpComm();
	~CFtpComm();

	void					init();
	void					reset();

	CFtpComm				getValue();
	void					setValue(CFtpComm& ftp_comm);

	i32						memSize();
	i32 					memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32 					memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

public:
	FtpOutputVector			m_output_vec;
};

struct OpcItem
{
	postring				opc_tag_name;		// OPC통신에 리용되는 태그이름
	i32						opc_access_mode;	// 접근방식. OpcAccessMode
	i32						opc_data_type;		// OPC의 자료형(상위기용: OutputDataType)

public:
	OpcItem();

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);
};
typedef std::vector<OpcItem> OpcItemVector;

class COpcComm : public CLockGuard
{
public:
	COpcComm();
	~COpcComm();

	void					init();
	void					reset();

	COpcComm				getValue();
	void					setValue(COpcComm& value);

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);

	bool					fileRead(FILE* fp);
	bool					fileWrite(FILE* fp);

public:
	OpcItemVector			m_opc_items;			// OPC통신입력에서 리용되는 목록
};

struct DBFilterHistory
{
	i32						cam_id;
	i32						job_id;
	DateTime				start_date;
	DateTime				end_date;
	u8						filter_mode;	//Enum ResultOperTypes 
	i32						offset;
	i32						limit_count;

public:
	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct DBHistoryThumb
{
	i64vector				record_id_vec;

public:
	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct DBJobResult
{
	i64						record_id;
	u8						cam_id;
	i32						job_id;
	postring				job_name;
	u8						tool_result;
	i32						tool_count;
	i32						proc_time;
	DateTime				created_time;
	bool					is_calib;
	f32						pixel_per_mm;
	i32						image_format;

public:
	DBJobResult();

	i32						memSize();
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
};

#pragma pack(pop)

#if defined(POR_DEVICE)
#include "sc_internal_struct.h"
#endif
