#ifndef RV_TAG_CELL_EDITOR_H
#define RV_TAG_CELL_EDITOR_H

#include <QWidget>
#include <re_core.h>
#include <rv_tag_processor.h>


class rv_tag_cell_editor : public QWidget
{
    Q_OBJECT
public:
    explicit rv_tag_cell_editor(QWidget *parent = 0);

    QSize sizeHint() const;

    void set_tag_value(int v);
    int get_new_tag_value();

signals:
     void editingFinished();

public slots:
     void set_va(re_addr_t va);
     void set_tag_processor(rv_tag_processor *p);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    re_addr_t my_va;
    int tag_val;
    int mouse_tag_position;
    rv_tag_processor *tag_processor;
};

#endif // RV_TAG_CELL_EDITOR_H
