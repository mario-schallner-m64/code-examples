#ifndef PLAYLIST_ITEM_WIDGET_H
#define PLAYLIST_ITEM_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QLayout>

#include <scrn_effect.h>


class playlist_item_widget : public QWidget
{
    Q_OBJECT
public:
    explicit playlist_item_widget(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);
signals:

public slots:

public:
    scrn_effect *eff;
};

#endif // PLAYLIST_ITEM_WIDGET_H
