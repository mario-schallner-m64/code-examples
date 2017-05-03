#ifndef RB_H
#define RB_H

#include <QObject>
#include <QColor>



class rb : public QObject
{
    Q_OBJECT
public:
    explicit rb(QObject *parent = 0);

    QString dump(int mode = 0, QString prefix = "BYTE", int max=8);

    static bool sort_rb_Z(rb *l, rb *r);

signals:

public slots:

public:
    int size;
    int y;
    int z;
    unsigned char values[512];
    QString name;

    unsigned char animations[256*256*16];

    unsigned char scrn_ani_buf[256];
};

#endif // RB_H
