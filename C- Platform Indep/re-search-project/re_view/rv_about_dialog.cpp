#include "rv_about_dialog.h"
#include "ui_rv_about_dialog.h"

rv_about_dialog::rv_about_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rv_about_dialog)
{
    ui->setupUi(this);
}

rv_about_dialog::~rv_about_dialog()
{
    delete ui;
}
