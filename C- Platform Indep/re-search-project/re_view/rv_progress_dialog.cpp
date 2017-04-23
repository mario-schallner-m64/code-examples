#include "rv_progress_dialog.h"
#include "ui_rv_progress_dialog.h"

rv_progress_dialog::rv_progress_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rv_progress_dialog)
{
    ui->setupUi(this);
}

rv_progress_dialog::~rv_progress_dialog()
{
    delete ui;
}

void rv_progress_dialog::set_progress_msg(int level, QString msg)
{
    if(level == RE_LOG_NORMAL)
    ui->label_progress->setText(msg);
}

void rv_progress_dialog::set_pct(int pct)
{
    ui->progressBar->setValue(pct);
}

void rv_progress_dialog::set_main_task_msg(QString msg)
{
    ui->label_maintask->setText(msg);
}
