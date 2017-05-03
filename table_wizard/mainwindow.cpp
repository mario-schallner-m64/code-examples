#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    // ui setup

    rb_tools::tools_init();

    ui->setupUi(this);

    rbd = new rb_displayer(this);
    rbd2 = new rb_displayer(this);
    rbd2->setMaximumWidth(1000);
    scrn = new rb_screen(this);
    cpicker = new rb_color_picker(this);
    the_screen = new render_scrn();
    the_screen->hide();

    ui->hl_disps->addWidget(rbd);
    //ui->horizontalLayout_rbsa->insertWidget(0, rbd2);
    ui->verticalLayout_rbd2->addWidget(rbd2);
    ui->hl_disps->addWidget(scrn);
    ui->hl_disps->addWidget(the_screen);

    ui->verticalLayout_cpicker->addWidget(cpicker);

    ui->tableWidget_rb->setRowCount(256);

    ui->groupBox_rb_animation->hide();
    ui->groupBox_rb_effects->hide();
    ui->groupBox_rb_effect_parameters->hide();

    // connects

    connect(ui->listWidget_rasterbars, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(rb_lwi_changed(QListWidgetItem*,QListWidgetItem*)));

    connect(ui->spinBox_rb_lines, SIGNAL(valueChanged(int)),
            this, SLOT(inc_dec_rb_size(int)));


    connect(cpicker, SIGNAL(color_selected(int)),
            this, SLOT(color_clicked(int)));

    connect(ui->spinBox_rb_y, SIGNAL(valueChanged(int)),
            this, SLOT(inc_dec_y(int)));

    connect(ui->spinBox_rb_z, SIGNAL(valueChanged(int)),
            this, SLOT(inc_dec_z(int)));

    connect(rbd, SIGNAL(y_changed(int)),
            this, SLOT(set_y(int)));

    connect(rbd2, SIGNAL(y_changed(int)),
            this, SLOT(set_y(int)));


    // -- animation stuff --

    connect(ui->checkBox_edit_animation, SIGNAL(toggled(bool)),
            this, SLOT(toggle_edit_animation(bool)));

    connect(ui->horizontalSlider_rb_ani_step, SIGNAL(valueChanged(int)),
            this, SLOT(rb_ani_current_frame_changed(int)));

    connect(ui->comboBox_ani_nr, SIGNAL(currentIndexChanged(int)),
            this, SLOT(rb_ani_nr_changed(int)));

    connect(ui->spinBox_rb_ani_frame_cnt, SIGNAL(valueChanged(int)),
            this, SLOT(rb_ani_frame_cnt_changed(int)));

    connect(ui->spinBox_rb_ani_delay, SIGNAL(valueChanged(int)),
            this, SLOT(rb_ani_delay_frames_changed(int)));

    connect(&rb_ani_timer, SIGNAL(timeout()),
            this, SLOT(rb_ani_timer_hit()));

    connect(ui->pushButton_rb_clone, SIGNAL(clicked()),
            this, SLOT(pushButton_rb_clone_clicked()));

    connect(ui->listWidget_rbsa_effects, SIGNAL(clicked(QModelIndex)),
            this, SLOT(rbsa_effect_selected(QModelIndex)));

    connect(ui->tableWidget_rbsa_eff_params, SIGNAL(cellChanged(int,int)),
            this, SLOT(rbsa_effect_param_val_changed(int,int)));

    edit_animation = false;
    rb_ani_nr = 0;
    rb_ani_frame_cnt = 0;
    rb_ani_current_frame_nr = 0;
    rb_ani_delay_frames = 1;
    rb_ani_running = false;

    init_rbsa_effects();

    // -- scrn animation stuff --
    connect(ui->checkBox_scrn_ani, SIGNAL(toggled(bool)),
            this, SLOT(toggle_edit_scrn_animation(bool)));

    connect(ui->listWidget_scrn_ani_effects, SIGNAL(clicked(QModelIndex)),
            this, SLOT(scrn_effect_selected(QModelIndex)));

    ui->groupBox_scrn_ani->hide();

    edit_scrn_ani = false;
    ui->checkBox_scrn_ani->setDisabled(true);

    init_scrn_effects();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_rb_new_clicked()
{
    rb* rbar = new rb();    

    if(rbar->name.isEmpty()) rbar->name =
            "rasterbar #" +
            QString::number(ui->listWidget_rasterbars->count());

    rbs.append(rbar);

    rb_listwidgetitem *lwi = new rb_listwidgetitem();
    lwi->setText(rbar->name);
    lwi->set_rbar(rbar);
    ui->listWidget_rasterbars->addItem(lwi);

    qSort(rbs.begin(), rbs.end(), rb::sort_rb_Z);

    scrn->rbars = rbs;

    scrn->update();
    ui->listWidget_rasterbars->setCurrentItem(lwi);

    ui->checkBox_edit_animation->setEnabled(true);
    ui->checkBox_scrn_ani->setEnabled(true);
}

