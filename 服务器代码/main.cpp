#include <QCoreApplication>
#include <QProcess>
#include "server.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    //设置服务器进程可切换后台 -d切换，不加-d不切换
    if(argc > 1 && QString(argv[1] == "-d"))
    {
        QProcess process;
        process.startDetached(argv[0]);
        return 0;
    }

    new Server;
    return app.exec();
}
