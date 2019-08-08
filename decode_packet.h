#ifndef DECODE_PACKET_H
#define DECODE_PACKET_H
//#include <libavformat/avformat.h>
#include <QObject>

class decode_packet : public QObject
{
    Q_OBJECT
public:
    explicit decode_packet(QObject *parent = nullptr);
    void decode_master(QString file_path);
signals:
    void update_image();
public slots:
    void start(){
        decode_master("/home/wangqinfeng/下载/a.mp4");
    };
private:
};

#endif // DECODE_PACKET_H
