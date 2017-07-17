//
// Created by cly10 on 17/07/11.
//

#ifndef HEICHE_JNIDEF_H
#define HEICHE_JNIDEF_H

#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <list>
#include <string>
#include <algorithm>
#include <functional>
using namespace std;

static const char* tag = "JNITAG";

jstring c2j(JNIEnv* env, string cstr);
string j2c(JNIEnv* env, jstring jstr);

// 修改一下打印调试信息的宏
#define MyError(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define MyWarn(fmt, ...) __android_log_print(ANDROID_LOG_WARN, tag, fmt, ##__VA_ARGS__)

#endif //HEICHE_JNIDEF_H
