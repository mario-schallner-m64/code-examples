#include "rv_tag_cell_editor.h"

#include <QtGui>


rv_tag_cell_editor::rv_tag_cell_editor(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);
    setAutoFillBackground(true);

    mouse_tag_position = 0;
    my_va = 0;
    tag_processor = 0;
    tag_val = 0;
    setFixedSize(sizeHint());
}

void rv_tag_cell_editor::set_tag_processor(rv_tag_processor *p)
{
    tag_processor = p;
}

void rv_tag_cell_editor::set_tag_value(int v)
{
    tag_val = v;
}

void rv_tag_cell_editor::set_va(re_addr_t va)
{
    my_va = va;
    if(tag_processor)
        tag_val = tag_processor->get_va_tag_value(my_va);
}

QSize rv_tag_cell_editor::sizeHint() const
 {
     return QSize(16*5+2, 16);
 }

 void rv_tag_cell_editor::paintEvent(QPaintEvent *)
 {
     QPainter painter(this);

     // -- paintme --
     QPixmap *icon;

     painter.save();

     painter.fillRect(rect(), palette().highlight());

     int x1, y1, i = 0;
     int icon_w;

     icon_w = 16;
     x1 = rect().x();
     y1 = rect().x();



     QLinearGradient linearGrad(QPointF(0, 0), QPointF(width()-20, height()));
          linearGrad.setColorAt(0, QColor(0xff, 0xff, 0xff));
          linearGrad.setColorAt(1, QColor(0xe0, 0xe0, 0xe0));

     painter.fillRect(rect(), linearGrad);

     painter.setPen(QPen(QBrush(QColor(0xa0, 0xa0, 0xc0).darker(120)), 1, Qt::DashLine));
     painter.drawRect(rect().x(), rect().y(),
                      rect().width()-1, rect().height()-1);


     if( (tag_val & RV_TAG_YEL) || (i == mouse_tag_position) ) {
         icon = new QPixmap(":/icons/bookmark_small.png");
         if( (tag_val & RV_TAG_YEL) && (i == mouse_tag_position) )
             icon = new QPixmap(":/icons/bookmark_gray_kill_small.png");
     }
     else icon = new QPixmap(":/icons/bookmark_gray_small.png");
     painter.drawPixmap(x1 + 16 * i, y1, 16, 16, *icon); i++;

     if( (tag_val & RV_TAG_BLU) || (i == mouse_tag_position) ) {
         icon = new QPixmap(":/icons/bookmark_blu_small.png");
         if( (tag_val & RV_TAG_BLU) && (i == mouse_tag_position) )
             icon = new QPixmap(":/icons/bookmark_gray_kill_small.png");
     }
     else icon = new QPixmap(":/icons/bookmark_gray_small.png");
     painter.drawPixmap(x1 + 16 * i, y1, 16, 16, *icon); i++;

     if( (tag_val & RV_TAG_RED) || (i == mouse_tag_position) ) {
         icon = new QPixmap(":/icons/bookmark_red_small.png");
         if( (tag_val & RV_TAG_RED) && (i == mouse_tag_position) )
             icon = new QPixmap(":/icons/bookmark_gray_kill_small.png");
     }
     else icon = new QPixmap(":/icons/bookmark_gray_small.png");
     painter.drawPixmap(x1 + 16 * i, y1, 16, 16, *icon); i++;

     if( (tag_val & RV_TAG_GRN) || (i == mouse_tag_position) ) {
         icon = new QPixmap(":/icons/bookmark_grn_small.png");
         if( (tag_val & RV_TAG_GRN) && (i == mouse_tag_position) )
             icon = new QPixmap(":/icons/bookmark_gray_kill_small.png");
     }
     else icon = new QPixmap(":/icons/bookmark_gray_small.png");
     painter.drawPixmap(x1 + 16 * i, y1, 16, 16, *icon); i++;


     if( (tag_val & RV_TAG_BLK) || (i == mouse_tag_position) ) {
         icon = new QPixmap(":/icons/bookmark_black_small.png");
         if( (tag_val & RV_TAG_BLK) && (i == mouse_tag_position) )
             icon = new QPixmap(":/icons/bookmark_gray_kill_small.png");
     }
     else icon = new QPixmap(":/icons/bookmark_gray_small.png");
     painter.drawPixmap(x1 + 16 * i, y1, 16, 16, *icon); i++;

     painter.restore();
 }

 int rv_tag_cell_editor::get_new_tag_value()
 {
     tag_val = tag_val ^ (1 << mouse_tag_position);

     return tag_val;
 }

 void rv_tag_cell_editor::mouseMoveEvent(QMouseEvent *event)
 {
     mouse_tag_position = event->x() / 16;

     update();
 }

 void rv_tag_cell_editor::mouseReleaseEvent(QMouseEvent * /* event */)
 {
     int new_tag_val = get_new_tag_value();
     if(tag_processor) {
        tag_processor->set_va_tag_value(my_va, new_tag_val);
        emit editingFinished();
     }
 }

