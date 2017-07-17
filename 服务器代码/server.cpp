#include "server.h"

//优化：当客户端收到result：err报文时，提取一下reason信息，在界面展示
//优化：status未使用
//优化：客户端每次重新定位时，需要重写发给服务器的定位报文，服务器接收后获取新的name，lat，lng，geohash值，给用户hash表更新

Server::Server(QObject *parent) : QObject(parent)
{
    //连接数据库
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("sz03");
    db.setUserName("root");
    db.setPassword("123456");
    if(!db.open())
        exit(1);

    //监听客户端信号
    server.listen (QHostAddress::Any, 10999);

    //接收处理客户端发来的信号
    connect(&server,
            SIGNAL(requestReady(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)),
            this,
            SLOT(onRequestReady(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)));
}

//接收处理客户端发来的信号
void Server::onRequestReady(HttpServerRequest &request,
                            HttpServerResponse &response)
{
    //信号接收成功，返回信号告诉客户端连接成功,HTTP OK 协议层没问题
    response.writeHead(HttpResponseStatus::OK);

    //规定以post方式接收信息 判断业务层是否正确
    if(request.method() != "POST")
    {
        response.end("not POST protocal");
        return;
    }

    // if POST, wait data upload，继续连接，接收到end信号后说明数据接收完毕，开始处理数据
    connect(&request, &HttpServerRequest::end, [&](){
        onPostReady(request, response);
    });
}

//数据接收
void Server::onPostReady(HttpServerRequest &request, HttpServerResponse &response)
{
    // recv POST data，读取全部数据
    QByteArray data = request.readBody();
    // data is JSON format packet
    /*接收的报文格式
{
    cmd: login ? redister ? nearbyDriver ?
    ...
}
*/
    qDebug() << "data from client" << data;


    QJsonDocument doc = QJsonDocument::fromJson(data);
    //获取JSON根节点
    QJsonObject root = doc.object();

    //获得命令,将全部JSON内容传给求他函数处理
    QJsonObject resp;   //准备回复的报文
    QString cmd = root.value("cmd").toString();
    if(cmd == "register")
    {
        resp = reg(root);
    }
    else if(cmd == "login")
    {
        resp = login(root);
    }
    else if(cmd == "nearbyDriver")
    {
        resp = nearbyDrivers(root);
    }
    else
    {
        resp.insert("result", "err");
        resp.insert("reason", "unkown cmd");
    }

    // resp 打包成 QByteArray 发回给客户端
    {
        QJsonDocument doc(resp);
        QByteArray buf = doc.toJson();
        response.end(buf);      //发给客户端

        qDebug() << "buf back to client" << buf;
    }
}

//负责将注册信息写入mysql
/*接收的报文格式
{
    cmd:register
    username:xxx
    password:xxx
}
*/
QJsonObject Server::reg(const QJsonObject& req)
{
    QJsonObject resp;               //准备回复的报文
    resp.insert("cmd", "register"); //命令标识

    QString username = req.value("username").toString();
    QString password = req.value("password").toString();
    //密码转成md5字符串
    QString md5Password = md5(password);
    //组建sql语句
    QString sql = QString("insert into tuser (fusername, fpassword) values ('%1', '%2')")
            .arg(username, md5Password);
    //执行sql语句
    QSqlQuery query = db.exec(sql);
    if(query.lastError().type() != QSqlError::NoError)//检查语句执行是否成功
    {
        resp.insert("result", "err");
        resp.insert("reason", "add user error");
        return resp;
    }

    resp.insert("result", "ok");
    return resp;  
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
}

