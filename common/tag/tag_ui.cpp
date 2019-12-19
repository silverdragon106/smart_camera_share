#include "tag_ui.h"
#include "tag_define_text.h"
#include "base.h"

UIString::UIString()
{
	init();
}

void UIString::init()
{
	for (i32 i = 0; i < kPOLangCount; i++)
	{
		ui_string[i] = L"";
	}
}

i32 UIString::memSize()
{
	i32 i, len = 0;
	for (i = 0; i < kPOLangCount; i++)
	{
		len += CPOBase::getStringMemSize(ui_string[i]);
	}
	return len;
}

i32 UIString::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	for (i32 i = 0; i < kPOLangCount; i++)
	{
		CPOBase::memWrite(buffer_ptr, buffer_size, ui_string[i]);
	}
	return buffer_ptr - buffer_pos;
}

i32 UIString::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	for (i32 i = 0; i < kPOLangCount; i++)
	{
		CPOBase::memRead(buffer_ptr, buffer_size, ui_string[i]);
	}
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
i32 RangeUIData::memSize()
{
	i32 len = 0;
	len += sizeof(min_value);
	len += sizeof(max_value);
	len += sizeof(step_value);
	len += CPOBase::getStringMemSize(minmax_reftag[0]);
	len += CPOBase::getStringMemSize(minmax_reftag[1]);
	return len;
}

i32 RangeUIData::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(min_value, buffer_ptr, buffer_size);
	CPOBase::memWrite(max_value, buffer_ptr, buffer_size);
	CPOBase::memWrite(step_value, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, minmax_reftag[0]);
	CPOBase::memWrite(buffer_ptr, buffer_size, minmax_reftag[1]);
	return buffer_ptr - buffer_pos;
}

i32 RangeUIData::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	POSAFE_DELETE_ARRAY(minmax_reftag);
	u8* buffer_pos = buffer_ptr;
	
	minmax_reftag = po_new postring[2];

	CPOBase::memRead(min_value, buffer_ptr, buffer_size);
	CPOBase::memRead(max_value, buffer_ptr, buffer_size);
	CPOBase::memRead(step_value, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, minmax_reftag[0]);
	CPOBase::memRead(buffer_ptr, buffer_size, minmax_reftag[1]);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
void ActionUIGroup::initActionGroup()
{
	count = 0;
	for (i32 i = 0; i < kPOActionCount; i++)
	{
		action_ui_data[i].action_tag_ptr = po_new postring[2];
	}
}

void ActionUIGroup::freeActionGroup()
{
	count = 0;
	for (i32 i = 0; i < kPOActionCount; i++)
	{
		POSAFE_DELETE_ARRAY(action_ui_data[i].action_tag_ptr);
	}
}

i32 ActionUIGroup::memSize()
{
	i32 i, len = 0;
	for (i = 0; i < count; i++)
	{
		len += action_ui_data[i].memSize();
	}
	return len + sizeof(count);
}

