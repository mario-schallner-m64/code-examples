#ifndef RB_DISPLAYER_H
#define RB_DISPLAYER_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

#include <rb.h>
#include <rb_tools.h>


class rb_displayer : public QWidget
{
    Q_OBJECT
public:
    explicit rb_displayer(QWidget *parent = 0);

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

signals:
    void y_changed(int y);

public slots:
    void set_rb(rb *r);
    void display_ani(int nr, int frame);


public:
    rb *rbar;
    int ani_nr;
    int frame_nr;

    bool move;
    bool ani_mode;
};

#endif // RB_DISPLAYER_H
