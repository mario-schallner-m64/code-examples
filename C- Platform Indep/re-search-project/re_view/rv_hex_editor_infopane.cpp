#include "rv_hex_editor_infopane.h"
#include "ui_rv_hex_editor_infopane.h"

rv_hex_editor_infopane::rv_hex_editor_infopane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rv_hex_editor_infopane)
{
    ui->setupUi(this);

    main_layout = 0;
    setup_side_infos();

    // forwards
    connect(ui->pushButton_copy_sel_into_new_buf, SIGNAL(clicked()),
            this, SIGNAL(copy_selection_into_new_buffer()));

    connect(ui->lineEdit_buffer_name, SIGNAL(textEdited(QString)),
            this, SIGNAL(buffer_rename(QString)));

    connect(ui->pushButton_add_targetfilebuffer, SIGNAL(clicked()),
            this, SIGNAL(new_target_buffer()));

    connect(ui->pushButton_add_buffer_from_file, SIGNAL(clicked()),
            this, SIGNAL(load_file_into_new_buffer_clicked()));

    connect(ui->pushButton_new_00_buffer, SIGNAL(clicked()),
            this, SIGNAL(new_00_buffer_clicked()));

    connect(ui->groupBox_buffers, SIGNAL(toggled(bool)),
            this, SIGNAL(buffers_toggled(bool)));

    connect(ui->pushButton_clear_navi_stack, SIGNAL(clicked()),
            this, SIGNAL(clear_navi_stack_clicked()));

    connect(ui->listWidget_navi_stack, SIGNAL(currentRowChanged(int)),
            this, SIGNAL(navi_stack_goto(int)));

    connect(ui->pushButton_evaluate_run, SIGNAL(clicked()),
            this, SIGNAL(evaluator_run_clicked()));

    connect(ui->pushButton_evaluate_abort, SIGNAL(clicked()),
            this, SIGNAL(evaluator_abort_clicked()));

    connect(ui->actionNew_display_widget_connected_to_cursor_move, SIGNAL(triggered()),
            this, SIGNAL(action_new_side_widget_cursor()));
    connect(ui->actionNew_display_widget_connected_to_selection_change, SIGNAL(triggered()),
            this, SIGNAL(action_new_side_widget_selection()));
    connect(ui->actionNew_display_widget_triggered_by_execute_button, SIGNAL(triggered()),
            this, SIGNAL(action_new_side_widget_button()));

    ui->pushButton_evaluate_abort->setEnabled(false);

    highlighter = new rv_script_highlighter(
                ui->plainTextEdit_evaluator_input->document());

    redo_layout(true);
}

rv_hex_editor_infopane::~rv_hex_editor_infopane()
{
    delete listWidget_side_items;
    delete toolbutton_new_sidewidgets;
    delete toolbar_sideitems;
    delete main_layout;

    delete ui;
}

// -- cursor info --
void rv_hex_editor_infopane::set_cursor_offset_hex(QString str)
{
    ui->label_cursor_offset_hex->setText(str);
}

void rv_hex_editor_infopane::set_cursor_offset_dec(QString str)
{
    ui->label_cursor_offset_dec->setText(str);
}

void rv_hex_editor_infopane::set_cursor_offset_va(QString str)
{
    ui->label_cursor_offset_va->setText(str);
}

void rv_hex_editor_infopane::set_cursor_byte(QString str)
{
    ui->label_cursor_val_8bit->setText(str);
}

void rv_hex_editor_infopane::set_cursor_word(QString str)
{
    ui->label_cursor_val_16bit->setText(str);
}

void rv_hex_editor_infopane::set_cursor_dword(QString str)
{
    ui->label_cursor_val_32bit->setText(str);
}

void rv_hex_editor_infopane::set_selection_info(QString start_hex,
                                                QString start_dec,
                                                QString start_va,
                                                QString end_hex,
                                                QString end_dec,
                                                QString end_va,
                                                QString sz)
{
    ui->label_sel_start_hex->setText(start_hex);
    ui->label_sel_start_dec->setText(start_dec);
    ui->label_sel_start_va->setText(start_va);

    ui->label_sel_end_hex->setText(end_hex);
    ui->label_sel_end_dec->setText(end_dec);
    ui->label_sel_end_va->setText(end_va);

    ui->label_sel_size->setText(sz);

}