i32 ActionUIGroup::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	freeActionGroup();

	i32 i, tmp_count = 0;
	CPOBase::memRead(tmp_count, buffer_ptr, buffer_size);
	if (!CPOBase::checkRange(tmp_count, kPOActionCount))
	{
		return buffer_ptr - buffer_pos;
	}

	count = tmp_count;
	for (i = 0; i < count; i++)
	{
		action_ui_data[i].memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 ActionUIGroup::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i32 i = 0; i < count; i++)
	{
		action_ui_data[i].memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
i32 ActionUIData::memSize()
{
	i32 len = 0;
	len += CPOBase::getStringMemSize(action_tag_ptr[0]);
	len += CPOBase::getStringMemSize(action_tag_ptr[1]);
	len += sizeof(mode);
	len += sizeof(src_gain);
	len += sizeof(dst_gain);
	len += sizeof(offset);
	return len;
}

i32 ActionUIData::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(buffer_ptr, buffer_size, action_tag_ptr[0]);
	CPOBase::memWrite(buffer_ptr, buffer_size, action_tag_ptr[1]);
	CPOBase::memWrite(mode, buffer_ptr, buffer_size);
	CPOBase::memWrite(src_gain, buffer_ptr, buffer_size);
	CPOBase::memWrite(dst_gain, buffer_ptr, buffer_size);
	CPOBase::memWrite(offset, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 ActionUIData::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	//must this value will be initialized, when call this function
	action_tag_ptr = po_new postring[2];

	CPOBase::memRead(buffer_ptr, buffer_size, action_tag_ptr[0]);
	CPOBase::memRead(buffer_ptr, buffer_size, action_tag_ptr[1]);
	CPOBase::memRead(mode, buffer_ptr, buffer_size);
	CPOBase::memRead(src_gain, buffer_ptr, buffer_size);
	CPOBase::memRead(dst_gain, buffer_ptr, buffer_size);
	CPOBase::memRead(offset, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
i32 EnumUIData::memSize()
{
	i32 i, len = 0;
	len += sizeof(count);
	if (!CPOBase::isCount(count) || !index_ptr || !string_ptr)
	{
		return len;
	}

	len += sizeof(u16)*count;
	for (i = 0; i < count; i++)
	{
		len += string_ptr[i].memSize();
	}
	return len;
}

i32 EnumUIData::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(count) && index_ptr != NULL && string_ptr != NULL)
	{
		CPOBase::memWrite(index_ptr, count, buffer_ptr, buffer_size);
		for (i32 i = 0; i < count; i++)
		{
			string_ptr[i].memWrite(buffer_ptr, buffer_size);
		}
	}
	return buffer_ptr - buffer_pos;
}

i32 EnumUIData::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	index = 0;
	u16 tmp_count = 0;
	CPOBase::memRead(tmp_count, buffer_ptr, buffer_size);
	if (CPOBase::isPositive(tmp_count))
	{
		count = tmp_count;

		//must this values will be initialized, when call this function
		index_ptr = po_new u16[count];
		string_ptr = po_new UIString[count];

		CPOBase::memRead(index_ptr, count, buffer_ptr, buffer_size);
		for (i32 i = 0; i < count; i++)
		{
			string_ptr[i].memRead(buffer_ptr, buffer_size);
		}
	}
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
void GroupUIData::initBuffer(i32 count)
{
	freeBuffer();
	if (count <= 0)
	{
		return;
	}
	
	this->limit = count;
	index_ptr = po_new i32[count];
	string_ptr = po_new postring[count];
	count = 0;
}

void GroupUIData::freeBuffer()
{
	POSAFE_DELETE_ARRAY(index_ptr);
	POSAFE_DELETE_ARRAY(string_ptr);
	limit = 0;
	count = 0;
}

void GroupUIData::addGroupUIData(i32 value, const postring& tag_name)
{
	if (count >= limit)
	{
		return;
	}

	index_ptr[count] = value;
	string_ptr[count] = tag_name;
	count++;
}

i32 GroupUIData::memSize()
{
	i32 len = 0;
	len += sizeof(count);
	len += sizeof(i32)*count;

	for (i32 i = 0; i < count; i++)
	{
		len += CPOBase::getStringMemSize(string_ptr[i]);
	}
	return len;
}

i32 GroupUIData::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	CPOBase::memWrite(index_ptr, count, buffer_ptr, buffer_size);

	for (i32 i = 0; i < count; i++)
	{
		CPOBase::memWrite(buffer_ptr, buffer_size, string_ptr[i]);
	}
	return buffer_ptr - buffer_pos;
}

