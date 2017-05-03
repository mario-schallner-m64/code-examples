#include "rb_color_picker.h"

rb_color_picker::rb_color_picker(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);
    setMinimumWidth(100);
    setMinimumHeight(60);

    selected_color_nr = 0;
    hover = 0;
}

void rb_color_picker::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    //p.fillRect(this->rect(), QColor(0,0,0));
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(width()-20, height()));
             linearGrad.setColorAt(0, QColor(0x80, 0x80, 0x90));
             linearGrad.setColorAt(1, QColor(0x40, 0x40, 0x50));

    p.fillRect(rect(), linearGrad);
    p.setPen(QColor(0,0,0));
    p.drawRect(0,0,width(), height()-1);

    for(int i=0; i<8; i++) {
        if(i==selected_color_nr)
            p.fillRect(PADDING + i*((width()-2*PADDING)/8),
                       PADDING,
                       ((width()-2*PADDING)/8)-CELLSPACING,
                       height()/2 - PADDING - CELLSPACING,
                       QColor(0xff, 0xff, 0xff));
        else
            if(i==hover)
                p.fillRect(PADDING + i*((width()-2*PADDING)/8),
                           PADDING,
                           ((width()-2*PADDING)/8)-CELLSPACING,
                           height()/2 - PADDING - CELLSPACING,
                           QColor(0xd0, 0xd0, 0xff));
            else
                p.fillRect(PADDING + i*((width()-2*PADDING)/8),
                           PADDING,
                           ((width()-2*PADDING)/8)-CELLSPACING,
                           height()/2 - PADDING - CELLSPACING,
                           QColor(0x00, 0x00, 0x00));

        p.fillRect(PADDING + i*((width()-2*PADDING)/8) + BORDER,
                   PADDING + BORDER,
                   ((width()-2*PADDING)/8)-CELLSPACING-2*BORDER,
                   height()/2 - PADDING - CELLSPACING-2*BORDER,
                   rb_tools::colortbl.at(i));
    }

    for(int i=0; i<8; i++) {
        if((i+8)==selected_color_nr)
            p.fillRect(PADDING + i*((width()-2*PADDING)/8),
                       height()/2,
                       ((width()-2*PADDING)/8)-CELLSPACING,
                       height()/2 - PADDING - CELLSPACING,
                       QColor(0xff, 0xff, 0xff));
        else
            if((i+8)==hover)
                p.fillRect(PADDING + i*((width()-2*PADDING)/8),
                           height()/2,
                           ((width()-2*PADDING)/8)-CELLSPACING,
                           height()/2 - PADDING - CELLSPACING,
                           QColor(0xd0, 0xd0, 0xff));
                else
                p.fillRect(PADDING + i*((width()-2*PADDING)/8),
                           height()/2,
                           ((width()-2*PADDING)/8)-CELLSPACING,
                           height()/2 - PADDING - CELLSPACING,
                           QColor(0x00, 0x00, 0x00));

        p.fillRect(PADDING + i*((width()-2*PADDING)/8) + BORDER,
                   height()/2+BORDER,
                   ((width()-2*PADDING)/8)-CELLSPACING-2*BORDER,
                   height()/2 - PADDING - CELLSPACING-2*BORDER,
                   rb_tools::colortbl.at(i+8));
    }
}

void rb_color_picker::mouseMoveEvent(QMouseEvent *e)
{
    int old_hover = hover;

    if(e->pos().y() < (height()/2)) {
        hover = e->pos().x()/(width()/8);
    }
    else {
        hover = e->pos().x()/(width()/8) + 8;
    }

    if(old_hover != hover) repaint();

}

void rb_color_picker::mousePressEvent(QMouseEvent *e)
{
    int old_selected = selected_color_nr;
    selected_color_nr = hover;

    if(old_selected != selected_color_nr) {
        repaint();
    }
    emit color_selected(selected_color_nr);
}
