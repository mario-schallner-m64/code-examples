#include "rb_table_widget.h"

rb_table_widget::rb_table_widget(QWidget *parent) :
    QWidget(parent)
{
    nr = 0;
    color = QColor(0x80,0x80,0xa0);
    setMinimumHeight(8);
    setMinimumWidth(8);

    ani_nr = -1;
}

void rb_table_widget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if(ani_nr >= 0) {
        p.fillRect(ANI_SPACER+8, 0, width()-8-ANI_SPACER, height(), color);
        p.fillRect(ANI_SPACER+10, 2 ,
                   80, height()-4, QColor(0x80, 0xa0, 0xa0));
        p.setPen(QPen(QColor(0xff, 0xff, 0xff)));
        p.drawText(ANI_SPACER+13, 13,
                   QString::number(ani_nr) + ":" +
                   QString::number(ani_frame)+ ":" +
                   QString::number(nr)
                   );
    }
    else {
        p.fillRect(SPACER+8, 0, width()-8-SPACER, height(), color);
        p.fillRect(SPACER+10, 2 ,
                   30, height()-4, QColor(0x80, 0x80, 0x80));
        p.setPen(QPen(QColor(0xff, 0xff, 0xff)));
        p.drawText(SPACER+13, 13, QString::number(nr));
    }

    QWidget::paintEvent(e);
}
