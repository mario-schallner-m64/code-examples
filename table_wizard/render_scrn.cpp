#include "render_scrn.h"

render_scrn::render_scrn(QWidget *parent) :
    QWidget(parent)
{
    setMinimumWidth(642);
    setMinimumHeight(400);
    setMaximumWidth(642);
    setMaximumHeight(400);

    frame_pos = 0;

    for(int i=0; i < 256*256; i++) {
        the_result[i] = 0;
    }
}

void render_scrn::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    p.fillRect(this->rect(), QBrush(QColor(0x80,0x80,0xa0)));
    p.fillRect(1,1, width()-2, height()-2, QBrush(QColor(0x00,0x00,0x00)));

    for(int i=0; i<256; i++) {
        p.setPen(QPen(rb_tools::colortbl.at(the_result[256*frame_pos+i])));

        p.drawLine(0,2*i,
                   width(),2*i);
        p.drawLine(0,2*i+1 ,
                   width(),2*i+1);
    }
}
