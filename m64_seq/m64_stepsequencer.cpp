#include "m64_stepsequencer.h"

m64_stepsequencer::m64_stepsequencer(QWidget *parent) : QWidget(parent)
{
    for(int i=0; i<16; i++) {
        sequencer = new m64_seq(this);
        sequencer->nr = i;
        sequencers.append(sequencer);
    }

    sequencer = sequencers.at(0);

    setFont(QFont("C64 Pro Mono", 12));

    this->setMouseTracking(true);

    t1.setInterval(1000);

    connect(&t1, SIGNAL(timeout()),
            this, SLOT(timer_hit()));
    t1.start();

    mousetimer.setInterval(800);
    connect(&mousetimer, SIGNAL(timeout()),
            this, SLOT(mousetimer_hit()));

    connect(&timer_message, SIGNAL(timeout()),
            this, SLOT(message_timer_hit()));


    mouse_X=0;
    mouse_Y=0;
    mouse_position =0;    
}

void m64_stepsequencer::parse(QString s)
{
    sequencer->parse(s);
}

void m64_stepsequencer::paintEvent(QPaintEvent *e)
{
    char note;
    QString s = "";
    int x_add = 0;


    QPainter painter(this);

    QFont tmpfont = painter.font();

    painter.setRenderHint(QPainter::Antialiasing);

    // BACKGROUND
    painter.fillRect(QRect(0,0,800,340),
                     QColor::fromRgb(0x08,0x08,0x08));

    QColor current_color;

    for(int y=0; y<4; y++) {
        for(int x=0; x<8; x++) {
            current_color = QColor::fromRgb(0x40+(x%4)*20, 0x40+(x%4)*20, 0x60+(x%4)*20);


            if(sequencer->play_position == (y*8+x)) {
                current_color = QColor::fromRgb(0xa0, 0xc0, 0xc0);
            }
            if(sequencer->position == (y*8+x)) {
                current_color = m64_stepcolor_on;
            }

            painter.fillRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x,MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y, MATRIX_CELL_WIDTH, MATRIX_CELL_HEIGHT, current_color );


            // BORDER
            if( sequencer->position == (y*8+x) ||
                    sequencer->play_position == (y*8+x)
                    ) painter.setPen(
                        QColor::fromRgb(0xff, 0xff, 0xff));
                else painter.setPen(QColor::fromRgb(0x00, 0x00, 0x00));
            painter.drawRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x-1,MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y-1, MATRIX_CELL_WIDTH+2, MATRIX_CELL_HEIGHT+2);


            if(sequencer->steps.at(x+8*y)->note != '.') {
            // NOTE

            // note background
            painter.fillRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10,  MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 20+20-14, 60, 20,
                             QColor::fromRgb(0x10, 0x10, 0x10));

            // note border
            painter.setPen(QColor::fromRgb(0xcf, 0xcf, 0xcf));
            painter.drawRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10,  MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 20+20-14, 60, 20);

            // note text
            painter.setPen(QColor::fromRgb(0xcf, 0xcf, 0xcf));

            s = "";
            x_add=0;
            if(sequencer->steps.count() > 0)
                if((x+8*y) < sequencer->steps.count())
                    if(sequencer->steps.at(x+8*y)->note != '.') {
                        note = sequencer->steps.at(x+8*y)->note;
                        s = QString::number(
                                    sequencer->steps.at(x+8*y)->octave);
                        if(note < 0x60) { s.prepend("#"); x_add = -5; }
                        else { note-= 0x20; x_add = 5; }
                        s.prepend(note);

                        painter.drawText(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 20 + x_add,
                                         MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 20+23,
                                         s);
                    }

            // VELOCITY BAR
            int v = sequencer->steps.at(x+8*y)->velocity;
            int vc = 127-0x20 + v;

            painter.fillRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10-2,  MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 10+20-14+35-2,
                             64, 5, QColor::fromRgb(0x20, 0x10, 0x20));
            painter.fillRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10-1,  MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 10+20-14+35-1,
                             62, 3, QColor::fromRgb(0x00, 0x60, 0x60));

            painter.setPen(QColor::fromRgb(vc+0x20, vc, vc+0x20));
            painter.drawLine(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10,  MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 10+20-14+35,

                             MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10 + v * 60 / 127,
                             MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 10+20-14+35
                             );
            painter.drawLine(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10 ,  MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 10+20-14+35+1,

                             MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 10 + v * 60 / 127,
                             MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 10+20-14+35+1
                             );


            // LENGTH NUMBER
            if(sequencer->steps.at(x+8*y)->length != 1)
                painter.setPen(QColor::fromRgb(0xff, 0xff, 0xff));
            else painter.setPen(QColor::fromRgb(0x20, 0x20, 0x30));
            painter.setFont(QFont("C64 Pro Mono", 7));
            painter.drawText(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*x + 5,
                             MATRIX_BORDER_U + (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y ) * y + 15,
                             QString::number(sequencer->steps.at(x+8*y)->length));
            painter.setFont(tmpfont);
            } // IF NOTE
        }
    }

    // SIDE LEDS

    if(status != 0) {
        painter.fillRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*8-10, 7, 10, 10,
                         QColor::fromRgb(0xff, 0xff, 0xe0)

                    );
    } else {
        painter.fillRect(MATRIX_BORDER_L + (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X)*8-10, 7, 10, 10,
                         QColor::fromRgb(0x00, 0x00, 0x00)

                    );
    }

    // MESSAGE

    if(b_show_message) {
        painter.setPen(QColor::fromRgb(0xff,0xff,0xff));


        QColor qc;  qc.setRgba(QRgb(0x80000000));
        painter.fillRect(
                    0, 0,
                    800, 340,
                    qc
                    );

        QStringList qsl = message.split("\n");

        for(int i=0; i<qsl.count(); i++) {
            painter.drawText(40, 40+20*i, qsl.at(i));
        }
    }
}

