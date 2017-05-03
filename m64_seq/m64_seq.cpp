#include "m64_seq.h"

m64_seq::m64_seq(QObject *parent) : QObject(parent)
{
    nr = 0;
    parse("................................................................");
}

void m64_seq::next_step()
{
    position++;
    if(position == len) position = 0;
}

bool m64_seq::parse(QString seq)
{
    steps.clear();
    len = 0;

    m64_seq_step *s = 0;

    for(int i=0; i < (seq.length()/2); i++) {
        s = new m64_seq_step(this);
        s->set_note((QString)seq.mid(i*2, 2));
        if(s->note !=  '.') s->on = true;
        steps.append(s);
        len++;
    }

    if(len>32) len = 32;

    sequence = seq;

    position = 0;

    return true;
}


bool m64_seq::parse_lengths(QString seq)
{
    int max = 0;
    int tmp;

    max = len;

    QStringList l = seq.split(",");
    if(l.count() < max) max = l.count();

    for(int i=0; i < max; i++) {
        tmp = ((QString)(l.at(i))).toInt();
        if(tmp < 0) tmp = 1;
        if(tmp >= 32) tmp = 0;
        steps.at(i)->length = tmp;
    }

    return true;
}



void m64_seq::set_position(int p)
{
    position = p;
    if((position < 0)||(position >= len)) position = 0;
}

QString m64_seq::serialize()
{
    sequence.clear();
    for(int i=0; i < steps.count(); i++) {
        sequence.append(steps.at(i)->note);
        sequence.append(QString::number(steps.at(i)->octave));
    }

    return sequence;
}

void m64_seq::create_midi_data(int gt)
{

    for(int i=0; i < steps.count(); i++) {
        steps.at(i)->note_to_mididata(gt);
    }
}
