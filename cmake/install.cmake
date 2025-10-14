# - INSTALL INCLUDES - #

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# - INSTALL SETUP - #

# prepare the underlying installation configuration variables
set(ASTER_CONFIG_TARGETS "${ASTER_TARGET_NAME}-config-targets.cmake")
set(ASTER_CONFIG_PACKAGE "${PROJECT_BINARY_DIR}/${ASTER_TARGET_NAME}-config-package.pc")
set(ASTER_CONFIG_VERSION "${PROJECT_BINARY_DIR}/${ASTER_TARGET_NAME}-config-version.cmake")
set(ASTER_CONFIG_PROJECT "${PROJECT_BINARY_DIR}/${ASTER_TARGET_NAME}-config-project.cmake")

# prepare the exports name to be used
set(ASTER_EXPORTS_NAME ${ASTER_TARGET_NAME}-exports)
set(ASTER_EXPORTS_DEST "${CMAKE_INSTALL_LIBDIR}/cmake/${ASTER_TARGET_NAME}")

# - PROJECT INSTALLATION - #

# generate the version, configuration and target files
write_basic_package_version_file(${ASTER_CONFIG_VERSION} VERSION ${ASTER_VERSION_SHORT} COMPATIBILITY AnyNewerVersion)
configure_file("cmake/${ASTER_TARGET_NAME}.pc.in" ${ASTER_CONFIG_PACKAGE} @ONLY) # ensure we have our details now

# actually install our details now
install(
    TARGETS ${ASTER_TARGET_NAME}
    EXPORT ${ASTER_EXPORTS_NAME}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# use a namespace because CMake provides better diagnostics for namespaced import targets
export(TARGETS ${ASTER_TARGET_NAME} NAMESPACE ${ASTER_TARGET_NAME}:: FILE "${CMAKE_CURRENT_BINARY_DIR}/${ASTER_CONFIG_TARGETS}")

# install the version, configuration and target files now
install(FILES ${ASTER_CONFIG_PACKAGE} DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(FILES ${ASTER_CONFIG_PROJECT} ${ASTER_CONFIG_VERSION} DESTINATION ${ASTER_CONFIG_TARGETS})
install(EXPORT ${ASTER_EXPORTS_NAME} NAMESPACE ${ASTER_TARGET_NAME}:: DESTINATION ${ASTER_CONFIG_TARGETS})