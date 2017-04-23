#ifndef RV_SPLITTER_HANDLE_H
#define RV_SPLITTER_HANDLE_H

#include <QSplitterHandle>
#include <QPainter>
#include <QPaintEvent>
#include <QImage>

class rv_splitter_handle : public QSplitterHandle
{
    Q_OBJECT
public:
    explicit rv_splitter_handle(Qt::Orientation orientation, QSplitter *parent = 0);
    bool sub_splitter;

protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
    QColor color_fg_h;
    QColor color_bg_h;
    QColor color_bg_h2;
    QBrush brush_fg_h;
    QBrush brush_bg_h;
    QBrush brush_bg_h2;
    QPen pen_fg_h;
    QPen pen_fg_h2;
    QPen pen_bg_h;

    QColor color_fg_v;
    QColor color_bg_v;
    QColor color_bg_v2;
    QBrush brush_fg_v;
    QBrush brush_bg_v;
    QBrush brush_bg_v2;
    QPen pen_fg_v;
    QPen pen_fg_v2;
    QPen pen_bg_v;    
};

#endif // RV_SPLITTER_HANDLE_H
