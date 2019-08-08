#ifndef API_H
#define API_H

#include <QObject>

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
