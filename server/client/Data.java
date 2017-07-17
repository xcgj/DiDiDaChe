package com.daheiche.heiche;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by cly10 on 17/07/15.
 */

/*构造一个单例，用来辨别用户进入地图界面后，是司机还是乘客*/
public class Data {
    //单例对象
    private static Data data = null;
    //用户向服务器发送位置，服务器返回司机信息，存储到链表中
    static public Map<String, NearbyDriver> nearbyDrivers;

    //单例函数
    public static Data instance() {
        if (data == null) {
            data = new Data();
            nearbyDrivers = new HashMap<String, NearbyDriver>();
        }
        return data;
    }

    //用户类型，默认设置乘客类型
    public boolean isDriver = false;
    public String username;
}
