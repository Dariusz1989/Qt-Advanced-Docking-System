cmake_minimum_required(VERSION 3.16)
project(ads_tests VERSION ${QtADS_VERSION})
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Test REQUIRED)
add_executable(CursorPosProviderTest CursorPosProviderTest.cpp)
target_include_directories(CursorPosProviderTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
target_link_libraries(CursorPosProviderTest PRIVATE ads::qtadvanceddocking
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
set_target_properties(CursorPosProviderTest PROPERTIES
    CXX_STANDARD 17
    AUTOMOC ON
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin")
add_test(NAME CursorPosProviderTest COMMAND CursorPosProviderTest)
