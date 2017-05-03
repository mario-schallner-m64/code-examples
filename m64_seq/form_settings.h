#ifndef FORM_SETTINGS_H
#define FORM_SETTINGS_H

#include <QWidget>
#include <QPainter>

namespace Ui {
class form_settings;
}

class form_settings : public QWidget
{
    Q_OBJECT

public:
    explicit form_settings(QWidget *parent = 0);
    ~form_settings();

    void paintEvent(QPaintEvent *e);

    bool timer_sync = false;

    int bpm = 140;
    int gtn = 0;
    int gto = 0;

signals:
    void M64_OK_Clicked();

public slots:
    void button_ok_clicked();
    void button_clock_clicked();

    void value_bpm_changed();
    void value_gtn_changed();
    void value_gto_changed();

private:
    Ui::form_settings *ui;
};

#endif // FORM_SETTINGS_H
