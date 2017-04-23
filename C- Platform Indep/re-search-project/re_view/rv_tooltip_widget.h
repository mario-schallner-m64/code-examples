#ifndef RV_TOOLTIP_WIDGET_H
#define RV_TOOLTIP_WIDGET_H

#include <QtGui>
#include <QWidget>

class rv_tooltip_widget : public QWidget
{
    Q_OBJECT
public:
    explicit rv_tooltip_widget(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *e);
    bool    is_above;

signals:

public slots:
    void setText(QString t);

private:
    QString     text;
    QLabel      l;
    QPlainTextEdit *pte;

    QColor color_bg;
    QColor color_line;

};

#endif // RV_TOOLTIP_WIDGET_H
