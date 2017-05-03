#include "m64_serial.h"
#include <QApplication>
#include <QThread>

m64_serial::m64_serial(QObject *parent) :
    QObject(parent)
{


}

bool m64_serial::init_serial()
{
        LastError = "";
        QTextStream standardOutput(stdout);

        m64_serial_port.setPortName(m64_serialdevice);

        int serialPortBaudRate = QSerialPort::Baud115200;
        m64_serial_port.setBaudRate(serialPortBaudRate);

        if (!m64_serial_port.open(QIODevice::ReadOnly)) {
            standardOutput <<
                QObject::tr("Failed to open port %1, error: %2").arg(
                m64_serialdevice).arg(m64_serial_port.errorString()) << endl;
            LastError = QObject::tr("Failed to open port %1,\nError: %2\n").arg(
                m64_serialdevice).arg(m64_serial_port.errorString());
            return false;
        }

        standardOutput << "Serial OK!" << endl;

        midi_file = new QFile(midi_device);

        if(!midi_file->open(QFile::WriteOnly | QFile::Unbuffered)) {
            standardOutput << "MIDI SHIT!" << endl;
            LastError = QObject::tr("Failed to open %1,\nError: %2\n").arg(
                        m64_serialdevice).arg(midi_file->errorString());
            return false;
        }

        standardOutput << "MIDI OK!" << endl;

        LastError = (QString)"SERIAL OK!\n" +
                    "MIDI OK!\n";




        return true;
}

void m64_serial::listen()
{

    if(do_listen == true) {
        do_listen = false;
        m64_serial_port.readAll();
        m64_serial_port.flush();
        return;
    }

    do_listen = true;   

    QTextStream standardOutput(stdout);
    standardOutput << "Starting to listen" << endl;
    m64_serial_port.flush();
    m64_serial_port.readAll();

    step_pos = sequencer->play_position;

    standardOutput << ("         at: " + QString::number(step_pos)) << endl;

    sent = 0;
    written = 0;

    while( do_listen ) {       
        if(m64_serial_port.bytesAvailable() ==3) {
            sent = 0;
            written = 0;
            m64_serial_port.readAll();


            // vorherigen PLAY beenden
            // wenn bei diesem Step aus
            //
            // NOTE OFF
            if(playing) {
                if((step_pos - playing_step_pos) == playing_step->length) {
                    if(!simulate) midi_file->write(
                        sequencer->steps.at(playing_step_pos)->midi_data_off,
                        3);
                    //midi_file->flush();

//                    standardOutput << (QString::number(step_pos) +
//                                      (QString) "   . NOTE OFF") << endl;

                    //QThread::msleep(15);

                    playing = false;
                    sent++;
                }
            }


            // standardOutput << "clock" << endl;
            if(sequencer->steps.at(step_pos)->note != '.') {
                // NOTE OFF
                if(playing) {
                    if(!simulate) midi_file->write(
                        sequencer->steps.at(playing_step_pos)->midi_data_off,
                        3);
                    //midi_file->flush();

//                    standardOutput << (QString::number(step_pos) +
//                                      (QString) "   . NOTE OFF") << endl;

                    //QThread::msleep(15);
                    sent++;
                }


                // MIDI

                // NOTE ON
                if(!simulate) written = midi_file->write(
                    sequencer->steps.at(step_pos)->midi_data_on,
                    3);


                playing = true;
                playing_step =  sequencer->steps.at(step_pos);
                playing_step_pos = step_pos;
                sent++;
            }

            step_pos++;
            if(step_pos == 32) step_pos = 0;

            emit M64_SERIAL_clocked(written);
            QApplication::processEvents();
        }

        QApplication::processEvents();

    }
}

int m64_serial::midi_play_step(int nr)
{
    sent = 0;
    written = 0;

    step_pos = nr;

    // vorherigen PLAY beenden
    // wenn bei diesem Step aus
    //
    // NOTE OFF
    if(playing && playing_step) {
        if((step_pos - playing_step_pos) == playing_step->length) {
            if(!simulate) midi_file->write(
                sequencer->steps.at(playing_step_pos)->midi_data_off,
                3);

            playing = false;
            sent++;
        }
    }

    // standardOutput << "clock" << endl;
    if(sequencer->steps.at(step_pos)->note != '.') {
        // NOTE OFF
        if(playing) {
            if(!simulate) midi_file->write(
                sequencer->steps.at(playing_step_pos)->midi_data_off,
                3);

            sent++;
        }

        // MIDI       
        if(!simulate) {
        written = midi_file->write(
            sequencer->steps.at(step_pos)->midi_data_on,
            3);
        } else written = 3;

        playing = true;
        playing_step =  sequencer->steps.at(step_pos);
        playing_step_pos = step_pos;
        sent++;
    }

    return written;
}
