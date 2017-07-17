package com.daheiche.heiche;

import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.amap.api.maps.AMap;
import com.amap.api.maps.MapView;
import com.amap.api.maps.model.BitmapDescriptorFactory;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.Marker;
import com.amap.api.maps.model.MarkerOptions;

import java.util.Map;

import static com.daheiche.heiche.Jni.tag;

public class MainActivity extends AppCompatActivity {

    private MapView mapView;    //地图对象
    private AMap aMap;          //操作地图对象
    private AMapLocation mylocation;                //我的当前位置，这个变量在另外的类里面，有点问题
    private AMapLocationClient locationClient;      //

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mapView = (MapView)findViewById(R.id.map);  //获得地图界面
        mapView.onCreate(savedInstanceState);       //初始化地图
        aMap = mapView.getMap();//绑定地图

        aMap.setMyLocationEnabled(true);            //获得当前位置

        //定位
        //locationClient = new AMapLocationClient(getApplicationContext());
        locationClient = new AMapLocationClient(this);

        //设置定位间隔，否则默认2秒重新定位一次
        AMapLocationClientOption locationClientOption = new AMapLocationClientOption();
        locationClientOption.setInterval(10000); //10秒一次
        locationClient.setLocationOption(locationClientOption);

        //设置位置监听器
        locationClient.setLocationListener(new AMapLocationListener() {
            @Override
            public void onLocationChanged(AMapLocation aMapLocation) {
                mylocation = aMapLocation;  //mylocation有问题
                //判断用户类型
                //如果是乘客，持续发送位置到服务器，并向服务器索要附近司机的位置
                if (Data.instance().isDriver == false)
                {
                    Log.e(tag, "==========passge lat============="+aMapLocation.getLatitude());
                    Log.e(tag, "===========passage lng============"+aMapLocation.getLongitude());
                    //检测周围的司机
                    Jni.instance().getNearbyDrivers(
                            Data.instance().username,   //用户名
                            aMapLocation.getLatitude(), //坐标信息
                            aMapLocation.getLongitude());

                    //显示周围的司机
                    // 获取司机表
                    Map<String, NearbyDriver> drivers = Data.instance().nearbyDrivers;
                    aMap.clear();
                    // 遍历司机表
                    for (Map.Entry<String, NearbyDriver> entry : drivers.entrySet()) {
                            NearbyDriver driver = entry.getValue();

                            MarkerOptions options = new MarkerOptions();
                            options.position(new LatLng(driver.lat, driver.lng));
                            options.title("黑车");
                            options.snippet(driver.name);
                            options.alpha((float) 0.5);//半透明
                        options.icon(BitmapDescriptorFactory.fromBitmap(
                                BitmapFactory.decodeResource(getResources(), R.drawable.car)
                        ));
                        Marker marker = aMap.addMarker(options);
                    }
                }
                else    //如果是司机，暂时不作动作（除了继续发送位置信息）
                {

                }

            }
        });
        locationClient.startLocation();//开始定位
    }

    //关联生命周期
    @Override
    protected void onDestroy() {
        super.onDestroy();
        mapView.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mapView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mapView.onSaveInstanceState(outState);
    }
}
