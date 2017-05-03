#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    img_m64 = new QPixmap(":/images/res/m64.png");
    img_nintendo = new QPixmap(":/images/res/nintendo_mario.png");

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    connect(ui->pushButton_exit, SIGNAL(clicked()),
            this, SLOT(button_exit_clicked()));

    step_display = new m64_qstepdisplay(this);

    ui->horizontalLayout_2->addWidget(step_display);
    ui->horizontalLayout_2->setAlignment(step_display, Qt::AlignTop);

    step_display->set_step(0);

    serial = new m64_serial(this);
    serial->init_serial();

    connect(serial, SIGNAL(M64_SERIAL_clocked(int)),
            this, SLOT(seq_step(int)));

    connect(ui->pushButton_play, SIGNAL(clicked()),
            this, SLOT(button_play_clicked()));

    connect(ui->pushButton_back, SIGNAL(clicked()),
            this, SLOT(button_back_clicked()));

    stepsequencer = new m64_stepsequencer(this);
    stepsequencer->setFixedWidth(800);
    stepsequencer->setFixedHeight(307);
    ui->verticalLayout_matrix->addWidget(stepsequencer);

    connect(stepsequencer, SIGNAL(sig_show_note_editor()),
            this, SLOT(show_note_editor()));
    connect(stepsequencer, SIGNAL(sig_position_changed(int)),
            this, SLOT(seq_pos_changed(int)));

    connect(stepsequencer, SIGNAL(paste_clicked()),
            this, SLOT(save_step()));

    fe = new Form_edit_note(this);
    fe->setGeometry(420,1,400,460);
    fe->hide();
    connect(fe, SIGNAL(save_clicked()),
            this, SLOT(save_step()));

    stepsequencer->parse((QString)"................"+
                         (QString)"................"+
                         (QString)"................"+
                         (QString)"................"
                         );

    connect(ui->pushButton_next_sequencer, SIGNAL(clicked()),
            this, SLOT(inc_seq_nr()));
    connect(ui->pushButton_prev_sequencer, SIGNAL(clicked()),
            this, SLOT(dec_seq_nr()));

    connect(ui->pushButton_disk, SIGNAL(clicked()),
            this, SLOT(button_disk_clicked()));


    connect(&timer_logo, SIGNAL(timeout()),
            this, SLOT(timer_logo_hit()));

    timer_logo.setInterval(10000);
    // timer_logo.start();
    toggle_logo = 0;

    stepsequencer->show_message((QString)
                " M64.LIVE.SEQUENCER 00.00.00\n" +
                "-----------------------------\n" +
                "                   (2016:M64)\n\n" +

                "GAMEBOY SYNCHRONIZED MIDI LOOP SEQUENCER\n\n" +
                serial->LastError +
                "\nREADY."
                , 3);

    fs = new form_settings(this);
    fs->setGeometry(100,50,600,330);
    fs->hide();
    connect(ui->pushButton_settings, SIGNAL(clicked(bool)),
            this, SLOT(button_settings_clicked()));

    connect(fs, SIGNAL(M64_OK_Clicked()),
            this, SLOT(apply_settings()));


    connect(&timer_bpm, SIGNAL(timeout()),
            this, SLOT(timer_bpm_hit()));
    timer_bpm.setInterval( (60 / 140.0)*250 );

    connect(ui->pushButton_clk, SIGNAL(clicked(bool)),
            this, SLOT(button_clk_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::button_exit_clicked()
{    
    this->close();
}

void MainWindow::button_play_clicked(){

    if(serial->do_listen == false) {
        ui->pushButton_play->setStyleSheet(ui->pushButton_play->styleSheet() + "background-color: #8080e0;");       
    } else {
        ui->pushButton_play->setStyleSheet(ui->pushButton_play->styleSheet().remove("background-color: #8080e0;"));       
    }

    stepsequencer->set_play_position(0);
    serial->sequencer = stepsequencer->sequencer;
    stepsequencer->sequencer->create_midi_data(
                fs->gtn + fs->gto * 8);
    serial->listen();
}

void MainWindow::button_back_clicked(){
    stepsequencer->set_play_position(0);
    step_display->set_step_play(0);
    stepsequencer->update();

    //stepsequencer->show_message("BACK AT 0", 2);
}

void MainWindow::seq_step(int w)
{
    int next_step = stepsequencer->sequencer->play_position + 1;
    if(next_step >= 32) next_step = 0;
    stepsequencer->status = w;
    stepsequencer->set_play_position(next_step);
    step_display->set_step_play(next_step);

    stepsequencer->update();
}


void MainWindow::show_note_editor()
{
    if((stepsequencer->sequencer->position % 8) < 4) {
        fe->setGeometry(400,1,400,480);
    } else {
        fe->setGeometry(0,1,400,480);
    }

    fe->set_note(stepsequencer->sequencer->steps.at(
                     stepsequencer->sequencer->position)->note);

    fe->set_octave(stepsequencer->sequencer->steps.at(
                     stepsequencer->sequencer->position)->octave);
    fe->set_velocity(stepsequencer->sequencer->steps.at(
                     stepsequencer->sequencer->position)->velocity);

    fe->set_length(  stepsequencer->sequencer->steps.at(
                     stepsequencer->sequencer->position)->length);

    fe->show();
}

void MainWindow::seq_pos_changed(int p)
{
    step_display->set_step(p);
}

void MainWindow::inc_seq_nr()
{
    if(disk_menu) {
        load_current_seq();
        button_disk_clicked();
        return;
    }
    if((stepsequencer->sequencer_nr +1) < stepsequencer->sequencers.count()) {
        stepsequencer->select_sequencer(stepsequencer->sequencer_nr+1);

        // seq_nr now increased!
        QString nrs = QString::number(stepsequencer->sequencer_nr);
        if(stepsequencer->sequencer_nr < 10) nrs.prepend("0");
        ui->label_seq_nr->setText(nrs);
    }
}

void MainWindow::dec_seq_nr()
{
    if(disk_menu) {
        save_current_seq();
        button_disk_clicked();
        return;
    }
    if((stepsequencer->sequencer_nr -1) >= 0) {
        stepsequencer->select_sequencer(stepsequencer->sequencer_nr-1);

        // seq_nr now increased!
        QString nrs = QString::number(stepsequencer->sequencer_nr);
        if(stepsequencer->sequencer_nr < 10) nrs.prepend("0");
        ui->label_seq_nr->setText(nrs);
    }
}

void MainWindow::save_step()
{
    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->note = fe->note;

    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->octave = fe->octave;

    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->velocity = fe->velocity;

    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->length = fe->length;

    //

    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->note_to_mididata(
                fs->gtn + fs->gto*12);

    stepsequencer->update();

    QString s = QString::number(fe->octave);
    char note = fe->note;
    if(note < 0x60) { s.prepend("#"); }
    else { note-= 0x20;}
    s.prepend(note);

    if(s.left(2) == ".#") s="---";
    else {
        s.append(" " + QString::number(fe->velocity));
        s.append(" " + QString::number(fe->length));
    }

    ui->label_buf->setText(s);
}

void MainWindow::seq_double_click()
{
    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->note = fe->note;

    stepsequencer->sequencer->steps.at(
                stepsequencer->sequencer->position)->octave = fe->octave;
    stepsequencer->update();

}

void MainWindow::button_disk_clicked()
{
    if(!disk_menu) {
        disk_menu = true;
        ui->pushButton_prev_sequencer->setText("S");
        ui->pushButton_next_sequencer->setText("L");
        ui->pushButton_prev_sequencer->setStyleSheet(
                    "background-color: #804040; color: #ffffff;");
        ui->pushButton_next_sequencer->setStyleSheet(
                    "background-color: #408040; color: #ffffff;");
        ui->pushButton_disk->setStyleSheet("color: rgb(255,255,255); background-color: #808080;");
    } else {
        disk_menu = false;
        ui->pushButton_prev_sequencer->setText("<");
        ui->pushButton_next_sequencer->setText(">");
        ui->pushButton_prev_sequencer->setStyleSheet(
                    "color: rgb(255,255,255); background-color: #606080;");
        ui->pushButton_next_sequencer->setStyleSheet(
                    "color: rgb(255,255,255); background-color: #606080;");
        ui->pushButton_disk->setStyleSheet("color: rgb(255,255,255); background-color: #404040;");
    }
}

void MainWindow::save_current_seq()
{
    QString o_file_name = "M64-SEQ-" + QString::number(stepsequencer->sequencer->nr); // or stepsequencer.sequencer_nr

    QFile o_file(o_file_name);
    o_file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream o_stream(&o_file);

    o_stream << stepsequencer->sequencer->serialize() << endl;
    for(int i=0; i < stepsequencer->sequencer->steps.count(); i++) {
        o_stream << QString::number(stepsequencer->sequencer->steps.at(i)->length);
        if(i < (stepsequencer->sequencer->steps.count()-1) ) o_stream << ",";
    }

    o_stream << endl;

    o_file.close();
}

void MainWindow::load_current_seq()
{
    QString i_file_name = "M64-SEQ-" + QString::number(stepsequencer->sequencer->nr); // or stepsequencer.sequencer_nr
    QString content;

    // Create a new file
    QFile i_file(i_file_name);

    if(!i_file.exists()) {
        stepsequencer->show_message("ERROR: FILE NOT FOUND!", 3);
        return;
    }

    i_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream i_stream(&i_file);

    content = i_stream.readLine();
    stepsequencer->sequencer->parse(content);

    content = i_stream.readLine();
    stepsequencer->sequencer->parse_lengths(content);

    stepsequencer->sequencer->create_midi_data(fs->gtn + fs->gto*8);

    i_file.close();
    stepsequencer->update();
}

void MainWindow::timer_logo_hit()
{
    toggle_logo = 1 - toggle_logo;

    if(toggle_logo) {
        ui->label_logo->setPixmap(*img_m64);
    } else {
        ui->label_logo->setPixmap(*img_nintendo);
    }
}

void MainWindow::button_settings_clicked()
{
    fs->show();
}

void MainWindow::timer_bpm_hit()
{
    if(!fs->timer_sync) return;

    int next_step = stepsequencer->sequencer->play_position + 1;
    if(next_step >= 32) next_step = 0;

    int status = serial->midi_play_step(next_step);

    stepsequencer->set_play_position(next_step);
    step_display->set_step_play(next_step);

    stepsequencer->status = status;
    stepsequencer->update();
}

void MainWindow::apply_settings()
{
    stepsequencer->sequencer->create_midi_data(fs->gtn + fs->gto*8);

    if(fs->timer_sync) {
        if(!timer_bpm.isActive()) {
            stepsequencer->set_play_position(0);
            serial->sequencer = stepsequencer->sequencer;
            serial->playing = false;
            timer_bpm.setInterval( (60.0 / fs->bpm)*250.0 );
            timer_bpm.start();
        } else {
            if(timer_bpm.interval() != ((60.0 / fs->bpm)*250.0) )
                timer_bpm.setInterval( (60.0 / fs->bpm)*250.0 );
        }
    } else {
        timer_bpm.stop();
    }
}

void MainWindow::button_clk_clicked()
{
    fs->button_clock_clicked();

    if(fs->timer_sync) {
        if(!timer_bpm.isActive()) {
            stepsequencer->set_play_position(0);
            serial->sequencer = stepsequencer->sequencer;
            serial->playing = false;
            timer_bpm.setInterval( (60.0 / fs->bpm)*250.0 );
            timer_bpm.start();
            ui->pushButton_clk->setStyleSheet("color: #ffffff; background-color: #e0a0e0;");
        } else {
            if(timer_bpm.interval() != ((60.0 / fs->bpm)*250.0) )
                timer_bpm.setInterval( (60.0 / fs->bpm)*250.0 );
        }
    } else {
        timer_bpm.stop();
        ui->pushButton_clk->setStyleSheet("color: #ffffff; background-color: #404060;");
    }
}

void MainWindow::set_simulate(bool yn)
{
    simulate = yn;
    if(serial) serial->simulate = yn;
}
