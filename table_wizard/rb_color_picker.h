#ifndef RB_COLOR_PICKER_H
#define RB_COLOR_PICKER_H

#include <QWidget>
#include <QPainter>
#include <QGradient>
#include <QMouseEvent>

#include <rb_tools.h>

#define PADDING 4
#define CELLSPACING 4
#define BORDER 2

class rb_color_picker : public QWidget
{
    Q_OBJECT
public:
    explicit rb_color_picker(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);

signals:
    void color_selected(int c);

public slots:

public:
    int selected_color_nr;
    int hover;
};

#endif // RB_COLOR_PICKER_H
