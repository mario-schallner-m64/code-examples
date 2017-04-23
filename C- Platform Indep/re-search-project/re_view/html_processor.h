#ifndef HTML_PROCESSOR_H
#define HTML_PROCESSOR_H

#include <QObject>
#include <QColor>
#include <QString>

class html_processor : public QObject
{
    Q_OBJECT
public:
    explicit html_processor(QObject *parent = 0);
static QString qcolor_to_html_color(QColor &color);

signals:

public slots:

};

#endif // HTML_PROCESSOR_H
