
#ifndef KEEP_H
#define KEEP_H

#include "jni.h"
#include "string.h"

class Keep {
    jboolean bringToTop(JNIEnv *env, jclass clazz, jobject context);
};

#endif