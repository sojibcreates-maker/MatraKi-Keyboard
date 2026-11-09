include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

find_package(Pthread REQUIRED)
find_package(DL REQUIRED)

add_library(matraki MODULE matraki.cpp)
target_link_libraries(matraki Fcitx5::Core riti Pthread::Pthread DL::DL)
target_include_directories(matraki PRIVATE ../riti/include)
set_target_properties(matraki PROPERTIES PREFIX "")
install(TARGETS matraki DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/matraki.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/matraki-addon.conf" RENAME matraki.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
