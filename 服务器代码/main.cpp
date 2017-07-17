#include <QCoreApplication>
#include "server.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    new Server;
    return app.exec();
}
