#include "rb_displayer.h"

rb_displayer::rb_displayer(QWidget *parent) :
    QWidget(parent)
{
    rbar = 0;

    setMinimumWidth(100);
    setMinimumHeight(400);
    setMaximumWidth(100);
    setMaximumHeight(400);

    setMouseTracking(true);

    move = false;
    ani_mode = false;
}

void rb_displayer::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.fillRect(this->rect(), QBrush(QColor(0,0,0)));

    if(!rbar) return;

    if(move) {
        p.setPen(QPen(QColor(0xe0, 0xf0, 0xff)));
        p.drawRect(0,2*rbar->y-1,width()-1, 2*rbar->size+2);
//        for(int i=0; i<rbar->size; i++) {
//            p.setPen(QPen(rb_tools::colortbl.at(rbar->values[i])));

//            p.drawLine(1,2*i + rbar->y*2,
//                       width()-2,2*i + rbar->y*2);
//            p.drawLine(1,2*i+1 + rbar->y*2,
//                       width()-2,2*i+1+ rbar->y*2);
//        }
    }

    if(ani_mode) {
        for(int i=0; i<rbar->size; i++) {
            p.setPen(QPen(rb_tools::colortbl.at(
                              rbar->animations[256*256*ani_nr + 256*frame_nr + i])));

            p.drawLine(0,2*i + rbar->y*2,
                       width(),2*i + rbar->y*2);
            p.drawLine(0,2*i+1 + rbar->y*2,
                       width(),2*i+1+ rbar->y*2);
        }
    }
    else { // normal
        for(int i=0; i<rbar->size; i++) {
            p.setPen(QPen(rb_tools::colortbl.at(rbar->values[i])));

            p.drawLine(0,2*i + rbar->y*2,
                       width(),2*i + rbar->y*2);
            p.drawLine(0,2*i+1 + rbar->y*2,
                       width(),2*i+1+ rbar->y*2);
        }
    }
}

void rb_displayer::set_rb(rb *r)
{
    ani_mode = false;
    rbar = r; repaint();
}


void rb_displayer::mousePressEvent(QMouseEvent *e)
{
    if(!rbar) return;

    if( (e->pos().y()/2 >= rbar->y) &&  (e->pos().y()/2 <= (rbar->y+rbar->size)))
        move = true;
    update();
}

void rb_displayer::mouseReleaseEvent(QMouseEvent *e)
{
    move = false;
    update();
}

void rb_displayer::mouseMoveEvent(QMouseEvent *e)
{
    if(!move) return;

    rbar->y = e->pos().y()/2;

    if(rbar->y < 0) rbar->y = 0;

    update();
    emit y_changed(rbar->y);
}

void rb_displayer::display_ani(int nr, int frame)
{
    ani_nr = nr;
    frame_nr = frame;

    ani_mode = true;
    repaint();
}
