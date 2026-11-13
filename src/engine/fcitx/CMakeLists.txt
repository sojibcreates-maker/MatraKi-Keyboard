cmake_minimum_required(VERSION 3.16)
project(fcitx5-matraki VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5Utils REQUIRED)
find_package(Threads REQUIRED)

add_library(matraki MODULE matraki.cpp)
target_link_libraries(matraki PRIVATE Fcitx5::Core Fcitx5::Utils Threads::Threads ${CMAKE_DL_LIBS})
target_include_directories(matraki PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../riti/include)
set_target_properties(matraki PROPERTIES PREFIX "")

# Link libriri if available
find_library(RITI_LIB riti PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/src/engine/riti/target/release /usr/lib /usr/local/lib)
if (RITI_LIB)
    target_link_libraries(matraki PRIVATE ${RITI_LIB})
endif()

if (NOT DEFINED FCITX_INSTALL_LIBDIR)
    set(FCITX_INSTALL_LIBDIR "/usr/lib/x86_64-linux-gnu/fcitx5")
endif()
if (NOT DEFINED FCITX_INSTALL_PKGDATADIR)
    set(FCITX_INSTALL_PKGDATADIR "/usr/share/fcitx5")
endif()

install(TARGETS matraki DESTINATION "${FCITX_INSTALL_LIBDIR}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/matraki.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/matraki-addon.conf" RENAME matraki.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
