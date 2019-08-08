#ifndef IMAGE_SRC_H
#define IMAGE_SRC_H
#include <QObject>
#include <QQuickImageProvider>
#include <QImage>
#include "api.h"
extern QImage image_src;
extern QByteArray image_byte_array;
extern Api api;
class ImageSrc: public QQuickImageProvider
{
public:
    ImageSrc();
    ~ImageSrc(){};
    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize);
//    QPixmap requestPixmap(const QString &id,QSize *size,const QSize &requestedSize);

signals:

public slots:
};

#endif // IMAGE_SRC_H
