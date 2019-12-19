#pragma once

#include "sc_define.h"
#include "variant.h"

const i32 kTagNodeMaxCount = 512;

class CTagNode;
typedef std::vector<CTagNode*>	TagNodeVector;

class CTagNode
{
public:
	CTagNode();
	virtual ~CTagNode();

	void						init();
	void						init(i32 node_type);
	void						init(i32 node_type, const postring& node_name);
	void						setNodeType(i32 node_type);
	void						setNodeName(const postring& node_name);
	void						setNodeReserved(const i32& node_reserved);
	void						update();
	void						updateReserved();
	void						removeAll();

	CVariant*					findTreeTag(const postring& path, bool is_linear = false);
	CTagNode*					findTreeNode(const postring& path, bool is_linear = false);
	CTagNode*					addTreeNode(const postring& path);
	bool						deleteTreeNode(const postring& path);
	void						addTreeTag(CTagNode& tag_node);
	void						addTreeTag(TagVector& tag_vec);
	void						setTreeTag(CTagNode& tag_node);
	void						clearResultTreeTag();
	void						removeTree();
	bool						removeTreeTag();

	i32							getTagCount();
	i32							getTagNodeCount();

	i32							memSize(i32 mode);
	i32							memTagSize(i32 mode);
	i32							memTagInfoSize();
	bool						memRead(u8*& buffer_ptr, i32& buffer_size);
	bool						memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size);
	bool						memTagRead(i32 mode, bool accept_node, u8*& buffer_ptr, i32& buffer_size);
	bool						memExpTagRead(i32 mode, u8*& buffer_ptr, i32& buffer_size);
	void						memExpTagsSkip(u8*& buffer_ptr, i32& buffer_size);
	void						memTagInfoWrite(u8*& buffer_ptr, i32& buffer_size);
	void						memTagInfoRead(u8*& buffer_ptr, i32& buffer_size);
	bool						fileWrite(FILE* fp);
	bool						fileRead(FILE* fp);

	CVariant*					getTag(i32 index);
	CVariant*					addTag(CVariant* tag_ptr);
	CVariant*					addTag(const postring& attr_name, DataType data_type, TagType tag_type,
									void* buffer_ptr, void* parent_ptr = NULL, f32 min_value = 0.0f, f32 max_value = -1.0f);
	bool						removeTag(CVariant& tag);
	bool						removeTag(const postring& attr_name);
	bool						removeTag(i32 index);
	bool						removeTagFromIndex(i32 index);
	bool						removeAllTag();

	CTagNode*					getTagNode(i32 index);
	CTagNode*					addTagNode(CTagNode* tag_node_ptr);
	CTagNode*					addTagNode(const postring& node_name, i32 node_type);
	bool						removeTagNode(CTagNode* tag_node_ptr);
	bool						removeTagNode(const postring& node_name);
	bool						removeTagNode(i32 index);
	bool						removeAllSubNode();
	bool						removeTreeSubNode();
	
	i32							getIndexOf(const postring& attr_name);
	i32							linearGetIndexOf(const postring& attr_name);

	i32							getNodeIndexOf(const postring& node_name);
	i32							linearGetNodeIndexOf(const postring& node_name);
	i32							linearGetReservedNodeIndexOf(const i32& node_reserved);

	CVariant*					findTag(const postring& attr_name);
	CVariant*					operator[](const postring& attr_name);
	
	inline postring				getNodeName() { return m_node_name; };
	inline i32					getNodeType() { return m_node_type; };
	inline i32					getNodeReserved() { return m_node_reserved; };

private:
	void						copyTreeTag(CTagNode& tag_node);
	CTagNode*					copyTagNode(CTagNode* tag_node_ptr);

public:
	postring					m_node_name;
	i32							m_node_type;
	i32							m_node_reserved;

	bool						m_is_tag_updated;
	bool						m_is_sub_node_updated;
	bool						m_is_reserved_updated;

	strmap						m_tag_strmap;
	strmap						m_sub_node_strmap;
	i32map						m_reserved_i32map;
	
	TagVector					m_tag_vec;
	TagNodeVector				m_sub_node_vec;
};

//TagNode MemTable

//---[]
//---------NodeName
//---------NodeType
//---------NodeReserved
//---------NodeDataLen
//---------[TagCount]
//----------TagData1
//----------------TagName
//----------------TagType
//----------------TagDataLen
//----------------TagContent
//----------TagData2
//------------...
//---------TagDataN
//---[SubTagNode Count]
//----SubTagNode1
//----SubTagNode2
//------...
//----SubTagNodeN
