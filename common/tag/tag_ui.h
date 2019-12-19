#pragma once
#include "sc_define.h"

const i32 kPOActionCount = 3;

enum TagFormatMaskTypes
{
	kTagFormatMaskUnit = 0x0F00,
	kTagFormatMaskDecimal = 0x0F0,
	kTagFormatMaskDataType = 0x000F
};

struct UIString
{
	powstring			ui_string[kPOLangCount];

public:
	UIString();

	void				init();

	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct RangeUIData
{
	f32					min_value;
	f32					max_value;
	f32					step_value;
	postring*			minmax_reftag;

public:
	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct ActionUIData
{
	postring*			action_tag_ptr;
	u8					mode;
	f32					src_gain;
	f32					dst_gain;
	f32					offset;

public:
	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct ActionUIGroup
{
	i32					count;
	ActionUIData		action_ui_data[kPOActionCount];

public:
	void				initActionGroup();
	void				freeActionGroup();

	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct EnumUIData
{
	u16					index;
	u16					count;
	u16*				index_ptr;
	UIString*			string_ptr;

public:
	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);
};

struct GroupUIData
{
	u16					limit;
	u16					count;
	i32*				index_ptr;
	postring*			string_ptr;

public:
	void				initBuffer(i32 num);
	void				freeBuffer();
	void				addGroupUIData(i32 value, const postring& tag_name);

	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);
};

class CTagUI
{
public:
	CTagUI();
	virtual ~CTagUI();

	void				freeBuffer();

	i32					memSize();
	i32					memRead(u8*& buffer_ptr, i32& buffer_size);
	i32					memWrite(u8*& buffer_ptr, i32& buffer_size);

	bool				setUIOption(const postring& tag_name, i32 type, i32 flag = kTagUIFlagNone, i32 count = 0);
	void				setUIString(const powstring* tag_text_str_ptr);
	void				setUIEnumString(i32 index, const powstring* tag_text_str_ptr);
	void				setUIRangeData(const postring& start_tag_name, const postring& end_tag_name, f32 step_value = 0);
	void				setUIAction(i32 action_mode, const postring& dst_tag_name, const postring& src_tag_name, f32 gain = 1.0f, f32 offset = 0.0f);
	void				setUIUnitFormat(i32 unit_format, const postring& unit_string = "");
	void				setUIDispFormat(const postring tag_disp_format);

	void				setUIGroup(i32 group_action, i32 link_count, i32 action_count);
	void				addUIGroupData(i32 value, const postring& tag_name);
	void				linkUIGroupData(const postring& tag_name);

	template <class T>
	void setUIRangeData(T min_value, T max_value, T step_value = 0)
	{
		if (m_tag_ui_type == kTagUINumber)
		{
			u.range.min_value = min_value;
			u.range.max_value = max_value;
			u.range.step_value = step_value;
		}
	}
	template <class T>
	void setUIRangeData(T min_value, const postring& max_reftag, T step_value = 0)
	{
		if (m_tag_ui_type == kTagUINumber && u.range.minmax_reftag != NULL)
		{
			u.range.min_value = min_value;
			u.range.minmax_reftag[1] = max_reftag;
			u.range.step_value = step_value;
		}
	}
	template <class T>
	void setUIRangeData(const postring& min_reftag, T max_value, T step_value = 0)
	{
		if (m_tag_ui_type == kTagUINumber && u.range.minmax_reftag != NULL)
		{
			u.range.minmax_reftag[0] = min_reftag;
			u.range.max_value = max_value;
			u.range.step_value = step_value;
		}
	}
	template <class T>
	void getUIRangeData(T& min_value, T& max_value, T& step_value)
	{
		if (m_tag_ui_type == kTagUINumber)
		{
			min_value = u.range.min_value;
			max_value = u.range.max_value;
			step_value = u.range.step_value;
		}
	}

public:
	postring				m_tag_name;
	u16						m_tag_ui_type;
	u16						m_tag_ui_flag;
	UIString				m_tag_ui_string;

	i32						m_tag_ui_format;
	postring				m_tag_ui_unit_string;
	postring				m_tag_ui_disp_format;

	union
	{
		RangeUIData			range;
		EnumUIData			enums;
		ActionUIGroup		action;
	} u;

	bool					m_is_group_ui;
	u16						m_group_action;
	GroupUIData				m_group_data;	
	GroupUIData				m_link_data;
};

class CTagUINode;
typedef std::vector<CTagUI*> TagUIVector;
typedef std::vector<CTagUINode*> TagUINodeVector;

class CTagUINode
{
public:
	CTagUINode();
	virtual ~CTagUINode();

	void					init();
	void					init(i32 node_type);
	void					init(i32 node_type, const postring& node_name);
	void					removeAll();

	void					setTagUINodeType(i32 node_type);
	void					setTagUINodeName(const postring& node_name);
	void					setTagUINodeString(const powstring* tagui_text_str_ptr);
	void					addTagUINode(CTagUINode* tag_ui_node_ptr);
	CTagUINode*				addTagUINode(const postring& tag_node_name);
	CTagUINode*				addTagUINode(i32 tag_node_type, const postring& tag_node_name);
	CTagUI*					addTagUI(const postring& tag_name, i32 type, i32 style = kTagUIFlagNone, i32 count = 0);
	
	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	CTagUI*					at(const postring& node_name);

public:
	i32						m_node_type;
	postring				m_node_name;
	UIString				m_node_disp_name;

	TagUIVector				m_tag_ui_vec;
	TagUINodeVector			m_tag_ui_node_vec;
};