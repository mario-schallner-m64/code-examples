#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QList>
#include <QFile>
#include <QPixmap>
#include <QTimer>

#include <m64_serial.h>
#include <m64_stepsequencer.h>
#include <m64_qstepdisplay.h>
#include <form_edit_note.h>
#include <form_settings.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    bool simulate = false;

    m64_stepsequencer *stepsequencer = 0;
    m64_serial *serial = 0;
    m64_qstepdisplay *step_display = 0;

    QPixmap *img_nintendo;
    QPixmap *img_m64;
    QTimer timer_logo;
    int toggle_logo = 0;

    Form_edit_note *fe = 0;
    form_settings *fs = 0;

    QTimer timer_bpm;
    int BPM;

    bool disk_menu = false;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void button_exit_clicked();
    void button_play_clicked();
    void button_back_clicked();
    void seq_step(int w);

    void show_note_editor();
    void seq_pos_changed(int p);

    void inc_seq_nr();
    void dec_seq_nr();

    void save_step();

    void seq_double_click();

    void button_disk_clicked();

    void save_current_seq();
    void load_current_seq();

    void timer_logo_hit();

    void button_settings_clicked();

    void timer_bpm_hit();

    void apply_settings();

    void set_simulate(bool yn);

    void button_clk_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
