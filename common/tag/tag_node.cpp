#include "tag_node.h"
#include "base.h"
#include "tag_define.h"

CTagNode::CTagNode()
{
	init();
}

CTagNode::~CTagNode()
{
	removeAll();
}

void CTagNode::init()
{
	m_node_name = "";
	m_node_type = kSCToolTypeBaseTool;
	m_node_reserved = -1;

	removeAll();
}

void CTagNode::init(i32 node_type)
{
	m_node_name = "";
	m_node_type = node_type;
	m_node_reserved = -1;

	removeAll();
}

void CTagNode::init(i32 node_type, const postring& node_name)
{
	setNodeName(node_name);
	setNodeType(node_type);
	m_node_reserved = -1;

	removeAll();
}

void CTagNode::setNodeType(i32 node_type)
{
	m_node_type = node_type;
}

void CTagNode::setNodeName(const postring& node_name)
{
	m_node_name = node_name;
}

void CTagNode::setNodeReserved(const i32& node_reserved)
{
	m_node_reserved = node_reserved;
}

void CTagNode::update()
{
	i32 i, count;
	postring tag_name;

	//update tag search map
	if (!m_is_tag_updated)
	{
		m_tag_strmap.clear();
		count = (i32)m_tag_vec.size();

		if (count > 0)
		{
			for (i = 0; i < count; i++)
			{
				tag_name = m_tag_vec[i]->getAttrName();
				CPOBase::toLower(tag_name);
				m_tag_strmap[tag_name] = i;
			}
			m_is_tag_updated = true;
		}
	}

	//update subnode search map
	if (!m_is_sub_node_updated)
	{
		m_sub_node_strmap.clear();
		count = (i32)m_sub_node_vec.size();

		if (count > 0)
		{
			for (i = 0; i < count; i++)
			{
				tag_name = m_sub_node_vec[i]->getNodeName();
				CPOBase::toLower(tag_name);
				m_sub_node_strmap[tag_name] = i;
			}
			m_is_sub_node_updated = true;
		}
	}

	//update subnode recursively
	count = (i32)m_sub_node_vec.size();
	for (i = 0; i < count; i++)
	{
		m_sub_node_vec[i]->update();
	}
}

void CTagNode::updateReserved()
{
	//update subnode reserved map
	if (!m_is_reserved_updated)
	{
		m_reserved_i32map.clear();
		i32 i, count = (i32)m_sub_node_vec.size();
		if (count > 0)
		{
			for (i = 0; i < count; i++)
			{
				m_reserved_i32map[m_sub_node_vec[i]->getNodeReserved()] = i;
			}
			m_is_reserved_updated = true;
		}
	}
}

void CTagNode::removeAll()
{
	removeAllTag();
	removeAllSubNode();
}

CVariant* CTagNode::getTag(i32 index)
{
	return m_tag_vec[index];
}

CVariant* CTagNode::addTag(CVariant* tag_ptr)
{
	if (!tag_ptr)
	{
		return NULL;
	}

	m_is_tag_updated = false;
	m_tag_vec.push_back(tag_ptr);
	return tag_ptr;
}

CVariant* CTagNode::addTag(const postring& attr_name, DataType data_type, TagType tag_type,
						void* buffer_ptr, void* parent_ptr, f32 min_value, f32 max_value)
{
	m_is_tag_updated = false;

	CVariant* tag_ptr = po_new CVariant(attr_name, data_type, tag_type, buffer_ptr);
	tag_ptr->setParent(parent_ptr);
	tag_ptr->setMinMaxValue(min_value, max_value);
	m_tag_vec.push_back(tag_ptr);
	return tag_ptr;
}

bool CTagNode::removeTag(CVariant& tag)
{
	return removeTag(tag.getAttrName());
}

bool CTagNode::removeTag(const postring& attr_name)
{
	return removeTag(getIndexOf(attr_name));
}

