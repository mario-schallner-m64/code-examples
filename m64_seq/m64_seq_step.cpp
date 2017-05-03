#include "m64_seq_step.h"

m64_seq_step::m64_seq_step(QObject *parent) : QObject(parent)
{
    note = '.';
    octave = 3;
    velocity = 100;
    length = 1;
}

void m64_seq_step::set_note(char n)
{
    note = n;
}

void m64_seq_step::set_octave(int o)
{
    octave = o;
}

void m64_seq_step::set_note(QString s)
{
    note =s.left(1).toLocal8Bit().at(0);
    octave = s.right(1).toInt();
    note_to_mididata(0);
}

void m64_seq_step::note_to_mididata(int gt)
{   
    char mk = 0;

    if(note == 'c') {
        mk = 24;
    }

    if(note == 'd') {
        mk = 26;
    }

    if(note == 'e') {
        mk = 28;
    }

    if(note == 'f') {
        mk = 29;
    }

    if(note == 'g') {
        mk = 31;
    }

    if(note == 'a') {
        mk = 33;
    }

    if(note == 'h') {
        mk = 35;
    }

    if(octave == 0) {
        mk += 0;
    }

    if(octave == 1) {
        mk += 12;
    }

    if(octave == 2) {
        mk += 24;
    }

    if(octave == 3) {
        mk += 36;
    }

    if(octave == 4) {
        mk += 48;
    }

    if(octave == 5) {
        mk += 60;
    }

    if(octave == 6) {
        mk += 72;
    }

    if(octave == 7) {
        mk += 84;
    }

    mk+=gt;

    midi_data_on[0] = 0x90;
    midi_data_on[1] = mk;
    midi_data_on[2] = velocity;

    midi_data_off[0] = 0x80;
    midi_data_off[1] = mk;
    midi_data_off[2] = 0x7f;

}