void MainWindow::pushButton_rb_clone_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar_orig =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    rb* rbar = new rb();

    for(int i=0; i<256; i++) {
        rbar->values[i] = rbar_orig->values[i];
    }
    for(int i=0; i<256*256*16; i++) {
        rbar->animations[i] = rbar_orig->animations[i];
    }

    if(rbar->name.isEmpty()) rbar->name =
            "rasterbar #" +
            QString::number(ui->listWidget_rasterbars->count());

    rbs.append(rbar);

    rb_listwidgetitem *lwi = new rb_listwidgetitem();
    lwi->setText(rbar->name);
    lwi->set_rbar(rbar);
    ui->listWidget_rasterbars->addItem(lwi);

    qSort(rbs.begin(), rbs.end(), rb::sort_rb_Z);
    scrn->rbars = rbs;

    scrn->update();
    ui->listWidget_rasterbars->setCurrentItem(lwi);
}

void MainWindow::on_pushButton_rb_delete_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    int row = ui->listWidget_rasterbars->currentRow();

    rb_listwidgetitem *lwi =
            (rb_listwidgetitem*)ui->listWidget_rasterbars->takeItem(row);

    rb *rbar = lwi->rbar;

    rbd->set_rb(0);
    rbd2->set_rb(0);
    rbs.removeOne(rbar);
    scrn->rbars = rbs;
    scrn->update();

    delete(lwi);
    delete(rbar);

    if(ui->listWidget_rasterbars->count() == 0) {
        ui->checkBox_edit_animation->setDisabled(true);
        ui->checkBox_scrn_ani->setDisabled(true);
    }
}

void MainWindow::on_pushButton_rb_dump_clicked()
{
    QString dumpstr;
    foreach(QListWidgetItem *lwi, ui->listWidget_rasterbars->selectedItems()) {
        dumpstr = ((rb_listwidgetitem *)lwi)->rbar->dump(
                    ui->comboBox_dump_mode->currentIndex(),
                    ui->lineEdit_dump_prefix->text(),
                    ui->comboBox_dump_linecnt->currentText().toInt());
        ui->textEdit->append("<pre>" + dumpstr + "\n</pre>");
    }
}

void MainWindow::rb_lwi_changed(QListWidgetItem *lwi_new, QListWidgetItem *lwi_old)
{

    if(!lwi_new) return;

    ui->checkBox_edit_animation->setChecked(false);

    rb *rbar = ((rb_listwidgetitem*)lwi_new)->rbar;
    rbd->set_rb(rbar);
    rbd2->set_rb(rbar);
    int old_size = ui->spinBox_rb_lines->value();
    ui->spinBox_rb_lines->setValue(rbar->size);
    ui->spinBox_rb_z->setValue(rbar->z);
    ui->spinBox_rb_y->setValue(rbar->y);
    rb2table(rbar);
}

void MainWindow::rb2table(rb *rbar)
{
    ui->tableWidget_rb->clear();
    if(!rbar->size) return;

    ui->tableWidget_rb->setRowCount(rbar->size);
    ui->tableWidget_rb->setColumnCount(1);

    if(!edit_animation) {
        // normal
        for(int i=0; i < rbar->size; i++) {
            rb_table_widget *tw = new rb_table_widget();
            tw->nr = i;
            tw->color = rb_tools::colortbl.at(rbar->values[i]);
            ui->tableWidget_rb->setCellWidget(i,0,tw);
        }
        rbd->set_rb(rbar);
        rbd2->set_rb(rbar);
    } else {
        for(int i=0; i < rbar->size; i++) {
            rb_table_widget *tw = new rb_table_widget();
            tw->nr = i;
            tw->ani_nr = rb_ani_nr;
            tw->ani_frame = rb_ani_current_frame_nr;
            tw->color = rb_tools::colortbl.at(
                        //rbar->values[i]
                        rbar->animations[256*256*rb_ani_nr +
                            256*rb_ani_current_frame_nr + i]
                        );
            ui->tableWidget_rb->setCellWidget(i,0,tw);
        }
        rbd->display_ani(rb_ani_nr, rb_ani_current_frame_nr);
        rbd2->display_ani(rb_ani_nr, rb_ani_current_frame_nr);
    }

    // update icon
    rb_listwidgetitem *lwi;
    if(!(lwi=((rb_listwidgetitem *)ui->listWidget_rasterbars->currentItem()))) return;
    lwi->set_rbar(rbar);
    scrn->update();
}