bool CTagNode::removeTag(i32 index)
{
	if (!CPOBase::checkIndex(index, (i32)m_tag_vec.size()))
	{
		return false;
	}

	CVariant* var_ptr = m_tag_vec[index];
	if (m_is_tag_updated)
	{
		postring tag_name = var_ptr->getAttrName();
		CPOBase::toLower(tag_name);

		m_tag_strmap.erase(m_tag_strmap.find(tag_name));
		if (m_tag_strmap.size() <= 0)
		{
			m_is_tag_updated = false;
		}
		else
		{
			//modify tag_strmap index
			strmap::iterator iter;
			for (iter = m_tag_strmap.begin(); iter != m_tag_strmap.end(); ++iter)
			{
				if (iter->second >= index)
				{
					iter->second--;
				}
			}
		}
	}

	POSAFE_DELETE(var_ptr);
	CPOBase::eraseInVector(m_tag_vec, index);
	return true;
}

bool CTagNode::removeTagFromIndex(i32 index)
{
	i32 i, count = (i32)m_tag_vec.size();
	if (!CPOBase::checkIndex(index, count))
	{
		return false;
	}
	
	for (i = index; i < count; i++)
	{
		POSAFE_DELETE(m_tag_vec[i]);
	}

	m_is_tag_updated = false;
	CPOBase::eraseInVector(m_tag_vec, index, count);
	return true;
}

bool CTagNode::removeAllTag()
{
	m_is_tag_updated = false;
	m_tag_strmap.clear();
	POSAFE_CLEAR(m_tag_vec);
	return true;
}

CTagNode* CTagNode::getTagNode(i32 index)
{
	if (!CPOBase::checkIndex(index, (i32)m_sub_node_vec.size()))
	{
		return NULL;
	}
	return m_sub_node_vec[index];
}

CTagNode* CTagNode::addTagNode(CTagNode* tag_node_ptr)
{
	m_is_sub_node_updated = false;
	m_is_reserved_updated = false;

	m_sub_node_vec.push_back(tag_node_ptr);
	return tag_node_ptr;
}

CTagNode* CTagNode::addTagNode(const postring& node_name, i32 node_type)
{
	m_is_sub_node_updated = false;
	m_is_reserved_updated = false;

	CTagNode* tag_node_ptr = CPOBase::pushBackNew(m_sub_node_vec);
	tag_node_ptr->setNodeName(node_name);
	tag_node_ptr->setNodeType(node_type);
	return tag_node_ptr;
}

CTagNode* CTagNode::copyTagNode(CTagNode* tag_node_ptr)
{
	CTagNode* new_node_ptr = CPOBase::pushBackNew(m_sub_node_vec);
	
	//copy all data
	new_node_ptr->m_node_name = tag_node_ptr->m_node_name;
	new_node_ptr->m_node_type = tag_node_ptr->m_node_type;
	new_node_ptr->m_node_reserved = tag_node_ptr->m_node_reserved;

	new_node_ptr->m_is_tag_updated = tag_node_ptr->m_is_tag_updated;
	new_node_ptr->m_is_sub_node_updated = tag_node_ptr->m_is_sub_node_updated;
	new_node_ptr->m_is_reserved_updated = tag_node_ptr->m_is_reserved_updated;

	new_node_ptr->m_tag_strmap = tag_node_ptr->m_tag_strmap;
	new_node_ptr->m_sub_node_strmap = tag_node_ptr->m_sub_node_strmap;
	new_node_ptr->m_reserved_i32map = tag_node_ptr->m_reserved_i32map;

	return new_node_ptr;
}

bool CTagNode::removeTagNode(CTagNode* tag_node_ptr)
{
	if (!tag_node_ptr)
	{
		return false;
	}
	return removeTagNode(tag_node_ptr->m_node_name);
}

bool CTagNode::removeTagNode(const postring& node_name)
{
	return removeTagNode(getNodeIndexOf(node_name));
}

