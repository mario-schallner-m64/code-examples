#ifndef M64_SERIAL_H
#define M64_SERIAL_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QFile>

#include <m64_seq.h>




class m64_serial : public QObject
{
    Q_OBJECT
public:
    explicit m64_serial(QObject *parent = 0);
    bool simulate = false;

    QString m64_serialdevice = "/dev/ttyACM0";
    QSerialPort m64_serial_port;

    QString midi_device = "/dev/midi1";
    // QSerialPort midi_port;
     QFile *midi_file;

    m64_seq *sequencer;

    bool do_listen = false;

    bool playing = false;
    m64_seq_step *playing_step = 0;
    int playing_step_pos = 0;


    int step_pos = 0;
    int sent = 0;
    int written = 0;

    QString LastError;

signals:
    void M64_SERIAL_clocked(int w);


public slots:
    bool init_serial();
    void listen();
    int midi_play_step(int nr);
};

#endif // M64_SERIAL_H
