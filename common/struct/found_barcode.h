#pragma once
#include "struct.h"

enum BarcodeSymbologyTypes
{
	kBarcodeNone = 0,

	kBarcode1DCode128 = 10,
	kBarcode1DCode39,
	kBarcode1DInterleaved2of5,
	kBarcode1DPharmacode,
	kBarcode1DUPCEAN,
	kBarcode1DCode93,
	kBarcode1DCodabar,
	kBarcode1DPDF417,
	kBarcode1DDataBar,
	kBarcode1DDataBarLimited,
	kBarcode1DDataBarExpanded,
	kBarcode1DEANUCCComposite,

	kBarcode2DDataMatrix = 30,
	kBarcode2DQRCode,

	kBarcodePostalPOSTNET = 40,
	kBarcodePostalPLANET,
	kBarcodePostalJPPost,
	kBarcodePostalAUSPost,
	kBarcodePostalUPU,
	kBarcodePostalIMB
};

enum BarcodeGradeTypes
{
	kBarcodeGradeNone = 0,
	kBarcodeGradeCount
};

class CFoundBarcode
{
public:
	CFoundBarcode();
	CFoundBarcode(const postring& id_string, const postring& str_symbology,
			f32 center_x, f32 center_y, f32 angle, i32 grade = kBarcodeGradeNone);
	~CFoundBarcode();

	void						init();
	void						setValue(CFoundBarcode* other_ptr);

	i32							memSize();
	i32							memRead(u8*& buffer_ptr, i32& buffer_size);
	i32							memWrite(u8*& buffer_ptr, i32& buffer_size);
	bool						fileRead(FILE* fp);
	bool						fileWrite(FILE* fp);

public:
	postring					m_string;
	postring					m_symbology;
	i32							m_grade;

	f32							m_center_x;		//mm, if calibed
	f32							m_center_y;		//mm, if calibed
	f32							m_angle_deg;	//deg
};
typedef std::vector<CFoundBarcode>	FoundBarcodeVector;