i32 GroupUIData::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	u16 tmp_count = 0;
	CPOBase::memRead(tmp_count, buffer_ptr, buffer_size);
	if (CPOBase::isPositive(tmp_count))
	{
		count = tmp_count;
		limit = tmp_count;

		//must this values will be initialized, when call this function
		index_ptr = po_new i32[count];
		string_ptr = po_new postring[count];

		//read data
		CPOBase::memRead(index_ptr, count, buffer_ptr, buffer_size);

		for (i32 i = 0; i < count; i++)
		{
			CPOBase::memRead(buffer_ptr, buffer_size, string_ptr[i]);
		}
	}
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
CTagUI::CTagUI()
{
	m_tag_name = "";
	m_tag_ui_type = kTagUIText;
	m_tag_ui_flag = kTagUIFlagNone;
	m_tag_ui_string.init();

	m_tag_ui_format = kTagUIUnitAuto;
	m_tag_ui_unit_string = "";
	m_tag_ui_disp_format = "";

	memset(&u, 0, sizeof(u));

	m_is_group_ui = false;
	m_group_action = kTagUIGroupActionCommon;
	memset(&m_group_data, 0, sizeof(GroupUIData));
	memset(&m_link_data, 0, sizeof(GroupUIData));
}

CTagUI::~CTagUI()
{
	freeBuffer();
}

void CTagUI::freeBuffer()
{
	switch (m_tag_ui_type)
	{
		case kTagUICombo:
		{
			POSAFE_DELETE_ARRAY(u.enums.index_ptr);
			POSAFE_DELETE_ARRAY(u.enums.string_ptr);
			break;
		}
		case kTagUINumber:
		{
			POSAFE_DELETE_ARRAY(u.range.minmax_reftag);
			break;
		}
		case kTagUIActionButton:
		{
			u.action.freeActionGroup();
			break;
		}
	}
	m_tag_ui_type = kTagUIText;

	POSAFE_DELETE_ARRAY(m_group_data.index_ptr);
	POSAFE_DELETE_ARRAY(m_group_data.string_ptr);

	POSAFE_DELETE_ARRAY(m_link_data.index_ptr);
	POSAFE_DELETE_ARRAY(m_link_data.string_ptr);
}

bool CTagUI::setUIOption(const postring& tag_name, i32 type, i32 flag, i32 count)
{
	freeBuffer();

	m_tag_name = tag_name;
	m_tag_ui_type = type;
	m_tag_ui_flag = flag;

	switch (type)
	{
		case kTagUICombo:
		{
			if (!CPOBase::isPositive(count))
			{
				return false;
			}

			u.enums.index = 0;
			u.enums.count = count;
			u.enums.index_ptr = po_new u16[count];
			u.enums.string_ptr = po_new UIString[count];
			break;
		}
		case kTagUINumber:
		{
			u.range.min_value = -PO_MAXINT;
			u.range.max_value = +PO_MAXINT;
			u.range.step_value = 0.0f; //auto
			u.range.minmax_reftag = po_new postring[2];
			break;
		}
		case kTagUIActionButton:
		{
			u.action.initActionGroup();
			break;
		}
	}
	return true;
}

void CTagUI::setUIString(const powstring* tag_text_str_ptr)
{
	if (!tag_text_str_ptr)
	{
		return;
	}

	m_tag_ui_string.ui_string[kPOLangEnglish] = tag_text_str_ptr[kTagUITextLangEn];
	m_tag_ui_string.ui_string[kPOLangChinese] = tag_text_str_ptr[kTagUITextLangCh];
	m_tag_ui_string.ui_string[kPOLangKorean] = tag_text_str_ptr[kTagUITextLangKo];
}

void CTagUI::setUIEnumString(i32 index, const powstring* tag_text_str_ptr)
{
	if (!tag_text_str_ptr)
	{
		return;
	}

	i32 enum_index = u.enums.index;
	if (m_tag_ui_type != kTagUICombo || !CPOBase::checkIndex(enum_index, u.enums.count))
	{
		return;
	}

	//store index
	u.enums.index_ptr[enum_index] = index;

	//store string
	UIString* string_ptr = u.enums.string_ptr + enum_index;
	string_ptr->ui_string[kPOLangEnglish] = tag_text_str_ptr[kTagUITextLangEn];
	string_ptr->ui_string[kPOLangChinese] = tag_text_str_ptr[kTagUITextLangCh];
	string_ptr->ui_string[kPOLangKorean] = tag_text_str_ptr[kTagUITextLangKo];

	u.enums.index++;
}

