#ifndef RV_TOOLTIP_XREFS_H
#define RV_TOOLTIP_XREFS_H

#include <QWidget>
#include <QtGui>

class rv_tooltip_xrefs : public QWidget
{
    Q_OBJECT
public:
    explicit rv_tooltip_xrefs(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *e);

signals:

public slots:
    void setText(QString s);

private:
    QLabel      l;

    QColor color_bg;
    QColor color_line;
};

#endif // RV_TOOLTIP_XREFS_H
