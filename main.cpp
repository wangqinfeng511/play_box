#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "decode_packet.h"
#include "image_src.h"
#include <QThread>
#include <QQmlContext>
#include "api.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    ImageSrc *image_src=new ImageSrc();
    engine.addImageProvider("image_src",image_src);
    decode_packet decode_p;
    QObject::connect(&decode_p,SIGNAL(update_image()),&api,SIGNAL(update_image()));
    QThread *decode_thread=new QThread();
    decode_p.moveToThread(decode_thread);
    QObject::connect(decode_thread,SIGNAL(started()),&decode_p,SLOT(start()));
    decode_thread->start();
//    decode_p->decode_master("/home/wangqinfeng/下载/a.mp4");
//    decode_p.decode_master("/home/wangqinfeng/桌面/郭静_心墙.mp3");
    engine.rootContext()->setContextProperty("Api",&api);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
