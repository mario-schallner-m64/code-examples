#ifndef RB_TABLE_WIDGET_H
#define RB_TABLE_WIDGET_H

#include <QWidget>
#include <QPainter>

#include <rb_tools.h>

#define SPACER 5
#define ANI_SPACER 5

class rb_table_widget : public QWidget
{
    Q_OBJECT
public:
    explicit rb_table_widget(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

signals:

public slots:

public:
    int nr;
    int ani_nr;
    int ani_frame;
    QColor color;
};

#endif // RB_TABLE_WIDGET_H
