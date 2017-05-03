#include "m64_qstepdisplay.h"

m64_qstepdisplay::m64_qstepdisplay(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hl1 = new QHBoxLayout(this);

    QLabel *tmplabel;

    px_off = new QPixmap(":/images/res/step_16_16_off.png");
    px_off_4 = new QPixmap(":/images/res/step_16_16_4_off.png");
    px_on = new QPixmap(":/images/res/step_16_play.png");
    px_on_play = new QPixmap(":/images/res/step_16_play2.png");

    for(int i=0; i< 32; i++) {
        tmplabel = new QLabel(this);
        QPixmap* tmppixmap = px_off;

        if(!(i%4)) tmppixmap = px_off_4;

        tmplabel->setPixmap(*tmppixmap);
        m64_seq_img_list.append(tmplabel);
        hl1->addWidget(tmplabel);
    }

    hl1->addSpacing(25);

    hl1->setMargin(0); hl1->setSpacing(3);
}

void m64_qstepdisplay::set_step(int i)
{
    int oldstep = m64_step;
    if(i >= 32) m64_step = 0;
    else m64_step = i;

    QPixmap* tmppixmap = px_off;
    if(!(oldstep%4)) tmppixmap = px_off_4;
    m64_seq_img_list.at(oldstep)->setPixmap(*tmppixmap);

    tmppixmap = px_on;
    m64_seq_img_list.at(m64_step)->setPixmap(*tmppixmap);

    tmppixmap = px_on_play;
    m64_seq_img_list.at(m64_step_play)->setPixmap(*tmppixmap);
}

int m64_qstepdisplay::get_step()
{
    return m64_step;
}

void m64_qstepdisplay::set_step_play(int i)
{
    int oldstep = m64_step_play;
    if(i >= 32) m64_step_play = 0;
    else m64_step_play = i;

    QPixmap* tmppixmap = px_off;
    if(!(oldstep%4)) tmppixmap = px_off_4;
    m64_seq_img_list.at(oldstep)->setPixmap(*tmppixmap);

    tmppixmap = px_on;
    m64_seq_img_list.at(m64_step)->setPixmap(*tmppixmap);

    tmppixmap = px_on_play;
    m64_seq_img_list.at(m64_step_play)->setPixmap(*tmppixmap);
}
