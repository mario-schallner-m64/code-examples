#ifndef RV_DISASM_XREFS_WIDGET_H
#define RV_DISASM_XREFS_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QtGui>
#include <re_core.h>

class rv_disasm_xrefs_widget : public QWidget
{
    Q_OBJECT
public:
    explicit rv_disasm_xrefs_widget(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *);
    bool          active;
    bool          top_line;

signals:
    void va_dblclicked(re_addr_t va);
    void jmp_from(re_addr_t va);

public slots:
    void set_desc(re_addr_desc *d, re_target *t);
    void tbl_cell_doubleclicked(int r, int c);

private:
    QTableWidget *table;
    re_addr_desc *my_desc;
    re_target    *my_target;
    QLabel *l1;
};

#endif // RV_DISASM_XREFS_WIDGET_H
