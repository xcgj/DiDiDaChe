#ifndef JSON_H
#define JSON_H

#include "jnidef.h"
#include "cJSON.h"

//============封装一个自己的Json类===============

class Json
{
public:
    Json();
    ~Json();

    //将键值对增加到JSON中
    void add(string key, string value); //用户名和密码
    void add(string key, double value); //经纬度坐标
    void add(string key, int value);    //geohash

    //解析报文到root中
    void parse(string json_buf);

    //用root提取报文中键的值
    string getString(string key);
    cJSON * getObject(string key);

    //将root中的json内容以字符串的形式打印到字符串中
    char * print();

private:
    //禁止Json对象发生拷贝
    Json(const Json&);
    Json& operator=(const Json&);

    cJSON * root;   //封装cJSON对象
    char * buf;     //json内容会打印到这个字符串中
};

#endif // JSON_H
