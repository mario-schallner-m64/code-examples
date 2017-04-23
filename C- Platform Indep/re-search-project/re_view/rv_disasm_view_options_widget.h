#ifndef RV_DISASM_VIEW_OPTIONS_WIDGET_H
#define RV_DISASM_VIEW_OPTIONS_WIDGET_H

#include <QWidget>
#include <QtGui>

class rv_disasm_view_options_widget : public QWidget
{
    Q_OBJECT
public:
    explicit rv_disasm_view_options_widget(QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *e);

signals:

public slots:

public:
    QCheckBox *checkbox;
    QSpinBox  *spinbox;

private:


};

#endif // RV_DISASM_VIEW_OPTIONS_WIDGET_H
