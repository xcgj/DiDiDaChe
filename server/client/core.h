//
// Created by cly10 on 17/07/11.
//

#ifndef HEICHE_CORE_H
#define HEICHE_CORE_H

#include "jnidef.h"

extern string lastError;

//登陆
bool Login(string username, string password,
           bool isDriver, double lat, double lng);

//注册
bool Reg(string username, string password);

//乘客获取附近司机的位置信息
bool GetNearbyDrivers(string username, double lat, double lng,
                      void(*callback)(string));//回调函数，参数是字符串

//获取geohash值
static int getGeohash(double lat, double lng);

#endif //HEICHE_CORE_H
