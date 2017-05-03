#include "form_edit_note.h"
#include "ui_form_edit_note.h"

Form_edit_note::Form_edit_note(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_edit_note)
{
    ui->setupUi(this);

    c_gray = "color: #000000; background-color: #404040;";
    c_blu =  "color: #ffffff; background-color: #404060;";

    connect(ui->pushButton_ok, SIGNAL(clicked(bool)),
            this, SLOT(button_ok_clicked()));

    connect(ui->pushButton_cancel, SIGNAL(clicked()),
            this, SLOT(button_cancel_clicked()));


    connect(ui->pushButton_C, SIGNAL(clicked()),
            this, SLOT(button_c_clicked()));
    connect(ui->pushButton_D, SIGNAL(clicked()),
            this, SLOT(button_d_clicked()));
    connect(ui->pushButton_E, SIGNAL(clicked()),
            this, SLOT(button_e_clicked()));
    connect(ui->pushButton_F, SIGNAL(clicked()),
            this, SLOT(button_f_clicked()));
    connect(ui->pushButton_G, SIGNAL(clicked()),
            this, SLOT(button_g_clicked()));
    connect(ui->pushButton_A, SIGNAL(clicked()),
            this, SLOT(button_a_clicked()));
    connect(ui->pushButton_H, SIGNAL(clicked()),
            this, SLOT(button_h_clicked()));

    connect(ui->pushButton_sharp, SIGNAL(clicked()),
            this, SLOT(button_sharp_clicked()));

    connect(ui->pushButton__o0, SIGNAL(clicked()),
            this, SLOT(button_0_clicked()));
    connect(ui->pushButton__o1, SIGNAL(clicked()),
            this, SLOT(button_1_clicked()));
    connect(ui->pushButton__o2, SIGNAL(clicked()),
            this, SLOT(button_2_clicked()));
    connect(ui->pushButton__o3, SIGNAL(clicked()),
            this, SLOT(button_3_clicked()));
    connect(ui->pushButton__o4, SIGNAL(clicked()),
            this, SLOT(button_4_clicked()));
    connect(ui->pushButton__o5, SIGNAL(clicked()),
            this, SLOT(button_5_clicked()));
    connect(ui->pushButton__o6, SIGNAL(clicked()),
            this, SLOT(button_6_clicked()));

    connect(ui->pushButton_del, SIGNAL(clicked()),
            this, SLOT(button_del_clicked()));

    connect(ui->horizontalSlider_velocity, SIGNAL(valueChanged(int)),
            this, SLOT(velocity_slided()));

    connect(ui->horizontalSlider_length, SIGNAL(valueChanged(int)),
            this, SLOT(length_slided()));
}

Form_edit_note::~Form_edit_note()
{
    delete ui;
}


void Form_edit_note::button_ok_clicked()
{
    emit save_clicked();
    this->hide();
}

void Form_edit_note::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.fillRect(0,0, this->width(), this->height(),
                     QColor::fromRgb(0x20,0x20,0x30));

    painter.setPen(QColor::fromRgb(0x0, 0x0, 0x0));
    painter.drawLine(0,0, 0, this->height());
    painter.drawLine(this->width()-1,0, this->width()-1, this->height());
    painter.drawLine(0,0, 0, this->height());
}

void Form_edit_note::button_cancel_clicked()
{
    this->hide();
}

void Form_edit_note::set_note(char nn)
{
    if(nn == '.') {
        note = nn;
        reset_colors();
        return;
    }

    ui->pushButton_C->setStyleSheet(c_gray);
    ui->pushButton_D->setStyleSheet(c_gray);
    ui->pushButton_E->setStyleSheet(c_gray);
    ui->pushButton_F->setStyleSheet(c_gray);
    ui->pushButton_G->setStyleSheet(c_gray);
    ui->pushButton_A->setStyleSheet(c_gray);
    ui->pushButton_H->setStyleSheet(c_gray);
    ui->pushButton_sharp->setStyleSheet(c_gray);

    char n2 = nn;
    if(n2 < 0x60) n2+= 0x20; // tolower
    if((n2 == 'c')) ui->pushButton_C->setStyleSheet(c_blu);
    if((n2 == 'd')) ui->pushButton_D->setStyleSheet(c_blu);
    if((n2 == 'e')) ui->pushButton_E->setStyleSheet(c_blu);
    if((n2 == 'f')) ui->pushButton_F->setStyleSheet(c_blu);
    if((n2 == 'g')) ui->pushButton_G->setStyleSheet(c_blu);
    if((n2 == 'a')) ui->pushButton_A->setStyleSheet(c_blu);
    if((n2 == 'h')) ui->pushButton_H->setStyleSheet(c_blu);

    if(nn < 0x60) ui->pushButton_sharp->setStyleSheet(c_blu);

    note = nn; // remember me
}


