# ============================================================
# QtStock
# 适配: Qt 5.14.2 + MinGW 7.3 32bit + CMake 3.14.0
# ============================================================

cmake_minimum_required(VERSION 3.14)                          # 适配CMake 3.14.0
project(QtStock VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)                                     # C++17标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)                                          # 自动MOC处理Q_OBJECT
set(CMAKE_AUTORCC ON)                                          # 自动RCC处理资源文件

# --- Qt5查找 ---
if(DEFINED ENV{QT_DIR})
    set(CMAKE_PREFIX_PATH "$ENV{QT_DIR}" ${CMAKE_PREFIX_PATH})
    message(STATUS "Qt path (from QT_DIR): $ENV{QT_DIR}")
endif()

find_package(Qt5 COMPONENTS Widgets Network Charts REQUIRED)  # Qt5 Widgets+Network+Charts

# --- 源文件 ---
set(SOURCES
    src/main.cpp
    src/core/sina_api.cpp
    src/core/dataengine.cpp
    src/core/csv_loader.cpp
    src/view/mainwindow.cpp
    src/view/stock_list.cpp
    src/view/kline_view.cpp
    src/indicator/ma.cpp
    src/indicator/macd.cpp
    src/indicator/boll.cpp
)

# --- 头文件 ---
set(HEADERS
    src/core/stockdata.h
    src/core/sina_api.h
    src/core/dataengine.h
    src/core/csv_loader.h
    src/view/mainwindow.h
    src/view/stock_list.h
    src/view/kline_view.h
    src/indicator/indicator_base.h
    src/indicator/ma.h
    src/indicator/macd.h
    src/indicator/boll.h
)

# --- 可执行目标 ---
add_executable(QtStock ${SOURCES} ${HEADERS})

# --- Include目录 ---
target_include_directories(QtStock PRIVATE
    src
    src/core
    src/view
    src/indicator
)

# --- Qt5链接 ---
target_link_libraries(QtStock PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Charts
)

# --- MinGW静态链接运行时库 ---
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(QtStock PRIVATE -static-libgcc -static-libstdc++)
endif()

# --- Windows部署 ---
if(WIN32)
    set_target_properties(QtStock PROPERTIES WIN32_EXECUTABLE ON)
endif()
