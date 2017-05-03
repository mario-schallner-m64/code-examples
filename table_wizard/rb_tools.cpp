#include "rb_tools.h"

QList<QColor> rb_tools::colortbl;
rb_lines_buffer rb_tools::copy_buffer;

rb_tools::rb_tools(QObject *parent) :
    QObject(parent)
{    
}

void rb_tools::tools_init()
{
    colortbl.clear();
    colortbl.append(QColor(0x00, 0x00, 0x00)); // 0
    colortbl.append(QColor(0xff, 0xff, 0xff)); // 1
    colortbl.append(QColor(0x88, 0x39, 0x32)); // 2
    colortbl.append(QColor(0x67, 0xb6, 0xbd)); // 3
    colortbl.append(QColor(0x8b, 0x3f, 0x96)); // 4
    colortbl.append(QColor(0x55, 0xa0, 0x49)); // 5
    colortbl.append(QColor(0x40, 0x31, 0x8d)); // 6
    colortbl.append(QColor(0xbf, 0xce, 0x72)); // 7
    colortbl.append(QColor(0x8b, 0x54, 0x29)); // 8
    colortbl.append(QColor(0x57, 0x42, 0x00)); // 9
    colortbl.append(QColor(0xb8, 0x69, 0x62)); // a
    colortbl.append(QColor(0x50, 0x50, 0x50)); // b
    colortbl.append(QColor(0x78, 0x78, 0x78)); // c
    colortbl.append(QColor(0x94, 0xe0, 0x89)); // d
    colortbl.append(QColor(0x78, 0x69, 0xc4)); // e
    colortbl.append(QColor(0x9f, 0x9f, 0x9f)); // f

    copy_buffer.have_copy = false;
    copy_buffer.start = 0;
    copy_buffer.end = 0;
}
