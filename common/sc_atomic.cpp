#include "sc_atomic.h"

//////////////////////////////////////////////////////////////////////////
void CRegionInfoEx::setValue(CRegionInfoEx* value_ptr)
{
	lock_guard();
	anlock_guard_ptr(value_ptr)
	CRegionInfo::setValue(dynamic_cast<CRegionInfo*>(value_ptr));
}

i32 CRegionInfoEx::getPlusRegionCount()
{
	lock_guard();
	return CRegionInfo::getPlusRegionCount();
}

i32 CRegionInfoEx::memSize()
{
	lock_guard();
	return CRegionInfo::memSize();
}

i32 CRegionInfoEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CRegionInfo::memRead(buffer_ptr, buffer_size);
}

i32 CRegionInfoEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CRegionInfo::memWrite(buffer_ptr, buffer_size);
}

bool CRegionInfoEx::fileRead(FILE* fp)
{
	lock_guard();
	return CRegionInfo::fileRead(fp);
}

bool CRegionInfoEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CRegionInfo::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CTransformEx::init()
{
	lock_guard();
	CTransform::init();
}

void CTransformEx::setValue(CTransformEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CTransform::setValue(dynamic_cast<CTransform*>(other_ptr));
}

CTransform CTransformEx::getValue()
{
	lock_guard();
	CTransform tr = *this;
	return tr;
}

void CTransformEx::setValue(const CTransform& other)
{
	lock_guard();
	CTransform::setValue(other);
}

i32 CTransformEx::memSize()
{
	lock_guard();
	return CTransform::memSize();
}

i32 CTransformEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CTransform::memRead(buffer_ptr, buffer_size);
}

i32 CTransformEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CTransform::memWrite(buffer_ptr, buffer_size);
}

bool CTransformEx::fileRead(FILE* fp)
{
	lock_guard();
	return CTransform::fileRead(fp);
}

bool CTransformEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CTransform::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CFoundObjectEx::setValue(CFoundObjectEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CFoundObject::setValue(dynamic_cast<CFoundObject*>(other_ptr));
}

i32 CFoundObjectEx::memSize()
{
	lock_guard();
	return CFoundObject::memSize();
}

i32 CFoundObjectEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundObject::memRead(buffer_ptr, buffer_size);
}

i32 CFoundObjectEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundObject::memWrite(buffer_ptr, buffer_size);
}

bool CFoundObjectEx::fileRead(FILE* fp)
{
	lock_guard();
	return CFoundObject::fileRead(fp);
}

bool CFoundObjectEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CFoundObject::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CFoundObjectVectorEx::init()
{
	lock_guard();
	FoundObjectVector::clear();
}

i32	CFoundObjectVectorEx::size()
{
	lock_guard();
	return (i32)FoundObjectVector::size();
}

void CFoundObjectVectorEx::setValue(CFoundObjectVectorEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*((FoundObjectVector*)this) = *((FoundObjectVector*)other_ptr);
}

FoundObjectVector& CFoundObjectVectorEx::getSharedVector()
{
	return *(dynamic_cast<FoundObjectVector*>(this));
}

CFoundObject CFoundObjectVectorEx::getValue(i32 index)
{
	lock_guard();
	return FoundObjectVector::at(index);
}

void CFoundObjectVectorEx::setValue(const CFoundObject& found_obj)
{
	lock_guard();
	FoundObjectVector::clear();
	FoundObjectVector::resize(1);
	(dynamic_cast<FoundObjectVector*>(this))->at(0) = found_obj;
}

void CFoundObjectVectorEx::setValue(const FoundObjectVector& found_vec)
{
	lock_guard();
	*(dynamic_cast<FoundObjectVector*>(this)) = found_vec;
}

void CFoundObjectVectorEx::addValue(const CFoundObject& found_obj)
{
	lock_guard();
	FoundObjectVector::push_back(found_obj);
}

