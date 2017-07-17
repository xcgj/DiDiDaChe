#include "curl.h"

Curl::Curl()
{
    curl = curl_easy_init();
}

Curl::~Curl()
{
    curl_easy_cleanup(curl);
}

//===============回调函数==================
size_t Curl::callback1(char *ptr, size_t size,
                       size_t nmemb, void *userdata)
{   //静态函数中定义了一个临时对象，调用了成员函数处理数据
    //这个临时对象保存了调用了这个静态函数的对象本身
    Curl * This = (Curl *)userdata;
    return This->callback(ptr, size, nmemb);
    //MyError("11111111111111111");
}

size_t Curl::callback(char *ptr, size_t size, size_t nmemb)
{
    //***将服务器返回的数据ptr拷贝到成员变量resp_buf中***
    copy(ptr, ptr+size*nmemb, back_inserter(resp_buf));
    //MyError("2222222222222222222222222");
    //MyError("resp_buf: %s", ptr);
    //MyError("resp_buf: %s", resp_buf.c_str());
    return size*nmemb;
}

//===============封装公共的opt函数==============
void Curl::setopt(const string &url)
{
    resp_buf.clear();
    //MyError("url:%s CURLE_OK:%d", url.c_str(), CURLE_OK);
    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL,url.c_str());   //连接服务器
    //MyError("1+++++++++++++++++++++++++++++++++++++++ %d", ret);
    ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl::callback1);//回调函数接收服务器返回的数据
    //MyError("2+++++++++++++++++++++++++++++++++++++++ %d", ret);
    ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);    //回调函数中传入对象本身，方便resp_buf接收数据
    //MyError("3+++++++++++++++++++++++++++++++++++++++ %d", ret);
}

//==============向服务器发送的方法================
//连接服务器
CURLcode Curl::get(string url)
{
    setopt(url);
    CURLcode ret =curl_easy_perform(curl);
    return ret;
}

//向服务器发送json数据
CURLcode Curl::post(string url, char *data, int len)
{
    setopt(url);
    CURLcode ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    if(len != 0)    //发送char*字符串不需要发送长度
    {   //如果发送的是二进制数据，需要发送长度
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
    }
    //MyError("4+++++++++++++++++++++++++++++++++++++++ %d %s", ret, data);
    ret = curl_easy_perform(curl);
    //MyError("5+++++++++++++++++++++++++++++++++++++++ %d", ret);
    //MyError("6 resp_buf: %s", resp_buf.c_str());
    return ret;
}


