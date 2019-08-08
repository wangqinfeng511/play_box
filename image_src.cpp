#include "image_src.h"
#include <QQuickImageProvider>
#include <QPixmap>
#include <QByteArray>;
#include "api.h"
#include <QDebug>
QImage image_src;
QByteArray image_byte_array;
Api api;
ImageSrc::ImageSrc():QQuickImageProvider(QQuickImageProvider::Image){};
QImage ImageSrc::requestImage(const QString &id,QSize *size,const QSize &requestedSize){
    qDebug()<<"image update";
    return image_src;
}
//QPixmap ImageSrc::requestPixmap(const QString &id,QSize *size,const QSize &requestedSize){
//    qDebug()<<"chufa";
//    QPixmap tmp_pix_map;
//    tmp_pix_map.loadFromData(image_byte_array);
//    return  tmp_pix_map;
//}

