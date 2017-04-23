#ifndef RV_SIDEINFO_LISTBOX_H
#define RV_SIDEINFO_LISTBOX_H

#include <QListWidget>

class rv_sideinfo_listbox : public QListWidget
{
    Q_OBJECT
public:
    explicit rv_sideinfo_listbox(QWidget *parent = 0);
    virtual void dropEvent(QDropEvent *event);

signals:
    void drop_happened();

public slots:

};

#endif // RV_SIDEINFO_LISTBOX_H
