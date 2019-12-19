#include "tool_core.h"
#include "base.h"

//////////////////////////////////////////////////////////////////////////
CToolCore::CToolCore()
{
	initInstance();
}

CToolCore::~CToolCore()
{
}

void CToolCore::initInstance()
{
	m_tool_type = kSCUnknownTool;
	m_tool_id = -1;

	m_filter_id = -1;
	m_fixture_id = vector2di(-1, -1);
	m_depends_vec.clear();
}

void CToolCore::exitInstance()
{
	m_tool_type = kSCUnknownTool;
	m_tool_id = -1;

	m_filter_id = -1;
	m_fixture_id = vector2di(-1, -1);
	m_depends_vec.clear();

	m_tag_vec.removeAll();
}

void CToolCore::copyFrom(CToolCore &tool_core)
{
#if defined(POR_EXPLORER)
	m_tool_type = tool_core.m_tool_type;
	m_tool_id = tool_core.m_tool_id;

	m_fixture_id = tool_core.m_fixture_id;
	m_filter_id = tool_core.m_filter_id;
	m_depends_vec = tool_core.m_depends_vec;

	m_model_region.setValue(&tool_core.m_model_region);
	m_search_region.setValue(&tool_core.m_search_region);

	//MEMCLONE(m_tag_vec, tool_core.m_tag_vec);
	{
		i32 size = tool_core.m_tag_vec.memSize(kTagTransPass);
		i32 buffer_size = size;
		u8* buffer = po_new u8[size];
		u8* wbuffer = buffer;
		tool_core.m_tag_vec.memWrite(kTagTransPass, wbuffer, buffer_size);
		wbuffer = buffer;
		buffer_size = size;
		m_tag_vec.memRead(wbuffer, buffer_size);
		POSAFE_DELETE_ARRAY(buffer);
	}
#endif
}

i32 CToolCore::memSize(i32 mode)
{
	i32 len = 0;
	len += sizeof(i32);
	len += sizeof(m_tool_type);
	len += sizeof(m_tool_id);

	len += sizeof(m_fixture_id);
	len += sizeof(m_filter_id);
	len += CPOBase::getVectorMemSize(m_depends_vec);

	len += m_model_region.memSize();
	len += m_search_region.memSize();

#if defined(POR_DEVICE)
	i32 tag_mode = kTagTypeR;
	if (CPOBase::bitCheck(mode, kToolWithStudy))
	{
		tag_mode |= kTagTypeSD;
	}
	if (CPOBase::bitCheck(mode, kToolWithHeavy))
	{
		tag_mode |= kTagTypeHeavy;
	}
#else
	i32 tag_mode = kTagTypeW;
#endif

	len += m_tag_vec.memSize(tag_mode);
	return len;
}

bool CToolCore::memRead(i32 /*mode*/, u8*& buffer_ptr, i32& buffer_size)
{
#if !defined(POR_DEVICE)
	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, SC_TOOL_SIGNCODE))
	{
		return false;
	}

	CPOBase::memRead(m_tool_type, buffer_ptr, buffer_size);
#else
	/*
	CPOBase::memRead(sign_code...)
	CPOBase::memRead(m_tool_type...)
	*/
#endif

	CPOBase::memRead(m_tool_id, buffer_ptr, buffer_size);
	CPOBase::memRead(m_fixture_id, buffer_ptr, buffer_size);
	CPOBase::memRead(m_filter_id, buffer_ptr, buffer_size);
	CPOBase::memReadVector(m_depends_vec, buffer_ptr, buffer_size);

	m_model_region.memRead(buffer_ptr, buffer_size);
	m_search_region.memRead(buffer_ptr, buffer_size);

#if defined(POR_DEVICE)
	return m_tag_vec.memTagRead(kTagTypeW, true, buffer_ptr, buffer_size);
#else
	return m_tag_vec.memRead(buffer_ptr, buffer_size);
#endif
}

