#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <QJsonObject>
#include <QDebug>
#include <QStringList>
#include <QString>

//============封装一个自己的Redis类===============

class Redis
{
public:
    Redis();
    ~Redis();

    //将用户加入到 q 集合 或者 d 集合
    int addSet(QString setType, QString username);

#if 0   //废弃原因：不能设置自动销毁
    //将用户的其他信息用json的格式插入到对应用户名的哈希集合中
    int addHash(QString name, QJsonObject obj);
    //将用户的其他信息用json的格式插入到对应用户名的哈希集合中--不定参数
    int addHash(QString name, ...);
#endif

    //设置 哈希键 自动更新时间，避免被集合删除
    void updataExpire(QString name, int timeout = 10);

    //将服务器传来的数据存入哈希表中，设置键的生存时间，数据传入格式：
    /*QStringList()<< "status" << "lat" << "lng" << "type" << "geohash",
    QStringList() << "0" << QString::number(lat) << QString::number(lng) << type << QString::number(geohash));*/
    int addHash(QString name, QStringList keys, QStringList values, int timeout = 10);

    //从哈希表中获得指定用户的信息--主要用来定位显示司机的位置，也可用于获取：
    //"status" << "lat" << "lng" << "type" << "geohash"
    QString getHashMember(QString name, QString info);

    //从同一个geohash集合中获得所有司机的名字--是一个json集合的数组
    QStringList getSet(QString set);

private:
    //redis对象
    redisContext * ctx;
};

#endif // REDIS_H
