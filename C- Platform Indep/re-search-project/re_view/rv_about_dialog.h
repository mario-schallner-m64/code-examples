#ifndef RV_ABOUT_DIALOG_H
#define RV_ABOUT_DIALOG_H

#include <QDialog>

namespace Ui {
    class rv_about_dialog;
}

class rv_about_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit rv_about_dialog(QWidget *parent = 0);
    ~rv_about_dialog();

private:
    Ui::rv_about_dialog *ui;
};

#endif // RV_ABOUT_DIALOG_H
