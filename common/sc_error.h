#pragma once

#include "po_error.h"

enum SCECode
{
	kSCErrInvalidPUState = kPOErrExtend,
	kSCErrInvalidCurJob,
	kSCErrInvalidJob,
	kSCErrInvalidCamCtrl,
	kSCErrInvalidSetting,
	kSCErrInvalidTool,
	kSCErrDeletedJob,

	kSCErrIsNotOffline = kPOErrExtend + 20,
	kSCErrJobCompatibility,

	kSCErrDevPowerOff = kPOErrExtend + 40,

	kSCErrEngine0 = kPOErrExtend + 80,
};
