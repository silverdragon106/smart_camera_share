#pragma once
#include "types.h"

enum ToolCategoryLangTypes
{
	kToolCategoryLangEn = 0,
	kToolCategoryLangCh,
	kToolCategoryLangKo,

	kTCC //kTagCategoryCount
};

//Category
extern const powstring kToolCategoryLocatorText[kTCC];
extern const powstring kToolCategoryInspectorText[kTCC];
extern const powstring kToolCategoryPresenceText[kTCC];
extern const powstring kToolCategoryMeasurementText[kTCC];
extern const powstring kToolCategoryCountingText[kTCC];
extern const powstring kToolCategoryGeometryText[kTCC];
extern const powstring kToolCategoryIdentifyText[kTCC];
extern const powstring kToolCategoryImageFilterText[kTCC];
extern const powstring kToolCategoryMathText[kTCC];
extern const powstring kToolCategoryPlotText[kTCC];
extern const powstring kToolCategoryDefectDetectionText[kTCC];
extern const powstring kToolCategoryCalibrationText[kTCC];

//Locator
extern const powstring kToolLocatorShapeBasedText[kTCC];
extern const powstring kToolLocatorShapeMaxText[kTCC];
extern const powstring kToolLocatorContentBasedText[kTCC];
extern const powstring kToolLocatorEdgeText[kTCC];
extern const powstring kToolLocatorEdgeIntersectionText[kTCC];
extern const powstring kToolLocatorBlobText[kTCC];
extern const powstring kToolLocatorBlobsText[kTCC];
extern const powstring kToolLocatorColorBlobText[kTCC];
extern const powstring kToolLocatorColorBlobsText[kTCC];
extern const powstring kToolLocatorCircleText[kTCC];
extern const powstring kToolLocatorComputeFixtureText[kTCC];

//Presence/Absence Tools
extern const powstring kToolPresenceLocatorText[kTCC];
extern const powstring kToolPresenceBrightnessText[kTCC];
extern const powstring kToolPresenceContrastText[kTCC];
extern const powstring kToolPresenceSharpnessText[kTCC];
extern const powstring kToolPresenceBlobText[kTCC];
extern const powstring kToolPresenceEdgeText[kTCC];
extern const powstring kToolPresenceCircleText[kTCC];

//Measurement Tools
extern const powstring kToolMeasurementDistanceText[kTCC];
extern const powstring kToolMeasurementAngleText[kTCC];
extern const powstring kToolMeasurementBlobAreaText[kTCC];
extern const powstring kToolMeasurementBlobAreasText[kTCC];
extern const powstring kToolMeasurementCircleDiameterText[kTCC];
extern const powstring kToolMeasurementCircleConcentricityText[kTCC];
extern const powstring kToolMeasurementCircleRadiusText[kTCC];
extern const powstring kToolMeasurementMinMaxPointsText[kTCC];

//Counting Tools
extern const powstring kToolCountingShapeLocatorText[kTCC];
extern const powstring kToolCountingShapeMaxLocatorText[kTCC];
extern const powstring kToolCountingBlobsText[kTCC];
extern const powstring kToolCountingColorBlobsText[kTCC];
extern const powstring kToolCountingEdgesText[kTCC];
extern const powstring kToolCountingEdgePairsText[kTCC];
extern const powstring kToolCountingPixelCounterText[kTCC];

//Identification Tools
extern const powstring kToolIdentification1DCodeText[kTCC];
extern const powstring kToolIdentification1DCodesText[kTCC];
extern const powstring kToolIdentification2DCodeText[kTCC];
extern const powstring kToolIdentification2DCodesText[kTCC];
extern const powstring kToolIdentificationPostalCodeText[kTCC];
extern const powstring kToolIdentificationOCRMaxText[kTCC];
extern const powstring kToolIdentificationPatternsText[kTCC];
extern const powstring kToolIdentificationBlobsText[kTCC];
extern const powstring kToolIdentificationColorBlobsText[kTCC];

//Geometry Tools
extern const powstring kToolGeometryP2PLineText[kTCC];
extern const powstring kToolGeometryP2PMidPointText[kTCC];
extern const powstring kToolGeometryP2PDimensionText[kTCC];
extern const powstring kToolGeometryPerpendicularLineText[kTCC];
extern const powstring kToolGeometryLineIntersectionText[kTCC];
extern const powstring kToolGeometryBisetAngleText[kTCC];
extern const powstring kToolGeometryFitLineNPointsText[kTCC];
extern const powstring kToolGeometryFitCircleNPointsText[kTCC];
extern const powstring kToolGeometryCircleLineIntersectionText[kTCC];
extern const powstring kToolGeometryLineFitText[kTCC];
extern const powstring kToolGeometryCircleFitText[kTCC];

//Math & Logic Tools
extern const powstring kToolMathMathText[kTCC];
extern const powstring kToolMathStringText[kTCC];
extern const powstring kToolMathLogicText[kTCC];
extern const powstring kToolMathTrendText[kTCC];
extern const powstring kToolMathStatisticsText[kTCC];
extern const powstring kToolMathGroupText[kTCC];
extern const powstring kToolMathSequenceText[kTCC];
extern const powstring kToolMathComputePointText[kTCC];
extern const powstring kToolMathVariablesText[kTCC];

//Plot Tools
extern const powstring kToolPlotArcText[kTCC];
extern const powstring kToolPlotCircleText[kTCC];
extern const powstring kToolPlotCrossText[kTCC];
extern const powstring kToolPlotLineText[kTCC];
extern const powstring kToolPlotPointText[kTCC];
extern const powstring kToolPlotRegionText[kTCC];
extern const powstring kToolPlotStringText[kTCC];

//Image Filter Tools
extern const powstring kToolImageFilterFilterText[kTCC];
extern const powstring kToolImageFilterColor2GrayText[kTCC];
extern const powstring kToolImageFilterColor2BinaryText[kTCC];
extern const powstring kToolImageFilterTransformText[kTCC];
extern const powstring kToolImageFilterCompareText[kTCC];

//Defect Detection Tools
extern const powstring kToolDefectSurfaceFlawText[kTCC];
extern const powstring kToolDefectEdgeText[kTCC];
extern const powstring kToolDefectEdgePairText[kTCC];
extern const powstring kToolDefectEdgeWidthText[kTCC];
extern const powstring kToolDefectBeadFinderText[kTCC];
extern const powstring kToolDefectBeadTrackerText[kTCC];

//Calibration Tools	
extern const powstring kToolCalibrationNPointsText[kTCC];
extern const powstring kToolCalibrationSequentialNPointsText[kTCC];