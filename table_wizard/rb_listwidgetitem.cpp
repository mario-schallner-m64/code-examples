#include "rb_listwidgetitem.h"

rb_listwidgetitem::rb_listwidgetitem(QListWidget *parent, int t) :
    QListWidgetItem(parent, t)
{
    rbar = 0;
    setFlags(flags()| Qt::ItemIsUserCheckable);
    setCheckState(Qt::Unchecked);
}


void rb_listwidgetitem::set_rbar(rb *r)
{
    rbar = r;
    setText(r->name);

    QPixmap *qpm = new QPixmap(16,16);
    QPainter *p = new QPainter(qpm);

    p->fillRect(qpm->rect(),QBrush(Qt::white));

    if(rbar->size <= 8)
        for(int i=0; i<rbar->size; i++) {           
            p->setPen(QPen(rb_tools::colortbl.at(rbar->values[i])));
            p->drawLine(0,2*i,15,2*i);
            p->drawLine(0,2*i+1,15,2*i+1);
        }
    else
        for(int i=0; i<rbar->size; i++) {
            if(i==16) break;            
            p->setPen(QPen(rb_tools::colortbl.at(rbar->values[i])));
            p->drawLine(0,i,15,i);
        }

    setIcon(QIcon(*qpm));
}
