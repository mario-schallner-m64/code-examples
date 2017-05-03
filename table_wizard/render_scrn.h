#ifndef RENDER_SCRN_H
#define RENDER_SCRN_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

#include <rb_tools.h>

class render_scrn : public QWidget
{
    Q_OBJECT
public:
    explicit render_scrn(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

signals:

public slots:

public:
    int frame_pos;
    unsigned char the_result[256*256];
};

#endif // RENDER_SCRN_H
