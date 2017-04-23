#ifndef RV_SPLITTER_H
#define RV_SPLITTER_H

#include <QSplitter>
#include <rv_splitter_handle.h>

class rv_splitter : public QSplitter
{
    Q_OBJECT
public:
    explicit rv_splitter(Qt::Orientation orientation, QWidget *parent = 0);


protected:
     QSplitterHandle *createHandle();

signals:

public slots:
     void set_sub_splitter(bool b);

private:
    rv_splitter_handle *my_handle;
    bool sub_splitter;
};

#endif // RV_SPLITTER_H