bool CTagNode::removeTagNode(i32 index)
{
	if (!CPOBase::checkIndex(index, (i32)m_sub_node_vec.size()))
	{
		return false;
	}

	CTagNode* tag_node_ptr = m_sub_node_vec[index];
	if (m_is_sub_node_updated)
	{
		postring tag_node_name = tag_node_ptr->m_node_name;
		CPOBase::toLower(tag_node_name);
		m_sub_node_strmap.erase(tag_node_name);
		if (m_sub_node_strmap.size() <= 0)
		{
			m_is_sub_node_updated = false;
		}
		else
		{
			//modify subnode_strmap index
			strmap::iterator iter;
			for (iter = m_sub_node_strmap.begin(); iter != m_sub_node_strmap.end(); ++iter)
			{
				if (iter->second >= index)
				{
					iter->second--;
				}
			}
		}
	}
	if (m_is_reserved_updated)
	{
		m_reserved_i32map.erase(tag_node_ptr->m_node_reserved);
		if (m_reserved_i32map.size() <= 0)
		{
			m_is_reserved_updated = false;
		}
		else
		{
			//modify reserved_map index
			i32map::iterator iter;
			for (iter = m_reserved_i32map.begin(); iter != m_reserved_i32map.end(); ++iter)
			{
				if (iter->second >= index)
				{
					iter->second--;
				}
			}
		}
	}

	POSAFE_DELETE(tag_node_ptr);
	CPOBase::eraseInVector(m_sub_node_vec, index);
	return true;
}

bool CTagNode::removeAllSubNode()
{
	m_is_sub_node_updated = false;
	m_is_reserved_updated = false;
	m_sub_node_strmap.clear();
	m_reserved_i32map.clear();

	POSAFE_CLEAR(m_sub_node_vec);
	return true;
}

bool CTagNode::removeTreeSubNode()
{
	m_is_sub_node_updated = false;
	m_is_reserved_updated = false;
	m_sub_node_strmap.clear();
	m_reserved_i32map.clear();

	i32 i, count = (i32)m_sub_node_vec.size();
	for (i = 0; i < count; i++)
	{
		m_sub_node_vec[i]->removeTree();
	}
	POSAFE_CLEAR(m_sub_node_vec);
	return true;
}

CVariant* CTagNode::operator[](const postring& attr_name)
{
	return findTag(attr_name);
}

CVariant* CTagNode::findTag(const postring& attr_name)
{
	i32 index = getIndexOf(attr_name);
	if (!CPOBase::checkIndex(index, (i32)m_tag_vec.size()))
	{
		printlog_lv3(QString("Can't find attr[%1].").arg(attr_name.c_str()));
		return NULL;
	}

	return m_tag_vec[index];
}

i32 CTagNode::getIndexOf(const postring& attr_name)
{
	if (attr_name.length() <= 0)
	{
		printlog_lvs2("GetIndexOf fail. TagName empty.", LOG_SCOPE_TAG);
		return -1;
	}

	postring tmp_attr_name = attr_name;
	CPOBase::toLower(tmp_attr_name);
	if (!m_is_tag_updated)
	{
		return linearGetIndexOf(tmp_attr_name);
	}

	strmap::iterator iter;
	iter = m_tag_strmap.find(tmp_attr_name);
	if (iter != m_tag_strmap.end())
	{
		return iter->second;
	}
	return -1;
}

i32 CTagNode::linearGetIndexOf(const postring& attr_name)
{
	i32 index = 0;
	postring tag_attr_name;
	TagVector::iterator iter;

	for (iter = m_tag_vec.begin(); iter != m_tag_vec.end(); ++iter, index++)
	{
		tag_attr_name = (*iter)->getAttrName();
		CPOBase::toLower(tag_attr_name);
		if (tag_attr_name == attr_name)
		{
			return index;
		}
	}
	return -1;
}

