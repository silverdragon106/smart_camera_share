#include "test_code.h"

#include "tool_data.h"
#include "tool_result.h"
#include "proc/image_proc.h"
#include "proc/camera/calibration.h"
#include "sc_application.h"

CTestCode::CTestCode()
{

}

CTestCode::~CTestCode()
{

}

/*
//////////////////////////////////////////////////////////////////////////
#include "cog_patmax_locator.h"

//(100,100,300,300)구역에 대한 PatMax도구의 학습
void TestCase1()
{
	i32 w, h;
	CameraCalib calib_param;
	LocatorParam loc_param;
	CPatMaxBasedLocator patmax_locator(&loc_param);
	u8* img_ptr = CImageProc::loadImgOpenCV(PO_LOG_PATH"1.bmp", w, h);
	if (!img_ptr)
	{
		return;
	}

	i32 hw = w / 2;
	i32 hh = h / 2;

	CLocPatMaxTool pat_tool(kSCLocatorToolPatMax);
	CLocPatMaxResult pat_tool_result(kSCLocatorToolPatMax);
	ResultDepend pat_result(&pat_tool_result);
	ResultDependVector depend_vec;
	depend_vec.push_back(pat_result);

	pat_tool.initToolParam();
	
	CSubRegion sub_region;
	sub_region.m_region_shape = kRegionShapeRect;
	sub_region.m_region_oper = kRegionOperPlus;
	sub_region.m_box = Rectf(-100, -100, 100, 100);
	sub_region.u.rect.m_x1 = -100;
	sub_region.u.rect.m_y1 = -100;
	sub_region.u.rect.m_x2 = 100;
	sub_region.u.rect.m_y2 = 100;

	pat_tool.m_model_region.m_box = Rectf(-100, -100, 100, 100);
	pat_tool.m_model_region.m_plus_region_count = 1;
	pat_tool.m_model_region.m_region_vec.push_back(sub_region);
	pat_tool.m_model_region.initSetup(0, 200, 200);
	pat_tool.m_model_region.update();

	sub_region.m_region_shape = kRegionShapeRect;
	sub_region.m_region_oper = kRegionOperPlus;
	sub_region.m_box = Rectf(-hw, -hh, hw, hh);
	sub_region.u.rect.m_x1 = -hw;
	sub_region.u.rect.m_y1 = -hh;
	sub_region.u.rect.m_x2 = hw;
	sub_region.u.rect.m_y2 = hh;

	pat_tool.m_search_region.m_plus_region_count = 1;
	pat_tool.m_search_region.m_region_vec.push_back(sub_region);
	pat_tool.m_search_region.m_box = Rectf(-hw, -hh, hw, hh);
	pat_tool.m_search_region.initSetup(0, hw, hh);
	pat_tool.m_search_region.update();

	pat_tool.setToolID(1);
	patmax_locator.studyPatMaxLocator(ImgPart(img_ptr, w, h), &pat_tool, depend_vec,
	&calib_param, kEngineOperCreated);
	patmax_locator.getPatMaxLocatorResult(ImgPart(img_ptr, w, h), &pat_tool, depend_vec,
	&calib_param, kEngineOperProcess);

	POSAFE_DELETE_ARRAY(img_ptr);
	depend_vec.clear();
	}
	*/
/*
//////////////////////////////////////////////////////////////////////////
#include "os/os_support.h"

//망설정을 변경
void TestCase2()
{
i32 ip_address = CPOBase::convertIPAddress("192.168.1.175");
i32 new_ip_address = CPOBase::convertIPAddress("192.168.1.23");
i32 ip_submask = CPOBase::convertIPAddress("255.255.255.0");
i32 ip_gateway = CPOBase::convertIPAddress("192.168.1.1");
i32 ip_dns_server = CPOBase::convertIPAddress("8.8.4.4");
COSBase::setNetworkAddress(ip_address, false, new_ip_address, ip_submask, ip_gateway, ip_dns_server);
//COSBase::setNetworkAddress(new_ip_address, true, 0, 0, 0, 0);
}
*/

/*
//////////////////////////////////////////////////////////////////////////
#include <QtConcurrent/QtConcurrent>

//QtConcurrent실험
i32 _testcase3()
{
while (true);
return 0;
}

void TestCase3()
{
i32 i, count = 2;
QFuture<i32> fcode[8];

for (i = 0; i < count; i++)
{
fcode[i] = QtConcurrent::run(_testcase3);
}
for (i = 0; i < count; i++)
{
fcode[i].waitForFinished();
}
}
*/
/*
//////////////////////////////////////////////////////////////////////////
#include "math_calculation.h"

//수학, 문자렬, 론리도구실험
void TestCase4()
{
postring math_expression = "len(\"asd\")+val(\"3.45\")+12*99";
postring str_expression = "upper(trim(\" asd\"))+formatd(3.4)";
postring logic_expression = "(4==5) or not(4<2)";

f32 math_result;
postring str_result;
bool logic_result;

CMathCalculation math_calc(0);
math_calc.calculateLogic(logic_expression, logic_result, 0);
math_calc.calculateMath(math_expression, math_result, 0);
math_calc.calculateString(str_expression, str_result, 0);
}
*/
/*
void TestCase5()
{
	cv::Mat img = CImageProc::loadImgOpenCV(L"d:\\1.bmp", cv::IMREAD_GRAYSCALE);
	sys_keep_time;
	for (i32 i = 0; i < 10; i++)
	{
		cv::Mat img_blur, img_sobel_x, img_sobel_y, img_grad;
		cv::GaussianBlur(img, img_blur, cv::Size(5, 5), 1);
		cv::Sobel(img_blur, img_sobel_x, CV_16S, 1, 0);
		cv::Sobel(img_blur, img_sobel_y, CV_16S, 0, 1);
		img_sobel_x = cv::abs(img_sobel_x);
		img_sobel_y = cv::abs(img_sobel_y);
		cv::addWeighted(img_sobel_x, 0.5, img_sobel_y, 0.5, 0, img_grad);
	}
	printlog_lv0(QString("test result: proc time is %1ms").arg((sys_get_time_ms) / 10));
	//cv::imwrite("d:\\2.bmp", img_grad);
}
*/
//////////////////////////////////////////////////////////////////////////
void CTestCode::run()
{
	//TestCase1();
	//TestCase2();
	//TestCase3();
	//TestCase4();
	//TestCase5();
}
