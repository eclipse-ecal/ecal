include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Continental Automotive Systems")

set(CPACK_PACKAGE_VERSION_MAJOR ${GIT_REVISION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GIT_REVISION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GIT_REVISION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "rex.schilasky@continental-corporation.com")
set(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_OUTPUT_FILE_PREFIX _deploy)

# if(WIN32)
  # set(CPACK_GENERATOR "WIX")
  # set(CPACK_WIX_PRODUCT_GUID "87A6B30D-670A-4752-A10F-37521D01822F")
  # set(CPACK_WIX_UPGRADE_GUID "A025EF91-496F-4E1C-96D1-1E0CC7B00C74")
  # set(CPACK_WIX_SKIP_PROGRAM_FOLDER ON)
  # cpack_add_component(libprotobuf HIDDEN)
  # cpack_add_component(libprotoc HIDDEN)
  # cpack_add_component(protoc HIDDEN)
  # set(CPACK_WIX_PRODUCT_ICON ${ECAL_PROJECT_ROOT}/cpack/ecalproduct.ico)
  # set(CPACK_WIX_UI_BANNER ${ECAL_PROJECT_ROOT}/cpack/banner.png)
  # set(CPACK_WIX_UI_DIALOG ${ECAL_PROJECT_ROOT}/cpack/background.png)
  # set(CPACK_WIX_UI_REF "WixUI_Mondo")
  # set(CPACK_WIX_EXTENSIONS "WixUtilExtension")
  ##set(CPACK_WIX_PATCH_FILE ${ECAL_PROJECT_ROOT}/cpack/ecal_path.xml)
  ##set(CPACK_WIX_TEMPLATE   "${ECAL_PROJECT_ROOT}/cpack/custom_template.wxs.in")
  ##configure_file("${ECAL_PROJECT_ROOT}/cpack/custom_template.wxs.in" "${CMAKE_BINARY_DIR}/custom_template.wxi" #@ONLY)

#endif()
if(WIN32)
  set(CPACK_GENERATOR "External")
  set(CPACK_EXTERNAL_ENABLE_STAGING ON)
  if(ECAL_CPACK_PACK_WITH_INNOSETUP)
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${ECAL_PROJECT_ROOT}/cpack/innosetup.cmake")
  endif()
endif()


if(UNIX)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_SOURCE_GENERATOR "TGZ")

  # When configuring the project, a header file with paths has been created.
  # These paths are used e.g. in eCAL Mon to find the plugin diretory. Thus we
  # need to use the same paths for cpack.
  set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
endif()

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rex Schilasky <rex.schilasky@continental-corporation.com>")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/eclipse-ecal/ecal")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/cpack/debscripts/postinst" "${CMAKE_BINARY_DIR}/cpack/debscripts/postrm")
set(CPACK_DEBIAN_PACKAGE_DEBUG ON)


set(CPACK_RESOURCE_FILE_LICENSE ${ECAL_PROJECT_ROOT}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README  ${ECAL_PROJECT_ROOT}/README.md)

get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
list(REMOVE_ITEM CPACK_COMPONENTS_ALL
  #"libprotobuf-lite"
  #"protobuf-export"
  #"protobuf-headers"
  #"protobuf-protos"
  "tinyxml2_config"
  "tinyxml2_headers"
  "tinyxml2_libraries"
)

include(CPack)
