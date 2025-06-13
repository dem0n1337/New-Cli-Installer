# This patch disables the filesystem check in CPR
file(WRITE ${CPR_SOURCE_DIR}/include/CMakeLists.txt [[
add_library(cpr_include INTERFACE)
target_include_directories(cpr_include INTERFACE
    $<BUILD_INTERFACE:${CPR_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS cpr_include EXPORT cpr_targets)
install(
    DIRECTORY "${CPR_SOURCE_DIR}/include/cpr"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT cpr_dev
)
]])