void m64_stepsequencer::set_position(int p)
{
    sequencer->set_position(p);
    update();
}

void m64_stepsequencer::timer_hit()
{
    if(cursor) cursor = false;
    else cursor = true;

    if(cursor) m64_stepcolor_on = QColor::fromRgb(0xe5, 0xec, 0xaa);
    else m64_stepcolor_on = QColor::fromRgb(0x60, 0x60, 0x50);
    update();
}

void m64_stepsequencer::mousePressEvent(QMouseEvent *event)
{
    // emit m64_mouse_click();
    mouse_X = event->x();
    mouse_Y = event->y();

    mousetimer.start();

    int tmpx, tmpy;

    tmpx = (mouse_X - MATRIX_BORDER_L) / (MATRIX_CELL_WIDTH + MATRIX_CELLSPACING_X);
    tmpy = (mouse_Y - MATRIX_BORDER_U) / (MATRIX_CELL_HEIGHT + MATRIX_CELLSPACING_Y);

    mouse_position = tmpx + tmpy * 8;
    sequencer->set_position(mouse_position);
    update();
    emit sig_position_changed(mouse_position);
}

void m64_stepsequencer::mouseReleaseEvent(QMouseEvent *event)
{
    if(mousetimer.isActive()) {
        if(mousetimer.remainingTime() < 600)
        emit paste_clicked();
        mousetimer.stop();
    }
}

void m64_stepsequencer::mousetimer_hit()
{
    mousetimer.stop();
    emit sig_show_note_editor();
}

void m64_stepsequencer::select_sequencer(int nr)
{
    if((nr <0) || (nr >= sequencers.count())) return;

    sequencer = sequencers.at(nr);
    sequencer_nr = nr;
    update();
}


void m64_stepsequencer::show_message(QString s, int secs)
{
    message = s;

    timer_message.setInterval(secs*1000);
    timer_message.start();
    b_show_message = true;
    update();
}

void m64_stepsequencer::message_timer_hit()
{
    b_show_message = false;
}

void m64_stepsequencer::set_play_position(int p)
{
    sequencer->play_position = p;
}
