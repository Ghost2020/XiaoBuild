# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Helper macro to prepare forwarding all set sbom options to some other function.
# Expects the options names to be set in the parent scope by calling
# _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
macro(_qt_internal_sbom_forward_sbom_add_target_options args_var_name)
    if(NOT opt_args)
        message(FATAL_ERROR
            "Expected opt_args to be set by _qt_internal_get_sbom_add_target_options")
    endif()
    if(NOT single_args)
        message(FATAL_ERROR
            "Expected single_args to be set by _qt_internal_get_sbom_add_target_options")
    endif()
    if(NOT multi_args)
        message(FATAL_ERROR
            "Expected multi_args to be set by _qt_internal_get_sbom_add_target_options")
    endif()
    _qt_internal_forward_function_args(
        FORWARD_PREFIX arg
        FORWARD_OUT_VAR ${args_var_name}
        FORWARD_OPTIONS
            ${opt_args}
        FORWARD_SINGLE
            ${single_args}
        FORWARD_MULTI
            ${multi_args}
    )
endmacro()

# Helper function to add a default supplier for a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_supplier target)
    _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
    list(APPEND single_args OUT_VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    _qt_internal_sbom_is_qt_entity_type("${arg_TYPE}" is_qt_entity_type)
    _qt_internal_sbom_is_qt_3rd_party_entity_type("${arg_TYPE}" is_qt_3rd_party_entity_type)

    set(supplier "")
    if(NOT arg_SUPPLIER
            AND (is_qt_entity_type OR is_qt_3rd_party_entity_type)
            AND NOT arg_NO_DEFAULT_QT_SUPPLIER)
        _qt_internal_sbom_get_default_supplier(supplier)
    endif()

    if(supplier)
        set(${arg_OUT_VAR} "${supplier}" PARENT_SCOPE)
    endif()
endfunction()

# Helper function to add a default package for a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_package_version target)
    _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
    list(APPEND single_args OUT_VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    _qt_internal_sbom_is_qt_entity_type("${arg_TYPE}" is_qt_entity_type)

    set(package_version "")
    if(NOT arg_PACKAGE_VERSION
            AND is_qt_entity_type
            AND NOT arg_NO_DEFAULT_QT_PACKAGE_VERSION)
        _qt_internal_sbom_get_default_qt_package_version(package_version)
    endif()

    if(package_version)
        set(${arg_OUT_VAR} "${package_version}" PARENT_SCOPE)
    endif()
endfunction()

# Helper function to add a default repo download location for a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_download_location target)
    _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
    list(APPEND single_args OUT_VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    _qt_internal_sbom_is_qt_entity_type("${arg_TYPE}" is_qt_entity_type)

    set(download_location "")
    if(NOT arg_DOWNLOAD_LOCATION AND is_qt_entity_type)
        _qt_internal_sbom_get_qt_repo_source_download_location(download_location)
    endif()

    if(download_location)
        set(${arg_OUT_VAR} "${download_location}" PARENT_SCOPE)
    endif()
endfunction()

# Helper function to add a default license expression for a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_license_expression target)
    _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
    list(APPEND single_args OUT_VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    _qt_internal_sbom_is_qt_entity_type("${arg_TYPE}" is_qt_entity_type)

    set(license_expression "")

    # For Qt entities, we have some special handling.
    if(is_qt_entity_type AND NOT arg_NO_DEFAULT_QT_LICENSE AND NOT arg_QT_LICENSE_ID)
        if(arg_TYPE STREQUAL "QT_TOOL" OR arg_TYPE STREQUAL "QT_APP")
            if(QT_SBOM_DEFAULT_QT_LICENSE_ID_EXECUTABLES
                    AND NOT arg_NO_DEFAULT_QT_LICENSE_ID_EXECUTABLES)
                # A repo might contain only the "gpl3" license variant as the default for all
                # executables, so allow setting it at the repo level to avoid having to repeat it
                # for each target.
                _qt_internal_sbom_get_spdx_license_expression(
                    "${QT_SBOM_DEFAULT_QT_LICENSE_ID_EXECUTABLES}" license_expression)
            else()
                # For tools and apps, we use the gpl exception variant by default.
                _qt_internal_sbom_get_spdx_license_expression("QT_COMMERCIAL_OR_GPL3_WITH_EXCEPTION"
                    license_expression)
            endif()

        elseif(QT_SBOM_DEFAULT_QT_LICENSE_ID_LIBRARIES
                AND NOT arg_NO_DEFAULT_QT_LICENSE_ID_LIBRARIES)
            # A repo might contain only the "gpl3" license variant as the default for all modules
            # and plugins, so allow setting it at the repo level to avoid having to repeat it
            # for each target.
            _qt_internal_sbom_get_spdx_license_expression(
                "${QT_SBOM_DEFAULT_QT_LICENSE_ID_LIBRARIES}" license_expression)

        else()
            # Otherwise, for modules and plugins we use the default qt license.
            _qt_internal_sbom_get_spdx_license_expression("QT_DEFAULT" license_expression)
        endif()
    endif()

    # Some Qt entities might request a specific license from the subset that we usually use.
    if(arg_QT_LICENSE_ID)
        _qt_internal_sbom_get_spdx_license_expression("${arg_QT_LICENSE_ID}"
            requested_license_expression)
        _qt_internal_sbom_join_two_license_ids_with_op(
            "${license_expression}" "AND" "${requested_license_expression}"
            license_expression)
    endif()

    # Allow setting a license expression string per directory scope via a variable.
    if(is_qt_entity_type AND QT_SBOM_LICENSE_EXPRESSION AND NOT arg_NO_DEFAULT_DIRECTORY_QT_LICENSE)
        set(qt_license_expression "${QT_SBOM_LICENSE_EXPRESSION}")
        _qt_internal_sbom_join_two_license_ids_with_op(
            "${license_expression}" "AND" "${qt_license_expression}"
            license_expression)
    endif()

    if(license_expression)
        set(${arg_OUT_VAR} "${license_expression}" PARENT_SCOPE)
    endif()
endfunction()

# Get the default qt copyright.
function(_qt_internal_sbom_get_default_qt_copyright_header out_var)
    set(${out_var}
        "Copyright (C) The Qt Company Ltd. and other contributors."
        PARENT_SCOPE)
endfunction()

# Helper function to add default copyrights for a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_copyrights target)
    _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
    list(APPEND single_args OUT_VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    _qt_internal_sbom_is_qt_entity_type("${arg_TYPE}" is_qt_entity_type)

    set(qt_default_copyright "")
    if(is_qt_entity_type AND NOT arg_NO_DEFAULT_QT_COPYRIGHTS)
        _qt_internal_sbom_get_default_qt_copyright_header(qt_default_copyright)
    endif()

    if(qt_default_copyright)
        set(${arg_OUT_VAR} "${qt_default_copyright}" PARENT_SCOPE)
    endif()
endfunction()

# Helper function to add default CPEs for a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_cpe target)
    _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
    list(APPEND single_args OUT_VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    _qt_internal_sbom_is_qt_entity_type("${arg_TYPE}" is_qt_entity_type)
    _qt_internal_sbom_is_qt_3rd_party_entity_type("${arg_TYPE}" is_qt_3rd_party_entity_type)

    set(cpe_list "")

    # Add the qt-specific CPE if the target is a Qt entity type, or if it's a 3rd party entity type
    # without any CPE specified.
    if(is_qt_entity_type OR (is_qt_3rd_party_entity_type AND NOT arg_CPE))
        _qt_internal_sbom_compute_security_cpe_for_qt(cpe_list)
    endif()

    if(cpe_list)
        set(${arg_OUT_VAR} "${cpe_list}" PARENT_SCOPE)
    endif()
endfunction()

# Helper macro to prepare forwarding all set purl options to some other function.
# Expects the options names to be set in the parent scope by calling
# _qt_internal_get_sbom_add_target_options(opt_args single_args multi_args)
macro(_qt_internal_sbom_forward_purl_handling_options args_var_name)
    if(NOT opt_args)
        message(FATAL_ERROR
            "Expected opt_args to be set by _qt_internal_get_sbom_purl_handling_options")
    endif()
    if(NOT single_args)
        message(FATAL_ERROR
            "Expected single_args to be set by _qt_internal_get_sbom_purl_handling_options")
    endif()
    if(NOT multi_args)
        message(FATAL_ERROR
            "Expected multi_args to be set by _qt_internal_get_sbom_purl_handling_options")
    endif()
    _qt_internal_forward_function_args(
        FORWARD_PREFIX arg
        FORWARD_OUT_VAR ${args_var_name}
        FORWARD_OPTIONS
            ${opt_args}
        FORWARD_SINGLE
            ${single_args}
        FORWARD_MULTI
            ${multi_args}
    )
endmacro()

# Helper function to decide which purl variants to add for a qt entity.
function(_qt_internal_sbom_handle_qt_entity_purl_variants)
    _qt_internal_get_sbom_purl_handling_options(opt_args single_args multi_args)
    list(APPEND single_args
        OUT_VAR # This is unused, but added by the calling function.
        OUT_VAR_VARIANTS OUT_VAR_IS_QT_PURL_ENTITY_TYPE
    )
    cmake_parse_arguments(PARSE_ARGV 0 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    set(third_party_types
        QT_THIRD_PARTY_MODULE
        QT_THIRD_PARTY_SOURCES
    )

    set(purl_variants "")

    if(arg_IS_QT_ENTITY_TYPE)
        # Qt entities have two purls by default, a QT generic one and a MIRROR hosted on github.
        list(APPEND purl_variants MIRROR QT)
    elseif(arg_TYPE IN_LIST third_party_types)
        # Third party libraries vendored in Qt also have at least two purls, like regular Qt
        # libraries, but might also have an upstream one.

        # The order in which the purls are generated matters for tools that consume the SBOM. Some
        # tools can only handle one PURL per package, so the first one should be the important one.
        # For now, I deem that the upstream one if present. Otherwise the github mirror.
        if(arg_PURL_3RDPARTY_UPSTREAM_ARGS)
            list(APPEND purl_variants 3RDPARTY_UPSTREAM)
        endif()

        list(APPEND purl_variants MIRROR QT)
    endif()

    if(arg_IS_QT_ENTITY_TYPE
            OR arg_TYPE STREQUAL "QT_THIRD_PARTY_MODULE"
            OR arg_TYPE STREQUAL "QT_THIRD_PARTY_SOURCES")
        set(is_qt_purl_entity_type TRUE)
    else()
        set(is_qt_purl_entity_type FALSE)
    endif()

    if(purl_variants)
        set(${arg_OUT_VAR_VARIANTS} "${purl_variants}" PARENT_SCOPE)
    endif()
    if(is_qt_purl_entity_type)
        set(${arg_OUT_VAR_IS_QT_PURL_ENTITY_TYPE} "${is_qt_purl_entity_type}" PARENT_SCOPE)
    endif()
endfunction()

# Helper function to add purl values for a specific purl variant of a qt entity type.
function(_qt_internal_sbom_handle_qt_entity_purl target)
    _qt_internal_get_sbom_purl_handling_options(opt_args single_args multi_args)
    list(APPEND opt_args IS_QT_PURL_ENTITY_TYPE)
    list(APPEND single_args
        OUT_VAR # This is unused, but added by the calling function.
        OUT_PURL_ARGS
        PURL_VARIANT
    )
    cmake_parse_arguments(PARSE_ARGV 1 arg "${opt_args}" "${single_args}" "${multi_args}")
    _qt_internal_validate_all_args_are_parsed(arg)

    set(purl_args "")

    # Qt entity types get special treatment purl.
    if(arg_IS_QT_PURL_ENTITY_TYPE AND NOT arg_NO_DEFAULT_QT_PURL AND
            (arg_PURL_VARIANT STREQUAL "QT" OR arg_PURL_VARIANT STREQUAL "MIRROR"))
        _qt_internal_sbom_get_root_project_name_lower_case(repo_project_name_lowercase)

        # Add a vcs_url to the generic QT variant.
        if(arg_PURL_VARIANT STREQUAL "QT")
            set(entity_vcs_url_version_option "")
            # Can be empty.
            if(QT_SBOM_GIT_HASH_SHORT)
                set(entity_vcs_url_version_option VERSION "${QT_SBOM_GIT_HASH_SHORT}")
            endif()

            _qt_internal_sbom_get_qt_entity_vcs_url(${target}
                REPO_NAME "${repo_project_name_lowercase}"
                ${entity_vcs_url_version_option}
                OUT_VAR vcs_url)
            list(APPEND purl_args PURL_QUALIFIERS "vcs_url=${vcs_url}")
        endif()

        # Add the subdirectory path where the target was created as a custom qualifier.
        _qt_internal_sbom_get_qt_entity_repo_source_dir(${target} OUT_VAR sub_path)
        if(sub_path)
            list(APPEND purl_args PURL_SUBPATH "${sub_path}")
        endif()

        # Add the target name as a custom qualifer.
        list(APPEND purl_args PURL_QUALIFIERS "library_name=${target}")

        # Can be empty.
        if(QT_SBOM_GIT_HASH_SHORT)
            list(APPEND purl_args VERSION "${QT_SBOM_GIT_HASH_SHORT}")
        endif()

        # Get purl args the Qt entity type, taking into account defaults.
        _qt_internal_sbom_get_qt_entity_purl_args(${target}
            NAME "${repo_project_name_lowercase}-${target}"
            REPO_NAME "${repo_project_name_lowercase}"
            SUPPLIER "${arg_SUPPLIER}"
            PURL_VARIANT "${arg_PURL_VARIANT}"
            ${purl_args}
            OUT_VAR purl_args
        )
    endif()

    if(purl_args)
        set(${arg_OUT_PURL_ARGS} "${purl_args}" PARENT_SCOPE)
    endif()
endfunction()


# Get the default qt package version.
function(_qt_internal_sbom_get_default_qt_package_version out_var)
    set(${out_var} "${QT_REPO_MODULE_VERSION}" PARENT_SCOPE)
endfunction()

# Get the default qt supplier.
function(_qt_internal_sbom_get_default_supplier out_var)
    set(${out_var} "TheQtCompany" PARENT_SCOPE)
endfunction()

# Get the default qt supplier url.
function(_qt_internal_sbom_get_default_supplier_url out_var)
    set(${out_var} "https://qt.io" PARENT_SCOPE)
endfunction()

# Get the default qt download location.
# If git info is available, includes the hash.
function(_qt_internal_sbom_get_qt_repo_source_download_location out_var)
    _qt_internal_sbom_get_root_project_name_lower_case(repo_project_name_lowercase)
    set(download_location "git://code.qt.io/qt/${repo_project_name_lowercase}.git")

    _qt_internal_sbom_get_git_version_vars()
    if(QT_SBOM_GIT_HASH)
        string(APPEND download_location "@${QT_SBOM_GIT_HASH}")
    endif()
    set(${out_var} "${download_location}" PARENT_SCOPE)
endfunction()
