#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "decode_packet.h"
#include "decode_img.h"
#include <QThread>
#include <QQmlContext>
#include "api.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    ImageSrc *image_src=new ImageSrc();
    Api api;
    engine.addImageProvider("image_src",image_src);
    //音频
    decode_packet decode_p;
    QThread *adudio_thread=new QThread();
    decode_p.moveToThread(adudio_thread);
    QObject::connect(adudio_thread,SIGNAL(started()),&decode_p,SLOT(start()));
    adudio_thread->start();
    //视频
     av_packet_open viduio_process ;
     QThread * viduio_thread=new QThread();
     viduio_process.moveToThread(viduio_thread);
     QObject::connect(&viduio_process,SIGNAL(update_image()),&api,SIGNAL(update_image()));
     QObject::connect(viduio_thread,SIGNAL(started()),&viduio_process,SLOT(start()));
    viduio_thread->start();

    engine.rootContext()->setContextProperty("Api",&api);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
