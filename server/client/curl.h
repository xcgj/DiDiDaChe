#ifndef CURL_H
#define CURL_H

#include "jnidef.h"
#include "curl/curl.h"

//============封装一个自己的Curl类===============

class Curl
{
public:
    Curl();
    ~Curl();

    //向服务器发送的方法
    CURLcode get(string url);                           //连接服务器
    CURLcode post(string url, char * data, int len = 0);//向服务器发送数据

    //接收服务器返回的数据
    string resp_buf;

private:
    //禁止Curl对象发生拷贝
    Curl(const Curl&);
    Curl& operator=(const Curl&);
    //封装公共的opt函数
    void setopt(const string & url);
    //回调函数
    static size_t callback1(char *ptr, size_t size, size_t nmemb, void *userdata);
    size_t callback(char *ptr, size_t size, size_t nmemb);

    CURL * curl;    //从源curl获得一个curl对象
};

#endif // CURL_H