void CTagUI::setUIRangeData(const postring& from_tag_name, const postring& to_tag_name, f32 step_value)
{
	if (m_tag_ui_type != kTagUINumber || u.range.minmax_reftag == NULL)
	{
		return;
	}

	u.range.step_value = step_value;
	u.range.minmax_reftag[0] = from_tag_name;
	u.range.minmax_reftag[1] = to_tag_name;
}

void CTagUI::setUIAction(i32 action_mode, const postring& dst_tag_name, const postring& src_tag_name, f32 gain, f32 offset)
{
	//action_mode: kTagUIActionSubStitute:	[dst_tag] = [src_tag]*gain+offset 
	//action_mode: kTagUIActionAdd:			[dst_tag] = [dst_tag]*dst_gain + [src_tag]*gain + offset 
	//action_mode: kTagUIActionSubstract:	[dst_tag] = [dst_tag]*dst_gain - [src_tag]*gain + offset 
	//action_mode: kTagUIActionMultiply:	[dst_tag] = [dst_tag]*[src_tag]*gain + offset 

	if (m_tag_ui_type != kTagUIActionButton)
	{
		return;
	}

	i32 index = u.action.count;
	if (!CPOBase::checkIndex(index, kPOActionCount))
	{
		return;
	}

	u.action.count++;
	u.action.action_ui_data[index].mode = action_mode;
	u.action.action_ui_data[index].action_tag_ptr[0] = dst_tag_name;
	u.action.action_ui_data[index].action_tag_ptr[1] = src_tag_name;
	u.action.action_ui_data[index].dst_gain = 1.0f;
	u.action.action_ui_data[index].src_gain = gain;
	u.action.action_ui_data[index].offset = offset;
}

void CTagUI::setUIUnitFormat(i32 unit_format, const postring& unit_string)
{
	CPOBase::bitRemove(m_tag_ui_format, kTagFormatMaskUnit);
	m_tag_ui_format += unit_format << 16;
	m_tag_ui_unit_string = unit_string;
}

void CTagUI::setUIDispFormat(const postring tag_disp_format)
{
	m_tag_ui_disp_format = tag_disp_format;
}

void CTagUI::setUIGroup(i32 group_action, i32 link_count, i32 action_count)
{
	m_is_group_ui = true;
	m_group_action = group_action;
	m_group_data.initBuffer(action_count);
	m_link_data.initBuffer(link_count);
}

void CTagUI::addUIGroupData(i32 value, const postring& tag_name)
{
	m_group_data.addGroupUIData(value, tag_name);
}

void CTagUI::linkUIGroupData(const postring& tag_name)
{
	m_link_data.addGroupUIData(0, tag_name);
}

i32	CTagUI::memSize()
{
	i32 len = 0;
	len += CPOBase::getStringMemSize(m_tag_name);
	len += sizeof(m_tag_ui_type);
	len += sizeof(m_tag_ui_flag);
	len += m_tag_ui_string.memSize();
	
	len += sizeof(m_tag_ui_format);
	len += CPOBase::getStringMemSize(m_tag_ui_unit_string);
	len += CPOBase::getStringMemSize(m_tag_ui_disp_format);

	switch (m_tag_ui_type)
	{
		case kTagUINumber:
		{
			len += u.range.memSize();
			break;
		}
		case kTagUICombo:
		{
			len += u.enums.memSize();
			break;
		}
		case kTagUIActionButton:
		{
			len += u.action.memSize();
			break;
		}
	}

	len += sizeof(m_is_group_ui);
	len += sizeof(m_group_action);
	len += m_group_data.memSize();
	len += m_link_data.memSize();
	return len;
}