i32 CTagNode::getNodeIndexOf(const postring& node_name)
{
	if (node_name.length() <= 0)
	{
		printlog_lvs2("GetNodeIndexOf fail. NodeName empty.", LOG_SCOPE_TAG);
		return -1;
	}

	postring tmp_node_name = node_name;
	CPOBase::toLower(tmp_node_name);
	if (tmp_node_name[0] == TAG_PREFIX_TOOL)
	{
		i32 node_reserved = -1;
		if (!CPOBase::stoi(tmp_node_name.substr(1), node_reserved))
		{
			printlog_lvs2("GetNodeIndexOf ToolIndex Failed.", LOG_SCOPE_TAG);
			return -1;
		}

		if (m_is_sub_node_updated)
		{
			i32map::iterator iter;
			iter = m_reserved_i32map.find(node_reserved);
			if (iter != m_reserved_i32map.end())
			{
				return iter->second;
			}
			printlog_lvs3(QString("GetNodeIndexOf reservedmap search fail. %1").arg(node_reserved), LOG_SCOPE_TAG);
		}
		else
		{
			return linearGetReservedNodeIndexOf(node_reserved);
		}
	}
	else
	{
		if (m_is_sub_node_updated)
		{
			strmap::iterator iter;
			iter = m_sub_node_strmap.find(tmp_node_name);
			if (iter != m_sub_node_strmap.end())
			{
				return iter->second;
			}
		}
		else
		{
			return linearGetIndexOf(tmp_node_name);
		}
	}
	return -1;
}

i32 CTagNode::linearGetNodeIndexOf(const postring& node_name)
{
	i32 index = 0;
	postring tree_node_name;
	postring node_name_l = node_name;
	CPOBase::toLower(node_name_l);

	TagNodeVector::iterator iter;
	for (iter = m_sub_node_vec.begin(); iter != m_sub_node_vec.end(); ++iter, index++)
	{
		tree_node_name = (*iter)->m_node_name;
		CPOBase::toLower(tree_node_name);
		if (tree_node_name == node_name_l)
		{
			return index;
		}
	}
	printlog_lvs2(QString("linearGetNodeIndexOf strmap search fail. %1").arg(node_name.c_str()), LOG_SCOPE_TAG);
	return -1;
}

i32 CTagNode::linearGetReservedNodeIndexOf(const i32& node_reserved)
{
	i32 index = 0;
	TagNodeVector::iterator iter;
	for (iter = m_sub_node_vec.begin(); iter != m_sub_node_vec.end(); ++iter, index++)
	{
		if ((*iter)->m_node_reserved == node_reserved)
		{
			return index;
		}
	}
	printlog_lvs2(QString("linearGetReservedNodeIndexOf search fail. %1").arg(node_reserved), LOG_SCOPE_TAG);
	return -1;
}

i32 CTagNode::memTagSize(i32 mode)
{
	i32 len = 0;

	//all tags
	CVariant* var_ptr;
	i32 i, count = (i32)m_tag_vec.size();
	for (i = 0; i < count; i++)
	{
		var_ptr = m_tag_vec[i];
		if (var_ptr->isAcceptable(mode))
		{
			len += var_ptr->memSize();
		}
	}
	len += sizeof(count);

	//all tagNode
	count = (i32)m_sub_node_vec.size();
	for (i = 0; i < count; i++)
	{
		len += m_sub_node_vec[i]->memSize(mode);
	}
	len += sizeof(count);
	return len;
}

i32 CTagNode::memTagInfoSize()
{
	i32 len = 0;
	i32 i, count = (i32)m_tag_vec.size();

	len += CPOBase::getStringMemSize(m_node_name);
	len += sizeof(m_node_type);
	len += sizeof(m_node_reserved);
	for (i = 0; i < count; i++)
	{
		len += m_tag_vec[i]->memInfoSize();
	}
	len += sizeof(count);

	//all tagNode
	count = (i32)m_sub_node_vec.size();
	for (i = 0; i < count; i++)
	{
		len += m_sub_node_vec[i]->memTagInfoSize();
	}
	len += sizeof(count);
	return len;
}

i32 CTagNode::memSize(i32 mode)
{
	i32 len = 0;
	len += CPOBase::getStringMemSize(m_node_name);
	len += sizeof(m_node_type);
	len += sizeof(m_node_reserved);
	len += sizeof(i32);
	len += memTagSize(mode);

	return len;
}

