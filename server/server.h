
#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <tufao-1/Tufao/HttpServer>
#include <tufao-1/Tufao/HttpServerRequest>
#include <tufao-1/Tufao/HttpServerResponse>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QCryptographicHash>

#include <hiredis/hiredis.h>
#include "redis.h"

using namespace Tufao;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

    QSqlDatabase db;    //数据库对象

    HttpServer server;  //Tufao的socket对象

    //处理数据,识别客户端的请求是哪一种
    void onPostReady(HttpServerRequest& request,
                         HttpServerResponse& response);

    //注册
    QJsonObject reg(const QJsonObject& req);
    //登陆
    QJsonObject login(const QJsonObject& req);
    //回复给客户端附近的司机信息
    QJsonObject nearbyDrivers(const QJsonObject& req);

    //密码转md5字符串
    QString md5(QString arg);

public slots:
    //接收信号，判断信号，并调用函数处理信号中的数据
    void onRequestReady(Tufao::HttpServerRequest& request,
                            Tufao::HttpServerResponse& response);
};

#endif // SERVER_H
