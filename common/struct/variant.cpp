#include "variant.h"
#include "base.h"
#include "struct.h"
#include "struct/run_table.h"
#include "region_info.h"
#include "found_object.h"
#include "found_blob.h"
#include "found_edge.h"
#include "found_flaw.h"
#include "sc_atomic.h"
#include <iomanip> // setprecision
#include <sstream> // stringstream

#define VAR_DELETE(dtype, pbuff)		{ dtype* data_ptr_##dtype = (dtype*)pbuff; POSAFE_DELETE(data_ptr_##dtype); pbuff = NULL; }
#define VAR_DELETE_ARRAY(dtype, pbuff)	{ dtype* data_ptr_##dtype = (dtype*)pbuff; POSAFE_DELETE_ARRAY(data_ptr_##dtype); pbuff = NULL; }

//////////////////////////////////////////////////////////////////////////
CVariant::CVariant()
{
	m_buffer_ptr = NULL;
	init();
}

CVariant::CVariant(const postring& attr_name, DataType data_type, TagType tag_type, void* buffer_ptr)
{
	m_buffer_ptr = NULL;
	init();

	//set variant information
	m_attr_name = attr_name;

	m_tag_type = tag_type;
	m_data_type = data_type;
	m_buffer_ptr = buffer_ptr;
	m_min_value = 0;
	m_max_value = -1;

	if (m_buffer_ptr)
	{
		m_use_external = true;
	}

	//init buffer
	if (!initBuffer(m_data_type))
	{
		printlog_lv3(QString("CVariant initBuffer failed[name:%1].").arg(attr_name.c_str()));
		return;
	}
}

CVariant::CVariant(const CVariant &robj)
{
	m_buffer_ptr = NULL;
	init();

	*this = robj;
}

CVariant::~CVariant()
{
	freeBuffer();
}

void CVariant::init()
{
	freeBuffer();

	m_attr_name = "";
	m_tag_type = kTagTypeNone;
	m_data_type = kDataTypeNone;
	m_data_size = 0;

	m_min_value = 0;
	m_max_value = 0;

	m_buffer_ptr = NULL;
	m_parent_ptr = NULL;
	m_use_external = false;
}

void* CVariant::initBuffer(DataType data_type)
{
	if (m_buffer_ptr || m_use_external)
	{
		return m_buffer_ptr;
	}
	
	m_data_type = data_type;
	switch (data_type)
	{
		case kDataTypeBool8:						m_buffer_ptr = po_new atbool; break;
		case kDataTypeBool32:						m_buffer_ptr = po_new ati32; break;
		case kDataTypeInt8:							m_buffer_ptr = po_new ati8; break;
		case kDataTypeInt16:						m_buffer_ptr = po_new ati16; break;
		case kDataTypeInt32:						m_buffer_ptr = po_new ati32; break;
		case kDataTypeInt64:						m_buffer_ptr = po_new ati64; break;
		case kDataTypeUInt8:						m_buffer_ptr = po_new atu8; break;
		case kDataTypeUInt16:						m_buffer_ptr = po_new atu16; break;
		case kDataTypeUInt32:						m_buffer_ptr = po_new atu32; break;
		case kDataTypeUInt64:						m_buffer_ptr = po_new atu64; break;
		case kDataTypeFloat32:						m_buffer_ptr = po_new atf32; break;
		case kDataTypeFloat64:						m_buffer_ptr = po_new atf64; break;
		case kDataTypeVector2di:					m_buffer_ptr = po_new atVector2di; break;
		case kDataTypeVector2df:					m_buffer_ptr = po_new atVector2df; break;
		case kDataTypeVector2dd:					m_buffer_ptr = po_new atVector2dd; break;
		case kDataTypeRectf:						m_buffer_ptr = po_new atRectf; break;
		case kDataTypeRecti:						m_buffer_ptr = po_new atRecti; break;
		case kDataTypeString:						m_buffer_ptr = po_new atstring; break;
		case kDataTypeWString:						m_buffer_ptr = po_new atwstring; break;
		case kDataTypeInt8Vector:					m_buffer_ptr = po_new ati8vector; break;
		case kDataTypeInt32Vector:					m_buffer_ptr = po_new ati32vector; break;
		case kDataTypeFloat32Vector:				m_buffer_ptr = po_new atf32vector; break;
		case kDataTypeStringVector:					m_buffer_ptr = po_new atstrvector; break;
		case kDataTypeImage:						m_buffer_ptr = po_new atImg(); break;
		case kDataTypeArbitraryImage:				m_buffer_ptr = po_new atArbitraryImg(); break;
		case kDataTypeRegion:						m_buffer_ptr = po_new CRegionInfoEx(); break;
		case kDataTypeImgRunTable:					m_buffer_ptr = po_new CImgRunTableEx(); break;
		case kDataTypeTransform:					m_buffer_ptr = po_new CTransformEx(); break;
		case kDataTypeFoundObject:					m_buffer_ptr = po_new CFoundObjectEx(); break;
		case kDataTypeFoundObjectVector:			m_buffer_ptr = po_new CFoundObjectVectorEx; break;
		case kDataTypeFoundPatMaxVector:			m_buffer_ptr = po_new CFoundPatMaxVectorEx; break;
		case kDataTypeFoundBlob:					m_buffer_ptr = po_new CFoundBlobEx(); break;
		case kDataTypeFoundBlobVector:				m_buffer_ptr = po_new CFoundBlobVectorEx; break;
		case kDataTypeFoundEdge:					m_buffer_ptr = po_new CFoundEdgeEx(); break;
		case kDataTypeFoundEdgeVector:				m_buffer_ptr = po_new CFoundEdgeVectorEx; break;
		case kDataTypeFoundCircle:					m_buffer_ptr = po_new CFoundCircleEx(); break;
		case kDataTypeFoundFlawVector:				m_buffer_ptr = po_new CFoundFlawVectorEx(); break;
		case kDataTypeFoundDefects:					m_buffer_ptr = po_new CFoundBeadDefectEx(); break;
		case kDataTypeFoundVarTable:				m_buffer_ptr = po_new CFoundVarTableEx(); break;
		case kDataTypeFoundBarcodeVector:			m_buffer_ptr = po_new CFoundBarcodeVectorEx(); break;
		case kDataTypeContours:						m_buffer_ptr = po_new CContoursEx(); break;
		case kDataTypeShapes:						m_buffer_ptr = po_new CShapesEx(); break;
		case kDataTypeVector2diVec:					m_buffer_ptr = po_new CVector2diVecEx(); break;
		case kDataTypeVector2dfVec:					m_buffer_ptr = po_new CVector2dfVecEx(); break;
		case kDataTypeVector2ddVec:					m_buffer_ptr = po_new CVector2ddVecEx(); break;
		case kDataTypeArc:							m_buffer_ptr = po_new CArcDrawEx(); break;
		case kDataTypeCircle:						m_buffer_ptr = po_new CCircleDrawEx(); break;
		case kDataTypeCross:						m_buffer_ptr = po_new CCrossDrawEx(); break;
		case kDataTypeLine:							m_buffer_ptr = po_new CLineDrawEx(); break;
		case kDataTypeRotateRect:					m_buffer_ptr = po_new CAngleDrawEx(); break;
		case kDataTypeDistanceDraw:					m_buffer_ptr = po_new CDistanceDrawEx(); break;
		case kDataTypeDimensionDraw:				m_buffer_ptr = po_new CDimensionDrawEx(); break;
		case kDataTypeAngleDraw:					m_buffer_ptr = po_new CAngleDrawEx(); break;
		case kDataTypeStringDraw:					m_buffer_ptr = po_new CStringDrawEx(); break;
		default:									m_buffer_ptr = NULL; break;
	}

	return m_buffer_ptr;
}

