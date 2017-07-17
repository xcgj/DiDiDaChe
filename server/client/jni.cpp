//
// Created by cly10 on 17/07/11.
//

#include "jnidef.h"
#include "core.h"

static JNIEnv * gEnv = NULL;    //全局java虚拟机对象
static jobject gObj = NULL;     //全局java activity对象

//c字符串转java字符串
jstring c2j(JNIEnv* env, string cstr)
{
    return env->NewStringUTF(cstr.c_str());
}

//java字符串转成c字符串
string j2c(JNIEnv* env, jstring jstr)
{
    string ret;
    //获得java字符串类
    jclass stringClass = env->FindClass("java/lang/String");
    //获得java函数
    jmethodID getBytes = env->GetMethodID(stringClass, "getBytes",
                                          "(Ljava/lang/String;)[B");

    // 把参数用到的字符串转化成java的字符
    jstring arg = c2j(env, "utf-8");

    jbyteArray jbytes = (jbyteArray)env->CallObjectMethod(jstr, getBytes, arg);

    // 从jbytes中，提取UTF8格式的内容
    jsize byteLen = env->GetArrayLength(jbytes);//总长度
    jbyte* JBuffer = env->GetByteArrayElements(jbytes, JNI_FALSE);//内容

    // 将内容拷贝到C++内存中
    if(byteLen > 0)
    {
        char* buf = (char*)JBuffer;
        std::copy(buf, buf+byteLen, back_inserter(ret));
    }

    // 释放
    env->ReleaseByteArrayElements(jbytes, JBuffer, 0);
    return ret;
}

//登陆函数调用C函数
extern "C"
jboolean Java_com_daheiche_heiche_Jni_login(
        JNIEnv* env, jobject obj,
        jstring username, jstring password, jboolean isDriver,
        jdouble lat, jdouble lng)
{
    string cUser = j2c(env, username);
    string cPass = j2c(env, password);
    // 业务函数，分离业务函数和JNI的目的更好的跨平台
    return Login(cUser, cPass, (bool)isDriver,
    (double)lat, (double)lng);
}

//注册函数调用C函数
extern "C"
jboolean Java_com_daheiche_heiche_Jni_reg(JNIEnv* env, jobject obj, jstring username, jstring password)
{
    string cUser = j2c(env, username);
    string cPass = j2c(env, password);
    // 业务函数，分离业务函数和JNI的目的更好的跨平台
    return Reg(cUser, cPass);
}

//服务器返回的一串json报文
static void callback(string json_buf)
{
    //获取java中的类
    jclass clazz = gEnv->GetObjectClass(gObj);
    //获取类中的方法，函数签名
    jmethodID method = gEnv->GetMethodID(
                clazz, "nearbyDrivers", "(Ljava/lang/String;)V");
    //使用方法--回到Jni.java去调用了nearbyDrivers这个函数
    gEnv->CallVoidMethod(gObj, method, c2j(gEnv, json_buf));
}

//乘客获取附近司机的位置信息
extern "C"
jboolean Java_com_daheiche_heiche_Jni_getNearbyDrivers(
        JNIEnv* env, jobject obj, jstring username,
        jdouble lat, jdouble lng)
{
    //先给全局的java对象赋值，给回调函数使用
    if(gEnv == NULL)
    {
        gEnv = env;
        gObj = obj;
    }
    string cUser = j2c(env, username);
    // 业务函数，分离业务函数和JNI的目的更好的跨平台
    return GetNearbyDrivers(cUser, lat, lng, callback);
}

//错误信息打印函数
extern "C"
jstring Java_com_daheiche_heiche_Jni_lastError(JNIEnv* env, jobject obj)
{
    return c2j(env, lastError);
}