i32 CFoundObjectVectorEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)FoundObjectVector::size();
	for (i = 0; i < count; i++)
	{
		len += FoundObjectVector::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundObjectVectorEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	FoundObjectVector::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	FoundObjectVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundObjectVector::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundObjectVectorEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = (i32)FoundObjectVector::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		FoundObjectVector::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundObjectVectorEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	FoundObjectVector::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	FoundObjectVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundObjectVector::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundObjectVectorEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)FoundObjectVector::size();
	CPOBase::fileWrite(count, fp);

	for (i = 0; i < count; i++)
	{
		FoundObjectVector::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFoundPatMaxVectorEx::init()
{
	lock_guard();
	FoundPatMaxVector::clear();
}

i32	CFoundPatMaxVectorEx::size()
{
	lock_guard();
	return (i32)FoundPatMaxVector::size();
}

void CFoundPatMaxVectorEx::setValue(CFoundPatMaxVectorEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*((FoundPatMaxVector*)this) = *((FoundPatMaxVector*)other_ptr);
}

FoundPatMaxVector& CFoundPatMaxVectorEx::getSharedVector()
{
	return *(dynamic_cast<FoundPatMaxVector*>(this));
}

CFoundPatMax CFoundPatMaxVectorEx::getValue(i32 index)
{
	lock_guard();
	return FoundPatMaxVector::at(index);
}

void CFoundPatMaxVectorEx::setValue(const CFoundPatMax& found_patmax)
{
	lock_guard();
	FoundPatMaxVector::clear();
	FoundPatMaxVector::resize(1);
	(dynamic_cast<FoundPatMaxVector*>(this))->at(0) = found_patmax;
}

void CFoundPatMaxVectorEx::setValue(const FoundPatMaxVector& found_vec)
{
	lock_guard();
	*(dynamic_cast<FoundPatMaxVector*>(this)) = found_vec;
}

void CFoundPatMaxVectorEx::setValue(const FoundPatMaxPVector& found_ptr_vec)
{
	lock_guard();
	i32 i, count = (i32)found_ptr_vec.size();
	FoundPatMaxVector::resize(count);
	for (i = 0; i < count; i++)
	{
		if (!found_ptr_vec[i])
		{
			continue;
		}
		FoundPatMaxVector::at(i) = *found_ptr_vec[i];
	}
}

void CFoundPatMaxVectorEx::addValue(const CFoundPatMax& found_obj)
{
	lock_guard();
	FoundPatMaxVector::push_back(found_obj);
}

i32 CFoundPatMaxVectorEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)FoundPatMaxVector::size();
	for (i = 0; i < count; i++)
	{
		len += FoundPatMaxVector::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundPatMaxVectorEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	FoundPatMaxVector::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	FoundPatMaxVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundPatMaxVector::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundPatMaxVectorEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 len = 0;
	i32 i, count = (i32)FoundPatMaxVector::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);
	for (i = 0; i < count; i++)
	{
		FoundPatMaxVector::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundPatMaxVectorEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	FoundPatMaxVector::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	FoundPatMaxVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundPatMaxVector::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundPatMaxVectorEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)FoundPatMaxVector::size();
	CPOBase::fileWrite(count, fp);

	for (i = 0; i < count; i++)
	{
		FoundPatMaxVector::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFoundBlobEx::setValue(CFoundBlobEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CFoundBlob::setValue(dynamic_cast<CFoundBlob*>(other_ptr));
}

i32 CFoundBlobEx::memSize()
{
	lock_guard();
	return CFoundBlob::memSize();
}

i32 CFoundBlobEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundBlob::memRead(buffer_ptr, buffer_size);
}

i32 CFoundBlobEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundBlob::memWrite(buffer_ptr, buffer_size);
}

bool CFoundBlobEx::fileRead(FILE* fp)
{
	lock_guard();
	return CFoundBlob::fileRead(fp);
}

bool CFoundBlobEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CFoundBlob::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CFoundBlobVectorEx::init()
{
	lock_guard();
	FoundBlobVector::clear();
}

i32	CFoundBlobVectorEx::size()
{
	lock_guard();
	return (i32)FoundBlobVector::size();
}

void CFoundBlobVectorEx::setValue(CFoundBlobVectorEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*(dynamic_cast<FoundBlobVector*>(this)) = *(dynamic_cast<FoundBlobVector*>(other_ptr));
}

FoundBlobVector& CFoundBlobVectorEx::getSharedVector()
{
	return *(dynamic_cast<FoundBlobVector*>(this));
}

CFoundBlob CFoundBlobVectorEx::getValue(i32 index)
{
	lock_guard();
	return FoundBlobVector::at(index);
}