void CVariant::freeBuffer()
{
	if (!m_use_external)
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:					VAR_DELETE(atbool, m_buffer_ptr); break;
			case kDataTypeBool32:					VAR_DELETE(ati32, m_buffer_ptr); break;
			case kDataTypeInt8:						VAR_DELETE(ati8, m_buffer_ptr); break;
			case kDataTypeInt16:					VAR_DELETE(ati16, m_buffer_ptr); break;
			case kDataTypeInt32:					VAR_DELETE(ati32, m_buffer_ptr); break;
			case kDataTypeInt64:					VAR_DELETE(ati64, m_buffer_ptr); break;
			case kDataTypeUInt8:					VAR_DELETE(atu8, m_buffer_ptr); break;
			case kDataTypeUInt16:					VAR_DELETE(atu16, m_buffer_ptr); break;
			case kDataTypeUInt32:					VAR_DELETE(atu32, m_buffer_ptr); break;
			case kDataTypeUInt64:					VAR_DELETE(atu64, m_buffer_ptr); break;
			case kDataTypeFloat32:					VAR_DELETE(atf32, m_buffer_ptr); break;
			case kDataTypeFloat64:					VAR_DELETE(atf64, m_buffer_ptr); break;
			case kDataTypeVector2di:				VAR_DELETE(atVector2di, m_buffer_ptr); break;
			case kDataTypeVector2df:				VAR_DELETE(atVector2df, m_buffer_ptr); break;
			case kDataTypeVector2dd:				VAR_DELETE(atVector2dd, m_buffer_ptr); break;
			case kDataTypeRectf:					VAR_DELETE(atRectf, m_buffer_ptr); break;
			case kDataTypeRecti:					VAR_DELETE(atRecti, m_buffer_ptr); break;
			case kDataTypeString:					VAR_DELETE(atstring, m_buffer_ptr); break;
			case kDataTypeWString:					VAR_DELETE(atwstring, m_buffer_ptr); break;
			case kDataTypeInt8Vector:				VAR_DELETE(ati8vector, m_buffer_ptr); break;
			case kDataTypeInt32Vector:				VAR_DELETE(ati32vector, m_buffer_ptr); break;
			case kDataTypeFloat32Vector:			VAR_DELETE(atf32vector, m_buffer_ptr); break;
			case kDataTypeStringVector:				VAR_DELETE(atstrvector, m_buffer_ptr); break;
			case kDataTypeImage:					VAR_DELETE(atImg, m_buffer_ptr); break;
			case kDataTypeArbitraryImage:			VAR_DELETE(atArbitraryImg, m_buffer_ptr); break;
			case kDataTypeRegion:					VAR_DELETE(CRegionInfoEx, m_buffer_ptr); break;
			case kDataTypeImgRunTable:				VAR_DELETE(CImgRunTableEx, m_buffer_ptr); break;
			case kDataTypeTransform:				VAR_DELETE(CTransformEx, m_buffer_ptr); break;
			case kDataTypeFoundObject:				VAR_DELETE(CFoundObjectEx, m_buffer_ptr); break;
			case kDataTypeFoundObjectVector:		VAR_DELETE(CFoundObjectVectorEx, m_buffer_ptr); break;
			case kDataTypeFoundPatMaxVector:		VAR_DELETE(CFoundPatMaxVectorEx, m_buffer_ptr); break;
			case kDataTypeFoundBlob:				VAR_DELETE(CFoundBlobEx, m_buffer_ptr); break;
			case kDataTypeFoundBlobVector:			VAR_DELETE(CFoundBlobVectorEx, m_buffer_ptr); break;
			case kDataTypeFoundEdge:				VAR_DELETE(CFoundEdgeEx, m_buffer_ptr); break;
			case kDataTypeFoundEdgeVector:			VAR_DELETE(CFoundEdgeVectorEx, m_buffer_ptr); break;
			case kDataTypeFoundCircle:				VAR_DELETE(CFoundCircleEx, m_buffer_ptr); break;
			case kDataTypeFoundFlawVector:			VAR_DELETE(CFoundFlawVectorEx, m_buffer_ptr); break;
			case kDataTypeFoundDefects:				VAR_DELETE(CFoundBeadDefectEx, m_buffer_ptr); break;
			case kDataTypeFoundVarTable:			VAR_DELETE(CFoundVarTableEx, m_buffer_ptr); break;
			case kDataTypeFoundBarcodeVector:		VAR_DELETE(CFoundBarcodeVectorEx, m_buffer_ptr); break;
			case kDataTypeContours:					VAR_DELETE(CContoursEx, m_buffer_ptr); break;
			case kDataTypeShapes:					VAR_DELETE(CShapesEx, m_buffer_ptr); break;
			case kDataTypeVector2diVec:				VAR_DELETE(CVector2diVecEx, m_buffer_ptr); break;
			case kDataTypeVector2dfVec:				VAR_DELETE(CVector2dfVecEx, m_buffer_ptr); break;
			case kDataTypeVector2ddVec:				VAR_DELETE(CVector2ddVecEx, m_buffer_ptr); break;
			case kDataTypeArc:						VAR_DELETE(CArcDrawEx, m_buffer_ptr); break;
			case kDataTypeCircle:					VAR_DELETE(CCircleDrawEx, m_buffer_ptr); break;
			case kDataTypeCross:					VAR_DELETE(CCrossDrawEx, m_buffer_ptr); break;
			case kDataTypeLine:						VAR_DELETE(CLineDrawEx, m_buffer_ptr); break;
			case kDataTypeRotateRect:				VAR_DELETE(CRotatedRectDrawEx, m_buffer_ptr); break;
			case kDataTypeDistanceDraw:				VAR_DELETE(CDistanceDrawEx, m_buffer_ptr); break;
			case kDataTypeDimensionDraw:			VAR_DELETE(CDimensionDrawEx, m_buffer_ptr); break;
			case kDataTypeAngleDraw:				VAR_DELETE(CAngleDrawEx, m_buffer_ptr); break;
			case kDataTypeStringDraw:				VAR_DELETE(CStringDrawEx, m_buffer_ptr); break;
			default:								VAR_DELETE_ARRAY(u8, m_buffer_ptr); break;
		}
	}
	m_use_external = false;
}