void rv_hex_editor_infopane::setup_side_infos()
{
    QListWidgetItem *li;
    QGroupBox *group_box;

    listWidget_side_items = new rv_sideinfo_listbox();
    listWidget_side_items->setAlternatingRowColors(true);
    listWidget_side_items->setDragDropMode(QAbstractItemView::InternalMove);

    connect(listWidget_side_items, SIGNAL(drop_happened()),
            this, SLOT(redo_layout()));

    ui->groupBox_side_items->layout()->addWidget(listWidget_side_items);

    info_items["Cursor Info"] = ui->groupBox_cursor;
    info_items["Selection Info"] = ui->groupBox_selection_info;
    info_items["Buffers"] = ui->groupBox_buffers;
    info_items["Navigation Stack"] = ui->groupBox_navi_stack;
    info_items["Evaluator"] = ui->groupBox_evaluator;
    info_items["Side Infos"] = ui->groupBox_side_items;


    QStringList ordering;
    ordering << "Cursor Info" <<
                "Selection Info" <<
                "Buffers" <<
                "Evaluator" <<
                "Navigation Stack";

    ui->groupBox_buffers->setChecked(false);
    ui->groupBox_navi_stack->setChecked(false);
    ui->groupBox_side_items->setChecked(false);
    ui->groupBox_evaluator->setChecked(false);
    ui->groupBox_selection_info->setChecked(false);


    foreach(QString li_str, ordering) {
        li = new QListWidgetItem(li_str);
        li->setCheckState(Qt::Checked);
        listWidget_side_items->addItem(li);
        group_box = (QGroupBox *)info_items.value(li_str);

        connect(group_box, SIGNAL(toggled(bool)),
                this, SLOT(redo_layout(bool)));
    }

    connect(ui->groupBox_side_items, SIGNAL(toggled(bool)),
            this, SLOT(redo_layout(bool)));

    connect(listWidget_side_items, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(show_hide_info_item(QListWidgetItem*)));

    // new script toolbar
    toolbar_sideitems = new QToolBar(this);
    toolbutton_new_sidewidgets = new QToolButton(this);
    toolbutton_new_sidewidgets->addAction(ui->actionNew_display_widget_connected_to_cursor_move);
    toolbutton_new_sidewidgets->addAction(ui->actionNew_display_widget_connected_to_selection_change);
    toolbutton_new_sidewidgets->addAction(ui->actionNew_display_widget_triggered_by_execute_button);
    toolbutton_new_sidewidgets->setAutoRaise(true);
    toolbutton_new_sidewidgets->setPopupMode(QToolButton::InstantPopup);
    toolbutton_new_sidewidgets->setIcon(QIcon(":/icons/re_script_run3.png"));
    toolbar_sideitems->addWidget(toolbutton_new_sidewidgets);

    ui->groupBox_side_items->layout()->addWidget(toolbar_sideitems);

    // do init dimenstions
    info_items_heights["Side Infos"] =info_items.count() * 20 + 40;
    info_items_widths["Side Infos"] = 200;

    info_items_heights["Cursor Info"] = 280;
    info_items_widths["Cursor Info"] = 200;

    info_items_heights["Selection Info"] = 200;
    info_items_widths["Selection Info"] = 200;

    info_items_heights["Buffers"] = 120;
    info_items_widths["Buffers"] = 200;

    info_items_heights["Navigation Stack"] = 150;
    info_items_widths["Navigation Stack"] = 200;

    info_items_heights["Evaluator"] = 220;
    info_items_widths["Evaluator"] = 200;

    foreach(QString name, info_items.keys()) {
        QWidget *box = info_items.value(name);
        box->setFixedHeight(info_items_heights.value(name));
        box->setMinimumWidth(info_items_widths.value(name));
    }
}

void rv_hex_editor_infopane::show_hide_info_item(QListWidgetItem *li)
{
    QWidget *w;
    if(!(w = info_items.value(li->text()))) return;

    if(li->checkState() == Qt::Checked) {
        info_items.value(li->text())->show();
    } else {
        info_items.value(li->text())->hide();

    }

    redo_layout(true);
}



