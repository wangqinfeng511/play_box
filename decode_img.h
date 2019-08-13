#ifndef DECODE_IMG_H
#define DECODE_IMG_H

#include <QObject>

#include <QObject>
#include <QQuickImageProvider>
#include <QImage>

class av_packet_open:public QObject{
    Q_OBJECT
public:
    explicit av_packet_open(QObject *parent=nullptr);

signals:
    void update_image();
public slots:
     void start();
};


class ImageSrc:public QQuickImageProvider
{
//    explicit adudio_out_device(QObject *parent = nullptr);

public:
    ImageSrc();
    ~ImageSrc(){}
    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize);
//    QPixmap requestPixmap(const QString &id,QSize *size,const QSize &requestedSize);

signals:

public slots:
};


#endif // DECODE_IMG_H
