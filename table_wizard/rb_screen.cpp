#include "rb_screen.h"

rb_screen::rb_screen(QWidget *parent) :
    QWidget(parent)
{


    setMinimumWidth(640);
    setMinimumHeight(400);
    setMaximumWidth(640);
    setMaximumHeight(400);

    mode = SCRN_MODE_RBARS;
}

void rb_screen::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.fillRect(this->rect(), QBrush(QColor(0,0,0)));

    if(rbars.isEmpty()) return;

    if(mode==SCRN_MODE_RBARS) {
        rb *rbar;

        for(int r=0; r<rbars.count(); r++) {
            rbar = rbars.at(r);

            for(int i=0; i<rbar->size; i++) {
                //p.setPen(QPen(rbar->colortbl.at(rbar->values[i])));
                p.setPen(QPen(rb_tools::colortbl.at(rbar->values[i])));

                p.drawLine(0,2*i + rbar->y*2,
                           width(),2*i + rbar->y*2);
                p.drawLine(0,2*i+1 + rbar->y*2,
                           width(),2*i+1+ rbar->y*2);
            }
        }
        return;
    }
}
