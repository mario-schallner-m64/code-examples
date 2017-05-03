#ifndef RB_SCREEN_H
#define RB_SCREEN_H

#include <QWidget>
#include <QPainter>

#include <rb.h>
#include <rb_tools.h>

#define SCRN_MODE_RBARS 0
#define SCRN_MODE_ANI   1

class rb_screen : public QWidget
{
    Q_OBJECT
public:
    explicit rb_screen(QWidget *parent = 0);

    void paintEvent(QPaintEvent *);

signals:

public slots:

public:
    QList<rb *> rbars;

    int mode;
};

#endif // RB_SCREEN_H
