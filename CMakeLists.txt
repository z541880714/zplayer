cmake_minimum_required(VERSION 3.26)
project(zPlayer)

set(CMAKE_CXX_STANDARD 23)


###  ffmpeg
set(ffmpeg_lib_dir D:/1.Environment/6_ffmpeg/ffmpeg/ffmpeg-6.0-full_build-shared)
include_directories(${ffmpeg_lib_dir}/include)
file(GLOB ffmpeg_libs ${ffmpeg_lib_dir}/bin/*.dll)

### fftw3
set(FFTW_LIBS)
include_directories(include/fftw3)
file(GLOB fftw3_lib libs/libfftw3.dll)
file(COPY ${fftw3_lib} DESTINATION /) #将dll 复制到执行目录,让程序顺畅运行.

# local source code
include_directories(include)
file(GLOB source_dir *.cpp *.c)
add_executable(${PROJECT_NAME} ${source_dir}
        Zfft.cpp
        Zfft.h)

target_link_libraries(${PROJECT_NAME}
        ${ffmpeg_libs}
        ${fftw3_lib}
)