//负责将登录信息校验，并用redis保存临时信息
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
QJsonObject Server::login(const QJsonObject& req)
{   //=============mysql校验===================
    QJsonObject resp;               //准备回复的报文
    resp.insert("cmd", "login");    //命令标识

    QString username = req.value("username").toString();
    QString password = req.value("password").toString();
    QString type = req.value("type").toString();//"d" "p"
    double lat = req.value("lat").toDouble();
    double lng = req.value("lng").toDouble();
    int geohash = req.value("geohash").toInt();

    //密码转成md5字符串
    QString md5Password = md5(password);
    //组建sql语句
    QString sql = QString("select * from tuser where fusername = '%1' and fpassword = '%2'")
            .arg(username, md5Password);
    //执行sql语句
    QSqlQuery query = db.exec(sql);
    if(query.lastError().type() != QSqlError::NoError)//检查语句执行是否成功
    {
        resp.insert("result", "err");
        resp.insert("reason", "select user error");
        return resp;
    }
    //检查执行返回的结果--受影响的行的数量
    if(query.size() != 1)
    {
        resp.insert("result", "err");
        resp.insert("reason", "username or password error");
        return resp;
    }

    //===============redis保存临时信息===================
    /* redis中存储信息的集合
        redis: who are login
        d: user1 user2 user3 user4 ...
        p: user5 user6 user7 user8 ...

        user1: status xxxx lat xxxx lng xxx type d geohash xxx
        user5: status xxxx lat xxxx lng xxx type p geohash xxx
        ...

        geoposition1: driver1 driver2 driver3 ...
        geoposition2: driver5 driver6 driver7 ...
        ...
    */

    Redis redis;

    //redis先保存本次登陆的用户是司机还是乘客，d和p集合
    int setret = redis.addSet(type, username);
    if(0 != setret)
    {
        resp.insert("result", "err");
        resp.insert("reason", "redis.addSet error");
        return resp;
    }

    //redis再另外保存每一位用户的详细信息，如果用户太久没发位置信息过来，需要自动删除，用户名的集合
    int hashset = redis.addHash(username,
                              QStringList()<<"status"<<"lat"<<"lng"<<"type"<<"geohash",
                              QStringList()<<"0"<<QString::number(lat)<<
                              QString::number(lng)<<type<<QString::number(geohash),
                                10000);
    if(0 != hashset)
    {
        resp.insert("result", "err");
        resp.insert("reason", "redis.addHash error");
        return resp;
    }

    //redis最后再保存一份以geohash为键，司机的名字为值的集合
    if(type == "d")
    {
        qDebug() << "司机的geohash:" << geohash;
        setret = redis.addSet(QString::number(geohash), username);
        if(0 != setret)
        {
            resp.insert("result", "err");
            resp.insert("reason", "geohash redis.addSet error");
            return resp;
        }
    }

/* 增加哈希表 键 自动删除功能，以下函数废弃
#if 0
    //redis保存每位用户登陆时携带的信息
    QJsonObject obj;    //先用一个Json报文保存用户的信息
    obj.insert("status", "0");
    obj.insert("lat", "todo");
    obj.insert("lng", "todo");
    obj.insert("type", type);
    int hashuser = redis.addHash(username, obj);
    if(0 != hashuser)
    {
        resp.insert("result", "err");
        resp.insert("reason", "redis.addHash error");
        return resp;
    }
#else  //对以上函数用不定参数的形式优化
    //redis保存每位用户登陆时携带的信息
    int hashret = redis.addHash(username,
                                "status", "0",
                                "lat", QString::number(lat).toUtf8().data(),//数字转成char*
                                "lng", QString::number(lng).toUtf8().data(),
                                "type", type.toUtf8().data(),
                                NULL);
    if(0 != hashret)
    {
        resp.insert("result", "err");
        resp.insert("reason", "redis.addHash error");
        return resp;
    }
#endif
*/
    if (type == "p")
        qDebug() << "passage" << username << "online...";
    else if (type == "d")
        qDebug() << "driver" << username << "online...";

    resp.insert("result", "ok");
    return resp;
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
}

//回复给客户端附近的司机信息
/*接收的报文格式
{
    cmd:nearbyDrivers
    username:xxx    //订单中用到
    lat:xxx         //订单中用到
    lng:xxx         //订单中用到
    geohash:xxx
}
*/
QJsonObject Server::nearbyDrivers(const QJsonObject &req)
{
    QJsonObject resp;                       //准备回复的报文
    resp.insert("cmd", "nearbyDrivers");    //命令标识

    int geohash = req.value("geohash").toInt();
    qDebug() << "nearbyDrivers: passage geohash: " << geohash;

    //通过这个geohash键，从服务器的redis的hash表中获取司机的集合
    Redis redis;
    QJsonArray driverArr;                   //JSON报文，存储司机们的信息
    int onlineDriver = 0;                   //返回给客户端的在线司机个数
    static int offsetGeohash[] = {0, 1, -1, 2, -2}; //从5个geohash区域找司机
    //遍历geohash区域
    for(int i=0; i<(int)(sizeof(offsetGeohash)/sizeof(offsetGeohash[0])); ++i)
    {
        int geokey = geohash + offsetGeohash[i];

        qDebug() << "nearbyDrivers geokey=" << geokey;
        qDebug() << "driversList";

        //获取当前区域的所有司机名字
        QStringList driversList = redis.getSet(QString::number(geokey));

        qDebug() << "driversList.size()=" << driversList.size();

        //遍历司机列表，从用户的集合中提取详细信息，放到QJsonArray中
        foreach(QString driverName, driversList)
        {
            QJsonObject driverInfo;
            driverInfo.insert("name", driverName);
            driverInfo.insert("lat", redis.getHashMember(driverName, "lat").toDouble());
            driverInfo.insert("lng", redis.getHashMember(driverName, "lng").toDouble());

            driverArr.append(driverInfo);
            ++onlineDriver;
            if(onlineDriver > 11)
                break;                      //只返回10个司机
        }
        if(onlineDriver > 11)
            break;                      //只返回10个司机
    }

    if(onlineDriver < 1)
    {
        resp.insert("result", "err");
        resp.insert("reason", "no drivers nearby");
        return resp;
    }

    resp.insert("result", "ok");
    resp.insert("drivers", driverArr);
    return resp;

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
}

//密码转md5字符串
QString Server::md5(QString arg)
{
    // 16bytes --> 128bit
    QByteArray pwd;
    pwd = QCryptographicHash::hash(arg.toUtf8(), QCryptographicHash::Md5);
    return pwd.toHex();// 16bytes --> 32bytes 二进制编码转成字符串,0xf8a9 --> "f8a9"
}