void rv_hex_editor_infopane::redo_layout()
{
    if(ui->gridLayout) { delete ui->gridLayout; ui->gridLayout = 0; }
    if(main_layout) delete main_layout;

    main_layout = new QHBoxLayout();

    QVBoxLayout *v_layout = new QVBoxLayout();

    // -- new try based on heights

    int cols=0;

    int my_height = height() - 15;

    int current_height = 0;

    QStringList names;

    QListWidgetItem *li;
    for(int i=0; i< listWidget_side_items->count(); i++) {
        li = listWidget_side_items->item(i);
        if(li->checkState() == Qt::Checked)
        names << li->text();
    }
    names << "Side Infos";

    QGroupBox *tmp_box;
    bool col_has_one_flexible = false;


    v_layout->setContentsMargins(6,0,0,0);
    foreach(QString name, names) {
        tmp_box = (QGroupBox *) info_items.value(name);

        if(tmp_box->isChecked())
            current_height += info_items_heights.value(name);
        else current_height+= 25;

        if(current_height > (my_height - 1)) {
            if(!col_has_one_flexible) v_layout->addStretch();
            main_layout->addLayout(v_layout);
            v_layout = new QVBoxLayout();
            v_layout->setContentsMargins(6,0,0,0);
            v_layout->addWidget(tmp_box);
            current_height = info_items_heights.value(name);
            col_has_one_flexible = false;
            cols++;
        } else {
            v_layout->addWidget(tmp_box);
        }

        if(tmp_box->isChecked()) {
            if(     (tmp_box == ui->groupBox_buffers) ||
                    (tmp_box == ui->groupBox_cursor) ||
                    (tmp_box == ui->groupBox_selection_info) ||
                    (tmp_box == ui->groupBox_side_items)
                    ) {
                tmp_box->setFixedHeight(info_items_heights.value(name));
            } else {
                if(qobject_cast<rv_sideinfo_custom_widget *>(tmp_box)
                        && ((rv_sideinfo_custom_widget*)tmp_box)->get_height()
                        ) {
                  tmp_box->setFixedHeight(((rv_sideinfo_custom_widget*)tmp_box)->get_height());
                } else {
                    tmp_box->setMinimumHeight(info_items_heights.value(name));
                    tmp_box->setMaximumHeight(1000);
                    col_has_one_flexible = true;
                }
            }
        }
        else
            tmp_box->setFixedHeight(20);
    }

    if(ui->groupBox_side_items->isChecked())
        ui->groupBox_side_items->setFixedHeight(
                    info_items.count() * 20 + 40);
    else ui->groupBox_side_items->setFixedHeight(20);

    if(!col_has_one_flexible) v_layout->addStretch();
    main_layout->addLayout(v_layout);
    main_layout->setContentsMargins(6,0,0,0);

    // --

    //scrollarea->setLayout(main_layout);
    setLayout(main_layout);

    // dirty force an update
    setMaximumWidth(10);
    setMaximumWidth(1000);
}

void rv_hex_editor_infopane::redo_layout(bool b)
{
    redo_layout();
}

QListWidget *rv_hex_editor_infopane::get_navi_stack_listbox()
{
    return ui->listWidget_navi_stack;
}


QPlainTextEdit *rv_hex_editor_infopane::get_evaluator_input_box()
{
    return ui->plainTextEdit_evaluator_input;
}

QPlainTextEdit *rv_hex_editor_infopane::get_evaluator_output_box()
{
    return ui->plainTextEdit_evaluator_output;
}

QPushButton *rv_hex_editor_infopane::get_evaluator_run_button()
{
    return ui->pushButton_evaluate_run;
}

QPushButton *rv_hex_editor_infopane::get_evaluator_abort_button()
{
    return ui->pushButton_evaluate_abort;
}

void rv_hex_editor_infopane::add_custom_widget(QString name, rv_sideinfo_custom_widget *widget)
{
    if(!widget) return;
    custom_widgets << widget;
    widget->show();
    info_items.insert(name, widget);

    info_items_heights.insert(name, 180);
    info_items_widths.insert(name, 200);

    connect(widget, SIGNAL(toggled(bool)),
            this, SLOT(redo_layout(bool)));
    connect(widget, SIGNAL(my_height_changed(rv_sideinfo_custom_widget*,int)),
            this, SLOT(widget_height_changed(rv_sideinfo_custom_widget*,int)));

    QListWidgetItem *li = new QListWidgetItem(name);
    li->setCheckState(Qt::Checked);
    listWidget_side_items->addItem(li);    
    redo_layout(true);
}

void rv_hex_editor_infopane::widget_height_changed(rv_sideinfo_custom_widget *w, int h)
{
    QString name = info_items.key(w);
    info_items_heights[name] = h;
    redo_layout();
}
