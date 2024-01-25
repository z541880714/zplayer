cmake_minimum_required(VERSION 3.26)
project(zPlayer)

set(CMAKE_CXX_STANDARD 23)


set(ffmpeg_lib_dir D:/1.Environment/6_ffmpeg/ffmpeg/ffmpeg-6.0-full_build-shared)
include_directories(${ffmpeg_lib_dir}/include)

#link_directories(${ffmpeg_lib_dir}/bin)
file(GLOB ffmpeg_libs ${ffmpeg_lib_dir}/bin/*.dll)

# local source code
include_directories(include)
file(GLOB source_dir *.cpp *.c)
add_executable(${PROJECT_NAME} ${source_dir})

target_link_libraries(${PROJECT_NAME}
        ${ffmpeg_libs}
)
