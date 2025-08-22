# CMakeLists.txt - Version 4.1 (Thêm module Concurrent)
# --- Cài đặt CMake tối thiểu và thông tin dự án ---
cmake_minimum_required(VERSION 3.16)
project(FrameCapture VERSION 3.0 LANGUAGES CXX)

# --- Cài đặt chuẩn C++ ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Cấu hình cho Qt6 ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# SỬA LỖI: Thêm "Concurrent" vào danh sách các module cần tìm
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Multimedia Concurrent)

# --- Cấu hình cho FFmpeg (Linh hoạt hơn) ---
if(NOT FFMPEG_DIR)
    set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/ffmpeg" CACHE PATH "Path to FFmpeg directory")
endif()

message(STATUS "Using FFmpeg from: ${FFMPEG_DIR}")

include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)

# --- Tạo file thực thi ---
add_executable(FrameCapture
    main.cpp
    mainwindow.cpp
    sidepanel.cpp 
    playerpanel.cpp
    librarypanel.cpp
    stylepanel.cpp
    exportpanel.cpp
    videoprocessor.cpp
    videowidget.cpp
    viewpanel.cpp
    libraryitemdelegate.cpp
    cropdialog.cpp
    librarywidget.cpp
    imageviewerdialog.cpp
    videoworker.cpp
    resources.qrc
)

# --- Liên kết các thư viện cần thiết ---
# SỬA LỖI: Thêm "Qt6::Concurrent" vào danh sách thư viện cần liên kết
target_link_libraries(FrameCapture PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
    Qt6::Multimedia
    Qt6::Concurrent
    avcodec
    avformat
    avutil
    swscale
    swresample
)

# --- Thêm các file header để IDE nhận diện ---
target_sources(FrameCapture PRIVATE
    mainwindow.h
    sidepanel.h
    playerpanel.h
    librarypanel.h
    stylepanel.h
    exportpanel.h
    helpers.h
    videoprocessor.h
    videowidget.h
    viewpanel.h
    libraryitemdelegate.h
    cropdialog.h
    librarywidget.h
    imageviewerdialog.h
    videoworker.h
)
