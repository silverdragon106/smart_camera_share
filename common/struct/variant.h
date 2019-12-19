#pragma once
#include "sc_define.h"
#include "struct.h"
#include "base.h"

class CVariant
{
public:
	CVariant();
	CVariant(const CVariant& robj);
	CVariant(const postring& attr_name, DataType data_type, TagType tag_type = kTagTypeNone, void* buffer_ptr = NULL);
	~CVariant();

	void						init();
	void*						initBuffer(DataType data_type);
	void						freeBuffer();

	CVariant&					operator=(const CVariant& robj);
	
	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

	void						memReadExpSkip(u8*& buffer_ptr, i32& buffer_size);
	void						memReadExpAttr(i32 data_type, u8*& buffer_ptr, i32& buffer_size);
	void						fileReadExpAttr(i32 data_type, FILE* fp);

	i32							memInfoSize();
	i32							memInfoWrite(u8*& buffer_ptr, i32& buffer_size);
	i32							memInfoRead(u8*& buffer_ptr, i32& buffer_size);

	bool						isNumeric();
	bool						isString();
	bool						isMixType();
	bool						isPoint2dType();
	bool						isRectType();
	bool						isAcceptable(i32 mode);

	void						setParent(void* parent_ptr);
	void						setMinValue(f32 min_value);
	void						setMaxValue(f32 max_value);
	void						setMinMaxValue(f32 min_value, f32 max_value);
	void						limitValue();

	bool						setValuebool(bool value);
	
	bool						setValuei8(i8 value);
	bool						setValuei16(i16 value);
	bool						setValuei32(i32 value);
	bool						setValuei64(i64 value);

	bool						setValueu8(u8 value);
	bool						setValueu16(u16 value);
	bool						setValueu32(u32 value);
	bool						setValueu64(u64 value);
	
	bool						setValuef32(f32 value);
	bool						setValuef64(f64 value);

	bool						setValueVector2di(const vector2di& point);
	bool						setValueVector2df(const vector2df& point);
	bool						setValueVector2dd(const vector2dd& point);
	bool						setValueRecti(const Recti& rt);
	bool						setValueRectf(const Rectf& rt);
	bool						setValueRectd(const Rectd& rt);

	bool						setValueString(const postring& str);
	bool						setValueWString(const powstring& str);
	bool						setValueImg(const Img& img);

	bool						toValuebool();
	i8							toValuei8();
	i16							toValuei16();
	i32							toValuei32();
	i64							toValuei64();

	u8							toValueu8();
	u16							toValueu16();
	u32							toValueu32();
	u64							toValueu64();

	f32							toValuef32();
	f64							toValuef64();
	postring					toString(i32 decimals = 2);
	powstring					toWString();
	vector2di					toVector2di();
	vector2df					toVector2df();
	vector2dd					toVector2dd();
	Recti						toRecti();
	Rectf						toRectf();
	Rectd						toRectd();

	inline bool					isNull() { return m_buffer_ptr == NULL; };
	inline bool					isReadable() { return CPOBase::bitCheck(m_tag_type, kTagTypeRead); };
	inline bool					isWritable() { return CPOBase::bitCheck(m_tag_type, kTagTypeWrite); };

	inline postring				getAttrName() { return m_attr_name; };
	inline DataType				getDataType() { return m_data_type; };
	inline TagType				getTagType() { return m_tag_type; };
	inline void*				getParent() { return m_parent_ptr; };

	inline f32					getMinValue() { return m_min_value; };
	inline f32					getMaxValue() { return m_max_value; };

public:
	postring					m_attr_name;
	TagType						m_tag_type;
	DataType					m_data_type;
	i32							m_data_size;

	f32							m_min_value;
	f32							m_max_value;
	void*						m_parent_ptr;		//tempoary

	void*						m_buffer_ptr;		//tempoary
	bool						m_use_external;		//tempoary
};

typedef std::vector<CVariant*>	TagVector;
