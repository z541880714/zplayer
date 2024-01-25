

#include <iostream>
#include "library.h"

extern "C" {
#include "libavutil/cpu.h"
#include "encode_audio.h"
#include "decode_audio.h"
}

using namespace std;


int main() {
    int ret;
    const char *mp3_path = R"(../res/Maria Arredondo - Burning.mp3)";
    const char *pcm_path = R"(C:\Users\lionel\Desktop\bbb.pcm)";
//    int ret = encode_test(mp3_path);
    print2("ret: %d\n", ret);
    ret = decode_test(mp3_path, pcm_path);
    print2("ret: %d\n", ret);
    return 0;
}

