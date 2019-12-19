#pragma once
#include "sc_define.h"
#include "sc_struct.h"

enum ToolUploadType
{
	kToolWithSimple	= 0x01,
	kToolWithStudy	= 0x02,
	kToolWithHeavy	= 0x04,
	
	kToolWithDetail	= kToolWithSimple | kToolWithStudy,
	kToolWithAll	= kToolWithSimple | kToolWithStudy | kToolWithHeavy
};

enum ResultUploadType
{
	kResultWithSimple	= 0x01,
	kResultWithDraw		= 0x02,
	kResultWithAll		= kResultWithSimple | kResultWithDraw
};

#if defined(POR_EXPLORER)
class CToolCore : public QObject
#else 
class CToolCore
#endif
{
public:
	CToolCore();
	virtual ~CToolCore();
	virtual void				initInstance();
	virtual void				exitInstance();
	virtual void				copyFrom(CToolCore &tool_core);
	
	virtual i32					memSize(i32 mode);
	virtual bool				memRead(i32 mode, u8* &buffer_ptr, i32& buffer_size);
	virtual bool				memWrite(i32 mode, u8* &buffer_ptr, i32& buffer_size);

	void						setFixtureID(i32 tool_id, i32 fixture_id = 0);
	void						setFilterID(i32 tool_id);

	inline i32					getToolID() { return m_tool_id; };
	inline i32					getToolType() { return m_tool_type; };
	inline i32					getFixtureID() { return m_fixture_id.x; };
	inline i32					getFixtureSubID() { return m_fixture_id.y; };
	inline i32					getFilterID() { return m_filter_id; };

public:
	i32							m_tool_type;
	i32							m_tool_id;

	// extra information non-controlled
	vector2di					m_fixture_id;
	i32							m_filter_id;
	std::vector<vector2di>		m_depends_vec;

	// search and model region
	CRegionInfo					m_model_region;  // Region to be trained. (Only for Locator Tools.)
	CRegionInfo					m_search_region; // Region to be searched for tool result. (Only for Region Tools.)

	CTagNode					m_tag_vec;	// TagNodes that contains Tool Parameter, Display Result and Show Result.
											// This one will have Tool Parameter Node for sure, 
											// but DisplayResult Node and Show Result Node may not be existed.							
};

typedef std::vector<CToolCore*>	CToolVector;
typedef std::list<CToolCore*> CToolList;

#if defined(POR_EXPLORER)
class CToolResultCore : public QObject
#else
class CToolResultCore
#endif
{
public:
	CToolResultCore();
	virtual ~CToolResultCore();

	virtual void				copyFrom(CToolResultCore& tool_result_core);

	virtual i32					memSize(i32 mode);
	virtual bool				memRead(i32 mode, u8*& buffer_ptr, i32& buffer_size);
	virtual bool				memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size);

	inline i32					getToolID() { return m_tool_id; };
	inline i32					getToolType() { return m_tool_type; };

public:
	i32							m_tool_type;
	i32							m_tool_id;

	CTagNode					m_tag_vec;
};

typedef std::vector<CToolResultCore*> CToolResultVector;