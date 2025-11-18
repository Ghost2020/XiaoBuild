/**
  * @author cxx2020@outlook.com
 */
#pragma once

#define XB_MAJOR_VERSION 1
#define XB_MINOR_VERSION 1
#define XB_PATCH_VERSION 2

// Macros for encoding strings
#define _VERSION_TEXT(x) TEXT(x)
#define _VERSION_STRINGIFY_2(x) _VERSION_TEXT(#x)
#define _VERSION_STRINGIFY(x) _VERSION_STRINGIFY_2(x)

#define XB_VERSION_STRING \
	_VERSION_STRINGIFY(XB_MAJOR_VERSION) \
	_VERSION_TEXT(".") \
	_VERSION_STRINGIFY(XB_MINOR_VERSION) \
	_VERSION_TEXT(".") \
	_VERSION_STRINGIFY(XB_PATCH_VERSION)

#define XB_COMPANY_NAME "Xiao Studio"
#define XB_COPYRIGHT_STRING "Copyright(C) 2025 Xiao Studio Ltd.All rights reserved."