void MainWindow::inc_dec_rb_size(int newsz)
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    rbar->size = newsz;
    rb2table(rbar);
}

void MainWindow::color_clicked(int c)
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    int r;
    if(!ui->tableWidget_rb->selectedRanges().isEmpty()) {
        foreach(QTableWidgetSelectionRange sr, ui->tableWidget_rb->selectedRanges()) {
            for(r = sr.topRow(); r<=sr.bottomRow(); r++) {
                if(!edit_animation) rbar->values[r] = c;
                else rbar->animations[256*256*rb_ani_nr +
                        256*rb_ani_current_frame_nr + r] = c;
            }
        }
        rb2table(rbar);
        ui->tableWidget_rb->selectRow(r); // is allready advanced by 1
    }
}

void MainWindow::inc_dec_y(int y)
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    rbar->y = y;

    rbd->set_rb(rbar);
    rbd2->set_rb(rbar);

    scrn->update();
}


void MainWindow::on_pushButton_rb_ins_line_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;
    if(ui->tableWidget_rb->selectedRanges().isEmpty()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    int r = ui->tableWidget_rb->selectedRanges().at(0).topRow();

    for(int i=rbar->size-1; i>=r; i--) {

        if(!edit_animation) rbar->values[i+1] = rbar->values[i];
        else rbar->animations[256*256*rb_ani_nr +
                256*rb_ani_current_frame_nr + i+1] =
                rbar->animations[256*256*rb_ani_nr +
                                256*rb_ani_current_frame_nr + i];
    }
    if(rbar->size<255) rbar->size++;
    ui->spinBox_rb_lines->setValue(rbar->size);
    ui->tableWidget_rb->selectRow(r);
}


void MainWindow::on_pushButton_rb_rmv_line_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;
    if(ui->tableWidget_rb->selectedRanges().isEmpty()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    if(rbar->size == 1) { rbar->values[0] = 0; return; }

    int r = ui->tableWidget_rb->selectedRanges().at(0).topRow();

    for(int i=r; i<rbar->size; i++) {
        if(!edit_animation) rbar->values[i] = rbar->values[i+1];
        else rbar->animations[256*256*rb_ani_nr +
                256*rb_ani_current_frame_nr + i] =
                rbar->animations[256*256*rb_ani_nr +
                                256*rb_ani_current_frame_nr + i+1];
    }
    rbar->size--;
    ui->spinBox_rb_lines->setValue(rbar->size);
    ui->tableWidget_rb->selectRow(r);
}

void MainWindow::on_pushButton_rb_copy_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    int r;
    if(!ui->tableWidget_rb->selectedRanges().isEmpty()) {

        QTableWidgetSelectionRange sr =
                ui->tableWidget_rb->selectedRanges().at(0);
        for(r = sr.topRow(); r<=sr.bottomRow(); r++) {
            if(!edit_animation)
                rb_tools::copy_buffer.buffer[r] = rbar->values[r];
            else
                rb_tools::copy_buffer.buffer[r] =
                        rbar->animations[256*256*rb_ani_nr +
                                256*rb_ani_current_frame_nr + r];
        }

        rb_tools::copy_buffer.start = sr.topRow();
        rb_tools::copy_buffer.end = sr.bottomRow();
        rb_tools::copy_buffer.have_copy = true;
    }
}

