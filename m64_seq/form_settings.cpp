#include "form_settings.h"
#include "ui_form_settings.h"

form_settings::form_settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::form_settings)
{
    ui->setupUi(this);
    connect(ui->pushButton_ok, SIGNAL(clicked(bool)),
            this, SLOT(button_ok_clicked()));

    connect(ui->pushButton_clock, SIGNAL(clicked(bool)),
            this, SLOT(button_clock_clicked()));

    connect(ui->horizontalSlider_bpm, SIGNAL(valueChanged(int)),
            this, SLOT(value_bpm_changed()));

    connect(ui->horizontalSlider_gtn, SIGNAL(valueChanged(int)),
            this, SLOT(value_gtn_changed()));

    connect(ui->horizontalSlider_gto, SIGNAL(valueChanged(int)),
            this, SLOT(value_gto_changed()));

    ui->pushButton_clock->hide();
}

form_settings::~form_settings()
{
    delete ui;
}


void form_settings::button_ok_clicked()
{
    emit M64_OK_Clicked();
    this->hide();
}


void form_settings::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.fillRect(0,0, this->width(), this->height(),
                     QColor::fromRgb(0x20,0x20,0x20));

    painter.setPen(QColor::fromRgb(0x0, 0x0, 0x0));
    painter.drawLine(0,0, 0, this->height());
    painter.drawLine(this->width()-1,0, this->width()-1, this->height());
    painter.drawLine(0,0, 0, this->height());

    //QWidget::paintEvent(e);
}


void form_settings::button_clock_clicked()
{
    if(!timer_sync) {
        timer_sync = true;
        ui->pushButton_clock->setStyleSheet("background-color: #f0f0c0;");
    } else {
        timer_sync = false;
        ui->pushButton_clock->setStyleSheet("background-color: #606060;");
    }

    //
}


void form_settings::value_bpm_changed()
{
    bpm = ui->horizontalSlider_bpm->value();
    ui->label_bpm->setText("BPM: " + QString::number(bpm));
}

void form_settings::value_gtn_changed()
{
    gtn = ui->horizontalSlider_gtn->value();
    ui->label_gtn->setText("GTN: " + QString::number(gtn));
}

void form_settings::value_gto_changed()
{
    gto = ui->horizontalSlider_gto->value();
    ui->label_gto->setText("GTO: " + QString::number(gto));
}