void Form_edit_note::set_octave(int o)
{
    octave = o;

    ui->pushButton__o0->setStyleSheet(c_gray);
    ui->pushButton__o1->setStyleSheet(c_gray);
    ui->pushButton__o2->setStyleSheet(c_gray);
    ui->pushButton__o3->setStyleSheet(c_gray);
    ui->pushButton__o4->setStyleSheet(c_gray);
    ui->pushButton__o5->setStyleSheet(c_gray);
    ui->pushButton__o6->setStyleSheet(c_gray);

    if(o==0) ui->pushButton__o0->setStyleSheet(c_blu);
    if(o==1) ui->pushButton__o1->setStyleSheet(c_blu);
    if(o==2) ui->pushButton__o2->setStyleSheet(c_blu);
    if(o==3) ui->pushButton__o3->setStyleSheet(c_blu);
    if(o==4) ui->pushButton__o4->setStyleSheet(c_blu);
    if(o==5) ui->pushButton__o5->setStyleSheet(c_blu);
    if(o==6) ui->pushButton__o6->setStyleSheet(c_blu);
}

void Form_edit_note::reset_colors()
{
    ui->pushButton_C->setStyleSheet(c_gray);
    ui->pushButton_D->setStyleSheet(c_gray);
    ui->pushButton_E->setStyleSheet(c_gray);
    ui->pushButton_F->setStyleSheet(c_gray);
    ui->pushButton_G->setStyleSheet(c_gray);
    ui->pushButton_A->setStyleSheet(c_gray);
    ui->pushButton_H->setStyleSheet(c_gray);
    ui->pushButton_sharp->setStyleSheet(c_gray);

    ui->pushButton__o0->setStyleSheet(c_gray);
    ui->pushButton__o1->setStyleSheet(c_gray);
    ui->pushButton__o2->setStyleSheet(c_gray);
    ui->pushButton__o3->setStyleSheet(c_gray);
    ui->pushButton__o4->setStyleSheet(c_gray);
    ui->pushButton__o5->setStyleSheet(c_gray);
    ui->pushButton__o6->setStyleSheet(c_gray);
}

void Form_edit_note::button_c_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('c');
    else set_note('C');
}

void Form_edit_note::button_d_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('d');
    else set_note('D');
}

void Form_edit_note::button_e_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('e');
    else set_note('E');
}

void Form_edit_note::button_f_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('f');
    else set_note('F');
}

void Form_edit_note::button_g_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('g');
    else set_note('G');
}

void Form_edit_note::button_a_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('a');
    else set_note('A');
}

void Form_edit_note::button_h_clicked()
{
    if((note > 0x60) || (note == '.'))
        set_note('h');
    else set_note('H');
}

void Form_edit_note::button_sharp_clicked()
{
    if(note == '.') return;

    if(note > 0x60) {
        note -= 0x20;
        ui->pushButton_sharp->setStyleSheet(c_blu);
    } else {
        note+=0x20;
        ui->pushButton_sharp->setStyleSheet(c_gray);
    }
}

void Form_edit_note::button_0_clicked()
{
    set_octave(0);
}

void Form_edit_note::button_1_clicked()
{
    set_octave(1);
}

void Form_edit_note::button_2_clicked()
{
    set_octave(2);
}

void Form_edit_note::button_3_clicked()
{
    set_octave(3);
}

void Form_edit_note::button_4_clicked()
{
    set_octave(4);
}

void Form_edit_note::button_5_clicked()
{
    set_octave(5);
}

void Form_edit_note::button_6_clicked()
{
    set_octave(6);
}

void Form_edit_note::button_del_clicked()
{
    set_note('.');
    reset_colors();
}

void Form_edit_note::velocity_slided()
{
    velocity = ui->horizontalSlider_velocity->value();
    ui->label_velocity->setText("V: " + QString::number(
                                    velocity));
}

void Form_edit_note::set_velocity(int v)
{
    ui->horizontalSlider_velocity->setValue(v);
    velocity = v;
}

void Form_edit_note::length_slided()
{
    length = ui->horizontalSlider_length->value();
    ui->label_length->setText("L: " + QString::number(
                                  length));
}
void Form_edit_note::set_length(int l)
{
    ui->horizontalSlider_length->setValue(l);
    length = l;
}





