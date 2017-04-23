#include "rv_splitter_handle.h"

rv_splitter_handle::rv_splitter_handle(Qt::Orientation orientation, QSplitter *parent):
    QSplitterHandle(orientation, parent)
{
    // horizontal
    color_fg_h = QColor(0xff, 0xff, 0xff);
    color_bg_h = QColor(0x90, 0x90, 0xa0);
    color_bg_h2 = QColor(0x40, 0x40, 0x50);

    brush_fg_h = QBrush(color_fg_h);
    brush_bg_h = QBrush(color_bg_h);
    brush_bg_h2 = QBrush(color_bg_h2);

    pen_fg_h = QPen(brush_fg_h, 1, Qt::DotLine);
    pen_fg_h2 = QPen(brush_fg_h, 1, Qt::DotLine);

    // vertical
    color_fg_v = QColor(0xff, 0xff, 0xff);
    color_bg_v = QColor(0xd0, 0xd0, 0xd8);
    color_bg_v2 = QColor(0x80, 0x80, 0x88);

    brush_fg_v = QBrush(color_fg_v);
    brush_bg_v = QBrush(color_bg_v);
    brush_bg_v2 = QBrush(color_bg_v2);

    pen_fg_v = QPen(brush_fg_v, 1, Qt::DotLine);
    pen_fg_v2 = QPen(brush_fg_v, 1, Qt::DotLine);

    sub_splitter = false;
}

void rv_splitter_handle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    int x, y;


    if (orientation() == Qt::Horizontal) {
        if(!sub_splitter) {
            x = width() / 2;
            painter.fillRect(rect(), brush_bg_h);
            painter.setPen(pen_fg_h);
            painter.drawLine(x, 5 ,
                             x, rect().height() -5 );

            painter.fillRect(0, rect().height() / 2 - 25,
                             rect().width(), 2  * 25, brush_bg_h);


            painter.fillRect(0, rect().height() / 2 - 20,
                             rect().width(), 2  * 20, brush_bg_h2);

            painter.setPen(pen_fg_h2);
            painter.drawLine(x, rect().height() / 2 - 10,
                             x, rect().height() / 2 + 10);
        } else {
            x = width() / 2;
            painter.fillRect(rect(), brush_bg_v);
            painter.setPen(pen_fg_h);
            painter.drawLine(x, 5 ,
                             x, rect().height() -5 );

            painter.fillRect(0, rect().height() / 2 - 25,
                             rect().width(), 2  * 25, brush_bg_v);


            painter.fillRect(0, rect().height() / 2 - 20,
                             rect().width(), 2  * 20, brush_bg_v2);

            painter.setPen(pen_fg_h2);
            painter.drawLine(x, rect().height() / 2 - 10,
                             x, rect().height() / 2 + 10);
        }

    } else {
        y = height() / 2;
        x = width() / 2;
        painter.fillRect(rect(), brush_bg_v);
        painter.setPen(pen_fg_v);
        painter.drawLine(5, y ,
                         rect().width() -5, y);

        painter.fillRect(rect().width() / 2 - 25, 0,
                         2  * 25, rect().height(), brush_bg_v);


        painter.fillRect(rect().width() / 2 - 20, 0,
                         2  * 20, rect().height(), brush_bg_v2);

        painter.setPen(pen_fg_v2);
        painter.drawLine(rect().width() / 2 - 10, y,
                         rect().width() / 2 + 10, y);
    }
}
