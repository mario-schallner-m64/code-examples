#include "rv_slider.h"

rv_slider::rv_slider(Qt::Orientation orientation, QWidget *parent) :
    QSlider(orientation, parent)
{
}

void rv_slider::mouseReleaseEvent(QMouseEvent *ev)
{
    emit mouse_release();
}

void rv_slider::paintEvent(QPaintEvent *ev)
{
    //p.fillRect(1, 1, rect().width()-2, rect().height()-2, Qt::lightGray);
    QSlider::paintEvent(ev);
    QPainter p(this);
    p.setPen(QPen(QBrush(QColor(112, 87, 130)), 3, Qt::DashLine));
    p.drawRect(0,0,rect().width()-1, rect().height()-1);
}
