#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>

#include <rb.h>
#include <rb_listwidgetitem.h>
#include <rb_displayer.h>
#include <rb_screen.h>
#include <rb_color_picker.h>
#include <rb_table_widget.h>

#include <rbsa_effect.h>
#include <scrn_effect.h>
#include <render_scrn.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_pushButton_rb_new_clicked();

    void on_pushButton_rb_dump_clicked();

    void on_pushButton_rb_ins_line_clicked();
    void on_pushButton_rb_rmv_line_clicked();

    void on_pushButton_rb_copy_clicked();
    void on_pushButton_rb_paste_clicked();

    void rb_lwi_changed(QListWidgetItem *lwi_new,
                        QListWidgetItem *lwi_old);

    void rb2table(rb *rbar);

    void inc_dec_rb_size(int newsz);
    void inc_dec_y(int y);
    void set_y(int y);
    void inc_dec_z(int z);

    void color_clicked(int c);

    // -- rb animation stuff --

    void toggle_edit_animation(bool yesno);
    void rb_ani_current_frame_changed(int f);
    void on_pushButton_rb_ani_rb2editor_clicked();
    void rb_ani_frame_cnt_changed(int cnt);
    void rb_ani_nr_changed(int nr);
    void rb_ani_delay_frames_changed(int f);
    void rb_ani_timer_hit();
    void on_pushButton_rb_ani_run_clicked();
    void on_pushButton_rb_ani_dup2next_clicked();
    void on_pushButton_rb_ani_frame_prev_clicked();
    void on_pushButton_rb_ani_frame_next_clicked();

    void pushButton_rb_clone_clicked();
    void on_pushButton_rb_delete_clicked();

    void init_rbsa_effects();

    void rbsa_effect_selected(QModelIndex idx);
    void rbsa_effect_param_val_changed(int r, int c);

    // -- scrn animation stuff --

    void toggle_edit_scrn_animation(bool yesno);
    void init_scrn_effects();
    void scrn_effect_selected(QModelIndex idx);

    void on_pushButton_playlist_additem_clicked();
    void on_pushButton_playlist_removeitem_clicked();

private:
    Ui::MainWindow *ui;
    rb_displayer *rbd;
    rb_screen *scrn;
    rb_color_picker *cpicker;


    QList<rb *> rbs;

    // -- animation stuff --
    rb_displayer *rbd2;
    unsigned char animation_undo_copy_buffer[256*256*16];
    bool edit_animation;
    bool edit_scrn_ani;

    int rb_ani_nr;
    int rb_ani_frame_cnt;
    int rb_ani_current_frame_nr;

    QTimer rb_ani_timer;
    int rb_ani_delay_frames;
    bool rb_ani_running;

    QList<scrn_effect *> playlist;
    render_scrn *the_screen;

};

#endif // MAINWINDOW_H
