#include "rv_hex_editor_bufferpane.h"
#include "ui_rv_hex_editor_bufferpane.h"

rv_hex_editor_bufferpane::rv_hex_editor_bufferpane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rv_hex_editor_bufferpane)
{
    ui->setupUi(this);

    // connect forwards
    connect(ui->pushButton_paste_sel_up, SIGNAL(clicked()),
            this, SIGNAL(paste_buffer_selection_to_main()));

    connect(ui->pushButton_paste_whole_buf_up, SIGNAL(clicked()),
            this, SIGNAL(paste_whole_buffer_to_main()));
}

rv_hex_editor_bufferpane::~rv_hex_editor_bufferpane()
{
    delete ui;
}

void rv_hex_editor_bufferpane::insert_buffer_tab_widget(QTabWidget *w)
{
    ui->verticalLayout->insertWidget(0, w);
}

void rv_hex_editor_bufferpane::enable_buffers(bool checked)
{
    if(checked) {
        ui->label_no_buffers->hide();
        ui->groupBox_paste->setEnabled(true);
        ui->groupBox_paste_sel->setEnabled(true);
    } else {
        ui->label_no_buffers->show();
        ui->groupBox_paste->setEnabled(false);
        ui->groupBox_paste_sel->setEnabled(false);
    }
}