void CFoundBlobVectorEx::setValue(const FoundBlobVector& found_vec)
{
	lock_guard();
	*(dynamic_cast<FoundBlobVector*>(this)) = found_vec;
}

i32 CFoundBlobVectorEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)FoundBlobVector::size();
	for (i = 0; i < count; i++)
	{
		len += FoundBlobVector::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundBlobVectorEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	FoundBlobVector::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	FoundBlobVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundBlobVector::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundBlobVectorEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	i32 i, count = (i32)FoundBlobVector::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		FoundBlobVector::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundBlobVectorEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	FoundBlobVector::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	FoundBlobVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundBlobVector::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundBlobVectorEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)FoundBlobVector::size();
	CPOBase::fileWrite(count, fp);

	FoundBlobVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundBlobVector::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFoundEdgeEx::init()
{
	lock_guard();
	CFoundEdge::init();
}

CFoundEdge CFoundEdgeEx::getValue()
{
	lock_guard();
	return *(dynamic_cast<CFoundEdge*>(this));
}

void CFoundEdgeEx::setValue(const CFoundEdge& edge)
{
	lock_guard();
	*(dynamic_cast<CFoundEdge*>(this)) = edge;
}

void CFoundEdgeEx::setValue(CFoundEdgeEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CFoundEdge::setValue(dynamic_cast<CFoundEdge*>(other_ptr));
}

i32 CFoundEdgeEx::memSize()
{
	lock_guard();
	return CFoundEdge::memSize();
}

i32 CFoundEdgeEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundEdge::memRead(buffer_ptr, buffer_size);
}

i32 CFoundEdgeEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundEdge::memWrite(buffer_ptr, buffer_size);
}

bool CFoundEdgeEx::fileRead(FILE* fp)
{
	lock_guard();
	return CFoundEdge::fileRead(fp);
}

bool CFoundEdgeEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CFoundEdge::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CFoundEdgeVectorEx::init()
{
	lock_guard();
	FoundEdgeVector::clear();
}

i32 CFoundEdgeVectorEx::size()
{
	lock_guard();
	return (i32)FoundEdgeVector::size();
}

void CFoundEdgeVectorEx::setValue(CFoundEdgeVectorEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*(dynamic_cast<FoundEdgeVector*>(this)) = *(dynamic_cast<FoundEdgeVector*>(other_ptr));
}

FoundEdgeVector& CFoundEdgeVectorEx::getSharedVector()
{
	return *this;
}

void CFoundEdgeVectorEx::setValue(const FoundEdgeVector& found_vec)
{
	lock_guard();
	*(dynamic_cast<FoundEdgeVector*>(this)) = found_vec;
}

i32 CFoundEdgeVectorEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)FoundEdgeVector::size();
	for (i = 0; i < count; i++)
	{
		len += FoundEdgeVector::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundEdgeVectorEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	FoundEdgeVector::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	FoundEdgeVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundEdgeVector::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundEdgeVectorEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	i32 i, count = (i32)FoundEdgeVector::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		FoundEdgeVector::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundEdgeVectorEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	FoundEdgeVector::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	FoundEdgeVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundEdgeVector::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundEdgeVectorEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)FoundEdgeVector::size();
	CPOBase::fileWrite(count, fp);

	for (i = 0; i < count; i++)
	{
		FoundEdgeVector::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFoundCircleEx::init()
{
	lock_guard();
	CFoundCircle::init();
}

void CFoundCircleEx::setValue(CFoundCircleEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CFoundCircle::setValue(dynamic_cast<CFoundCircle*>(other_ptr));
}

CFoundCircle CFoundCircleEx::getValue()
{
	lock_guard();
	return *(dynamic_cast<CFoundCircle*>(this));
}

void CFoundCircleEx::setValue(CFoundCircle& circle)
{
	lock_guard();
	*(dynamic_cast<CFoundCircle*>(this)) = circle;
}

i32 CFoundCircleEx::memSize()
{
	lock_guard();
	return CFoundCircle::memSize();
}

i32 CFoundCircleEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundCircle::memRead(buffer_ptr, buffer_size);
}

i32 CFoundCircleEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CFoundCircle::memWrite(buffer_ptr, buffer_size);
}

