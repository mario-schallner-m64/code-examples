#include "rv_splitter.h"

rv_splitter::rv_splitter(Qt::Orientation orientation, QWidget *parent)
    : QSplitter(orientation,  parent)
{
    my_handle = 0;
    sub_splitter = false;
}

QSplitterHandle *rv_splitter::createHandle()
{
     my_handle = new rv_splitter_handle(orientation(), this);
     my_handle->sub_splitter = sub_splitter;

     return my_handle;
}


void rv_splitter::set_sub_splitter(bool b)
{
    sub_splitter = b;
    if(my_handle) my_handle->sub_splitter = b;
}