bool CTagNode::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	removeAll();

	i32 i, count, data_len, rbytes;
	i32 read_count = 0;
	CPOBase::memRead(buffer_ptr, buffer_size, m_node_name);
	CPOBase::memRead(m_node_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_node_reserved, buffer_ptr, buffer_size);
	CPOBase::memRead(data_len, buffer_ptr, buffer_size);

	//read all tag
	count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	CVariant* var_ptr;
	for (i = 0; i < count; i++)
	{
		var_ptr = CPOBase::pushBackNew(m_tag_vec);
		rbytes = var_ptr->memRead(buffer_ptr, buffer_size);
		read_count++;

		printlog_lvs3(QString("Tag MemRead %1, %2").arg(i).arg(rbytes), LOG_SCOPE_TAG);
	}

	//read all tagNode
	count = -1;
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	CTagNode* tag_node_ptr;
	for (i = 0; i < count; i++)
	{
		tag_node_ptr = CPOBase::pushBackNew(m_sub_node_vec);
		tag_node_ptr->memRead(buffer_ptr, buffer_size);
	}
	return true;
}

bool CTagNode::memWrite(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	CPOBase::memWrite(buffer_ptr, buffer_size, m_node_name);
	CPOBase::memWrite(m_node_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_node_reserved, buffer_ptr, buffer_size);

	i32 data_len = memTagSize(mode);
	CPOBase::memWrite(data_len, buffer_ptr, buffer_size);

	//write all tag
	CVariant* var_ptr;
	i32 i, wbytes, write_count = 0;
	i32 count = (i32)m_tag_vec.size();

	for (i = 0; i < count; i++)
	{
		var_ptr = m_tag_vec[i];
		if (var_ptr->isAcceptable(mode))
		{
			write_count++;
		}
	}

	CPOBase::memWrite(write_count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		var_ptr = m_tag_vec[i];
		if (var_ptr->isAcceptable(mode))
		{
			wbytes = var_ptr->memWrite(buffer_ptr, buffer_size);
			printlog_lvs3(QString("Tag MemWrite %1, %2, %3")
							.arg(i).arg(wbytes).arg(var_ptr->m_attr_name.c_str()), LOG_SCOPE_TAG);
		}
	}

	//write all tagNode
	count = (i32)m_sub_node_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_sub_node_vec[i]->memWrite(mode, buffer_ptr, buffer_size);
	}
	return true;
}

void CTagNode::memTagInfoWrite(u8*& buffer_ptr, i32& buffer_size)
{
	i32 i, count = (i32)m_tag_vec.size();
	CPOBase::memWrite(buffer_ptr, buffer_size, m_node_name);
	CPOBase::memWrite(m_node_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_node_reserved, buffer_ptr, buffer_size);
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_tag_vec[i]->memInfoWrite(buffer_ptr, buffer_size);
	}

	//all tagNode
	count = (i32)m_sub_node_vec.size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		m_sub_node_vec[i]->memTagInfoWrite(buffer_ptr, buffer_size);
	}
}

void CTagNode::memTagInfoRead(u8*& buffer_ptr, i32& buffer_size)
{
	removeAll();

	i32 i, count;
	CPOBase::memRead(buffer_ptr, buffer_size, m_node_name);
	CPOBase::memRead(m_node_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_node_reserved, buffer_ptr, buffer_size);
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return;
	}
	m_tag_vec.reserve(count);
	for (i = 0; i < count; i++)
	{
		CVariant* new_tag_ptr = CPOBase::pushBackNew(m_tag_vec);
		new_tag_ptr->memInfoRead(buffer_ptr, buffer_size);
	}

	//all tagNode
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return;
	}
	m_sub_node_vec.reserve(count);
	for (i = 0; i < count; i++)
	{
		CTagNode* sub_node_ptr = CPOBase::pushBackNew(m_sub_node_vec);
		sub_node_ptr->memTagInfoRead(buffer_ptr, buffer_size);
	}
}