void MainWindow::on_pushButton_rb_paste_clicked()
{
    if(!rb_tools::copy_buffer.have_copy) return;

    if(!ui->listWidget_rasterbars->currentItem()) return;
    if(ui->tableWidget_rb->selectedRanges().isEmpty()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    int r = ui->tableWidget_rb->selectedRanges().at(0).topRow();

    int buffer_size = rb_tools::copy_buffer.end - rb_tools::copy_buffer.start + 1;

    int r_dest = r + buffer_size - 1; // "overwrite" teil nach unten

    int r_buf = rb_tools::copy_buffer.start;

    // first extend and move down

    if(!ui->checkBox_paste_ov->isChecked()) {
        int tmpdest = rbar->size-r;
        for(int i=rbar->size-1; i>=r; i--) {
            if(!edit_animation)
                rbar->values[r_dest+tmpdest] = rbar->values[i];
            else
                rbar->animations[256*256*rb_ani_nr +
                        256*rb_ani_current_frame_nr + r_dest+tmpdest] =
                        rbar->animations[256*256*rb_ani_nr +
                                256*rb_ani_current_frame_nr + i];
            tmpdest--;
        }

        for(int i=0; i<buffer_size; i++) {
            if((i + r_dest)>=256) break;
             if(!edit_animation)
                rbar->values[r+i] = rb_tools::copy_buffer.buffer[r_buf+i];
             else
                rbar->animations[256*256*rb_ani_nr +
                         256*rb_ani_current_frame_nr + r+i]  =
                        rb_tools::copy_buffer.buffer[r_buf+i];
        }

        rbar->size =  rbar->size +buffer_size;
        if(rbar->size > 256) rbar->size = 256;
    }
    else {
        for(int i=0; i < buffer_size; i++)
        if(!edit_animation)
            rbar->values[r+i] = rb_tools::copy_buffer.buffer[r_buf+i];
        else
            rbar->animations[256*256*rb_ani_nr +
                    256*rb_ani_current_frame_nr + r+i] =
                    rb_tools::copy_buffer.buffer[r_buf+i];
        rb2table(rbar);
    }
    ui->spinBox_rb_lines->setValue(rbar->size);
    ui->tableWidget_rb->selectRow(r);
}

void MainWindow::inc_dec_z(int z)
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    rbar->z = z;

    qSort(rbs.begin(), rbs.end(), rb::sort_rb_Z);

    scrn->rbars = rbs;

    scrn->update();
}

void MainWindow::set_y(int y)
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    ui->spinBox_rb_y->setValue(y);
}

// ---------------------------------- rb animation stuff -----------------------

void MainWindow::toggle_edit_animation(bool yesno)
{
    edit_animation = yesno;

    if(edit_animation) {
        ui->groupBox_scrn_ani->hide();
        ui->checkBox_scrn_ani->setChecked(false);

        rbd->hide();
        scrn->hide();
        the_screen->hide();
        ui->textEdit->hide();
        ui->groupBox_scrn_ani->hide();
        ui->groupBox_rb_animation->show();
        ui->groupBox_rb_effects->show();
        ui->groupBox_rb_effect_parameters->show();
    }
    else {
        ui->groupBox_rb_animation->hide();
        ui->groupBox_rb_effects->hide();
        ui->groupBox_rb_effect_parameters->hide();
        ui->textEdit->show();
        scrn->show();
        rbd->show();
    }
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;
    rb2table(rbar);
}

void MainWindow::rb_ani_current_frame_changed(int f)
{
    rb_ani_current_frame_nr = f;
    if(!ui->listWidget_rasterbars->currentItem()) return;

    ui->lineEdit_rb_ani_frame_nr->setText(QString::number(rb_ani_current_frame_nr));

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    rb2table(rbar);
}

void MainWindow::on_pushButton_rb_ani_rb2editor_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    for(int i=0; i<256; i++)     {
        rbar->animations[256*256*rb_ani_nr +
                                256*rb_ani_current_frame_nr + i] =
                rbar->values[i];
    }

    rb2table(rbar);
}

void MainWindow::rb_ani_frame_cnt_changed(int cnt)
{
    ui->horizontalSlider_rb_ani_step->setMaximum(cnt-1);
    rb_ani_frame_cnt = cnt;
}

void MainWindow::rb_ani_nr_changed(int nr)
{
    rb_ani_nr = nr;
}

void MainWindow::rb_ani_delay_frames_changed(int f)
{
    rb_ani_delay_frames = f;
}

void MainWindow::on_pushButton_rb_ani_run_clicked()
{
    if(!rb_ani_running) {
        rb_ani_running = true;
        rb_ani_timer.setInterval(20*rb_ani_delay_frames);
        rb_ani_timer.start();
    } else {
        rb_ani_running = false;
        rb_ani_timer.stop();
    }
}