bool CFoundCircleEx::fileRead(FILE* fp)
{
	lock_guard();
	return CFoundCircle::fileRead(fp);
}

bool CFoundCircleEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CFoundCircle::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CFoundFlawVectorEx::init()
{
	lock_guard();
	CFoundFlawVector::clear();
}

i32 CFoundFlawVectorEx::size()
{
	lock_guard();
	return (i32)(CFoundFlawVector::size());
}

CFoundFlawVector& CFoundFlawVectorEx::getSharedVector()
{
	return (*this);
}

CFoundFlaw CFoundFlawVectorEx::getValue(i32 index)
{
	lock_guard();
	if (!CPOBase::checkIndex(index, (i32)CFoundFlawVector::size()))
	{
		return CFoundFlaw();
	}
	return CFoundFlawVector::at(index);
}

void CFoundFlawVectorEx::setValue(const CFoundFlawVector& found_vec)
{
	lock_guard();
	*(dynamic_cast<CFoundFlawVector*>(this)) = found_vec;
}

void CFoundFlawVectorEx::setValue(CFoundFlawVectorEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*(dynamic_cast<CFoundFlawVector*>(this)) = *(dynamic_cast<CFoundFlawVector*>(other_ptr));
}

i32 CFoundFlawVectorEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)CFoundFlawVector::size();
	for (i = 0; i < count; i++)
	{
		len += CFoundFlawVector::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundFlawVectorEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	CFoundFlawVector::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	CFoundFlawVector::resize(count);
	for (i = 0; i < count; i++)
	{
		CFoundFlawVector::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundFlawVectorEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	i32 i, count = (i32)CFoundFlawVector::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		CFoundFlawVector::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundFlawVectorEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	CFoundFlawVector::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	CFoundFlawVector::resize(count);
	for (i = 0; i < count; i++)
	{
		CFoundFlawVector::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundFlawVectorEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)CFoundFlawVector::size();
	CPOBase::fileWrite(count, fp);

	CFoundFlawVector::resize(count);
	for (i = 0; i < count; i++)
	{
		CFoundFlawVector::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFoundBeadDefectEx::init()
{
	lock_guard();
	CBeadDefect::clear();
}

i32 CFoundBeadDefectEx::size()
{
	lock_guard();
	return (i32)(CBeadDefect::size());
}

CBeadDefect& CFoundBeadDefectEx::getSharedVector()
{
	return (*this);
}

CBeadDefectItem CFoundBeadDefectEx::getValue(i32 index)
{
	lock_guard();
	if (!CPOBase::checkIndex(index, (i32)CBeadDefect::size()))
	{
		return CBeadDefectItem();
	}
	return CBeadDefect::at(index);
}

void CFoundBeadDefectEx::setValue(const CBeadDefect& found_vec)
{
	lock_guard();
	*(dynamic_cast<CBeadDefect*>(this)) = found_vec;
}

void CFoundBeadDefectEx::setValue(CFoundBeadDefectEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*(dynamic_cast<CBeadDefect*>(this)) = *(dynamic_cast<CBeadDefect*>(other_ptr));
}

i32 CFoundBeadDefectEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)CBeadDefect::size();
	for (i = 0; i < count; i++)
	{
		len += CBeadDefect::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundBeadDefectEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	CBeadDefect::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	CBeadDefect::resize(count);
	for (i = 0; i < count; i++)
	{
		CBeadDefect::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundBeadDefectEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	i32 i, count = (i32)CBeadDefect::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		CBeadDefect::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundBeadDefectEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	CBeadDefect::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	CBeadDefect::resize(count);
	for (i = 0; i < count; i++)
	{
		CBeadDefect::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundBeadDefectEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)CBeadDefect::size();
	CPOBase::fileWrite(count, fp);

	CBeadDefect::resize(count);
	for (i = 0; i < count; i++)
	{
		CBeadDefect::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
CFoundVarTableEx::CFoundVarTableEx()
{
	init();
}

CFoundVarTableEx::~CFoundVarTableEx()
{
}

void CFoundVarTableEx::init()
{
	lock_guard();
	reset();
}

void CFoundVarTableEx::reset()
{
	for (i32 i = 0; i < SC_MAX_VAR_COUNT; i++)
	{
		m_var_name[i].clear();
		m_var_type[i] = kDataTypeNone;
		m_var_index[i] = -1;
		
		m_var_bool[i] = false;
		m_var_i32[i] = 0;
		m_var_f32[i] = 0;
		m_var_str[i] = "";
	}
	m_var_count = 0;
	m_use_initial_value = false;
}

void CFoundVarTableEx::setValue(CFoundVarTableEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);

	m_var_count = other_ptr->m_var_count;
	m_use_initial_value = other_ptr->m_use_initial_value;
	if (!CPOBase::checkRange(m_var_count, SC_MAX_VAR_COUNT))
	{
		m_var_count = 0;
	}

	for (i32 i = 0; i < m_var_count; i++)
	{
		m_var_name[i] = other_ptr->m_var_name[i];
		m_var_type[i] = other_ptr->m_var_type[i];
		m_var_index[i] = other_ptr->m_var_index[i];

		m_var_bool[i] = other_ptr->m_var_bool[i];
		m_var_i32[i] = other_ptr->m_var_i32[i];
		m_var_f32[i] = other_ptr->m_var_f32[i];
		m_var_str[i] = other_ptr->m_var_str[i];
	}
}

i32 CFoundVarTableEx::memSize()
{
	lock_guard();
	if (!CPOBase::checkRange(m_var_count, SC_MAX_VAR_COUNT))
	{
		m_var_count = SC_MAX_VAR_COUNT;
	}

	i32 i, len = 0;
	len += sizeof(m_var_count);
	len += sizeof(m_use_initial_value);
	for (i = 0; i < m_var_count; i++)
	{
		len += CPOBase::getStringMemSize(m_var_name[i]);
		len += sizeof(m_var_type[i]);
		len += sizeof(m_var_index[i]);

		switch (m_var_type[i])
		{
			case kDataTypeBool8:
			{
				len += sizeof(m_var_bool[i]);
				break;
			}
			case kDataTypeInt32:
			{
				len += sizeof(m_var_i32[i]);
				break;
			}
			case kDataTypeFloat32:
			{
				len += sizeof(m_var_f32[i]);
				break;
			}
			case kDataTypeString:
			{
				len += CPOBase::getStringMemSize(m_var_str[i]);
				break;
			}
		}
	}
	return len;
}

i32 CFoundVarTableEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	lock_guard();
	reset();
	
	CPOBase::memRead(m_var_count, buffer_ptr, buffer_size);
	CPOBase::memRead(m_use_initial_value, buffer_ptr, buffer_size);
	if (!CPOBase::checkRange(m_var_count, SC_MAX_VAR_COUNT))
	{
		return buffer_ptr - buffer_pos;
	}

	for (i32 i = 0; i < m_var_count; i++)
	{
		CPOBase::memRead(buffer_ptr, buffer_size, m_var_name[i]);
		CPOBase::memRead(m_var_type[i], buffer_ptr, buffer_size);
		CPOBase::memRead(m_var_index[i], buffer_ptr, buffer_size);

		switch (m_var_type[i])
		{
			case kDataTypeBool8:
			{
				CPOBase::memRead(m_var_bool[i], buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeInt32:
			{
				CPOBase::memRead(m_var_i32[i], buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFloat32:
			{
				CPOBase::memRead(m_var_f32[i], buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeString:
			{
				CPOBase::memRead(buffer_ptr, buffer_size, m_var_str[i]);
				break;
			}
		}
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundVarTableEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	lock_guard();
	if (!CPOBase::checkRange(m_var_count, SC_MAX_VAR_COUNT))
	{
		m_var_count = SC_MAX_VAR_COUNT;
	}

	CPOBase::memWrite(m_var_count, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_use_initial_value, buffer_ptr, buffer_size);
	for (i32 i = 0; i < m_var_count; i++)
	{
		CPOBase::memWrite(buffer_ptr, buffer_size, m_var_name[i]);
		CPOBase::memWrite(m_var_type[i], buffer_ptr, buffer_size);
		CPOBase::memWrite(m_var_index[i], buffer_ptr, buffer_size);

		switch (m_var_type[i])
		{
			case kDataTypeBool8:
			{
				CPOBase::memWrite(m_var_bool[i], buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeInt32:
			{
				CPOBase::memWrite(m_var_i32[i], buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFloat32:
			{
				CPOBase::memWrite(m_var_f32[i], buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeString:
			{
				CPOBase::memWrite(buffer_ptr, buffer_size, m_var_str[i]);
				break;
			}
		}
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundVarTableEx::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	lock_guard();
	reset();

	CPOBase::fileRead(m_var_count, fp);
	CPOBase::fileRead(m_use_initial_value, fp);
	if (!CPOBase::checkRange(m_var_count, SC_MAX_VAR_COUNT))
	{
		return false;
	}

	for (i32 i = 0; i < m_var_count; i++)
	{
		CPOBase::fileRead(fp, m_var_name[i]);
		CPOBase::fileRead(m_var_type[i], fp);
		CPOBase::fileRead(m_var_index[i], fp);

		switch (m_var_type[i])
		{
			case kDataTypeBool8:
			{
				CPOBase::fileRead(m_var_bool[i], fp);
				break;
			}
			case kDataTypeInt32:
			{
				CPOBase::fileRead(m_var_i32[i], fp);
				break;
			}
			case kDataTypeFloat32:
			{
				CPOBase::fileRead(m_var_f32[i], fp);
				break;
			}
			case kDataTypeString:
			{
				CPOBase::fileRead(fp, m_var_str[i]);
				break;
			}
		}
	}
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CFoundVarTableEx::fileWrite(FILE* fp)
{
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);

	lock_guard();
	m_var_count = po::_min(m_var_count, SC_MAX_VAR_COUNT);
	
	CPOBase::fileWrite(m_var_count, fp);
	CPOBase::fileWrite(m_use_initial_value, fp);
	for (i32 i = 0; i < m_var_count; i++)
	{
		CPOBase::fileWrite(fp, m_var_name[i]);
		CPOBase::fileWrite(m_var_type[i], fp);
		CPOBase::fileWrite(m_var_index[i], fp);

		switch (m_var_type[i])
		{
			case kDataTypeBool8:
			{
				CPOBase::fileWrite(m_var_bool[i], fp);
				break;
			}
			case kDataTypeInt32:
			{
				CPOBase::fileWrite(m_var_i32[i], fp);
				break;
			}
			case kDataTypeFloat32:
			{
				CPOBase::fileWrite(m_var_f32[i], fp);
				break;
			}
			case kDataTypeString:
			{
				CPOBase::fileWrite(fp, m_var_str[i]);
				break;
			}
		}
	}
	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFoundBarcodeVectorEx::init()
{
	lock_guard();
	FoundBarcodeVector::clear();
}

i32 CFoundBarcodeVectorEx::size()
{
	lock_guard();
	return (i32)(FoundBarcodeVector::size());
}

FoundBarcodeVector CFoundBarcodeVectorEx::getValue()
{
	return (*this);
}

CFoundBarcode CFoundBarcodeVectorEx::getValue(i32 index)
{
	lock_guard();
	if (!CPOBase::checkIndex(index, (i32)FoundBarcodeVector::size()))
	{
		return CFoundBarcode();
	}
	return FoundBarcodeVector::at(index);
}

void CFoundBarcodeVectorEx::setValue(const FoundBarcodeVector& found_vec)
{
	lock_guard();
	*(dynamic_cast<FoundBarcodeVector*>(this)) = found_vec;
}

void CFoundBarcodeVectorEx::setValue(CFoundBarcodeVectorEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	*(dynamic_cast<FoundBarcodeVector*>(this)) = *(dynamic_cast<FoundBarcodeVector*>(other_ptr));
}

i32 CFoundBarcodeVectorEx::memSize()
{
	lock_guard();
	i32 len = 0;
	i32 i, count = (i32)FoundBarcodeVector::size();
	for (i = 0; i < count; i++)
	{
		len += FoundBarcodeVector::at(i).memSize();
	}
	len += sizeof(count);
	return len;
}

i32 CFoundBarcodeVectorEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;

	i32 i, count = -1;
	FoundBarcodeVector::clear();
	CPOBase::memRead(count, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(count))
	{
		return buffer_ptr - buffer_pos;
	}

	FoundBarcodeVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundBarcodeVector::at(i).memRead(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

i32 CFoundBarcodeVectorEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	u8* buffer_pos = buffer_ptr;
	i32 i, count = (i32)FoundBarcodeVector::size();
	CPOBase::memWrite(count, buffer_ptr, buffer_size);

	for (i = 0; i < count; i++)
	{
		FoundBarcodeVector::at(i).memWrite(buffer_ptr, buffer_size);
	}
	return buffer_ptr - buffer_pos;
}

bool CFoundBarcodeVectorEx::fileRead(FILE* fp)
{
	lock_guard();
	i32 i, count;
	FoundBarcodeVector::clear();
	CPOBase::fileRead(count, fp);
	if (!CPOBase::isCount(count))
	{
		return false;
	}

	FoundBarcodeVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundBarcodeVector::at(i).fileRead(fp);
	}
	return true;
}

bool CFoundBarcodeVectorEx::fileWrite(FILE* fp)
{
	lock_guard();
	i32 i, count = (i32)FoundBarcodeVector::size();
	CPOBase::fileWrite(count, fp);

	FoundBarcodeVector::resize(count);
	for (i = 0; i < count; i++)
	{
		FoundBarcodeVector::at(i).fileWrite(fp);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CDistanceDrawEx::init()
{
	lock_guard();
	CDistanceDraw::init();
}

void CDistanceDrawEx::setValue(CDistanceDraw& draw_distance)
{
	lock_guard();
	CDistanceDraw::setValue(&draw_distance);
}

void CDistanceDrawEx::setValue(CDistanceDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CDistanceDraw::setValue(dynamic_cast<CDistanceDraw*>(other_ptr));
}

i32 CDistanceDrawEx::memSize()
{
	lock_guard();
	return CDistanceDraw::memSize();
}

i32 CDistanceDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CDistanceDraw::memRead(buffer_ptr, buffer_size);
}

i32 CDistanceDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CDistanceDraw::memWrite(buffer_ptr, buffer_size);
}

bool CDistanceDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CDistanceDraw::fileRead(fp);
}

bool CDistanceDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CDistanceDraw::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CDimensionDrawEx::init()
{
	lock_guard();
	CDimensionDraw::init();
}

void CDimensionDrawEx::setValue(CDimensionDraw& draw_dimension)
{
	lock_guard();
	CDimensionDraw::setValue(&draw_dimension);
}

void CDimensionDrawEx::setValue(CDimensionDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CDimensionDraw::setValue(dynamic_cast<CDimensionDraw*>(other_ptr));
}

i32 CDimensionDrawEx::memSize()
{
	lock_guard();
	return CDimensionDraw::memSize();
}

i32 CDimensionDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CDimensionDraw::memRead(buffer_ptr, buffer_size);
}

i32 CDimensionDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CDimensionDraw::memWrite(buffer_ptr, buffer_size);
}

bool CDimensionDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CDimensionDraw::fileRead(fp);
}

bool CDimensionDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CDimensionDraw::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CAngleDrawEx::init()
{
	lock_guard();
	CAngleDraw::init();
}

void CAngleDrawEx::setValue(CAngleDraw& circle)
{
	lock_guard();
	CAngleDraw::setValue(&circle);
}

void CAngleDrawEx::setValue(CAngleDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CAngleDraw::setValue(dynamic_cast<CAngleDraw*>(other_ptr));
}

i32 CAngleDrawEx::memSize()
{
	lock_guard();
	return CAngleDraw::memSize();
}

i32 CAngleDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CAngleDraw::memRead(buffer_ptr, buffer_size);
}

i32 CAngleDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CAngleDraw::memWrite(buffer_ptr, buffer_size);
}

bool CAngleDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CAngleDraw::fileRead(fp);
}

bool CAngleDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CAngleDraw::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CArcDrawEx::init()
{
	lock_guard();
	CPOArc::init();
}

void CArcDrawEx::setValue(CPOArc& other)
{
	lock_guard();
	CPOArc::setValue(other);
}

void CArcDrawEx::setValue(CArcDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CPOArc::setValue(other_ptr);
}

i32 CArcDrawEx::memSize()
{
	lock_guard();
	return CPOArc::memSize();
}

i32 CArcDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOArc::memRead(buffer_ptr, buffer_size);
}

i32 CArcDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOArc::memWrite(buffer_ptr, buffer_size);
}

bool CArcDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CPOArc::fileRead(fp);
}

bool CArcDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CPOArc::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CCircleDrawEx::init()
{
	lock_guard();
	CPOCircle::init();
}

void CCircleDrawEx::setValue(CPOCircle& other)
{
	lock_guard();
	CPOCircle::setValue(other);
}

void CCircleDrawEx::setValue(CCircleDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CPOCircle::setValue(other_ptr);
}

i32 CCircleDrawEx::memSize()
{
	lock_guard();
	return CPOCircle::memSize();
}

i32 CCircleDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOCircle::memRead(buffer_ptr, buffer_size);
}

i32 CCircleDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOCircle::memWrite(buffer_ptr, buffer_size);
}

bool CCircleDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CPOCircle::fileRead(fp);
}

bool CCircleDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CPOCircle::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CCrossDrawEx::init()
{
	lock_guard();
	CPOCross::init();
}

void CCrossDrawEx::setValue(CPOCross& other)
{
	lock_guard();
	CPOCross::setValue(other);
}

void CCrossDrawEx::setValue(CCrossDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CPOCross::setValue(other_ptr);
}

i32 CCrossDrawEx::memSize()
{
	lock_guard();
	return CPOCross::memSize();
}

i32 CCrossDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOCross::memRead(buffer_ptr, buffer_size);
}

i32 CCrossDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOCross::memWrite(buffer_ptr, buffer_size);
}

bool CCrossDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CPOCross::fileRead(fp);
}

bool CCrossDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CPOCross::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CLineDrawEx::init()
{
	lock_guard();
	CPOLine::init();
}

void CLineDrawEx::setValue(CPOLine& other)
{
	lock_guard();
	CPOLine::setValue(other);
}

void CLineDrawEx::setValue(CLineDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CPOLine::setValue(other_ptr);
}

i32 CLineDrawEx::memSize()
{
	lock_guard();
	return CPOLine::memSize();
}

i32 CLineDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOLine::memRead(buffer_ptr, buffer_size);
}