bool CTagNode::fileRead(FILE* fp)
{
	removeAll();

	i32 i, count = 0;
	CPOBase::fileRead(m_node_name, fp);
	CPOBase::fileRead(m_node_type, fp);
	CPOBase::fileRead(m_node_reserved, fp);

	//file read all tag
	CVariant* var_ptr;
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	m_tag_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		var_ptr = po_new CVariant();
		var_ptr->fileRead(fp);
		m_tag_vec[i] = var_ptr;
	}

	//file read all tagNode
	CTagNode* tag_node_ptr;
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	m_sub_node_vec.resize(count);
	for (i = 0; i < count; i++)
	{
		tag_node_ptr = po_new CTagNode();
		if (!tag_node_ptr->fileRead(fp))
		{
			return false;
		}

		m_sub_node_vec[i] = tag_node_ptr;
	}

	return true;
}

bool CTagNode::fileWrite(FILE* fp)
{
	CPOBase::fileWrite(fp, m_node_name);
	CPOBase::fileWrite(m_node_type, fp);
	CPOBase::fileWrite(m_node_reserved, fp);

	//write all tag
	i32 i, count = (i32)m_tag_vec.size();
	CPOBase::fileWrite(count, fp);

	for (i = 0; i < count; i++)
	{
		m_tag_vec[i]->fileWrite(fp);
	}

	//write all tagNode
	count = (i32)m_sub_node_vec.size();
	CPOBase::fileWrite(count, fp);

	for (i = 0; i < count; i++)
	{
		if (!m_sub_node_vec[i]->fileWrite(fp))
		{
			return false;
		}
	}

	return true;
}

bool CTagNode::memTagRead(i32 mode, bool accept_node, u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	u8* buffer_sub_pos = buffer_ptr;
	i32 buffer_sub_size;

	postring node_name;
	i32 i, count, index;
	i32 node_type = 0, node_reserved = 0;
	bool is_success = true;
	
	CPOBase::memRead(buffer_ptr, buffer_size, node_name);
	CPOBase::memRead(node_type, buffer_ptr, buffer_size);
	CPOBase::memRead(node_reserved, buffer_ptr, buffer_size);

	if (accept_node || (node_name == m_node_name && node_type == m_node_type))
	{
		//set node reserved
		m_node_reserved = node_reserved;

		//read all tag
		is_success &= memExpTagRead(mode, buffer_ptr, buffer_size);

		count = -1;
		CPOBase::memRead(count, buffer_ptr, buffer_size);
		if (!CPOBase::isCount(count))
		{
			buffer_ptr = buffer_pos;
			return false;
		}

		//read all sub tagnode
		for (i = 0; i < count; i++)
		{
			buffer_sub_pos = buffer_ptr;
			buffer_sub_size = buffer_size;
			CPOBase::memRead(buffer_ptr, buffer_size, node_name);
			CPOBase::memRead(node_type, buffer_ptr, buffer_size);

			index = getNodeIndexOf(node_name);
			if (CPOBase::checkIndex(index, (i32)m_sub_node_vec.size()))
			{
				buffer_ptr = buffer_sub_pos;
				buffer_size = buffer_sub_size;
				is_success &= m_sub_node_vec[index]->memTagRead(mode, accept_node, buffer_ptr, buffer_size);
			}
		}
	}
	else
	{
		memExpTagsSkip(buffer_ptr, buffer_size);
		return false;
	}
	return is_success;
}

void CTagNode::memExpTagsSkip(u8*& buffer_ptr, i32& buffer_size)
{
	i32 len;
	if (CPOBase::memRead(len, buffer_ptr, buffer_size))
	{
		buffer_ptr += len;
		buffer_size -= len;
	}
}

