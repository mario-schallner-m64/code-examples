#include "rv_tooltip_widget.h"

rv_tooltip_widget::rv_tooltip_widget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vl = new QVBoxLayout();

    setStyleSheet("color: rgb(32, 32, 32);");

    vl->addWidget(&l);
    setLayout(vl);

    QFont my_font = font();
    my_font.setPointSize(9);
    my_font.setBold(true);
    l.setFont(my_font);
    l.setText("Destination disassembly");
    //l.setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    l.hide();

    pte = new QPlainTextEdit();
    pte->setFont(QFont("Liberation Mono", 8));
    pte->setWordWrapMode(QTextOption::NoWrap);
    pte->setStyleSheet("color: rgb(64, 64, 64);");
    pte->setTabStopWidth(60);

    vl->addWidget(&l);
    vl->addWidget(pte);

    color_bg   = QColor(0xff, 0xff, 0xff);
    color_line = QColor(0xa0, 0xa0, 0xc0);

    setFixedSize(400, 250);
    is_above = false;
}


void rv_tooltip_widget::setText(QString t)
{
    text = t;
    pte->setPlainText(t);
}

void rv_tooltip_widget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    QLinearGradient linearGrad(QPointF(0, 20), QPointF(20, height()));
         linearGrad.setColorAt(0, QColor(0xff, 0xff, 0xff));
         linearGrad.setColorAt(1, QColor(0xe0, 0xe0, 0xe0));

    //painter.fillRect(rect(), color_bg);
    painter.fillRect(rect(), linearGrad);

    // * painter.setPen(QPen(QBrush(color_line), 1, Qt::DashLine));
    //painter.setPen(QPen(QBrush(color_line), 1, Qt::SolidLine));
    // * painter.setPen(QPen(QBrush(QColor(0xc0, 0xc0, 0xc8)), 1, Qt::DashLine));
    painter.setPen(QPen(QBrush(QColor(0xa0, 0xa0, 0xc0).darker(120)), 1, Qt::DashLine));
    painter.drawRect(rect().x(), rect().y(),
                     rect().width()-1, rect().height()-1);

}
