#ifndef UNTITLED2_LIBRARY_H
#define UNTITLED2_LIBRARY_H


static int print2(const char *_format, ...) {
    int _retval;
    __builtin_va_list _local_argv;
    __builtin_va_start(_local_argv, _format);
    _retval = vfprintf_s(stdout, _format, _local_argv);
    __builtin_va_end(_local_argv);
    return _retval;
}


#endif //UNTITLED2_LIBRARY_H