void MainWindow::rb_ani_timer_hit()
{
    rb_ani_current_frame_nr++;
    if(rb_ani_current_frame_nr == rb_ani_frame_cnt) rb_ani_current_frame_nr=0;

    ui->lineEdit_rb_ani_frame_nr->setText(QString::number(rb_ani_current_frame_nr));

    if(!ui->listWidget_rasterbars->currentItem()) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    rb2table(rbar);
}

void MainWindow::on_pushButton_rb_ani_dup2next_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    if(rb_ani_current_frame_nr == (rb_ani_frame_cnt-1)) return;

    rb *rbar =
            ((rb_listwidgetitem*)ui->listWidget_rasterbars->currentItem())->rbar;

    for(int i=0; i<256; i++)     {
        rbar->animations[
                256*256*rb_ani_nr +
                256*(rb_ani_current_frame_nr + 1) + i] =

                rbar->animations[
                        256*256*(rb_ani_nr) +
                        256*rb_ani_current_frame_nr + i];

    }

    rb_ani_current_frame_nr++;

    ui->horizontalSlider_rb_ani_step->setValue(rb_ani_current_frame_nr);
}

void MainWindow::on_pushButton_rb_ani_frame_next_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    if(rb_ani_current_frame_nr == (rb_ani_frame_cnt-1)) return;

    rb_ani_current_frame_nr++;

    ui->horizontalSlider_rb_ani_step->setValue(rb_ani_current_frame_nr);
}

void MainWindow::on_pushButton_rb_ani_frame_prev_clicked()
{
    if(!ui->listWidget_rasterbars->currentItem()) return;

    if(rb_ani_current_frame_nr == 0) return;

    rb_ani_current_frame_nr--;

    ui->horizontalSlider_rb_ani_step->setValue(rb_ani_current_frame_nr);
}

void MainWindow::init_rbsa_effects()
{
    rbsa_effect::init_effects();

    for(int i=0; i < rbsa_effect::effects.count(); i++) {
        ui->listWidget_rbsa_effects->addItem(rbsa_effect::effects.at(i)->name);
    }
    ui->tableWidget_rbsa_eff_params->resizeColumnsToContents();
}

void MainWindow::rbsa_effect_selected(QModelIndex idx)
{
    int e = idx.row();

    if(e < 0) return;
    if(e >= rbsa_effect::effects.count()) return;

    rbsa_effect *eff = rbsa_effect::effects.at(e);

    ui->textEdit_rbsa_desc->setText(eff->desc);

    ui->tableWidget_rbsa_eff_params->clear();
    ui->tableWidget_rbsa_eff_params->setRowCount(0);
    ui->tableWidget_rbsa_eff_params->setColumnCount(3);

    QStringList headers;
    headers.append("name");
    headers.append("type");
    headers.append("value");
    ui->tableWidget_rbsa_eff_params->setHorizontalHeaderLabels(headers);

    if(eff->parameters.isEmpty()) return;

    ui->tableWidget_rbsa_eff_params->setRowCount(
                eff->parameters.count());

    for(int i=0; i<eff->parameters.count(); i++) {
        QTableWidgetItem *twi;

        twi = new QTableWidgetItem();
        twi->setText(eff->parameters.at(i).name);
        twi->setFlags(Qt::ItemIsEnabled);
        ui->tableWidget_rbsa_eff_params->setItem(i, 0, twi);

        twi = new QTableWidgetItem();
        twi->setText(eff->parameters.at(i).type);
        twi->setFlags(Qt::ItemIsEnabled);
        ui->tableWidget_rbsa_eff_params->setItem(i, 1, twi);

        twi = new QTableWidgetItem();
        twi->setTextAlignment(Qt::AlignRight);
        if(eff->parameters.at(i).type == "integer") {
            twi->setText(QString::number(eff->parameters.at(i).i_value));
        }
        if(eff->parameters.at(i).type == "float") {
            twi->setText(QString::number(eff->parameters.at(i).f_value));
        }
        ui->tableWidget_rbsa_eff_params->setItem(i, 2, twi);
    }

    ui->tableWidget_rbsa_eff_params->resizeColumnsToContents();
    ui->tableWidget_rbsa_eff_params->resizeRowsToContents();
}