bool CTagNode::memExpTagRead(i32 mode, u8*& buffer_ptr, i32& buffer_size)
{
	i32 i, len = 0, count = -1;
	i32 index, data_type = 0, tag_type = 0;
	f32 min_value, max_value;
	postring tag_name;
	CVariant* dst_var_ptr;

	bool is_success = true;
	u8* buffer_pos = buffer_ptr;
	i32 buffer_len = buffer_size;
	CPOBase::memRead(len, buffer_ptr, buffer_size);

	//read tag and update tag content
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		i32 skip_size = len - sizeof(count);
		buffer_ptr = buffer_pos + skip_size;
		buffer_size = buffer_len - skip_size;
		return false;
	}

	for (i = 0; i < count; i++)
	{
		//read tag infomation
		CPOBase::memRead(buffer_ptr, buffer_size, tag_name);
		CPOBase::memRead(tag_type, buffer_ptr, buffer_size);

		CPOBase::memRead(min_value, buffer_ptr, buffer_size);
		CPOBase::memRead(max_value, buffer_ptr, buffer_size);
		CPOBase::memRead(data_type, buffer_ptr, buffer_size);
		
		//find tag in current Node
		index = getIndexOf(tag_name);
		if (CPOBase::checkIndex(index, (i32)m_tag_vec.size()))
		{
			dst_var_ptr = m_tag_vec[index];
			if (dst_var_ptr->isAcceptable(mode) && dst_var_ptr->getDataType() == data_type)
			{
				u8* buffer_pos = buffer_ptr;
				dst_var_ptr->memReadExpAttr(data_type, buffer_ptr, buffer_size);

				printlog_lvs3(QString("Tag MemExpRead %1, %2").arg(i).arg(buffer_ptr - buffer_pos), LOG_SCOPE_TAG);
			}
			else
			{
				is_success = false;
				dst_var_ptr->memReadExpSkip(buffer_ptr, buffer_size);
			}
		}
	}
	return is_success;
}

CTagNode* CTagNode::addTreeNode(const postring& path)
{
	strvector str_vec;
	CPOBase::spiltToVector(path, kTagDotfix, str_vec);

	CTagNode* tag_node_ptr = this;
	i32 i, index, count = (i32)str_vec.size();
	if (!CPOBase::isPositive(count))
	{
		return NULL;
	}

	for (i = 0; i < count; i++)
	{
		index = tag_node_ptr->getNodeIndexOf(str_vec[i]);
		if (CPOBase::checkIndex(index, kTagNodeMaxCount))
		{
			tag_node_ptr = tag_node_ptr->getTagNode(index);
		}
		else
		{
			tag_node_ptr = tag_node_ptr->addTagNode(str_vec[i], kSCToolSubTree);
		}
	}

	update();
	return tag_node_ptr;
}

bool CTagNode::deleteTreeNode(const postring& path)
{
	strvector str_vec;
	CPOBase::spiltToVector(path, kTagDotfix, str_vec);

	CTagNode* tag_node_ptr = this;
	CTagNode* parent_node_ptr = NULL;
	i32 i, index, count = (i32)str_vec.size();
	if (!CPOBase::isPositive(count))
	{
		return false;
	}

	index = -1;
	for (i = 0; i < count; i++)
	{
		index = tag_node_ptr->getNodeIndexOf(str_vec[i]);
		if (!CPOBase::checkIndex(index, tag_node_ptr->getTagNodeCount()))
		{
			return false;
		}
		parent_node_ptr = tag_node_ptr;
		tag_node_ptr = tag_node_ptr->getTagNode(index);
	}

	if (!tag_node_ptr || !parent_node_ptr)
	{
		return false;
	}

	tag_node_ptr->removeTree();
	parent_node_ptr->removeTagNode(index);
	return true;
}

CVariant* CTagNode::findTreeTag(const postring& path, bool is_linear)
{
	strvector str_vec;
	CPOBase::spiltToVector(path, kTagDotfix, str_vec);

	CVariant* tag_ptr = NULL;
	CTagNode* tag_node_ptr = this;
	i32 i, index, count = (i32)str_vec.size();
	if (!CPOBase::isPositive(count))
	{
		return NULL;
	}

	for (i = 0; i < count; i++)
	{
		if (i == count - 1)
		{
			/* variant */
			index = tag_node_ptr->getIndexOf(str_vec[i]);
			if (!CPOBase::checkIndex(index, tag_node_ptr->getTagCount()))
			{
				return NULL;
			}
			tag_ptr = tag_node_ptr->getTag(index);
		}
		else
		{
			/* node */
			if (is_linear)
			{
				index = tag_node_ptr->linearGetNodeIndexOf(str_vec[i]);
			}
			else
			{
				index = tag_node_ptr->getNodeIndexOf(str_vec[i]);
			}
			if (!CPOBase::checkIndex(index, tag_node_ptr->getTagNodeCount()))
			{
				return NULL;
			}
			tag_node_ptr = tag_node_ptr->getTagNode(index);
		}
	}
	return tag_ptr;
}

