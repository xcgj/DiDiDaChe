package com.daheiche.heiche;

import android.util.Log;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;


/**
 * Created by cly10 on 17/07/11.
 */

public class Jni {
    static {
        System.loadLibrary("jni"); // libjni.so libxxx.so
    }

    static public final String tag = "heiche";
    static Jni This = new Jni();
    static public Jni instance()
    {
        return This;
    }

    //用户登录函数
    public native boolean login(String username, String password, boolean isDriver, double lat, double lng);
    //用户注册函数
    public native boolean reg(String username, String password);
    //司机获取函数
    public native boolean getNearbyDrivers(String username, double lat, double lng);
    //调试信息打印函数
    public native String lastError();

    //乘客周围有司机后，C++回调java函数依次获取司机结构体存储到链表中，链表放在Data中方便使用
/*    public void nearbyDriver(String name, double lat, double lng)
    {
        NearbyDriver driver = new NearbyDriver();
        driver.name = name;
        driver.lat = lat;
        driver.lng = lng;
        Data.instance().nearbyDrivers.put(name, driver);
    }*/
    public void nearbyDrivers(String json)  //直接调试打印服务器报文
    {
        //Log.e(tag, "nearby Drivers:" + json);
        //获取整个JSON报文
        JSONObject jobj = JSONObject.fromString(json);
        //只提取"drivers"键的json数组值
        JSONArray jary = jobj.getJSONArray("drivers");
        int driversNum = jary.length();
        Log.e(tag, "数组中司机人数的总数为："+driversNum);
        for (int i=0; i<driversNum; ++i)
        {
            //临时存储每个司机信息的一个结构体
            NearbyDriver driver = new NearbyDriver();//项目出错点，之前将这个结构体放在了for循环外，导致总是获取最后一个司机信息
            //从数组中依此提取每个司机的信息
            JSONObject EachDriverInfo = jary.getJSONObject(i);
            driver.name = EachDriverInfo.getString("name");
            driver.lat = EachDriverInfo.getDouble("lat");
            driver.lng = EachDriverInfo.getDouble("lng");
            Log.e(tag, "司机："+driver.name+"纬度："+driver.lat+"经度："+driver.lng);
            if(driver.lat != 0)
                //由于geohash集合不会自动删除，而用户哈希集合会自动删除，
                // 会导致geohash集合里的已经下线的司机没有经纬度数据
                // 筛选，将有有效经纬度的司机加入链表
            {
                Data.instance().nearbyDrivers.put(driver.name, driver);
            }
        }
    }
}
