#ifndef M64_STEPSEQUENCER_H
#define M64_STEPSEQUENCER_H

#include <QWidget>
#include <m64_seq.h>
#include <QPainter>
#include <QtGui>
#include <QTimer>
#include <QElapsedTimer>
#include <QColor>

#define MATRIX_BORDER_L 15
#define MATRIX_BORDER_U 7
#define MATRIX_CELL_WIDTH    81
#define MATRIX_CELL_HEIGHT   65
#define MATRIX_CELLSPACING_X   15
#define MATRIX_CELLSPACING_Y   10



class m64_stepsequencer : public QWidget
{
    Q_OBJECT
public:
    explicit m64_stepsequencer(QWidget *parent = 0);

    m64_seq *sequencer;
    QList<m64_seq *> sequencers;
    int sequencer_nr = 0;

    void show_message(QString s, int secs = 2);


    void paintEvent(QPaintEvent *e);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public:
    QString message;
    QTimer timer_message;
    bool   b_show_message = false;

    QColor m64_stepcolor_on;
    bool cursor = false;


    QTimer t1;
    QTimer mousetimer;

    int mouse_X;
    int mouse_Y;

    int mouse_position;

    int status = 0;

signals:
     void sig_show_note_editor();
     void sig_position_changed(int p);
     void paste_clicked();

public slots:
    void parse(QString s);
    void set_position(int p);
    void set_play_position(int p);

    void timer_hit();
    void mousetimer_hit();
    void select_sequencer(int nr);

    void message_timer_hit();


};

#endif // M64_STEPSEQUENCER_H
