cmake_minimum_required(VERSION 3.19)
project(stream_tools)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )


add_executable(opencv_ffmpeg_rtsp opencv_ffmpeg_rtsp.cc)
target_link_libraries(opencv_ffmpeg_rtsp PkgConfig::LIBAV ${OpenCV_LIBS})

add_executable(rtsp_receiver rtsp_receiver.cc rtsp_receiver_main.cc)
target_link_libraries(rtsp_receiver PkgConfig::LIBAV ${OpenCV_LIBS})

