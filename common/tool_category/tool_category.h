#pragma once
#include "sc_define.h"
#include "tag_ui.h"

class CToolDescriptor
{
public:
	CToolDescriptor();
	~CToolDescriptor();

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);

public:
	i32						m_tool_type;
	UIString				m_tool_name;
	UIString				m_tool_desc;
	i32						m_tool_flag;
	i32						m_model_region_flag;
	i32						m_search_region_flag;
};
typedef std::vector<CToolDescriptor> ToolDescriptorVector;

class CToolCategory
{
public:
	CToolCategory();
	~CToolCategory();

	void					init();
	CToolDescriptor*		addToolDescriptor(i32 tool_type,
											i32 tool_flag,
											i32 model_region_flags, 
											i32 search_region_flags, 
											const powstring* tool_name_ptr, 
											const powstring* tool_desc_ptr = NULL);
	CToolDescriptor*		findTool(i32 tool_type);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);
	
public:
	i32						m_category_type;
	UIString				m_category_name;
	UIString				m_category_desc;
	ToolDescriptorVector	m_tool_descriptor_vec;
};
typedef std::vector<CToolCategory> ToolCategoryVector;

class CToolCategorys
{
public:
	CToolCategorys();
	~CToolCategorys();

	void					init();
	CToolCategory*			addToolCategory(i32 category_type, 
										const powstring* category_name_ptr, 
										const powstring* category_desc_ptr = NULL);

	i32						memSize();
	i32						memRead(u8*& buffer_ptr, i32& buffer_size);
	i32						memWrite(u8*& buffer_ptr, i32& buffer_size);

public:
	ToolCategoryVector		m_category_vec;
};