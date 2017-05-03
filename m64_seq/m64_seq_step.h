#ifndef M64_SEQ_STEP_H
#define M64_SEQ_STEP_H

#include <QObject>
#include <QThread>

class m64_seq_step : public QObject
{
    Q_OBJECT
public:
    explicit m64_seq_step(QObject *parent = 0);

    // features
    bool on = false;
    int nr = 0;

    // midi
    char midi_data_on[3];
    char midi_data_off[3];
    char midi_note;
    char midi_velocity;

    // representation
    char note;
    int  octave;
    int  velocity;
    int length = 1;

    // methods
    void set_note(QString s); // use this


    // helpers
    void set_note(char n);
    void set_octave(int o);

    void note_to_mididata(int gt=0);

signals:

public slots:
};

#endif // M64_SEQ_STEP_H
