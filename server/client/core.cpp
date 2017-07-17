//
// Created by cly10 on 17/07/11.
//

#include "core.h"
//#include "cJSON.h"
//#include "curl/curl.h"
#include "json.h"
#include "curl.h"

string lastError;
static const char * url = "http://119.29.97.137:10999";

//===========注册函数==============
bool Reg(string username, string password)
{
    //组建Json报文
    /*发送的报文格式
    {
        cmd: register
        username: xxx
        password: xxx
    }
    */
    Json root;                      //自己封装的Json类的对象
    root.add("username", username); //拼装Json报文内容
    root.add("password", password);
    root.add("cmd", "register");

    //向服务器发送报文并接收回复
    //回复的报文储存在curl.resp_buf中
    Curl curl;
    CURLcode ret = curl.post(url,root.print());
    if (ret!=CURLE_OK)
    {
        MyError("curl error return = %d", (int)ret);
        lastError = "network error";
        return false;
    }

    //解析服务器返回的数据
    /*返回数据的报文格式：
    {
        cmd:register
        result:ok
    }
    或者：
    {
        cmd:register
        result:err
        reason:add user error
    }
    */
    Json resp;
    resp.parse(curl.resp_buf);          //将服务器的内容放在了curl对象的resp_buf中，取出来放到resp的root变量中
    string result = resp.getString("result");   //解析root中存储的报文
    if(result == "ok")
    {
        MyError("register ok");
        return true;
    }

    lastError = resp.getString("reason");
    MyError("register error:%s", lastError.c_str());
    return false;
}



//===========登陆函数============
bool Login(string username, string password,
           bool isDriver, double lat, double lng)
{
    //Json报文
    /*接收的报文格式
    {
        cmd:login
        username:xxx
        password:xxx
        type:q?d
        lat:xxx
        lng:xxx
        geohash:xxx
    }
    */
    Json root;
    root.add("username", username);
    root.add("password", password);
    root.add("cmd", "login");
    root.add("type", isDriver?"d":"p");
    root.add("lat", lat);
    root.add("lng", lng);

    int geohash = getGeohash(lat, lng);
    root.add("geohash", geohash);

    //向服务器发送报文并接收回复
    //回复的报文储存在curl.resp_buf中
    Curl curl;
    CURLcode ret =curl.post(url,root.print());
    //MyError("==========================================");
    if(ret != CURLE_OK)
    {
        MyError("curl error return = %d", (int)ret);
        lastError = "network error";
        return false;
    }

    //解析服务器返回的数据
    /*返回数据的报文格式：
    {
        cmd:login
        result:ok
    }
    或者：
    {
        cmd:login
        result:err
        reason:select user error或redis.addHash error或geohash redis.addSet error
    }
    */
    Json resp;
    resp.parse(curl.resp_buf);//将服务器的内容放在了curl对象的resp_buf中，取出来放到resp的root变量中
    string result = resp.getString("result");//解析root中存储的报文
    if(result == "ok")
    {
        MyError("Login ok");
        return true;
    }

    lastError = resp.getString("reason");
    MyError("Login error: %s", lastError.c_str());
    return false;
}

//===============乘客获取附近司机的位置信息===================
bool GetNearbyDrivers(string username, double lat, double lng,
                      void(*callback)(string))
{
    //Json报文
    /*接收的报文格式
    {
        cmd:nearbyDrivers
        username:xxx    //订单中用到
        lat:xxx         //订单中用到
        lng:xxx         //订单中用到
        geohash:xxx
    }
    */

    MyError("纬度：%g \n经度：%g ", lat, lng);

    Json req;
    req.add("cmd", "nearbyDriver");
    req.add("username", username);
    req.add("lat", lat);
    req.add("lng", lng);

    int geohash = getGeohash(lat, lng);
    req.add("geohash", geohash);

    //向服务器发送报文并接收回复
    //回复的报文储存在curl.resp_buf中
    Curl curl;
    CURLcode ret = curl.post(url, req.print());
    if (ret != CURLE_OK)    //网络通信错误
    {
        MyError("GetNearbyDrivers curl error return = %d", (int)ret);
        lastError = "GetNearbyDrivers network error";
        return false;
    }

    //解析服务器返回的数据
    /*返回的JSON报文格式：
    {
        cmd: nearbyDrivers,
        result: ok,
        drivers:
            [
                {
                    name: dirvername,
                    lat: 39.111,
                    lng: 119.111
                },
                {
                    name: drivername,
                    lat:xxx
                    lng:xxxx
                },
                ...
            ]//(<=10)
    }
    或者：
    {
        cmd:nearbyDrivers
        result:err
        reason:no drivers nearby
    }
    */
    Json resp;
    resp.parse(curl.resp_buf);//将服务器的内容放在了curl对象的resp_buf中，取出来放到resp的root变量中
    string result = resp.getString("result");//解析root中存储的报文
    //MyError("1+++++++++++++++++++++++++++++++++++++++%s", result.c_str());
    if (result == "ok")
    {
        MyError("GetNearbyDrivers ok");
        //回调函数返回整个报文给jni处理
        //MyError("2+++++++++++++++++++++++++++++++++++++++");
        callback(curl.resp_buf);
        return true;
    }
    
    lastError = resp.getString("reason");
    MyError("GetNearbyDrivers error: %s", lastError.c_str());
    return false;
}

