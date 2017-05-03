#include "playlist_item_widget.h"

playlist_item_widget::playlist_item_widget(QWidget *parent) :
    QWidget(parent)
{
    eff = 0;

    QHBoxLayout *h1 = new QHBoxLayout();


    setLayout(h1);
}


void playlist_item_widget::paintEvent(QPaintEvent *e)
{

}
