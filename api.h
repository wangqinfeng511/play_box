#ifndef API_H
#define API_H

#include <QObject>
#include <QImage>
extern QImage image_src;
extern QByteArray image_byte_array;
extern float sleep_time;
class Api : public QObject
{
    Q_OBJECT
public:
    explicit Api(QObject *parent = nullptr);

signals:
    void update_image();
public slots:
};

#endif // API_H
