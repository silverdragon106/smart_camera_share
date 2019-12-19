#include "tool_category.h"
#include "tool_category_text.h"
#include "base.h"

CToolDescriptor::CToolDescriptor()
{
	m_tool_type = kSCUnknownTool;
	m_tool_name.init();
	m_tool_desc.init();
	m_tool_flag = kToolFlagColorNone;
	m_model_region_flag = kToolRegionFlagNone;
	m_search_region_flag = kToolRegionFlagAll;
}

CToolDescriptor::~CToolDescriptor()
{
}

i32 CToolDescriptor::memSize()
{
	i32 len = 0;
	len += sizeof(m_tool_type);
	len += m_tool_name.memSize();
	len += m_tool_desc.memSize();
	len += sizeof(m_tool_flag);
	len += sizeof(m_model_region_flag);
	len += sizeof(m_search_region_flag);
	return len;
}

i32 CToolDescriptor::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_tool_type, buffer_ptr, buffer_size);
	m_tool_name.memRead(buffer_ptr, buffer_size);
	m_tool_desc.memRead(buffer_ptr, buffer_size);
	CPOBase::memRead(m_tool_flag, buffer_ptr, buffer_size);
	CPOBase::memRead(m_model_region_flag, buffer_ptr, buffer_size);
	CPOBase::memRead(m_search_region_flag, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

i32 CToolDescriptor::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_tool_type, buffer_ptr, buffer_size);
	m_tool_name.memWrite(buffer_ptr, buffer_size);
	m_tool_desc.memWrite(buffer_ptr, buffer_size);
	CPOBase::memWrite(m_tool_flag, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_model_region_flag, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_search_region_flag, buffer_ptr, buffer_size);

	return buffer_ptr - buffer_pos;
}

//////////////////////////////////////////////////////////////////////////
CToolCategory::CToolCategory()
{
	init();
}

CToolCategory::~CToolCategory()
{
	m_tool_descriptor_vec.clear();
}

void CToolCategory::init()
{
	m_category_type = kSCUnknownTool;
	m_category_name.init();
	m_category_desc.init();
	m_tool_descriptor_vec.clear();
}

i32 CToolCategory::memSize()
{
	i32 len = 0;
	len += sizeof(m_category_type);
	len += m_category_name.memSize();
	len += m_category_desc.memSize();

	i32 i, count = (i32)m_tool_descriptor_vec.size();
	for (i = 0; i < count; i++)
	{
		len += m_tool_descriptor_vec[i].memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CToolCategory::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(m_category_type, buffer_ptr, buffer_size);
	m_category_name.memRead(buffer_ptr, buffer_size);
	m_category_desc.memRead(buffer_ptr, buffer_size);

	i32 i, count = -1;
	m_tool_descriptor_vec.clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	m_tool_descriptor_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		m_tool_descriptor_vec[i].memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CToolCategory::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memWrite(m_category_type, buffer_ptr, buffer_size);
	m_category_name.memWrite(buffer_ptr, buffer_size);
	m_category_desc.memWrite(buffer_ptr, buffer_size);

	i32 i, count = (i32)m_tool_descriptor_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		m_tool_descriptor_vec[i].memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

CToolDescriptor* CToolCategory::addToolDescriptor(i32 tool_type, i32 tool_flag, i32 model_region_flags, i32 search_region_flags,
								const powstring* tool_name_ptr, const powstring* tool_desc_ptr)
{
	CToolDescriptor* tool_descriptor_ptr = CPOBase::pushBackNew(m_tool_descriptor_vec);
	if (!tool_descriptor_ptr)
	{
		return NULL;
	}

	tool_descriptor_ptr->m_tool_type = tool_type;
	tool_descriptor_ptr->m_tool_flag = tool_flag;
	tool_descriptor_ptr->m_model_region_flag = model_region_flags;
	tool_descriptor_ptr->m_search_region_flag = search_region_flags;
	tool_descriptor_ptr->m_tool_name.init();
	tool_descriptor_ptr->m_tool_desc.init();

	if (tool_name_ptr)
	{
		tool_descriptor_ptr->m_tool_name.ui_string[kPOLangEnglish] = tool_name_ptr[kToolCategoryLangEn];
		tool_descriptor_ptr->m_tool_name.ui_string[kPOLangChinese] = tool_name_ptr[kToolCategoryLangCh];
		tool_descriptor_ptr->m_tool_name.ui_string[kPOLangKorean] = tool_name_ptr[kToolCategoryLangKo];
	}
	if (tool_desc_ptr)
	{
		tool_descriptor_ptr->m_tool_desc.ui_string[kPOLangEnglish] = tool_desc_ptr[kToolCategoryLangEn];
		tool_descriptor_ptr->m_tool_desc.ui_string[kPOLangChinese] = tool_desc_ptr[kToolCategoryLangCh];
		tool_descriptor_ptr->m_tool_desc.ui_string[kPOLangKorean] = tool_desc_ptr[kToolCategoryLangKo];
	}
	return tool_descriptor_ptr;
}

CToolDescriptor* CToolCategory::findTool(i32 tool_type)
{
	i32 count = (i32)m_tool_descriptor_vec.size();
	for (i32 i = 0; i < count; i++)
	{
		if (m_tool_descriptor_vec[i].m_tool_type == tool_type)
		{
			return &m_tool_descriptor_vec[i];
		}
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////
CToolCategorys::CToolCategorys()
{
	init();
}

CToolCategorys::~CToolCategorys()
{
	init();
}

void CToolCategorys::init()
{
	m_category_vec.clear();
}

i32 CToolCategorys::memSize()
{
	i32 len = 0;
	i32 i, count = (i32)m_category_vec.size();
	for (i = 0; i < count; i++)
	{
		len += m_category_vec[i].memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CToolCategorys::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	m_category_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		m_category_vec[i].memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CToolCategorys::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
		
	i32 i, count = (i32)m_category_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		m_category_vec[i].memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

CToolCategory* CToolCategorys::addToolCategory(i32 category_type, const powstring* category_name_ptr, const powstring* category_desc_ptr)
{
	CToolCategory* tool_category_ptr = CPOBase::pushBackNew(m_category_vec);
	if (!tool_category_ptr)
	{
		return NULL;
	}

	tool_category_ptr->m_category_type = category_type;
	tool_category_ptr->m_category_name.init();
	tool_category_ptr->m_category_desc.init();
	
	if (category_name_ptr)
	{
		tool_category_ptr->m_category_name.ui_string[kPOLangEnglish] = category_name_ptr[kToolCategoryLangEn];
		tool_category_ptr->m_category_name.ui_string[kPOLangChinese] = category_name_ptr[kToolCategoryLangCh];
		tool_category_ptr->m_category_name.ui_string[kPOLangKorean] = category_name_ptr[kToolCategoryLangKo];
	}
	if (category_desc_ptr)
	{
		tool_category_ptr->m_category_desc.ui_string[kPOLangEnglish] = category_desc_ptr[kToolCategoryLangEn];
		tool_category_ptr->m_category_desc.ui_string[kPOLangChinese] = category_desc_ptr[kToolCategoryLangCh];
		tool_category_ptr->m_category_desc.ui_string[kPOLangKorean] = category_desc_ptr[kToolCategoryLangKo];
	}
	return tool_category_ptr;
}