//获取geohash值
static int getGeohash(double lat, double lng)
{
    uint32_t lngLatBits = 0;

    double lngMin = -180;
    double lngMax = 180;
    double latMin = -90;
    double latMax = 90;

    for(int i=0; i<15; ++i)
    {
        lngLatBits <<=1;

        double latMid = (latMax + latMin)/2;
        if(lat > latMid)
        {
            lngLatBits += 1;
            latMin = latMid;
        }
        else
        {
            latMax = latMid;
        }

        lngLatBits<<=1;

        double lngMid = (lngMax + lngMin)/2;
        if(lng > lngMid)
        {
            lngLatBits += 1;
            lngMin = lngMid;
        }
        else
        {
            lngMax = lngMid;
        }
    }
    return lngLatBits;
}


/*
代码版本v1.0，
舍弃原因：不符合低耦合高内聚要求
需要把JSON，Curl封装成类，完成内存自动释放、函数简易调用等功能

//获取libcurl服务器回应的回调函数
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
 string& str = *(string*)userdata;

 int len = size * nmemb;
 copy(ptr, ptr+len, back_inserter(str));

 return len;
}

//登陆C函数
bool Login(string username, string password)
{
 cJSON* root = cJSON_CreateObject();        //创建一个Json根节点
 //根节点添加数据
 cJSON_AddStringToObject(root, "username", username.c_str());
 cJSON_AddStringToObject(root, "password", password.c_str());
 cJSON_AddStringToObject(root, "cmd", "login");
 //JSON报文拷贝到字符串，并为buf申请了堆空间
 char* buf = cJSON_Print(root);
 cJSON_Delete(root);

 // curl(buf) --> server，利用libcurl向服务器发送数据
 CURL* curl = curl_easy_init();             //curl对象
 curl_easy_setopt(curl, CURLOPT_URL, "http://119.29.97.137:10099");
 curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);

 string str;                                // 实际是用来存储服务器返回的Json
 curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);//回调函数处理JSON报文
 curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

 CURLcode ret = curl_easy_perform(curl);    //执行以上curl_easy_setopt操作
 free(buf);

 if(ret != CURLE_OK)
 {
     MyError("curl error return = %d", (int)ret);
     lastError = "network error";
     return false;
 }

 MyError("curl perform ok");

 {
    //解析从服务器收到的JSON报文
    cJSON* root = cJSON_Parse(str.c_str());
    // result,
    // reason
    cJSON* result = cJSON_GetObjectItem(root, "result");//获得对应键的值
    if(result && strcmp(result->valuestring, "ok") == 0)
    {
        cJSON_Delete(root);
        MyError("login ok");
        return true;
    }
    else
    {
        cJSON* reason = cJSON_GetObjectItem(root, "reason");
        MyError("error is: %s", reason->valuestring);
        lastError = reason->valuestring;
    }

    cJSON_Delete(root);
 }

 return false;
}

//注册C函数
bool Reg(string username, string password)
{
 cJSON* root = cJSON_CreateObject();        //创建一个Json根节点
 //根节点添加数据
 cJSON_AddStringToObject(root, "username", username.c_str());
 cJSON_AddStringToObject(root, "password", password.c_str());
 cJSON_AddStringToObject(root, "cmd", "register");
 //JSON报文拷贝到字符串，并为buf申请了堆空间
 char* buf = cJSON_Print(root);
 cJSON_Delete(root);

 // curl(buf) --> server，利用libcurl向服务器发送数据
 CURL* curl = curl_easy_init();             //curl对象
 curl_easy_setopt(curl, CURLOPT_URL, "http://119.29.97.137:10099");
 curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);

 string str;                                // 实际是用来存储服务器返回的Json
 curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);//回调函数处理JSON报文
 curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

 CURLcode ret = curl_easy_perform(curl);    //执行以上curl_easy_setopt操作
 free(buf);
 //检查curl_easy_perform是否执行成功
 if(ret != CURLE_OK)
 {
     MyError("curl error return = %d", (int)ret);
     lastError = "network error";
     return false;
 }

 MyError("curl perform ok");

 {
    //解析从服务器收到的JSON报文
    cJSON* root = cJSON_Parse(str.c_str());
    // result,
    // reason
    cJSON* result = cJSON_GetObjectItem(root, "result");//获得对应键的值
    if(result && strcmp(result->valuestring, "ok") == 0)
    {
        cJSON_Delete(root);
        MyError("login ok");
        return true;
    }
    else
    {
        cJSON* reason = cJSON_GetObjectItem(root, "reason");
        MyError("error is: %s", reason->valuestring);
        lastError = reason->valuestring;
    }

    cJSON_Delete(root);
 }

 return false;
}
*/





