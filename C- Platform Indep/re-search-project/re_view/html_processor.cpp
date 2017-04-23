#include "html_processor.h"
#include <stdio.h>

html_processor::html_processor(QObject *parent) :
    QObject(parent)
{
}

QString html_processor::qcolor_to_html_color(QColor &color)
{
    char buf[1024];
    int r, g, b;
    color.getRgb(&r, &g, &b);

    ::sprintf(buf, (char *)"#%02X%02X%02X;", r, g, b);
    return QString::fromAscii(buf);
}

