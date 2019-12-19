#include "tool_category_text.h"

//Category
const powstring kToolCategoryLocatorText[kTCC] = { L"Locator Tools", L"定位工具" };
const powstring kToolCategoryInspectorText[kTCC] = { L"Inspector Tools", L"检测工具" };
const powstring kToolCategoryPresenceText[kTCC] = { L"Presence Tools", L"有无检测工具" };
const powstring kToolCategoryMeasurementText[kTCC] = { L"Measurement Tools", L"测量工具" };
const powstring kToolCategoryCountingText[kTCC] = { L"Counting Tools", L"计数工具" };
const powstring kToolCategoryGeometryText[kTCC] = { L"Geometry Tools", L"几何工具" };
const powstring kToolCategoryIdentifyText[kTCC] = { L"Identification Tools", L"识别工具" };
const powstring kToolCategoryImageFilterText[kTCC] = { L"Image Filter Tools", L"图像滤波工具" };
const powstring kToolCategoryMathText[kTCC] = { L"Math&String Tools", L"数学&字符工具" };
const powstring kToolCategoryPlotText[kTCC] = { L"Plot Tools", L"绘图工具" };
const powstring kToolCategoryDefectDetectionText[kTCC] = { L"Defect Detection Tools", L"缺陷检测工具" };
const powstring kToolCategoryCalibrationText[kTCC] = { L"Calibration Tools", L"校准工具" };

//Locator
const powstring kToolLocatorShapeBasedText[kTCC] = { L"ShapeBased Pattern", L"形状图案" };
const powstring kToolLocatorShapeMaxText[kTCC] = { L"ShapeMax Pattern", L"ShapeMax图案" };
const powstring kToolLocatorContentBasedText[kTCC] = { L"ContentBased Pattern", L"轮廓图案" };
const powstring kToolLocatorEdgeText[kTCC] = { L"Edge", L"边缘" };
const powstring kToolLocatorEdgeIntersectionText[kTCC] = { L"Edge Intersection", L"边缘交点" };
const powstring kToolLocatorBlobText[kTCC] = { L"Blob", L"斑点" };
const powstring kToolLocatorBlobsText[kTCC] = { L"Blobs(1-10)", L"斑点（1-10）" };
const powstring kToolLocatorColorBlobText[kTCC] = { L"Color Blob", L"颜色斑点" };
const powstring kToolLocatorColorBlobsText[kTCC] = { L"Color Blobs(1-10)", L"颜色斑点（1-10）" };
const powstring kToolLocatorCircleText[kTCC] = { L"Circle", L"圆" };
const powstring kToolLocatorComputeFixtureText[kTCC] = { L"Compute Fixture", L"计算定位点" };

//Presence/Absence Tools
const powstring kToolPresenceLocatorText[kTCC] = { L"Locator", L"定位器" };
const powstring kToolPresenceBrightnessText[kTCC] = { L"Brightness", L"亮度" };
const powstring kToolPresenceContrastText[kTCC] = { L"Contrast", L"对比度" };
const powstring kToolPresenceSharpnessText[kTCC] = { L"Sharpness", L"锐度" };
const powstring kToolPresenceBlobText[kTCC] = { L"Blob", L"斑点" };
const powstring kToolPresenceEdgeText[kTCC] = { L"Edge", L"边缘" };
const powstring kToolPresenceCircleText[kTCC] = { L"Circle", L"圆" };

//Measurement Tools
const powstring kToolMeasurementDistanceText[kTCC] = { L"Distance", L"距离" };
const powstring kToolMeasurementAngleText[kTCC] = { L"Angle", L"角度" };
const powstring kToolMeasurementBlobAreaText[kTCC] = { L"Blob Area", L"斑点面积" };
const powstring kToolMeasurementBlobAreasText[kTCC] = { L"Blob Areas", L"多个斑点面积" };
const powstring kToolMeasurementCircleDiameterText[kTCC] = { L"Circle Diameter", L"圆直径" };
const powstring kToolMeasurementCircleConcentricityText[kTCC] = { L"Circle Concentricity", L"圆同心度" };
const powstring kToolMeasurementCircleRadiusText[kTCC] = { L"Measure Radius", L"半径测量" };
const powstring kToolMeasurementMinMaxPointsText[kTCC] = { L"MinMax Points", L"最大/最小位置点" };

//Counting Tools
const powstring kToolCountingShapeLocatorText[kTCC] = { L"ShapeBased Patterns", L"形状定位器" };
const powstring kToolCountingShapeMaxLocatorText[kTCC] = { L"ShapeMax Patterns", L"ShapeMax定位器" };
const powstring kToolCountingBlobsText[kTCC] = { L"Blobs", L"斑点" };
const powstring kToolCountingColorBlobsText[kTCC] = { L"Color Blobs", L"多个颜色斑点" };
const powstring kToolCountingEdgesText[kTCC] = { L"Edge", L"边缘" };
const powstring kToolCountingEdgePairsText[kTCC] = { L"Edge Pairs", L"边缘对" };
const powstring kToolCountingPixelCounterText[kTCC] = { L"Pixel Counter", L"像素计数" };

