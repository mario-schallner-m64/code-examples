#ifndef FORM_EDIT_NOTE_H
#define FORM_EDIT_NOTE_H

#include <QWidget>
#include <QPainter>
#include <QtGui>

namespace Ui {
class Form_edit_note;
}

class Form_edit_note : public QWidget
{
    Q_OBJECT

public:
    explicit Form_edit_note(QWidget *parent = 0);
    ~Form_edit_note();

    void paintEvent(QPaintEvent *e);

    char note = '.';
    char note_shadow;
    int octave = 0;
    int octave_shadow;
    int velocity = 100;
    int length = 1;

    QString c_gray;
    QString c_blu;

public slots:
    void button_ok_clicked();
    void button_cancel_clicked();
    void set_note(char nn);
    void set_octave(int o);
    void set_velocity(int v);
    void set_length(int l);
    void reset_colors();

    void button_c_clicked();
    void button_d_clicked();
    void button_e_clicked();
    void button_f_clicked();
    void button_g_clicked();
    void button_a_clicked();
    void button_h_clicked();
    void button_sharp_clicked();

    void button_0_clicked();
    void button_1_clicked();
    void button_2_clicked();
    void button_3_clicked();
    void button_4_clicked();
    void button_5_clicked();
    void button_6_clicked();

    void button_del_clicked();

    void velocity_slided();
    void length_slided();


signals:
    void save_clicked();

private:
    Ui::Form_edit_note *ui;
};

#endif // FORM_EDIT_NOTE_H
