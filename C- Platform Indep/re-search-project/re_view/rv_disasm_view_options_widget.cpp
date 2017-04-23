#include "rv_disasm_view_options_widget.h"

rv_disasm_view_options_widget::rv_disasm_view_options_widget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hl = new QHBoxLayout();

    QFont mf = QFont();
    mf.setPointSize(8);
    setFont(mf);

    checkbox = new QCheckBox("offsets");
    hl->addWidget(checkbox);
    hl->addStretch();

    QLabel *l = new QLabel("#bytes:");
    hl->addWidget(l);
    spinbox = new QSpinBox();
    spinbox->setMinimum(0);
    spinbox->setMaximum(20);

    hl->addWidget(spinbox);

    setLayout(hl);
}

void rv_disasm_view_options_widget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(width()-20, height()));
         linearGrad.setColorAt(0, QColor(0xff, 0xff, 0xff));
         linearGrad.setColorAt(1, QColor(0xe0, 0xe0, 0xe0));

    painter.fillRect(rect(), linearGrad);

    painter.setPen(QPen(QBrush(QColor(0xa0, 0xa0, 0xc0).darker(120)), 1, Qt::SolidLine));
    painter.drawRect(rect().x(), rect().y(),
                     rect().width()-1, rect().height()-1);
}
