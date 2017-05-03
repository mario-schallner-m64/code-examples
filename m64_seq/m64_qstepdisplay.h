#ifndef M64_QSTEPDISPLAY_H
#define M64_QSTEPDISPLAY_H

#include <QWidget>
#include <QImage>
#include <QtGui>
#include <QList>
#include <QLayout>
#include <QLabel>

class m64_qstepdisplay : public QWidget
{
    Q_OBJECT
public:

    int m64_step = 0;
    int m64_step_play = 0;

    QList<QLabel *> m64_seq_img_list;

    explicit m64_qstepdisplay(QWidget *parent = 0);

    QPixmap *px_off = 0;
    QPixmap *px_on  = 0;
    QPixmap *px_off_4 = 0;
    QPixmap *px_on_play = 0;


signals:

public slots:
    void set_step( int i);
    int get_step();
    void set_step_play(int i);

};

#endif // M64_QSTEPDISPLAY_H
