#include "rv_tooltip_xrefs.h"

rv_tooltip_xrefs::rv_tooltip_xrefs(QWidget *parent) :
    QWidget(parent)
{
    setStyleSheet("color: rgb(32, 32, 32);");

    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget(&l);

    hl->setContentsMargins(6, 0, 6, 0);
    l.setFont(QFont("Liberation Mono", 7));
    l.setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    setLayout(hl);
    setFixedSize(100, 20);
}

void rv_tooltip_xrefs::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(width()-20, height()));
         linearGrad.setColorAt(0, QColor(0xff, 0xff, 0xff));
         linearGrad.setColorAt(1, QColor(0xe0, 0xe0, 0xe0));

    painter.fillRect(rect(), linearGrad);

    painter.setPen(QPen(QBrush(QColor(0xa0, 0xa0, 0xc0).darker(120)), 1, Qt::DashLine));
    painter.drawRect(rect().x(), rect().y(),
                     rect().width()-1, rect().height()-1);
}

void rv_tooltip_xrefs::setText(QString s)
{
    l.setText(s);
}
