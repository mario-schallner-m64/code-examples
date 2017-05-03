#ifndef RB_TOOLS_H
#define RB_TOOLS_H

#include <QObject>

#include <QList>
#include <QColor>

typedef struct rb_lines_buffer_s {
    unsigned char buffer[256];
    bool have_copy;
    int start;
    int end;

} rb_lines_buffer;

class rb_tools : public QObject
{
    Q_OBJECT
public:
    explicit rb_tools(QObject *parent = 0);

    static void tools_init();

signals:

public slots:


public:
    static QList<QColor> colortbl;
    static rb_lines_buffer copy_buffer;
};

#endif // RB_TOOLS_H
