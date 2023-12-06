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

add_executable(main2 main2.cc )
target_link_libraries( main2 ${OpenCV_LIBS} )

add_executable(camera_encoder main.cc)
target_link_libraries(camera_encoder ${OpenCV_LIBS} PkgConfig::LIBAV)

add_executable(encode encode.cc)
target_link_libraries(encode  PkgConfig::LIBAV)
add_executable(decode decode.cc)
target_link_libraries(decode PkgConfig::LIBAV)

add_executable(transcoding transcoding.cc video_debugging.cc)
target_link_libraries(transcoding PkgConfig::LIBAV)

add_executable(opencv_ffmpeg_rtsp opencv_ffmpeg_rtsp.cc)
target_link_libraries(opencv_ffmpeg_rtsp PkgConfig::LIBAV ${OpenCV_LIBS})