i32	CTagUI::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(buffer_ptr, buffer_size, m_tag_name);
	CPOBase::memWrite(m_tag_ui_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_tag_ui_flag, buffer_ptr, buffer_size);
	m_tag_ui_string.memWrite(buffer_ptr, buffer_size);

	CPOBase::memWrite(m_tag_ui_format, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, m_tag_ui_unit_string);
	CPOBase::memWrite(buffer_ptr, buffer_size, m_tag_ui_disp_format);

	switch (m_tag_ui_type)
	{
		case kTagUINumber:
		{
			u.range.memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kTagUICombo:
		{
			u.enums.memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kTagUIActionButton:
		{
			u.action.memWrite(buffer_ptr, buffer_size);
			break;
		}
	}

	CPOBase::memWrite(m_is_group_ui, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_group_action, buffer_ptr, buffer_size);
	m_group_data.memWrite(buffer_ptr, buffer_size);
	m_link_data.memWrite(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32	CTagUI::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	freeBuffer();

	CPOBase::memRead(buffer_ptr, buffer_size, m_tag_name);
	CPOBase::memRead(m_tag_ui_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_tag_ui_flag, buffer_ptr, buffer_size);
	m_tag_ui_string.memRead(buffer_ptr, buffer_size);

	CPOBase::memRead(m_tag_ui_format, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, m_tag_ui_unit_string);
	CPOBase::memRead(buffer_ptr, buffer_size, m_tag_ui_disp_format);

	switch (m_tag_ui_type)
	{
		case kTagUINumber:
		{
			u.range.memRead(buffer_ptr, buffer_size);
			break;
		}
		case kTagUICombo:
		{
			u.enums.memRead(buffer_ptr, buffer_size);
			break;
		}
		case kTagUIActionButton:
		{
			u.action.memRead(buffer_ptr, buffer_size);
			break;
		}
	}

	CPOBase::memRead(m_is_group_ui, buffer_ptr, buffer_size);
	CPOBase::memRead(m_group_action, buffer_ptr, buffer_size);
	m_group_data.memRead(buffer_ptr, buffer_size);
	m_link_data.memRead(buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
CTagUINode::CTagUINode()
{
	init();
}

CTagUINode::~CTagUINode()
{
	removeAll();
}

void CTagUINode::init()
{
	removeAll();
	m_node_type = kSCUnknownTool;
	m_node_name = "";
}

void CTagUINode::init(i32 node_type)
{
	removeAll();
	m_node_type = node_type;
	m_node_name = "";
}

void CTagUINode::init(i32 node_type, const postring& node_name)
{
	removeAll();
	m_node_type = node_type;
	m_node_name = node_name;
}

void CTagUINode::removeAll()
{
	POSAFE_CLEAR(m_tag_ui_vec);
	POSAFE_CLEAR(m_tag_ui_node_vec);
}

void CTagUINode::setTagUINodeType(i32 node_type)
{
	m_node_type = node_type;
}

void CTagUINode::setTagUINodeName(const postring& node_name)
{
	m_node_name = node_name;
}

void CTagUINode::setTagUINodeString(const powstring* tagui_text_str_ptr)
{
	if (!tagui_text_str_ptr)
	{
		return;
	}

	m_node_disp_name.init();
	m_node_disp_name.ui_string[kPOLangEnglish] = tagui_text_str_ptr[kTagUITextLangEn];
	m_node_disp_name.ui_string[kPOLangChinese] = tagui_text_str_ptr[kTagUITextLangCh];
	m_node_disp_name.ui_string[kPOLangKorean] = tagui_text_str_ptr[kTagUITextLangKo];
}

CTagUI* CTagUINode::addTagUI(const postring& tag_name, i32 type, i32 style, i32 count)
{
	CTagUI* tag_ui_ptr = po_new CTagUI();
	if (!tag_ui_ptr->setUIOption(tag_name, type, style, count))
	{
		POSAFE_DELETE(tag_ui_ptr);
		return NULL;
	}

	m_tag_ui_vec.push_back(tag_ui_ptr);
	return tag_ui_ptr;
}

CTagUINode* CTagUINode::addTagUINode(const postring& tag_node_name)
{
	CTagUINode* tag_ui_node_ptr = po_new CTagUINode();
	tag_ui_node_ptr->setTagUINodeName(tag_node_name);
	m_tag_ui_node_vec.push_back(tag_ui_node_ptr);
	return tag_ui_node_ptr;
}

CTagUINode* CTagUINode::addTagUINode(i32 tag_node_type, const postring& tag_node_name)
{
	CTagUINode* tag_ui_node_ptr = po_new CTagUINode();
	tag_ui_node_ptr->setTagUINodeType(tag_node_type);
	tag_ui_node_ptr->setTagUINodeName(tag_node_name);
	m_tag_ui_node_vec.push_back(tag_ui_node_ptr);
	return tag_ui_node_ptr;
}

void CTagUINode::addTagUINode(CTagUINode* tag_ui_node_ptr)
{
	if (tag_ui_node_ptr == NULL)
	{
		return;
	}
	m_tag_ui_node_vec.push_back(tag_ui_node_ptr);
}

CTagUI* CTagUINode::at(const postring& tag_name)
{
	i32 index = 0;
	TagUIVector::iterator iter;
	for (iter = m_tag_ui_vec.begin(); iter != m_tag_ui_vec.end(); ++iter, index++)
	{
		if ((*iter)->m_tag_name == tag_name)
		{
			return m_tag_ui_vec[index];
		}
	}

	return NULL;
}

i32 CTagUINode::memSize()
{
	i32 i, count;
	i32 len = 0;
	len += sizeof(m_node_type);
	len += CPOBase::getStringMemSize(m_node_name);
	len += m_node_disp_name.memSize();

	count = (i32)m_tag_ui_vec.size();
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += m_tag_ui_vec[i]->memSize();
	}

	count = (i32)m_tag_ui_node_vec.size();
	len += sizeof(count);
	for (i = 0; i < count; i++)
	{
		len += m_tag_ui_node_vec[i]->memSize();
	}
	return len;
}

i32 CTagUINode::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	removeAll();

	CPOBase::memRead(m_node_type, buffer_ptr, buffer_size);
	CPOBase::memRead(buffer_ptr, buffer_size, m_node_name);
	m_node_disp_name.memRead(buffer_ptr, buffer_size);

	//mem read tag_ui
	i32 i, count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(count))
	{
		for (i = 0; i < count; i++)
		{
			CTagUI* tag_ui_ptr = CPOBase::pushBackNew(m_tag_ui_vec);
			tag_ui_ptr->memRead(buffer_ptr, buffer_size);
		}
	}

	//mem read tag_ui_node
	count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (CPOBase::isCount(count))
	{
		for (i = 0; i < count; i++)
		{
			CTagUINode* tag_ui_node_ptr = CPOBase::pushBackNew(m_tag_ui_node_vec);
			tag_ui_node_ptr->memRead(buffer_ptr, buffer_size);
		}
	}
	return buffer_ptr - buffer_pos;
}

i32 CTagUINode::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(m_node_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(buffer_ptr, buffer_size, m_node_name);
	m_node_disp_name.memWrite(buffer_ptr, buffer_size);

	//mem write tag_ui
	i32 i, count = (i32)m_tag_ui_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_tag_ui_vec[i]->memWrite(buffer_ptr, buffer_size);
	}

	//mem write tag_ui_node
	count = (i32)m_tag_ui_node_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_tag_ui_node_vec[i]->memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}