//Identification Tools
const powstring kToolIdentification1DCodeText[kTCC] = { L"Read 1DCode", L"一维码读取" };
const powstring kToolIdentification1DCodesText[kTCC] = { L"Read 1DCodes(1-16)", L"一维码读取（1-16）" };
const powstring kToolIdentification2DCodeText[kTCC] = { L"Read 2DCode", L"二维码读取" };
const powstring kToolIdentification2DCodesText[kTCC] = { L"Read 2DCodes(1-16)", L"二维码读取（1-16）" };
const powstring kToolIdentificationPostalCodeText[kTCC] = { L"Read PostalCode", L"邮政代码读取" };
const powstring kToolIdentificationOCRMaxText[kTCC] = { L"Read Text", L"文本读取" };
const powstring kToolIdentificationPatternsText[kTCC] = { L"ShapeBased Pattern", L"形状图案" };
const powstring kToolIdentificationBlobsText[kTCC] = { L"Blobs", L"斑点" };
const powstring kToolIdentificationColorBlobsText[kTCC] = { L"Color Blobs", L"颜色斑点" };

//Geometry Tools
const powstring kToolGeometryP2PLineText[kTCC] = { L"Point To Point: Line", L"点-点：直线" };
const powstring kToolGeometryP2PMidPointText[kTCC] = { L"Point To Point: MidPoint", L"点-点：中点" };
const powstring kToolGeometryP2PDimensionText[kTCC] = { L"Point To Point: Dimension", L"点-点：尺寸" };
const powstring kToolGeometryPerpendicularLineText[kTCC] = { L"Perpendicular Line", L"垂线" };
const powstring kToolGeometryLineIntersectionText[kTCC] = { L"Line Intersection", L"直线交点" };
const powstring kToolGeometryBisetAngleText[kTCC] = { L"Bisect Angle", L"角平分线" };
const powstring kToolGeometryFitLineNPointsText[kTCC] = { L"Line From N-Points", L"N点拟合直线" };
const powstring kToolGeometryFitCircleNPointsText[kTCC] = { L"Circle From N-Points", L"N点拟合圆" };
const powstring kToolGeometryCircleLineIntersectionText[kTCC] = { L"Circle-Line Intersection", L"圆线交点" };
const powstring kToolGeometryLineFitText[kTCC] = { L"Line Fit", L"直线拟合" };
const powstring kToolGeometryCircleFitText[kTCC] = { L"Circle Fit", L"拟合圆" };

//Math & Logic Tools
const powstring kToolMathMathText[kTCC] = { L"Math", L"数学" };
const powstring kToolMathStringText[kTCC] = { L"String", L"字符串" };
const powstring kToolMathLogicText[kTCC] = { L"Logic", L"逻辑" };
const powstring kToolMathTrendText[kTCC] = { L"Trend", L"趋势" };
const powstring kToolMathStatisticsText[kTCC] = { L"Statistics", L"统计" };
const powstring kToolMathGroupText[kTCC] = { L"Group", L"分组" };
const powstring kToolMathSequenceText[kTCC] = { L"Sequence", L"序列" };
const powstring kToolMathComputePointText[kTCC] = { L"Compute Point", L"计算点" };
const powstring kToolMathVariablesText[kTCC] = { L"Variables", L"变量" };

//Plot Tools
const powstring kToolPlotArcText[kTCC] = { L"Arc", L"圆弧" };
const powstring kToolPlotCircleText[kTCC] = { L"Circle", L"圆" };
const powstring kToolPlotCrossText[kTCC] = { L"Cross", L"交叉线" };
const powstring kToolPlotLineText[kTCC] = { L"Line", L"直线" };
const powstring kToolPlotPointText[kTCC] = { L"Point", L"点" };
const powstring kToolPlotRegionText[kTCC] = { L"Region", L"区域" };
const powstring kToolPlotStringText[kTCC] = { L"String", L"字符串" };

//Image Filter Tools
const powstring kToolImageFilterFilterText[kTCC] = { L"Filter", L"滤波" };
const powstring kToolImageFilterColor2GrayText[kTCC] = { L"Color To Gray", L"颜色转灰色" };
const powstring kToolImageFilterColor2BinaryText[kTCC] = { L"Color To Binary", L"颜色转二进制" };
const powstring kToolImageFilterTransformText[kTCC] = { L"Transform", L"转换" };
const powstring kToolImageFilterCompareText[kTCC] = { L"Compare", L"比较" };

//Defect Detection Tools
const powstring kToolDefectSurfaceFlawText[kTCC] = { L"Surface Flaw", L"表面瑕疵" };
const powstring kToolDefectEdgeText[kTCC] = { L"Edge", L"边沿" };
const powstring kToolDefectEdgePairText[kTCC] = { L"Edge Pair", L"边沿对" };
const powstring kToolDefectEdgeWidthText[kTCC] = { L"Edge Width", L"边沿宽度" };
const powstring kToolDefectBeadFinderText[kTCC] = { L"Bead Finder", L"焊珠查找" };
const powstring kToolDefectBeadTrackerText[kTCC] = { L"Bead Tracker", L"焊珠追踪" };

//Calibration Tools	
const powstring kToolCalibrationNPointsText[kTCC] = { L"N Points", L"N点" };
const powstring kToolCalibrationSequentialNPointsText[kTCC] = { L"Sequential N Points", L"N点序列" };