#include <QCoreApplication>
#include <QDebug>

#include "dataanalyzer.h"
#include <QtHttpServer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QHttpServer httpServer;

    httpServer.afterRequest([](QHttpServerResponse &&resp) {
        resp.setHeader("Access-Control-Allow-Origin", "http://rpi-server.fritz.box");
        return std::move(resp);
    });

    DataAnalyzer dataAnalyzer(&httpServer);

    return a.exec();
}
