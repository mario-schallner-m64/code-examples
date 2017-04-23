#ifndef RV_PROGRESS_DIALOG_H
#define RV_PROGRESS_DIALOG_H

#include <QDialog>
#include <re_core.h>

namespace Ui {
    class rv_progress_dialog;
}

class rv_progress_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit rv_progress_dialog(QWidget *parent = 0);
    ~rv_progress_dialog();

public slots:
    void    set_main_task_msg(QString msg);
    void    set_progress_msg(int level, QString msg);
    void    set_pct(int pct);

private:
    Ui::rv_progress_dialog *ui;
};

#endif // RV_PROGRESS_DIALOG_H
