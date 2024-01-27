cmake_minimum_required(VERSION 3.26)
project(zPlayer)

set(CMAKE_CXX_STANDARD 23)


include_directories(include/ffmpeg)
include_directories(include/fftw3)

file(GLOB share_libs libs/*.dll)
file(COPY ${share_libs} DESTINATION /) #将dll 复制到执行目录,让程序顺畅运行.

# local source code
file(GLOB source_dir *.cpp *.c)
add_executable(${PROJECT_NAME} ${source_dir}
        Zfft.cpp
        Zfft.h
        stringutil.h)

target_link_libraries(${PROJECT_NAME}
        ${share_libs}
)
