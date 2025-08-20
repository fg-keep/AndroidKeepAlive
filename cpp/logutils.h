
#ifndef LOGUTILS_H
#define LOGUTILS_H

#include <android/log.h>

#define TAG "cpp"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)

#endif
