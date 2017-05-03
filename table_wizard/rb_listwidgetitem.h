#ifndef RB_LISTWIDGETITEM_H
#define RB_LISTWIDGETITEM_H

#include <QListWidgetItem>

#include <QIcon>
#include <QPixmap>
#include <QPainter>

#include <rb.h>
#include <rb_tools.h>


class rb_listwidgetitem : public QListWidgetItem
{

public:
    explicit rb_listwidgetitem(QListWidget *parent = 0,
                               int t=QListWidgetItem::UserType);

    void set_rbar(rb *r);

public:
    rb *rbar;

};

#endif // RB_LISTWIDGETITEM_H