void MainWindow::rbsa_effect_param_val_changed(int r, int c)
{
    if(c != 2) return;
    if(r < 0) return;

    int e = ui->listWidget_rbsa_effects->currentIndex().row();
    if(e < 0) return;
    if(e >= rbsa_effect::effects.count()) return;

    rbsa_effect *eff = rbsa_effect::effects.at(e);
    if(r >= eff->parameters.count()) return;

    QTableWidgetItem *twi = ui->tableWidget_rbsa_eff_params->item(r, c);

    QString val = twi->text();

    if(eff->parameters.at(r).type == "integer") {
        int i_val = val.toInt();
        eff->parameters[r].i_value = i_val;
    }

    if(eff->parameters.at(r).type == "float") {
        float f_val = val.toFloat();
        eff->parameters[r].f_value = f_val;
    }
}

void MainWindow::toggle_edit_scrn_animation(bool yesno)
{
    if(yesno) {
        ui->checkBox_edit_animation->setChecked(false);
        ui->groupBox_rb_animation->hide();
        ui->groupBox_rb_effects->hide();
        ui->groupBox_rb_effect_parameters->hide();
        scrn->hide();
        ui->textEdit->hide();
        ui->groupBox_scrn_ani->show();
        the_screen->show();
        rbd->show();
        ui->checkBox_edit_animation->setChecked(false);
    } else {
        ui->groupBox_scrn_ani->hide();
        the_screen->hide();
        ui->textEdit->show();
        scrn->show();
    }
}

void MainWindow::init_scrn_effects()
{
    scrn_effect::init_effects();

    for(int i=0; i < scrn_effect::effects.count(); i++) {
        ui->listWidget_scrn_ani_effects->addItem(scrn_effect::effects.at(i)->name);
    }
    ui->tableWidget_rbsa_eff_params->resizeColumnsToContents();

    ui->tableWidget_scrn_ani->clear();
    ui->tableWidget_scrn_ani->setRowCount(0);
    ui->tableWidget_scrn_ani->setColumnCount(2);

    QStringList headers;

//    headers.append("Start");
//    headers.append("RB#");
//    headers.append("Len");
//    headers.append("End");
//    headers.append("Speed");
//    headers.append("Effect");
//    headers.append("P1");
//    headers.append("P2");
//    headers.append("P3");

    headers.append("#");
    headers.append("Command");
    ui->tableWidget_scrn_ani->setHorizontalHeaderLabels(headers);

    ui->tableWidget_scrn_ani->resizeColumnsToContents();
}

void MainWindow::scrn_effect_selected(QModelIndex idx)
{
    int e = idx.row();

    if(e < 0) return;
    if(e >= scrn_effect::effects.count()) return;

    scrn_effect *eff = scrn_effect::effects.at(e);

    ui->textEdit_scrn_ani->setText(eff->desc);

    ui->tableWidget_scrn_ani_parameters->clear();
    ui->tableWidget_scrn_ani_parameters->setRowCount(0);
    //ui->tableWidget_scrn_ani_parameters->setColumnCount(3);
    ui->tableWidget_scrn_ani_parameters->setColumnCount(2);

    QStringList headers;
    headers.append("name");
    headers.append("type");
    //headers.append("value");
    ui->tableWidget_scrn_ani_parameters->setHorizontalHeaderLabels(headers);

    if(eff->parameters.isEmpty()) return;

    ui->tableWidget_scrn_ani_parameters->setRowCount(
                eff->parameters.count());

    for(int i=0; i<eff->parameters.count(); i++) {
        QTableWidgetItem *twi;

        twi = new QTableWidgetItem();
        twi->setText(eff->parameters.at(i).name);
        twi->setFlags(Qt::ItemIsEnabled);
        ui->tableWidget_scrn_ani_parameters->setItem(i, 0, twi);

        twi = new QTableWidgetItem();
        twi->setText(eff->parameters.at(i).type);
        twi->setFlags(Qt::ItemIsEnabled);
        ui->tableWidget_scrn_ani_parameters->setItem(i, 1, twi);

//        twi = new QTableWidgetItem();
//        twi->setTextAlignment(Qt::AlignRight);
//        if(eff->parameters.at(i).type == "integer") {
//            twi->setText(QString::number(eff->parameters.at(i).i_value));
//        }
//        if(eff->parameters.at(i).type == "float") {
//            twi->setText(QString::number(eff->parameters.at(i).f_value));
//        }
//        ui->tableWidget_scrn_ani_parameters->setItem(i, 2, twi);
    }

    ui->tableWidget_scrn_ani_parameters->resizeColumnsToContents();
    ui->tableWidget_scrn_ani_parameters->resizeRowsToContents();
}

void MainWindow::on_pushButton_playlist_additem_clicked()
{

}

void MainWindow::on_pushButton_playlist_removeitem_clicked()
{

}