i32 CLineDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPOLine::memWrite(buffer_ptr, buffer_size);
}

bool CLineDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CPOLine::fileRead(fp);
}

bool CLineDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CPOLine::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CRotatedRectDrawEx::init()
{
	lock_guard();
	CPORotatedRect::init();
}

void CRotatedRectDrawEx::setValue(CPORotatedRect& other)
{
	lock_guard();
	CPORotatedRect::setValue(other);
}

void CRotatedRectDrawEx::setValue(CRotatedRectDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CPORotatedRect::setValue(other_ptr);
}

i32 CRotatedRectDrawEx::memSize()
{
	lock_guard();
	return CPORotatedRect::memSize();
}

i32 CRotatedRectDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPORotatedRect::memRead(buffer_ptr, buffer_size);
}

i32 CRotatedRectDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CPORotatedRect::memWrite(buffer_ptr, buffer_size);
}

bool CRotatedRectDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CPORotatedRect::fileRead(fp);
}

bool CRotatedRectDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CPORotatedRect::fileWrite(fp);
}

//////////////////////////////////////////////////////////////////////////
void CStringDrawEx::init()
{
	lock_guard();
	CStringDraw::init();
}

void CStringDrawEx::setValue(CStringDraw& other)
{
	lock_guard();
	CStringDraw::setValue(&other);
}

void CStringDrawEx::setValue(CStringDrawEx* other_ptr)
{
	if (!other_ptr)
	{
		return;
	}

	lock_guard();
	anlock_guard_ptr(other_ptr);
	CStringDraw::setValue(other_ptr);
}

i32 CStringDrawEx::memSize()
{
	lock_guard();
	return CStringDraw::memSize();
}

i32 CStringDrawEx::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CStringDraw::memRead(buffer_ptr, buffer_size);
}

i32 CStringDrawEx::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	lock_guard();
	return CStringDraw::memWrite(buffer_ptr, buffer_size);
}

bool CStringDrawEx::fileRead(FILE* fp)
{
	lock_guard();
	return CStringDraw::fileRead(fp);
}

bool CStringDrawEx::fileWrite(FILE* fp)
{
	lock_guard();
	return CStringDraw::fileWrite(fp);
}
