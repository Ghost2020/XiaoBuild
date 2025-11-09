/**
  * @author cxx2020@outlook.com
 */
#pragma once

#include "Runtime/Launch/Resources/Version.h"

#define XB_MAJOR_VERSION 1
#define XB_MINOR_VERSION 1
#define XB_PATCH_VERSION 2

#define XB_VERSION_STRING \
	VERSION_STRINGIFY(XB_MAJOR_VERSION) \
	VERSION_TEXT(".") \
	VERSION_STRINGIFY(XB_MINOR_VERSION) \
	VERSION_TEXT(".") \
	VERSION_STRINGIFY(XB_PATCH_VERSION)

#define XB_COMPANY_NAME "Xiao Studio"
#define XB_COPYRIGHT_STRING "Copyright(C) 2024 - 2025 Xiao Studio Ltd.All rights reserved."