bool CToolCore::memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	if (!CPOBase::memSignWrite(buffer_ptr, buffer_size, SC_TOOL_SIGNCODE))
	{
		return false;
	}

	CPOBase::memWrite(m_tool_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_tool_id, buffer_ptr, buffer_size);

	CPOBase::memWrite(m_fixture_id, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_filter_id, buffer_ptr, buffer_size);
	CPOBase::memWriteVector(m_depends_vec, buffer_ptr, buffer_size);

	m_model_region.memWrite(buffer_ptr, buffer_size);
	m_search_region.memWrite(buffer_ptr, buffer_size);

#if defined(POR_DEVICE)
	i32 tag_mode = kTagTypeR;
	if (CPOBase::bitCheck(mode, kToolWithStudy))
	{
		tag_mode |= kTagTypeSD;
	}
	if (CPOBase::bitCheck(mode, kToolWithHeavy))
	{
		tag_mode |= kTagTypeHeavy;
	}
#else
	i32 tag_mode = kTagTypeW;
#endif

	return m_tag_vec.memWrite(tag_mode, buffer_ptr, buffer_size);
}

void CToolCore::setFixtureID(i32 tool_id, i32 fixture_id)
{
	m_fixture_id = vector2di(tool_id, fixture_id);
}

void CToolCore::setFilterID(i32 tool_id)
{
	m_filter_id = tool_id;
}

//////////////////////////////////////////////////////////////////////////
CToolResultCore::CToolResultCore()
{
}

CToolResultCore::~CToolResultCore()
{
}

void CToolResultCore::copyFrom(CToolResultCore& tool_result_core)
{
#if defined(POR_EXPLORER)
	m_tool_type = tool_result_core.m_tool_type;
	m_tool_id = tool_result_core.m_tool_id;

	//MEMCLONE_MODE(m_tag_vec, tool_result_core.m_tag_vec);
	{
		i32 size = tool_result_core.m_tag_vec.memSize(kTagTransPass);
		i32 buffer_size = size;
		u8* buffer = po_new u8[size];
		u8* wbuffer = buffer;
		tool_result_core.m_tag_vec.memWrite(kTagTransPass, wbuffer, buffer_size);
		wbuffer = buffer;
		buffer_size = size;
		m_tag_vec.memRead(wbuffer, buffer_size);
		POSAFE_DELETE_ARRAY(buffer);
	}
#endif
}

i32 CToolResultCore::memSize(i32 mode)
{
	i32 len = 0;
	len += sizeof(i32);
	len += sizeof(m_tool_type);
	len += sizeof(m_tool_id);

#if defined(POR_DEVICE)
	i32 tag_mode = kTagTypeR;
	if (CPOBase::bitCheck(mode, kResultWithDraw))
	{
		tag_mode |= kTagTypeDraw;
	}
	len += m_tag_vec.memSize(tag_mode);
#else
	len += m_tag_vec.memSize(kTagTypeW);
#endif
	return len;
}

bool CToolResultCore::memRead(i32 /*mode*/, u8*& buffer_ptr, i32& buffer_size)
{
#if defined(POR_DEVICE)
	/*
	CPOBase::memRead(sign_code, ...)
	CPOBase::memRead(m_tool_type, ...)
	*/
	CPOBase::memRead(m_tool_id, buffer_ptr, buffer_size);
	return m_tag_vec.memTagRead(kTagTypeW, true, buffer_ptr, buffer_size);
#else
	if (!CPOBase::memSignRead(buffer_ptr, buffer_size, SC_RESULT_SIGNCODE))
	{
		return false;
	}

	CPOBase::memRead(m_tool_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_tool_id, buffer_ptr, buffer_size);
	return m_tag_vec.memRead(buffer_ptr, buffer_size);
#endif
}

bool CToolResultCore::memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	if (!CPOBase::memSignWrite(buffer_ptr, buffer_size, SC_RESULT_SIGNCODE))
	{
		return false;
	}

	CPOBase::memWrite(m_tool_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_tool_id, buffer_ptr, buffer_size);

#if defined(POR_DEVICE)
	i32 tag_mode = kTagTypeR;
	if (CPOBase::bitCheck(mode, kResultWithDraw))
	{
		tag_mode |= kTagTypeDraw;
	}
	return m_tag_vec.memWrite(tag_mode, buffer_ptr, buffer_size);
#else
	return m_tag_vec.memWrite(kTagTypeW, buffer_ptr, buffer_size);
#endif
}
