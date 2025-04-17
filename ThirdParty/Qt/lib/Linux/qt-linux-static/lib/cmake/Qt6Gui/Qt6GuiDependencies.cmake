# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Make sure Qt6 is found before anything else.
set(Qt6Gui_FOUND FALSE)

if("${_qt_cmake_dir}" STREQUAL "")
    set(_qt_cmake_dir "${QT_TOOLCHAIN_RELOCATABLE_CMAKE_DIR}")
endif()
set(__qt_use_no_default_path_for_qt_packages "NO_DEFAULT_PATH")
if(QT_DISABLE_NO_DEFAULT_PATH_IN_QT_PACKAGES)
    set(__qt_use_no_default_path_for_qt_packages "")
endif()

# Don't propagate REQUIRED so we don't immediately FATAL_ERROR, rather let the find_dependency calls
# set _NOT_FOUND_MESSAGE which will be displayed by the includer of the Dependencies file.
set(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED FALSE)

if(NOT Qt6_FOUND)
    find_dependency(Qt6 6.8.3
        PATHS
            ${QT_BUILD_CMAKE_PREFIX_PATH}
            "${CMAKE_CURRENT_LIST_DIR}/.."
            "${_qt_cmake_dir}"
            ${_qt_additional_packages_prefix_paths}
        ${__qt_use_no_default_path_for_qt_packages}
    )
endif()


# note: _third_party_deps example: "ICU\\;FALSE\\;1.0\\;i18n uc data;ZLIB\\;FALSE\\;\\;"
set(__qt_Gui_third_party_deps "WrapOpenGL\;FALSE\;\;\;;EGL\;FALSE\;\;\;;WrapPNG\;FALSE\;\;\;;WrapHarfbuzz\;FALSE\;\;\;;WrapFreetype\;FALSE\;\;\;;Fontconfig\;FALSE\;\;\;;WrapZLIB\;FALSE\;\;\;;X11\;FALSE\;\;\;;GLIB2\;FALSE\;\;\;;XKB\;TRUE\;0.5.0\;\;;WrapVulkanHeaders\;TRUE\;\;\;")
set(__qt_Gui_third_party_package_WrapOpenGL_provided_targets "WrapOpenGL::WrapOpenGL")
set(__qt_Gui_third_party_package_EGL_provided_targets "EGL::EGL")
set(__qt_Gui_third_party_package_WrapPNG_provided_targets "WrapPNG::WrapPNG")
set(__qt_Gui_third_party_package_WrapHarfbuzz_provided_targets "WrapHarfbuzz::WrapHarfbuzz")
set(__qt_Gui_third_party_package_WrapFreetype_provided_targets "WrapFreetype::WrapFreetype")
set(__qt_Gui_third_party_package_Fontconfig_provided_targets "Fontconfig::Fontconfig")
set(__qt_Gui_third_party_package_WrapZLIB_provided_targets "WrapZLIB::WrapZLIB")
set(__qt_Gui_third_party_package_X11_provided_targets "X11::X11;X11::SM;X11::ICE")
set(__qt_Gui_third_party_package_GLIB2_provided_targets "GLIB2::GLIB2")
set(__qt_Gui_third_party_package_XKB_provided_targets "XKB::XKB")
set(__qt_Gui_third_party_package_WrapVulkanHeaders_provided_targets "WrapVulkanHeaders::WrapVulkanHeaders")

_qt_internal_find_third_party_dependencies("Gui" __qt_Gui_third_party_deps)
unset(__qt_Gui_third_party_deps)

# Find Qt tool package.
set(__qt_Gui_tool_deps "Qt6GuiTools\;6.8.3")
_qt_internal_find_tool_dependencies("Gui" __qt_Gui_tool_deps)
unset(__qt_Gui_tool_deps)

# note: target_deps example: "Qt6Core\;5.12.0;Qt6Gui\;5.12.0"
set(__qt_Gui_target_deps "Qt6Core\;6.8.3;Qt6DBus\;6.8.3")
set(__qt_Gui_find_dependency_paths "${CMAKE_CURRENT_LIST_DIR}/.." "${_qt_cmake_dir}")
_qt_internal_find_qt_dependencies("Gui" __qt_Gui_target_deps
                                  __qt_Gui_find_dependency_paths)
unset(__qt_Gui_target_deps)
unset(__qt_Gui_find_dependency_paths)

set(_Qt6Gui_MODULE_DEPENDENCIES "Core;DBus")
set(Qt6Gui_FOUND TRUE)
