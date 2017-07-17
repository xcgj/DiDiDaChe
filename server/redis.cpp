#include "redis.h"

Redis::Redis()
{
    ctx = redisConnect("127.0.0.1",6379);
}

Redis::~Redis()
{
    redisFree(ctx);
}

//done
//将用户加入到 q 表 或者 d 表
int Redis::addSet(QString setType, QString username)
{
    //组建redis命令
    QString cmd = QString("sadd %1 %2").arg(setType.toUtf8().data(), username.toUtf8().data());
    //向redis数据库发送指令，并接收执行结果
    redisReply * ret = (redisReply *)redisCommand(ctx, cmd.toUtf8().data());
    if(ret == NULL)
    {
        return -1;
    }

    qDebug() << "reply type is: " << ret->type;
    qDebug() << "reply str is：" << ret->str;
    qDebug() << "reply int is: " << ret->integer;

    freeReplyObject(ret);
    return 0;
}

//done
//设置 哈希键 自动更新时间，避免被集合删除
void Redis::updataExpire(QString name, int timeout)
{
    redisReply * ret = (redisReply *)redisCommand(
                ctx, "expire %s %d", name.toUtf8().data(), timeout);
    if(ret)
        freeReplyObject(ret);
}

//done
//将服务器传来的数据存入哈希表中，设置键的生存时间
int Redis::addHash(QString name, QStringList keys, QStringList values, int timeout)
{
    int keyCount = keys.size();
    int valCount = values.size();
    if(valCount < keyCount)//检查链表元素个数
        return -1;

    for(int i=0; i<keyCount; ++i)
    {   //同时遍历两个链表，键值对放入hash集合
        QString key = keys[i];
        QString value = values[i];

        redisReply * ret = (redisReply *)redisCommand(
                    ctx, "hset %s %s %s",
                    name.toUtf8().data(),
                    key.toUtf8().data(),
                    value.toUtf8().data());
        if(ret) freeReplyObject(ret);
    }

    //设置、更新生存时间
    updataExpire(name, timeout);
    return 0;
}

//done
//从哈希表中获得指定用户的信息--主要用来定位显示司机的位置，也可用于获取：
//"status" << "lat" << "lng" << "type" << "geohash"
QString Redis::getHashMember(QString name, QString info)
{
    QString retStr = NULL;
    redisReply * ret = (redisReply *)redisCommand(
                ctx, "hget %s %s",
                name.toUtf8().data(),
                info.toUtf8().data());
    if(NULL == ret)
    {
        qDebug() << "error! getHashMember ret is null";
        return retStr;
    }

    qDebug() << "getHashMember ret type is:" << ret->type;

    if(ret->type == REDIS_REPLY_INTEGER)
        retStr = QString::number(ret->integer); //是整数就转字符串
    else
        retStr = ret->str;

    if(ret) freeReplyObject(ret);
    return retStr;
}

//done
//从同一个geohash集合中获得所有司机的名字--是一个数组
QStringList Redis::getSet(QString set)
{
    QStringList retList;
    //返回redis命令执行结果信息，是一个结构体
    redisReply * ret = (redisReply *)redisCommand(
                ctx, "smembers %s", set.toUtf8().data());
    if(NULL == ret)
    {
        qDebug() << "error! getSet ret is null";
        return retList;
    }

    qDebug() << "getSet: geohash get driver names array";

    //提取名字
    if(ret->type == REDIS_REPLY_ARRAY)
    {
        unsigned int arrayLen = ret->elements;  //数组长度
        for(unsigned int i=0; i<arrayLen; ++i)
        {   //struct redisReply **element; element是结构体指针
            redisReply * r = ret->element[i];   //提取单个结构体的信息
            retList.append(r->str);             //链表增加司机名字
            //if(r) freeReplyObject(r);         //尝试打开，看看retList会不会挂
        }
    }

    //if(ret) freeReplyObject(ret);             //尝试打开，看看retList会不会挂
    return retList;
}

#if 0
//将用户的其他信息用json的格式插入到对应用户名的哈希集合中
int Redis::addHash(QString name, QJsonObject obj)
{
    //先从json根节点获得所有的键
    QStringList keys = obj.keys();
    //再遍历键，逐个hset
    foreach (QString key, keys)
    {
        QString value = obj[key].toString();    //值
        //组建redis命令
        QString cmd = QString("hset %1 %2 %3").arg(name.toUtf8().data(), key, value);
        //向redis数据库发送指令，并接收执行结果
        redisReply * ret = (redisReply *)redisCommand(ctx, cmd.toUtf8().data());
        if(NULL == ret)
            continue;
        freeReplyObject(ret);
    }

    return 0;
}

//将用户的其他信息用json的格式插入到对应用户名的哈希集合中--不定参数
int Redis::addHash(QString name, ...)
{
    va_list ap;
    va_start(ap, name);
    while(1)
    {
        const char * key = va_arg(ap, const char *);
        if(NULL == key)
            break;
        const char * value = va_arg(ap, const char *);
        //组建redis命令
        QString cmd = QString("hmset %1 %2 %3").arg(name.toUtf8().data(), key, value);
        //向redis数据库发送指令，并接收执行结果
        redisReply * ret = (redisReply *)redisCommand(ctx, cmd.toUtf8().data());
        if(NULL == ret)
            continue;
        freeReplyObject(ret);
    }

    va_end(ap);
    return 0;
}
#endif