CTagNode* CTagNode::findTreeNode(const postring& path, bool is_linear)
{
	strvector str_vec;
	CPOBase::spiltToVector(path, kTagDotfix, str_vec);

	CTagNode* tag_node_ptr = this;
	i32 i, index, count = (i32)str_vec.size();
	for (i = 0; i < count; i++)
	{
		/* node */
		if (is_linear)
		{
			index = tag_node_ptr->linearGetNodeIndexOf(str_vec[i]);
		}
		else
		{
			index = tag_node_ptr->getNodeIndexOf(str_vec[i]);
		}
		if (!CPOBase::checkIndex(index, tag_node_ptr->getTagNodeCount()))
		{
			return NULL;
		}
		tag_node_ptr = tag_node_ptr->getTagNode(index);
	}
	return tag_node_ptr;
}

void CTagNode::copyTreeTag(CTagNode& tag_node)
{
	m_tag_vec = tag_node.m_tag_vec;

	CTagNode* new_node_ptr;
	CTagNode* child_node_ptr;
	i32 i, count = tag_node.getTagNodeCount();
	for (i = 0; i < count; i++)
	{
		child_node_ptr = tag_node.m_sub_node_vec[i];
		new_node_ptr = copyTagNode(child_node_ptr);
		new_node_ptr->copyTreeTag(*child_node_ptr);
	}
}

void CTagNode::setTreeTag(CTagNode& tag_node)
{
	removeTree();
	copyTreeTag(tag_node);
	update();
}

void CTagNode::addTreeTag(CTagNode& tag_node)
{
	m_tag_vec.insert(m_tag_vec.end(), tag_node.m_tag_vec.begin(), tag_node.m_tag_vec.end());
	m_is_tag_updated = false;

	CTagNode* new_node_ptr;
	CTagNode* child_node_ptr;
	i32 i, count = tag_node.getTagNodeCount();
	for (i = 0; i < count; i++)
	{
		child_node_ptr = tag_node.m_sub_node_vec[i];
		new_node_ptr = copyTagNode(child_node_ptr);
		new_node_ptr->copyTreeTag(*child_node_ptr);
	}
	update();
}

void CTagNode::addTreeTag(TagVector& tag_vec)
{
	m_tag_vec.insert(m_tag_vec.end(), tag_vec.begin(), tag_vec.end());

	m_is_tag_updated = false;
	update();
}

void CTagNode::clearResultTreeTag()
{
	bool is_updated = false;

	//remove tag data has mode
	i32 i, count = (i32)m_tag_vec.size();
	for (i = count - 1; i >= 0; i--)
	{
		if (CPOBase::bitCheck(m_tag_vec[i]->getTagType(), kTagTypeResult))
		{
			is_updated = true;
			CPOBase::eraseInVector(m_tag_vec, i);
		}
	}

	//remove tagnode data has mode
	CTagNode* tag_node_ptr;
	count = (i32)m_sub_node_vec.size();
	for (i = count - 1; i >= 0; i--)
	{
		tag_node_ptr = m_sub_node_vec[i];
		if (tag_node_ptr->getNodeType() == kSCToolComplexDataType)
		{
			is_updated = true;
			CPOBase::eraseInVector(m_sub_node_vec, i);
			tag_node_ptr->removeTree();
			POSAFE_DELETE(tag_node_ptr);
		}
	}
	if (is_updated)
	{
		m_is_sub_node_updated = false;
		update();
	}
}

void CTagNode::removeTree()
{
	removeTreeTag();
	removeTreeSubNode();
}

bool CTagNode::removeTreeTag()
{
	m_is_tag_updated = false;
	m_tag_strmap.clear();
	m_tag_vec.clear();
	return true;
}

i32 CTagNode::getTagCount()
{
	return (i32)m_tag_vec.size();
}

i32 CTagNode::getTagNodeCount()
{
	return (i32)m_sub_node_vec.size();
}
