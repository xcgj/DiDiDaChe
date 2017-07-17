# 设置当前目录
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := curl
# 连接curl库
LOCAL_SRC_FILES := curl/armeabi/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

# 清理变量
include $(CLEAR_VARS)

# 链接上Log库
LOCAL_LDLIBS += -llog -lz
# 输出的动态库名字叫什么，libjni.so或者libjni.a
LOCAL_MODULE    := jni

# 输入文件
LOCAL_SRC_FILES := cJSON.c core.cpp jni.cpp json.cpp curl.cpp
LOCAL_STATIC_LIBRARIES := curl
LOCAL_CPPFLAGS := --std=c++11

# 编译类型，编译动态库
include $(BUILD_SHARED_LIBRARY)