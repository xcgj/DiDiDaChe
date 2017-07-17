#include "json.h"

Json::Json()
{
    root = cJSON_CreateObject();
    buf = NULL;
}

Json::~Json()
{
    cJSON_Delete(root);
    if(buf)
        free(buf);
}

//=============将键值对增加到JSON中=============
//用户名和密码
void Json::add(string key, string value)
{
    cJSON_AddStringToObject(root, key.c_str(), value.c_str());
}

//经纬度坐标
void Json::add(string key, double value)
{
    cJSON_AddNumberToObject(root, key.c_str(), value);
}

//geohash
void Json::add(string key, int value)
{
    cJSON_AddNumberToObject(root, key.c_str(), value);
}

//=============解析报文到root中================
void Json::parse(string json_buf)
{
    cJSON * obj = cJSON_Parse(json_buf.c_str());
    if (obj == NULL)
        return;
    cJSON_Delete(root);
    root = obj;
}

//==========用root提取报文中键的值===========
string Json::getString(string key)
{   //提取键的值，值是单个字符串
    cJSON * obj = cJSON_GetObjectItem(root, key.c_str());
    return obj->valuestring;
}

cJSON *Json::getObject(string key)
{   //提取键的值，值是JSON，问题函数，实际是用来提起JSON数组，后来代码废弃，该函数并未使用
    cJSON * obj = cJSON_GetObjectItem(root, key.c_str());
    return obj;
}


//================将root中的json内容以字符串的形式打印到字符串中===============
char *Json::print()
{
    if(NULL != buf)
    {
        free(buf);
    }
    buf = cJSON_Print(root);
    return buf;
}
