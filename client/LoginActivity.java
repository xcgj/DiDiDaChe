package com.daheiche.heiche;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Toast;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationListener;

public class LoginActivity extends AppCompatActivity {

    boolean isDriver = false;
    private AMapLocationClient locationClient;
    private AMapLocation location;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);    //定位

        //绑定checkbox对象
        CheckBox box = (CheckBox)findViewById(R.id.checkbox);
        //设置监听器，监听勾选动作
        box.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isDriver = isChecked;
                //记录用户类型的信息
                Data.instance().isDriver = isChecked;
                //优化：可以将Data.instance().isDriver定义为全局变量，代替isDriver的功能
            }
        });

        //定位设置
        locationClient = new AMapLocationClient(this);
        //设置监听器，监听位置改变信息
        locationClient.setLocationListener(new AMapLocationListener() {
            @Override
            public void onLocationChanged(AMapLocation aMapLocation) {
                location = aMapLocation;
                Toast.makeText(getApplicationContext(), "定位成功", Toast.LENGTH_LONG).show();
                locationClient.stopLocation();
                locationClient.onDestroy(); //一旦获得地址后销毁，避免重复监听
            }
        });
        locationClient.startLocation();     //开始定位
    }

    //登陆和注册按钮点击的回调函数，处理用户名和密码信息
    public void onClick(View view)//view是按钮对象
    {
        String username = ((EditText)findViewById(R.id.username)).getText().toString();
        String password = ((EditText)findViewById(R.id.password)).getText().toString();
        if(username.length() == 0 || password.length() == 0)
            return;

        //等待定位成功的信号获取用户地址
        if (null == location)
        {
            Toast.makeText(this, "正在定位中...请稍等", Toast.LENGTH_LONG).show();
            return;
        }

        //登陆按钮
        if (view.getId() == R.id.button)
        {   //回调Jni的登陆函数
            if (!Jni.instance().login(username, password, isDriver,
                    location.getLatitude(), location.getLongitude()))
            {   //登陆不成功
                Log.e(Jni.tag, Jni.instance().lastError());
                return;
            }

            Data.instance().username = username;    //地图界面同步用户名

            //登陆成功，跳转页面到main
            Intent intent = new Intent(getApplication(), MainActivity.class);
            startActivity(intent);
            //后台结束
            finish();
        }

        //注册按钮
        else if (view.getId() == R.id.register)
        {   //回调Jni的注册函数
            if (!Jni.instance().reg(username, password))
            {
                Log.e(Jni.tag, Jni.instance().lastError());
            }
            else
            {
                //界面通知登陆成功，并提示：选择用户类型，按登陆按钮跳转页面
                Toast.makeText(this, "注册成功！请选择用户类型登陆", Toast.LENGTH_LONG).show();
            }
        }
        else
        {
            Log.e(Jni.tag, "未知按钮被点击");
        }
    }
}