CVariant& CVariant::operator=(const CVariant& robj)
{
	init();
		
	m_attr_name = robj.m_attr_name;
	m_tag_type = robj.m_tag_type;
	m_data_type = robj.m_data_type;

	m_min_value = robj.m_min_value;
	m_max_value = robj.m_max_value;
	m_parent_ptr = robj.m_parent_ptr;

	void* data_ptr = initBuffer(robj.m_data_type);
	if (data_ptr)
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:
			{
				*((atbool*)data_ptr) = (bool)*((atbool*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeBool32:
			{
				*((ati32*)data_ptr) = (i32)*((ati32*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeInt8:
			{
				*((ati8*)data_ptr) = (i8)*((ati8*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeInt16:
			{
				*((ati16*)data_ptr) = (i16)*((ati16*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeInt32:
			{
				*((ati32*)data_ptr) = (i32)*((ati32*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeInt64:
			{
				*((ati64*)data_ptr) = (i64)*((ati64*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeUInt8:
			{
				*((atu8*)data_ptr) = (u8)*((atu8*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeUInt16:
			{
				*((atu16*)data_ptr) = (u16)*((atu16*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeUInt32:
			{
				*((atu32*)data_ptr) = (u32)*((atu32*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeUInt64:
			{
				*((atu64*)data_ptr) = (u64)*((atu64*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeFloat32:
			{
				*((atf32*)data_ptr) = (f32)*((atf32*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeFloat64:
			{
				*((atf64*)data_ptr) = (f64)*((atf64*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeVector2di:
			{
				((atVector2di*)data_ptr)->setValue(((atVector2di*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeVector2df:
			{
				((atVector2df*)data_ptr)->setValue(((atVector2df*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeVector2dd:
			{
				((atVector2dd*)data_ptr)->setValue(((atVector2dd*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeRectf:
			{
				((atRectf*)data_ptr)->setValue(((atRectf*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeRecti:
			{
				((atRecti*)data_ptr)->setValue(((atRecti*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeString:
			{
				((atstring*)data_ptr)->setValue(((atstring*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeWString:
			{
				((atwstring*)data_ptr)->setValue(((atwstring*)(robj.m_buffer_ptr))->getValue());
				break;
			}
			case kDataTypeInt8Vector:
			{
				((ati8vector*)data_ptr)->setValue((ati8vector*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeInt32Vector:
			{
				((ati32vector*)data_ptr)->setValue((ati32vector*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeFloat32Vector:
			{
				((atf32vector*)data_ptr)->setValue((atf32vector*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeStringVector:
			{
				((atstrvector*)data_ptr)->setValue((atstrvector*)(robj.m_buffer_ptr));
				break;
			}
			case kDataTypeImage:
			{
				((atImg*)data_ptr)->setValue((atImg*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeArbitraryImage:
			{
				((atArbitraryImg*)data_ptr)->setValue((atArbitraryImg*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeRegion:
			{
				((CRegionInfoEx*)data_ptr)->setValue((CRegionInfoEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeImgRunTable:
			{
				((CImgRunTableEx*)data_ptr)->setValue((CImgRunTableEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeTransform:
			{
				((CTransformEx*)data_ptr)->setValue((CTransformEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundObject:
			{
				((CFoundObjectEx*)data_ptr)->setValue((CFoundObjectEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundObjectVector:
			{
				((CFoundObjectVectorEx*)data_ptr)->setValue((CFoundObjectVectorEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundPatMaxVector:
			{
				((CFoundPatMaxVectorEx*)data_ptr)->setValue((CFoundPatMaxVectorEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundBlob:
			{
				((CFoundBlobEx*)data_ptr)->setValue((CFoundBlobEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundBlobVector:
			{
				((CFoundBlobVectorEx*)data_ptr)->setValue((CFoundBlobVectorEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundEdge:
			{
				((CFoundEdgeEx*)data_ptr)->setValue((CFoundEdgeEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundEdgeVector:
			{
				((CFoundEdgeVectorEx*)data_ptr)->setValue((CFoundEdgeVectorEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundCircle:
			{
				((CFoundCircleEx*)data_ptr)->setValue((CFoundCircleEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundFlawVector:
			{
				((CFoundFlawVectorEx*)data_ptr)->setValue((CFoundFlawVectorEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundDefects:
			{
				((CFoundBeadDefectEx*)data_ptr)->setValue((CFoundBeadDefectEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundVarTable:
			{
				((CFoundVarTableEx*)data_ptr)->setValue((CFoundVarTableEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeFoundBarcodeVector:
			{
				((CFoundBarcodeVectorEx*)data_ptr)->setValue((CFoundBarcodeVectorEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeContours:
			{
				((CContoursEx*)data_ptr)->setValue((CContoursEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeShapes:
			{
				((CShapesEx*)data_ptr)->setValue((CShapesEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeVector2diVec:
			{
				((CVector2diVecEx*)data_ptr)->setValue((CVector2diVecEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeVector2dfVec:
			{
				((CVector2dfVecEx*)data_ptr)->setValue((CVector2dfVecEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeVector2ddVec:
			{
				((CVector2ddVecEx*)data_ptr)->setValue((CVector2ddVecEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeArc:
			{
				((CArcDrawEx*)data_ptr)->setValue((CArcDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeCircle:
			{
				((CCircleDrawEx*)data_ptr)->setValue((CCircleDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeCross:
			{
				((CCrossDrawEx*)data_ptr)->setValue((CCrossDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeLine:
			{
				((CLineDrawEx*)data_ptr)->setValue((CLineDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeRotateRect:
			{
				((CRotatedRectDrawEx*)data_ptr)->setValue((CRotatedRectDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeDistanceDraw:
			{
				((CDistanceDrawEx*)data_ptr)->setValue((CDistanceDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeDimensionDraw:
			{
				((CDimensionDrawEx*)data_ptr)->setValue((CDimensionDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeAngleDraw:
			{
				((CAngleDrawEx*)data_ptr)->setValue((CAngleDrawEx*)robj.m_buffer_ptr);
				break;
			}
			case kDataTypeStringDraw:
			{
				((CStringDrawEx*)data_ptr)->setValue((CStringDrawEx*)robj.m_buffer_ptr);
				break;
			}
			default:
			{
				printlog_lvs2(QString("CVariant operator= fail. type:%1").arg(m_data_type), LOG_SCOPE_TAG);
				break;
			}
		}
	}
	else
	{
		//output error message, if can't substitute
		printlog_lv1(QString("Can't substitute [%1]!=[%2]").arg(m_attr_name.c_str()).arg(robj.m_attr_name.c_str()));
	}
	return *this;
}

i32 CVariant::memSize()
{
	i32 len = 0;
	len += CPOBase::getStringMemSize(m_attr_name);
	len += sizeof(m_tag_type);

	len += sizeof(m_min_value);
	len += sizeof(m_max_value);

	len += sizeof(m_data_type);
	len += sizeof(m_data_size);
	if (m_buffer_ptr)
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:
			case kDataTypeInt8:
			case kDataTypeUInt8:
			{
				m_data_size = sizeof(i8);
				break;
			}
			case kDataTypeInt16:
			case kDataTypeUInt16:
			{
				m_data_size = sizeof(i16);
				break;
			}
			case kDataTypeBool32:
			case kDataTypeInt32:
			case kDataTypeUInt32:
			case kDataTypeFloat32:
			{
				m_data_size = sizeof(i32);
				break;
			}
			case kDataTypeInt64:
			case kDataTypeUInt64:
			case kDataTypeFloat64:
			{
				m_data_size = sizeof(i64);
				break;
			}
			case kDataTypeVector2di:
			case kDataTypeVector2df:
			{
				m_data_size = sizeof(vector2di);
				break;
			}
			case kDataTypeVector2dd:
			{
				m_data_size = sizeof(vector2dd);
				break;
			}
			case kDataTypeRectf:
			case kDataTypeRecti:
			{
				m_data_size = sizeof(Recti);
				break;
			}
			case kDataTypeString:
			{
				m_data_size = ((atstring*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeWString:
			{
				m_data_size = ((atwstring*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeInt8Vector:
			{
				m_data_size = ((ati8vector*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeInt32Vector:
			{
				m_data_size = ((ati32vector*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFloat32Vector:
			{
				m_data_size = ((atf32vector*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeStringVector:
			{
				m_data_size = ((atstrvector*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeImage:
			{
				m_data_size = ((atImg*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeArbitraryImage:
			{
				m_data_size = ((atArbitraryImg*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeRegion:
			{
				m_data_size = ((CRegionInfoEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeImgRunTable:
			{
				m_data_size = ((CImgRunTableEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeTransform:
			{
				m_data_size = ((CTransformEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundObject:
			{
				m_data_size = ((CFoundObjectEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundObjectVector:
			{
				m_data_size = ((CFoundObjectVectorEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundPatMaxVector:
			{
				m_data_size = ((CFoundPatMaxVectorEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundBlob:
			{
				m_data_size = ((CFoundBlobEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundBlobVector:
			{
				m_data_size = ((CFoundBlobVectorEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundEdge:
			{
				m_data_size = ((CFoundEdgeEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundEdgeVector:
			{
				m_data_size = ((CFoundEdgeVectorEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundCircle:
			{
				m_data_size = ((CFoundCircleEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundFlawVector:
			{
				m_data_size = ((CFoundFlawVectorEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundDefects:
			{
				m_data_size = ((CFoundBeadDefectEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundVarTable:
			{
				m_data_size = ((CFoundVarTableEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeFoundBarcodeVector:
			{
				m_data_size = ((CFoundBarcodeVectorEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeContours:
			{
				m_data_size = ((CContoursEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeShapes:
			{
				m_data_size = ((CShapesEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeVector2diVec:
			{
				m_data_size = ((CVector2diVecEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeVector2dfVec:
			{
				m_data_size = ((CVector2dfVecEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeVector2ddVec:
			{
				m_data_size = ((CVector2ddVecEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeArc:
			{
				m_data_size = ((CArcDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeCircle:
			{
				m_data_size = ((CCircleDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeCross:
			{
				m_data_size = ((CCrossDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeLine:
			{
				m_data_size = ((CLineDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeRotateRect:
			{
				m_data_size = ((CRotatedRectDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeDistanceDraw:
			{
				m_data_size = ((CDistanceDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeDimensionDraw:
			{
				m_data_size = ((CDimensionDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeAngleDraw:
			{
				m_data_size = ((CAngleDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			case kDataTypeStringDraw:
			{
				m_data_size = ((CStringDrawEx*)m_buffer_ptr)->memSize();
				break;
			}
			default:
			{
				m_data_size = 0;
				break;
			}
		}
	}

	len += m_data_size;
	return len;
}

void CVariant::memReadExpSkip(u8*& buffer_ptr, i32& buffer_size)
{
	i32 data_len;
	if (CPOBase::memRead(data_len, buffer_ptr, buffer_size))
	{
		buffer_ptr += data_len;
		buffer_size -= data_len;
	}
}

void CVariant::memReadExpAttr(i32 data_type, u8*& buffer_ptr, i32& buffer_size)
{
	i32 data_len = -1;
	void* data_ptr = NULL;
	CPOBase::memRead(data_len, buffer_ptr, buffer_size);
	if (!CPOBase::isCount(data_len))
	{
		printlog_lvs2(QString("TagDataLen invalid:%1").arg(data_len), LOG_SCOPE_TAG);
		return;
	}

	if (m_use_external)
	{
		if (m_data_type == data_type && m_buffer_ptr)
		{
			data_ptr = (u8*)m_buffer_ptr;
		}
	}
	else
	{
		data_ptr = initBuffer((DataType)data_type);
	}

	if (data_ptr)
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:
			{
				bool tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atbool*)data_ptr) = tmp;
				break;
			}
			case kDataTypeBool32:
			{
				i32 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((ati32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt8:
			{
				i8 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((ati8*)data_ptr)= tmp;
				break;
			}
			case kDataTypeInt16:
			{
				i16 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((ati16*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt32:
			{
				i32 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((ati32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt64:
			{
				i64 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((ati64*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt8:
			{
				u8 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atu8*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt16:
			{
				u16 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atu16*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt32:
			{
				u32 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atu32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt64:
			{
				u64 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atu64*)data_ptr) = tmp;
				break;
			}
			case kDataTypeFloat32:
			{
				f32 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atf32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeFloat64:
			{
				f64 tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				*((atf64*)data_ptr) = tmp;
				break;
			}
			case kDataTypeVector2di:
			{
				vector2di tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				((atVector2di*)data_ptr)->setValue(tmp);
				break;
			}
			case kDataTypeVector2df:
			{
				vector2df tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				((atVector2df*)data_ptr)->setValue(tmp);
				break;
			}
			case kDataTypeVector2dd:
			{
				vector2dd tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				((atVector2dd*)data_ptr)->setValue(tmp);
				break;
			}
			case kDataTypeRectf:
			{
				Rectf tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				((atRectf*)data_ptr)->setValue(tmp);
				break;
			}
			case kDataTypeRecti:
			{
				Recti tmp;
				CPOBase::memRead(tmp, buffer_ptr, buffer_size);
				((atRecti*)data_ptr)->setValue(tmp);
				break;
			}
			case kDataTypeString:
			{
				((atstring*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeWString:
			{
				((atwstring*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeInt8Vector:
			{
				((ati8vector*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeInt32Vector:
			{
				((ati32vector*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFloat32Vector:
			{
				((atf32vector*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeStringVector:
			{
				((atstrvector*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeImage:
			{
				((atImg*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeArbitraryImage:
			{
				((atArbitraryImg*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeRegion:
			{
				((CRegionInfoEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeImgRunTable:
			{
				((CImgRunTableEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeTransform:
			{
				((CTransformEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundObject:
			{
				((CFoundObjectEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundObjectVector:
			{
				((CFoundObjectVectorEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundPatMaxVector:
			{
				((CFoundPatMaxVectorEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundBlob:
			{
				((CFoundBlobEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundBlobVector:
			{
				((CFoundBlobVectorEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundEdge:
			{
				((CFoundEdgeEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundEdgeVector:
			{
				((CFoundEdgeVectorEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundCircle:
			{
				((CFoundCircleEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundFlawVector:
			{
				((CFoundFlawVectorEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundDefects:
			{
				((CFoundBeadDefectEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundVarTable:
			{
				((CFoundVarTableEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeFoundBarcodeVector:
			{
				((CFoundBarcodeVectorEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeContours:
			{
				((CContoursEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeShapes:
			{
				((CShapesEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeVector2diVec:
			{
				((CVector2diVecEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeVector2dfVec:
			{
				((CVector2dfVecEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeVector2ddVec:
			{
				((CVector2ddVecEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeArc:
			{
				((CArcDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeCircle:
			{
				((CCircleDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeCross:
			{
				((CCrossDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeLine:
			{
				((CLineDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeRotateRect:
			{
				((CRotatedRectDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeDistanceDraw:
			{
				((CDistanceDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeDimensionDraw:
			{
				((CDimensionDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeAngleDraw:
			{
				((CAngleDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			case kDataTypeStringDraw:
			{
				((CStringDrawEx*)data_ptr)->memRead(buffer_ptr, buffer_size);
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else
	{
		buffer_ptr += data_len;
		buffer_size -= data_len;
	}
}

i32 CVariant::memRead(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;

	CPOBase::memRead(buffer_ptr, buffer_size, m_attr_name);
	CPOBase::memRead(m_tag_type, buffer_ptr, buffer_size);

	CPOBase::memRead(m_min_value, buffer_ptr, buffer_size);
	CPOBase::memRead(m_max_value, buffer_ptr, buffer_size);

	CPOBase::memRead(m_data_type, buffer_ptr, buffer_size);
	memReadExpAttr(m_data_type, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CVariant::memWrite(u8*& buffer_ptr, i32& buffer_size)
{
	if (m_buffer_ptr == NULL)
	{
		return 0;
	}

	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(buffer_ptr, buffer_size, m_attr_name);
	CPOBase::memWrite(m_tag_type, buffer_ptr, buffer_size);

	CPOBase::memWrite(m_min_value, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_max_value, buffer_ptr, buffer_size);

	CPOBase::memWrite(m_data_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_data_size, buffer_ptr, buffer_size);
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			bool tmp = *((atbool*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeBool32:
		{
			i32 tmp = *((ati32*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeInt8:
		{
			i8 tmp = *((ati8*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeInt16:
		{
			i16 tmp = *((ati16*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeInt32:
		{
			i32 tmp = *((ati32*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeInt64:
		{
			i64 tmp = *((ati64*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeUInt8:
		{
			u8 tmp = *((atu8*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeUInt16:
		{
			u16 tmp = *((atu16*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeUInt32:
		{
			u32 tmp = *((atu32*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeUInt64:
		{
			u64 tmp = *((atu64*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFloat32:
		{
			f32 tmp = *((atf32*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFloat64:
		{
			f64 tmp = *((atf64*)m_buffer_ptr);
			CPOBase::memWrite(tmp, buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeVector2di:
		{
			((atVector2di*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeVector2df:
		{
			((atVector2df*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeVector2dd:
		{
			((atVector2dd*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeRectf:
		{
			((atRectf*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeRecti:
		{
			((atRecti*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeString:
		{
			((atstring*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeWString:
		{
			((atwstring*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeInt8Vector:
		{
			((ati8vector*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeInt32Vector:
		{
			((ati32vector*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFloat32Vector:
		{
			((atf32vector*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeStringVector:
		{
			((atstrvector*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeImage:
		{
			((atImg*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeArbitraryImage:
		{
			((atArbitraryImg*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeRegion:
		{
			((CRegionInfoEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeImgRunTable:
		{
			((CImgRunTableEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeTransform:
		{
			((CTransformEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundObject:
		{
			((CFoundObjectEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundObjectVector:
		{
			((CFoundObjectVectorEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundPatMaxVector:
		{
			((CFoundPatMaxVectorEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundBlob:
		{
			((CFoundBlobEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundBlobVector:
		{
			((CFoundBlobVectorEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundEdge:
		{
			((CFoundEdgeEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundEdgeVector:
		{
			((CFoundEdgeVectorEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundCircle:
		{
			((CFoundCircleEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundFlawVector:
		{
			((CFoundFlawVectorEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundDefects:
		{
			((CFoundBeadDefectEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundVarTable:
		{
			((CFoundVarTableEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeFoundBarcodeVector:
		{
			((CFoundBarcodeVectorEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeContours:
		{
			((CContoursEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeShapes:
		{
			((CShapesEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeVector2diVec:
		{
			((CVector2diVecEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeVector2dfVec:
		{
			((CVector2dfVecEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}	
		case kDataTypeVector2ddVec:
		{
			((CVector2ddVecEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeArc:
		{
			((CArcDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeCircle:
		{
			((CCircleDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeCross:
		{
			((CCrossDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeLine:
		{
			((CLineDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeRotateRect:
		{
			((CRotatedRectDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeDistanceDraw:
		{
			((CDistanceDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeDimensionDraw:
		{
			((CDimensionDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeAngleDraw:
		{
			((CAngleDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		case kDataTypeStringDraw:
		{
			((CStringDrawEx*)m_buffer_ptr)->memWrite(buffer_ptr, buffer_size);
			break;
		}
		default:
		{
			break;
		}
	}
	return buffer_ptr - buffer_pos;
}

void CVariant::fileReadExpAttr(i32 data_type, FILE* fp)
{
	i32 data_len;
	CPOBase::fileRead(data_len, fp);
	u64 next_pos = CPOBase::filePos(fp) + data_len;
	
	bool is_success = false;
	void* data_ptr = NULL;
	
	if (m_use_external)
	{
		if (m_data_type == data_type && m_buffer_ptr)
		{
			data_ptr = m_buffer_ptr;
		}
	}
	else
	{
		data_ptr = initBuffer((DataType)data_type);
	}

	if (data_ptr)
	{
		is_success = true;
		switch (m_data_type)
		{
			case kDataTypeBool8:
			{
				bool tmp;
				CPOBase::fileRead(tmp, fp);
				*((atbool*)data_ptr) = tmp;
				break;
			}
			case kDataTypeBool32:
			{
				i32 tmp;
				CPOBase::fileRead(tmp, fp);
				*((ati32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt8:
			{
				i8 tmp;
				CPOBase::fileRead(tmp, fp);
				*((ati8*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt16:
			{
				i16 tmp;
				CPOBase::fileRead(tmp, fp);
				*((ati16*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt32:
			{
				i32 tmp;
				CPOBase::fileRead(tmp, fp);
				*((ati32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeInt64:
			{
				i64 tmp;
				CPOBase::fileRead(tmp, fp);
				*((ati64*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt8:
			{
				u8 tmp;
				CPOBase::fileRead(tmp, fp);
				*((atu8*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt16:
			{
				u16 tmp;
				CPOBase::fileRead(tmp, fp);
				*((atu16*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt32:
			{
				u32 tmp;
				CPOBase::fileRead(tmp, fp);
				*((atu32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeUInt64:
			{
				u64 tmp;
				CPOBase::fileRead(tmp, fp);
				*((atu64*)data_ptr) = tmp;
				break;
			}
			case kDataTypeFloat32:
			{
				f32 tmp;
				CPOBase::fileRead(tmp, fp);
				*((atf32*)data_ptr) = tmp;
				break;
			}
			case kDataTypeFloat64:
			{
				f64 tmp;
				CPOBase::fileRead(tmp, fp);
				*((atf64*)data_ptr) = tmp;
				break;
			}
			case kDataTypeVector2di:
			{
				is_success = ((atVector2di*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeVector2df:
			{
				is_success = ((atVector2df*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeVector2dd:
			{
				is_success = ((atVector2dd*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeRectf:
			{
				is_success = ((atRectf*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeRecti:
			{
				is_success = ((atRecti*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeString:
			{
				is_success = ((atstring*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeWString:
			{
				is_success = ((atwstring*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeInt8Vector:
			{
				is_success = ((ati8vector*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeInt32Vector:
			{
				is_success = ((ati32vector*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFloat32Vector:
			{
				is_success = ((atf32vector*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeStringVector:
			{
				is_success = ((atstrvector*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeImage:
			{
				is_success = ((atImg*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeArbitraryImage:
			{
				is_success = ((atArbitraryImg*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeRegion:
			{
				is_success = ((CRegionInfoEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeImgRunTable:
			{
				is_success = ((CImgRunTableEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeTransform:
			{
				is_success = ((CTransformEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundObject:
			{
				is_success = ((CFoundObjectEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundObjectVector:
			{
				is_success = ((CFoundObjectVectorEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundPatMaxVector:
			{
				is_success = ((CFoundPatMaxVectorEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundBlob:
			{
				is_success = ((CFoundBlobEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundBlobVector:
			{
				is_success = ((CFoundBlobVectorEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundEdge:
			{
				is_success = ((CFoundEdgeEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundEdgeVector:
			{
				is_success = ((CFoundEdgeVectorEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundCircle:
			{
				is_success = ((CFoundCircleEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundFlawVector:
			{
				is_success = ((CFoundFlawVectorEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundDefects:
			{
				is_success = ((CFoundBeadDefectEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundVarTable:
			{
				is_success = ((CFoundVarTableEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeFoundBarcodeVector:
			{
				is_success = ((CFoundBarcodeVectorEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeContours:
			{
				is_success = ((CContoursEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeShapes:
			{
				is_success = ((CShapesEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeVector2diVec:
			{
				is_success = ((CVector2diVecEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeVector2dfVec:
			{
				is_success = ((CVector2dfVecEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeVector2ddVec:
			{
				is_success = ((CVector2ddVecEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeArc:
			{
				is_success = ((CArcDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeCircle:
			{
				is_success = ((CCircleDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeCross:
			{
				is_success = ((CCrossDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeLine:
			{
				is_success = ((CLineDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeRotateRect:
			{
				is_success = ((CRotatedRectDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeDistanceDraw:
			{
				is_success = ((CDistanceDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeDimensionDraw:
			{
				is_success = ((CDimensionDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeAngleDraw:
			{
				is_success = ((CAngleDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			case kDataTypeStringDraw:
			{
				is_success = ((CStringDrawEx*)data_ptr)->fileRead(fp);
				break;
			}
			default:
			{
				is_success = false;
				break;
			}
		}
	}
	
	if (!is_success)
	{
		CPOBase::fileSeek(fp, next_pos);
		printlog_lvs2(QString("FileReadExpAttr fail, type:%1, len:%2").arg(data_type).arg(data_len), LOG_SCOPE_TAG);
	}
}

bool CVariant::fileRead(FILE* fp)
{
	if (!CPOBase::fileSignRead(fp, PO_SIGN_CODE))
	{
		return false;
	}

	CPOBase::fileRead(m_attr_name, fp);
	CPOBase::fileRead(m_tag_type, fp);

	CPOBase::fileRead(m_min_value, fp);
	CPOBase::fileRead(m_max_value, fp);

	CPOBase::fileRead(m_data_type, fp);
	fileReadExpAttr(m_data_type, fp);
	return CPOBase::fileSignRead(fp, PO_SIGN_ENDCODE);
}

bool CVariant::fileWrite(FILE* fp)
{
	if (m_buffer_ptr == NULL)
	{
		return false;
	}

	bool is_success = true;
	CPOBase::fileSignWrite(fp, PO_SIGN_CODE);
	CPOBase::fileWrite(m_attr_name, fp);
	CPOBase::fileWrite(m_tag_type, fp);
	
	CPOBase::fileWrite(m_min_value, fp);
	CPOBase::fileWrite(m_max_value, fp);
	
	CPOBase::fileWrite(m_data_type, fp);
	CPOBase::fileWrite(m_data_size, fp);
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			CPOBase::fileWrite(*((atbool*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeBool32:
		{
			CPOBase::fileWrite(*((ati32*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeInt8:
		{
			CPOBase::fileWrite(*((ati8*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeInt16:
		{
			CPOBase::fileWrite(*((ati16*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeInt32:
		{
			CPOBase::fileWrite(*((ati32*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeInt64:
		{
			CPOBase::fileWrite(*((ati64*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeUInt8:
		{
			CPOBase::fileWrite(*((atu8*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeUInt16:
		{
			CPOBase::fileWrite(*((atu16*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeUInt32:
		{
			CPOBase::fileWrite(*((atu32*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeUInt64:
		{
			CPOBase::fileWrite(*((atu64*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeFloat32:
		{
			CPOBase::fileWrite(*((atf32*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeFloat64:
		{
			CPOBase::fileWrite(*((atf64*)m_buffer_ptr), fp);
			break;
		}
		case kDataTypeVector2di:
		{
			is_success = ((atVector2di*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeVector2df:
		{
			is_success = ((atVector2df*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeVector2dd:
		{
			is_success = ((atVector2dd*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeRectf:
		{
			is_success = ((atRectf*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeRecti:
		{
			is_success = ((atRecti*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeString:
		{
			is_success = ((atstring*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeWString:
		{
			is_success = ((atwstring*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeInt8Vector:
		{
			is_success = ((ati8vector*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeInt32Vector:
		{
			is_success = ((ati32vector*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFloat32Vector:
		{
			is_success = ((atf32vector*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeStringVector:
		{
			is_success = ((atstrvector*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeImage:
		{
			is_success = ((atImg*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeArbitraryImage:
		{
			is_success = ((atArbitraryImg*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeRegion:
		{
			is_success = ((CRegionInfoEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeImgRunTable:
		{
			is_success = ((CImgRunTableEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeTransform:
		{
			is_success = ((CTransformEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundObject:
		{
			is_success = ((CFoundObjectEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundObjectVector:
		{
			is_success = ((CFoundObjectVectorEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundPatMaxVector:
		{
			is_success = ((CFoundPatMaxVectorEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundBlob:
		{
			is_success = ((CFoundBlobEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundBlobVector:
		{
			is_success = ((CFoundBlobVectorEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundEdge:
		{
			is_success = ((CFoundEdgeEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundEdgeVector:
		{
			is_success = ((CFoundEdgeVectorEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundCircle:
		{
			is_success = ((CFoundCircleEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundFlawVector:
		{
			is_success = ((CFoundFlawVectorEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundDefects:
		{
			is_success = ((CFoundBeadDefectEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundVarTable:
		{
			is_success = ((CFoundVarTableEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeFoundBarcodeVector:
		{
			is_success = ((CFoundBarcodeVectorEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeContours:
		{
			is_success = ((CContoursEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeShapes:
		{
			is_success = ((CShapesEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeVector2diVec:
		{
			is_success = ((CVector2diVecEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeVector2dfVec:
		{
			is_success = ((CVector2dfVecEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeVector2ddVec:
		{
			is_success = ((CVector2ddVecEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeArc:
		{
			is_success = ((CArcDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeCircle:
		{
			is_success = ((CCircleDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeCross:
		{
			is_success = ((CCrossDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeLine:
		{
			is_success = ((CLineDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeRotateRect:
		{
			is_success = ((CRotatedRectDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeDistanceDraw:
		{
			is_success = ((CDistanceDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeDimensionDraw:
		{
			is_success = ((CDimensionDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeAngleDraw:
		{
			is_success = ((CAngleDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		case kDataTypeStringDraw:
		{
			is_success = ((CStringDrawEx*)m_buffer_ptr)->fileWrite(fp);
			break;
		}
		default:
		{
			is_success = false;
			break;
		}
	}

	if (!is_success)
	{
		printlog_lvs2(QString("Variant FileWrite fail, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}

	CPOBase::fileSignWrite(fp, PO_SIGN_ENDCODE);
	return is_success;
}

bool CVariant::isAcceptable(i32 mode)
{
	if (mode == kTagTransPass)
	{
		return true;
	}
	
	i32 cur_type = m_tag_type & kTagTypeRW;
	i32 chk_type = mode & kTagTypeRW;
	i32 chk_sub_type = mode & kTagTypeSHD;
	i32 cur_sub_type = m_tag_type & kTagTypeSHD;	//kTagTypeDraw | kTagTypeStudy | kTagTypeHeavy,

	return CPOBase::bitCheck(cur_type, chk_type) && CPOBase::bitCheck(chk_sub_type, cur_sub_type);
}

bool CVariant::isNumeric()
{
	return CPOBase::checkRange(m_data_type, kDataTypeInt8, kDataTypeFloat64);
}

bool CVariant::isString()
{
	return CPOBase::checkRange(m_data_type, kDataTypeString, kDataTypeWString);
}

bool CVariant::isMixType()
{
	return CPOBase::checkRange(m_data_type, kDataTypeVector2di, kDataTypeCount);
}

bool CVariant::isPoint2dType()
{
	return CPOBase::checkRange(m_data_type, kDataTypeVector2di, kDataTypeVector2dd);
}

bool CVariant::isRectType()
{
	return CPOBase::checkRange(m_data_type, kDataTypeRecti, kDataTypeRectd);
}

bool CVariant::setValuebool(bool value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != value);
			*((atbool*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuebool failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValuei8(i8 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != value);
			*((ati8*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuei8 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}
	
	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValuei16(i16 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value; 
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != value);
			*((ati16*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuei16 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValuei32(i32 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value; 
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != value);
			*((ati32*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuei32 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValuei64(i64 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed  = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != value);
			*((ati64*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuei64 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueu8(u8 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != value);
			*((atu8*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value; 
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueu8 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueu16(u16 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != value);
			*((atu16*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueu16 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueu32(u32 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != value);
			*((atu32*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueu32 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueu64(u64 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != value);
			*((atu64*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueu64 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValuef32(f32 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != value);
			*((atf32*)m_buffer_ptr) = value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != (f64)value);
			*((atf64*)m_buffer_ptr) = (f64)value;
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuef32 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValuef64(f64 value)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeBool8:
		{
			is_changed = (*((atbool*)m_buffer_ptr) != (value != 0));
			*((atbool*)m_buffer_ptr) = (value != 0);
			break;
		}
		case kDataTypeBool32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt8:
		{
			is_changed = (*((ati8*)m_buffer_ptr) != (i8)value);
			*((ati8*)m_buffer_ptr) = (i8)value;
			break;
		}
		case kDataTypeInt16:
		{
			is_changed = (*((ati16*)m_buffer_ptr) != (i16)value);
			*((ati16*)m_buffer_ptr) = (i16)value;
			break;
		}
		case kDataTypeInt32:
		{
			is_changed = (*((ati32*)m_buffer_ptr) != (i32)value);
			*((ati32*)m_buffer_ptr) = (i32)value;
			break;
		}
		case kDataTypeInt64:
		{
			is_changed = (*((ati64*)m_buffer_ptr) != (i64)value);
			*((ati64*)m_buffer_ptr) = (i64)value;
			break;
		}
		case kDataTypeUInt8:
		{
			is_changed = (*((atu8*)m_buffer_ptr) != (u8)value);
			*((atu8*)m_buffer_ptr) = (u8)value;
			break;
		}
		case kDataTypeUInt16:
		{
			is_changed = (*((atu16*)m_buffer_ptr) != (u16)value);
			*((atu16*)m_buffer_ptr) = (u16)value;
			break;
		}
		case kDataTypeUInt32:
		{
			is_changed = (*((atu32*)m_buffer_ptr) != (u32)value);
			*((atu32*)m_buffer_ptr) = (u32)value;
			break;
		}
		case kDataTypeUInt64:
		{
			is_changed = (*((atu64*)m_buffer_ptr) != (u64)value);
			*((atu64*)m_buffer_ptr) = (u64)value;
			break;
		}
		case kDataTypeFloat32:
		{
			is_changed = (*((atf32*)m_buffer_ptr) != (f32)value);
			*((atf32*)m_buffer_ptr) = (f32)value;
			break;
		}
		case kDataTypeFloat64:
		{
			is_changed = (*((atf64*)m_buffer_ptr) != value);
			*((atf64*)m_buffer_ptr) = value; break;
		}
		default:
		{
			printlog_lvs2(QString("SetValuef64 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueVector2di(const vector2di& point)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeVector2di:
		{
			is_changed = !((atVector2di*)m_buffer_ptr)->isEqual(point);
			((atVector2di*)m_buffer_ptr)->setValue(point);
			break;
		}
		case kDataTypeVector2df:
		{
			is_changed = !((atVector2df*)m_buffer_ptr)->isEqual(point);
			((atVector2df*)m_buffer_ptr)->setValue(point);
			break;
		}
		case kDataTypeVector2dd:
		{
			is_changed = !((atVector2dd*)m_buffer_ptr)->isEqual(point);
			((atVector2dd*)m_buffer_ptr)->setValue(point);
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueVector2di failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueVector2df(const vector2df& point)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeVector2di:
		{
			is_changed = !((atVector2di*)m_buffer_ptr)->isEqual(point);
			((atVector2di*)m_buffer_ptr)->setValue(vector2di(point.x, point.y));
			break;
		}
		case kDataTypeVector2df:
		{
			is_changed = !((atVector2df*)m_buffer_ptr)->isEqual(point);
			((atVector2df*)m_buffer_ptr)->setValue(point);
			break;
		}
		case kDataTypeVector2dd:
		{
			is_changed = !((atVector2dd*)m_buffer_ptr)->isEqual(point);
			((atVector2dd*)m_buffer_ptr)->setValue(vector2dd(point.x, point.y));
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueVector2df failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueVector2dd(const vector2dd& point)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeVector2di:
		{
			is_changed = !((atVector2di*)m_buffer_ptr)->isEqual(point);
			((atVector2di*)m_buffer_ptr)->setValue(vector2di(point.x, point.y));
			break;
		}
		case kDataTypeVector2df:
		{
			is_changed = !((atVector2df*)m_buffer_ptr)->isEqual(point);
			((atVector2df*)m_buffer_ptr)->setValue(vector2df(point.x, point.y));
			break;
		}
		case kDataTypeVector2dd:
		{
			is_changed = !((atVector2dd*)m_buffer_ptr)->isEqual(point);
			((atVector2dd*)m_buffer_ptr)->setValue(point);
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueVector2dd failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueRecti(const Recti& rt)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeRecti:
		{
			is_changed = !((atRecti*)m_buffer_ptr)->isEqual(rt);
			((atRecti*)m_buffer_ptr)->setValue(rt);
			break;
		}
		case kDataTypeRectf:
		{
			is_changed = !((atRectf*)m_buffer_ptr)->isEqual(rt);
			((atRectf*)m_buffer_ptr)->setValue(Rectf(rt));
			break;
		}
		case kDataTypeRectd:
		{
			is_changed = !((atRectd*)m_buffer_ptr)->isEqual(rt);
			((atRectd*)m_buffer_ptr)->setValue(Rectd(rt));
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueRecti failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueRectf(const Rectf& rt)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeRecti:
		{
			is_changed = !((atRecti*)m_buffer_ptr)->isEqual(rt);
			((atRecti*)m_buffer_ptr)->setValue(Recti(rt));
			break;
		}
		case kDataTypeRectf:
		{
			is_changed = !((atRectf*)m_buffer_ptr)->isEqual(rt);
			((atRectf*)m_buffer_ptr)->setValue(rt);
			break;
		}
		case kDataTypeRectd:
		{
			is_changed = !((atRectd*)m_buffer_ptr)->isEqual(rt);
			((atRectd*)m_buffer_ptr)->setValue(Rectd(rt));
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueRectf failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueRectd(const Rectd& rt)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeRecti:
		{
			is_changed = !((atRecti*)m_buffer_ptr)->isEqual(rt);
			((atRecti*)m_buffer_ptr)->setValue(Recti(rt));
			break;
		}
		case kDataTypeRectf:
		{
			is_changed = !((atRectf*)m_buffer_ptr)->isEqual(rt);
			((atRectf*)m_buffer_ptr)->setValue(Rectf(rt));
			break;
		}
		case kDataTypeRectd:
		{
			is_changed = !((atRectd*)m_buffer_ptr)->isEqual(rt);
			((atRectd*)m_buffer_ptr)->setValue(rt);
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueRectd failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueString(const postring& str)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeString:
		{
			is_changed = !((atstring*)m_buffer_ptr)->isEqual(str);
			((atstring*)m_buffer_ptr)->setValue(str);
			break;
		}
		case kDataTypeWString:
		{
			powstring tmp_str = CPOBase::stringToWString(str);
			is_changed = !((atwstring*)m_buffer_ptr)->isEqual(tmp_str);
			((atwstring*)m_buffer_ptr)->setValue(tmp_str);
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueString failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueWString(const powstring& str)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	bool is_changed = false;
	switch (m_data_type)
	{
		case kDataTypeString:
		{
			postring tmp_str = CPOBase::wstringToString(str);
			is_changed = !((atstring*)m_buffer_ptr)->isEqual(tmp_str);
			((atstring*)m_buffer_ptr)->setValue(tmp_str);
			break;
		}
		case kDataTypeWString:
		{
			is_changed = !((atwstring*)m_buffer_ptr)->isEqual(str);
			((atwstring*)m_buffer_ptr)->setValue(str);
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueWString failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	if (is_changed)
	{
		limitValue();
	}
	return is_changed;
}

bool CVariant::setValueImg(const Img& img)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeWrite))
	{
		return false;
	}

	switch (m_data_type)
	{
		case kDataTypeImage:
		{
			((atImg*)m_buffer_ptr)->setValue(img);
			break;
		}
		default:
		{
			printlog_lvs2(QString("SetValueImg failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
			return false;
		}
	}

	limitValue();
	return true;
}

bool CVariant::toValuebool()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return false;
	}

	switch (m_data_type)
	{
		case kDataTypeBool8:			return *((atbool*)m_buffer_ptr);
		case kDataTypeBool32:			return *((ati32*)m_buffer_ptr);
		case kDataTypeInt8:				return *((ati8*)m_buffer_ptr) != 0;
		case kDataTypeInt16:			return *((ati16*)m_buffer_ptr) != 0;
		case kDataTypeInt32:			return *((ati32*)m_buffer_ptr) != 0;
		case kDataTypeInt64:			return *((ati64*)m_buffer_ptr) != 0;
		case kDataTypeUInt8:			return *((atu8*)m_buffer_ptr) != 0;
		case kDataTypeUInt16:			return *((atu16*)m_buffer_ptr) != 0;
		case kDataTypeUInt32:			return *((atu32*)m_buffer_ptr) != 0;
		case kDataTypeUInt64:			return *((atu64*)m_buffer_ptr) != 0;
		case kDataTypeFloat32:			return *((atf32*)m_buffer_ptr) != 0;
		case kDataTypeFloat64:			return *((atf64*)m_buffer_ptr) != 0;
		case kDataTypeString:			return ((atstring*)m_buffer_ptr)->size() != 0;
		case kDataTypeWString:			return ((atwstring*)m_buffer_ptr)->size() != 0;
		case kDataTypeInt8Vector:		return ((ati8vector*)m_buffer_ptr)->size() != 0;
		case kDataTypeInt32Vector:		return ((ati32vector*)m_buffer_ptr)->size() != 0;
		case kDataTypeFloat32Vector:	return ((atf32vector*)m_buffer_ptr)->size() != 0;
		case kDataTypeStringVector:		return ((atstrvector*)m_buffer_ptr)->size() != 0;
		case kDataTypeImage:			return ((atImg*)m_buffer_ptr)->isValid();
		case kDataTypeArbitraryImage:	return ((atArbitraryImg*)m_buffer_ptr)->isValid();
		case kDataTypeRegion:			return ((CRegionInfoEx*)m_buffer_ptr)->getPlusRegionCount() != 0;
	}

	printlog_lvs2(QString("ToValuebool failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return false;
}

i8 CVariant::toValuei8()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:			return (i8)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:			return (i8)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:				return (i8)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:			return (i8)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:			return (i8)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:			return (i8)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:			return (i8)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:			return (i8)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:			return (i8)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:			return (i8)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:			return (i8)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:			return (i8)*((atf64*)m_buffer_ptr);
			case kDataTypeString:			{ i32 tmp = 0; CPOBase::stoi(((atstring*)m_buffer_ptr)->getValue(), tmp); return (i8)tmp; }
			case kDataTypeWString:			{ i32 tmp = 0; CPOBase::stoi(((atwstring*)m_buffer_ptr)->getValue(), tmp); return (i8)tmp; }
		}

		printlog_lvs2(QString("ToValuei8 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValuei8 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

i16 CVariant::toValuei16()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:			return (i16)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:			return (i16)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:				return (i16)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:			return (i16)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:			return (i16)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:			return (i16)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:			return (i16)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:			return (i16)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:			return (i16)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:			return (i16)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:			return (i16)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:			return (i16)*((atf64*)m_buffer_ptr);
			case kDataTypeString:			{ i32 tmp = 0; CPOBase::stoi(((atstring*)m_buffer_ptr)->getValue(), tmp); return (i16)tmp; }
			case kDataTypeWString:			{ i32 tmp = 0; CPOBase::stoi(((atwstring*)m_buffer_ptr)->getValue(), tmp); return (i16)tmp; }
		}

		printlog_lvs2(QString("ToValuei16 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValuei16 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

i32 CVariant::toValuei32()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (i32)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (i32)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (i32)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (i32)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (i32)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (i32)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (i32)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (i32)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (i32)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (i32)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (i32)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (i32)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ i32 tmp = 0; CPOBase::stoi(((atstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
			case kDataTypeWString:		{ i32 tmp = 0; CPOBase::stoi(((atwstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
		}

		printlog_lvs2(QString("ToValuei32 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValuei32 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

i64 CVariant::toValuei64()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (i64)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (i64)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (i64)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (i64)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (i64)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (i64)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (i64)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (i64)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (i64)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (i64)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (i64)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (i64)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ i64 tmp = 0; CPOBase::stoll(((atstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
			case kDataTypeWString:		{ i64 tmp = 0; CPOBase::stoll(((atwstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
		}

		printlog_lvs2(QString("ToValuei64 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValuei64 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

u8 CVariant::toValueu8()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (u8)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (u8)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (u8)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (u8)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (u8)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (u8)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (u8)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (u8)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (u8)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (u8)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (u8)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (u8)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ u32 tmp = 0; CPOBase::stoul(((atstring*)m_buffer_ptr)->getValue(), tmp); return (u8)tmp; }
			case kDataTypeWString:		{ u32 tmp = 0; CPOBase::stoul(((atwstring*)m_buffer_ptr)->getValue(), tmp); return (u8)tmp; }
		}

		printlog_lvs2(QString("ToValueu8 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValueu8 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

u16 CVariant::toValueu16()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (u16)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (u16)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (u16)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (u16)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (u16)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (u16)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (u16)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (u16)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (u16)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (u16)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (u16)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (u16)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ u32 tmp = 0; CPOBase::stoul(((atstring*)m_buffer_ptr)->getValue(), tmp); return (u16)tmp; }
			case kDataTypeWString:		{ u32 tmp = 0; CPOBase::stoul(((atwstring*)m_buffer_ptr)->getValue(), tmp); return (u16)tmp; }
		}

		printlog_lvs2(QString("ToValueu16 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValueu16 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

u32 CVariant::toValueu32()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (u32)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (u32)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (u32)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (u32)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (u32)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (u32)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (u32)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (u32)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (u32)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (u32)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (u32)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (u32)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ u32 tmp = 0; CPOBase::stoul(((atstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
			case kDataTypeWString:		{ u32 tmp = 0; CPOBase::stoul(((atwstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
		}

		printlog_lvs2(QString("ToValueu32 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValueu32 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

u64 CVariant::toValueu64()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (u64)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (u64)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (u64)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (u64)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (u64)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (u64)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (u64)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (u64)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (u64)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (u64)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (u64)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (u64)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ u64 tmp = 0; CPOBase::stoull(((atstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
			case kDataTypeWString:		{ u64 tmp = 0; CPOBase::stoull(((atwstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
		}

		printlog_lvs2(QString("ToValueu64 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValueu64 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

f32 CVariant::toValuef32()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (f32)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (f32)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (f32)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (f32)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (f32)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (f32)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (f32)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (f32)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (f32)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (f32)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (f32)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (f32)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ f32 tmp = 0; CPOBase::stof(((atstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
			case kDataTypeWString:		{ f32 tmp = 0; CPOBase::stof(((atwstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
		}

		printlog_lvs2(QString("ToValuef32 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValuef32 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

f64 CVariant::toValuef64()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return 0;
	}

	try
	{
		switch (m_data_type)
		{
			case kDataTypeBool8:		return (f64)*((atbool*)m_buffer_ptr);
			case kDataTypeBool32:		return (f64)*((ati32*)m_buffer_ptr);
			case kDataTypeInt8:			return (f64)*((ati8*)m_buffer_ptr);
			case kDataTypeInt16:		return (f64)*((ati16*)m_buffer_ptr);
			case kDataTypeInt32:		return (f64)*((ati32*)m_buffer_ptr);
			case kDataTypeInt64:		return (f64)*((ati64*)m_buffer_ptr);
			case kDataTypeUInt8:		return (f64)*((atu8*)m_buffer_ptr);
			case kDataTypeUInt16:		return (f64)*((atu16*)m_buffer_ptr);
			case kDataTypeUInt32:		return (f64)*((atu32*)m_buffer_ptr);
			case kDataTypeUInt64:		return (f64)*((atu64*)m_buffer_ptr);
			case kDataTypeFloat32:		return (f64)*((atf32*)m_buffer_ptr);
			case kDataTypeFloat64:		return (f64)*((atf64*)m_buffer_ptr);
			case kDataTypeString:		{ f64 tmp = 0; CPOBase::stod(((atstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
			case kDataTypeWString:		{ f64 tmp = 0; CPOBase::stod(((atwstring*)m_buffer_ptr)->getValue(), tmp); return tmp; }
		}

		printlog_lvs2(QString("ToValuef64 failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	catch (...)
	{
		printlog_lvs2(QString("ToValuef64 exception, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	}
	return 0;
}

postring CVariant::toString(i32 decimals)
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return postring();
	}

	switch (m_data_type)
	{
		case kDataTypeBool8:		return std::to_string((i32)*((atbool*)m_buffer_ptr));
		case kDataTypeBool32:		return std::to_string((i32)*((ati32*)m_buffer_ptr));
		case kDataTypeInt8:			return std::to_string((i8)*((ati8*)m_buffer_ptr));
		case kDataTypeInt16:		return std::to_string((i16)*((ati16*)m_buffer_ptr));
		case kDataTypeInt32:		return std::to_string((i32)*((ati32*)m_buffer_ptr));
		case kDataTypeInt64:		return std::to_string((i64)*((ati64*)m_buffer_ptr));
		case kDataTypeUInt8:		return std::to_string((u8)*((atu8*)m_buffer_ptr));
		case kDataTypeUInt16:		return std::to_string((u16)*((atu16*)m_buffer_ptr));
		case kDataTypeUInt32:		return std::to_string((u32)*((atu32*)m_buffer_ptr));
		case kDataTypeUInt64:		return std::to_string((u64)*((atu64*)m_buffer_ptr));
		case kDataTypeString:		return ((atstring*)m_buffer_ptr)->getValue();
		case kDataTypeWString:		return CPOBase::wstringToString(((atwstring*)m_buffer_ptr)->getValue());
		case kDataTypeFloat32:
		{
			postring str;
			if (!CPOBase::isCount(decimals))
			{
				str = std::to_string(*((f32*)m_buffer_ptr));
			}
			else
			{
				std::stringstream ss;
				ss << std::fixed << std::setprecision(decimals) << (*((f32*)m_buffer_ptr));
				str = ss.str();
			}
			return str;
		}
		case kDataTypeFloat64:
		{
			postring str;
			if (!CPOBase::isCount(decimals))
			{
				str = std::to_string(*((f64*)m_buffer_ptr));
			}
			else
			{
				std::stringstream ss;
				ss << std::fixed << std::setprecision(decimals) << (*((f64*)m_buffer_ptr));
				str = ss.str();
			}
			return str;
		}

		case kDataTypeVector2dd:
		{
			postring str_x, str_y;
			atVector2dd *avec = (atVector2dd *)m_buffer_ptr;
			vector2dd vec = avec->getValue();
			
			if (!CPOBase::isCount(decimals))
			{
				str_x = std::to_string(vec.x);
				str_y = std::to_string(vec.y);
			}
			else
			{
				std::stringstream ss;
				ss << std::fixed << std::setprecision(decimals) << vec.x;
				str_x = ss.str();

				ss.str(std::string());
				ss.clear();
				ss << std::fixed << std::setprecision(decimals) << vec.y;
				str_y = ss.str();
			}
			return "(" + str_x + ", " + str_y + ")";
		}
		case kDataTypeVector2df:
		{
			postring str_x, str_y;
			atVector2df *avec = (atVector2df*)m_buffer_ptr;
			vector2df vec = avec->getValue();

			if (!CPOBase::isCount(decimals))
			{
				str_x = std::to_string(vec.x);
				str_y = std::to_string(vec.y);
			}
			else
			{
				std::stringstream ss;
				ss << std::fixed << std::setprecision(decimals) << vec.x;
				str_x = ss.str();

				ss.str(std::string());
				ss.clear();
				ss << std::fixed << std::setprecision(decimals) << vec.y;
				str_y = ss.str();
			}
			return "(" + str_x + ", " + str_y + ")";
		}
		case kDataTypeVector2di:
		{
			atVector2di *avec = (atVector2di *)m_buffer_ptr;
			vector2di vec = avec->getValue();
			postring str_x = std::to_string(vec.x);
			postring str_y = std::to_string(vec.y);

			return "(" + str_x + ", " + str_y + ")";
		}
	}

	printlog_lvs2(QString("ToValueString failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return postring();
}

powstring CVariant::toWString()
{
	if (!m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return powstring();
	}

	switch (m_data_type)
	{
		case kDataTypeBool8:		return std::to_wstring((i32)*((atbool*)m_buffer_ptr));
		case kDataTypeBool32:		return std::to_wstring((i32)*((ati32*)m_buffer_ptr));
		case kDataTypeInt8:			return std::to_wstring((i8)*((ati8*)m_buffer_ptr));
		case kDataTypeInt16:		return std::to_wstring((i16)*((ati16*)m_buffer_ptr));
		case kDataTypeInt32:		return std::to_wstring((i32)*((ati32*)m_buffer_ptr));
		case kDataTypeInt64:		return std::to_wstring((i64)*((ati64*)m_buffer_ptr));
		case kDataTypeUInt8:		return std::to_wstring((u8)*((atu8*)m_buffer_ptr));
		case kDataTypeUInt16:		return std::to_wstring((u16)*((atu16*)m_buffer_ptr));
		case kDataTypeUInt32:		return std::to_wstring((u32)*((atu32*)m_buffer_ptr));
		case kDataTypeUInt64:		return std::to_wstring((u64)*((atu64*)m_buffer_ptr));
		case kDataTypeFloat32:		return std::to_wstring((f32)*((atf32*)m_buffer_ptr));
		case kDataTypeFloat64:		return std::to_wstring((f64)*((atf64*)m_buffer_ptr));
		case kDataTypeString:		return CPOBase::stringToWString(((atstring*)m_buffer_ptr)->getValue());
		case kDataTypeWString:		return ((atwstring*)m_buffer_ptr)->getValue();
	}

	printlog_lvs2(QString("ToValueWString failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return powstring();
}

vector2di CVariant::toVector2di()
{
	if (!isPoint2dType() || !m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return vector2di();
	}

	switch (m_data_type)
	{
		case kDataTypeVector2di:
		{
			return ((atVector2di*)m_buffer_ptr)->getValue();
		}
		case kDataTypeVector2df:
		{
			vector2df tmp = ((atVector2df*)m_buffer_ptr)->getValue();
			return vector2di(tmp.x, tmp.y);
		}
		case kDataTypeVector2dd:
		{
			vector2dd tmp = ((atVector2dd*)m_buffer_ptr)->getValue();
			return vector2di(tmp.x, tmp.y);
		}
	}

	printlog_lvs2(QString("ToValueVector2di failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return vector2di();
}

vector2df CVariant::toVector2df()
{
	if (!isPoint2dType() || !m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return vector2df();
	}

	switch (m_data_type)
	{
		case kDataTypeVector2di:
		{
			vector2di tmp = ((atVector2di*)m_buffer_ptr)->getValue();
			return vector2df(tmp.x, tmp.y);
		}
		case kDataTypeVector2df:
		{
			return ((atVector2df*)m_buffer_ptr)->getValue();
		}
		case kDataTypeVector2dd:
		{
			vector2dd tmp = ((atVector2dd*)m_buffer_ptr)->getValue();
			return vector2df(tmp.x, tmp.y);
		}
	}

	printlog_lvs2(QString("ToValueVector2df failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return vector2df();
}

vector2dd CVariant::toVector2dd()
{
	if (!isPoint2dType() || !m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return vector2dd();
	}

	switch (m_data_type)
	{
		case kDataTypeVector2di:
		{
			vector2di tmp = ((atVector2di*)m_buffer_ptr)->getValue();
			return vector2dd(tmp.x, tmp.y);
		}
		case kDataTypeVector2df:
		{
			vector2df tmp = ((atVector2df*)m_buffer_ptr)->getValue();
			return vector2dd(tmp.x, tmp.y);
		}
		case kDataTypeVector2dd:
		{
			return ((atVector2dd*)m_buffer_ptr)->getValue();
		}
	}

	printlog_lvs2(QString("ToValueVector2dd failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return vector2dd();
}

Recti CVariant::toRecti()
{
	if (!isRectType() || !m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return Recti();
	}

	switch (m_data_type)
	{
		case kDataTypeRecti:
		{
			return ((atRecti*)m_buffer_ptr)->getValue();
		}
		case kDataTypeRectf:
		{
			Rectf tmp = ((atRectf*)m_buffer_ptr)->getValue();
			return Recti(tmp);
		}
		case kDataTypeRectd:
		{
			Rectd tmp = ((atRectd*)m_buffer_ptr)->getValue();
			return Recti(tmp);
		}
	}

	printlog_lvs2(QString("ToValueRecti failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return Recti();
}

Rectf CVariant::toRectf()
{
	if (!isRectType() || !m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return Rectf();
	}

	switch (m_data_type)
	{
		case kDataTypeRecti:
		{
			Recti tmp = ((atRecti*)m_buffer_ptr)->getValue();
			return Rectf(tmp);
		}
		case kDataTypeRectf:
		{
			return ((atRectf*)m_buffer_ptr)->getValue();
		}
		case kDataTypeRectd:
		{
			Rectd tmp = ((atRectd*)m_buffer_ptr)->getValue();
			return Rectf(tmp);
		}
	}

	printlog_lvs2(QString("ToValueRectf failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return Rectf();
}

Rectd CVariant::toRectd()
{
	if (!isRectType() || !m_buffer_ptr || !CPOBase::bitCheck(m_tag_type, kTagTypeRead))
	{
		return Rectd();
	}

	switch (m_data_type)
	{
		case kDataTypeRecti:
		{
			Recti tmp = ((atRecti*)m_buffer_ptr)->getValue();
			return Rectd(tmp);
		}
		case kDataTypeRectf:
		{
			Rectf tmp = ((atRectf*)m_buffer_ptr)->getValue();
			return Rectd(tmp);
		}
		case kDataTypeRectd:
		{
			return ((atRectd*)m_buffer_ptr)->getValue();
		}
	}

	printlog_lvs2(QString("ToValueRectd failed, datatype:%1").arg(m_data_type), LOG_SCOPE_TAG);
	return Rectd();
}

void CVariant::setParent(void* parent_ptr)
{
	m_parent_ptr = parent_ptr;
}

void CVariant::setMinValue(f32 min_value)
{
	m_min_value = min_value;
}

void CVariant::setMaxValue(f32 max_value)
{
	m_max_value = max_value;
}

void CVariant::setMinMaxValue(f32 min_value, f32 max_value)
{
	m_min_value = min_value;
	m_max_value = max_value;
}

void CVariant::limitValue()
{
	if (!m_buffer_ptr || m_min_value > m_max_value)
	{
		return;
	}

	switch (m_data_type)
	{
		case kDataTypeInt8:
		{
			i8 tmp = *((ati8*)m_buffer_ptr);
			*((ati8*)m_buffer_ptr) = (i8)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeInt16:
		{
			i16 tmp = *((ati16*)m_buffer_ptr);
			*((ati16*)m_buffer_ptr) = (i16)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeInt32:
		{
			i32 tmp = *((ati32*)m_buffer_ptr);
			*((ati32*)m_buffer_ptr) = (i32)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeInt64:
		{
			i64 tmp = *((ati64*)m_buffer_ptr);
			*((ati64*)m_buffer_ptr) = (i64)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeUInt8:
		{
			u8 tmp = *((atu8*)m_buffer_ptr);
			*((atu8*)m_buffer_ptr) = (u8)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeUInt16:
		{
			u16 tmp = *((atu16*)m_buffer_ptr);
			*((atu16*)m_buffer_ptr) = (u16)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeUInt32:
		{
			u32 tmp = *((atu32*)m_buffer_ptr);
			*((atu32*)m_buffer_ptr) = (u32)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeUInt64:
		{
			u64 tmp = *((atu64*)m_buffer_ptr);
			*((atu64*)m_buffer_ptr) = (u64)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeFloat32:
		{
			f32 tmp = *((atf32*)m_buffer_ptr);
			*((atf32*)m_buffer_ptr) = (f32)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
		case kDataTypeFloat64:
		{
			f64 tmp = *((atf64*)m_buffer_ptr);
			*((atf64*)m_buffer_ptr) = (f64)po::_max(po::_min(tmp, m_max_value), m_min_value);
			break;
		}
	}
}

i32 CVariant::memInfoSize()
{
	i32 len = 0;
	len += CPOBase::getStringMemSize(m_attr_name);
	len += sizeof(m_tag_type);
	len += sizeof(m_data_type);
	len += sizeof(m_min_value);
	len += sizeof(m_max_value);
	return len;
}

i32 CVariant::memInfoWrite(u8*& buffer_ptr, i32& buffer_size)
{
	u8* buffer_pos = buffer_ptr;
	CPOBase::memWrite(buffer_ptr, buffer_size, m_attr_name);
	CPOBase::memWrite(m_tag_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_data_type, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_min_value, buffer_ptr, buffer_size);
	CPOBase::memWrite(m_max_value, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}

i32 CVariant::memInfoRead(u8*& buffer_ptr, i32& buffer_size)
{
	freeBuffer();

	u8* buffer_pos = buffer_ptr;
	CPOBase::memRead(buffer_ptr, buffer_size, m_attr_name);
	CPOBase::memRead(m_tag_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_data_type, buffer_ptr, buffer_size);
	CPOBase::memRead(m_min_value, buffer_ptr, buffer_size);
	CPOBase::memRead(m_max_value, buffer_ptr, buffer_size);
	return buffer_ptr - buffer_pos;
}
