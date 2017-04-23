#include "rv_hex_editor.h"

rv_hex_editor::rv_hex_editor(QTabWidget *attach_to, QWidget *parent) :
    QWidget(parent)
{
    tabwidget_attach_to = attach_to;
    main_layout = new QVBoxLayout(this);

    splitter_buffers = new rv_splitter(Qt::Vertical, this);
    splitter_buffers->set_sub_splitter(true);
    splitter_hexview = new rv_splitter(Qt::Horizontal, this);

    hv = new rv_hex_edit(0, this);

    // buffer_tab widget setup
    tabwidget_buffers = new QTabWidget(this);
    tabwidget_buffers->setDocumentMode(true);
    tabwidget_buffers->setTabsClosable(true);
    tabwidget_buffers->setMovable(true);
    tabwidget_buffers->setIconSize(QSize(22, 16));

//    pushbutton_buffers_chg_orientation = new QPushButton(QIcon(":/icons/horz_vert2.png"),
//                                                 "", this);

    pushbutton_buffers_chg_orientation =
            new QPushButton(QIcon(":/icons/chg_orientation_small2.png"),
                                                 "", this);

    connect(pushbutton_buffers_chg_orientation, SIGNAL(clicked()),
            this, SLOT(change_orientation()));

    pushbutton_buffers_chg_orientation->setToolTip("Change orientation");
    pushbutton_buffers_chg_orientation->setFlat(true);

    tabwidget_buffers->setCornerWidget(pushbutton_buffers_chg_orientation,
                                        Qt::TopRightCorner);

    connect(tabwidget_buffers, SIGNAL(tabCloseRequested(int)),
            this, SLOT(close_buffer_tab(int)));

    infopane = new rv_hex_editor_infopane(this);

    bufferpane = new rv_hex_editor_bufferpane(this);

    connect(hv, SIGNAL(cursor_offset_changed_to(re_addr_t)),
            this, SLOT(display_cursor_info(re_addr_t)));

    connect(hv, SIGNAL(cursor_offset_changed_to(re_addr_t)),
            this, SLOT(do_offset_changed_forward(re_addr_t)));

    connect(hv, SIGNAL(selection_changed(re_addr_t,re_addr_t)),
            this, SLOT(display_selection_info(re_addr_t,re_addr_t)));
    connect(hv, SIGNAL(selection_changed(re_addr_t,re_addr_t)),
            this, SLOT(do_selection_changed_forward(re_addr_t,re_addr_t)));

    connect(hv, SIGNAL(selection_cleared()),
            this, SLOT(clear_selection_info()));

    // infopane connects

    connect(infopane, SIGNAL(buffers_toggled(bool)),
            this, SLOT(buffers_box_toggled(bool)));

    connect (infopane, SIGNAL(copy_selection_into_new_buffer()),
             this, SLOT(new_buffer_from_main_selection()));

    connect(infopane, SIGNAL(buffer_rename(QString)),
            this, SLOT(buffer_rename(QString)));

    connect(infopane, SIGNAL(new_target_buffer()),
           this, SLOT(new_target_buffer()));

    connect(infopane, SIGNAL(new_00_buffer_clicked()),
            this, SLOT(new_00_buffer()));

    connect(infopane, SIGNAL(load_file_into_new_buffer_clicked()),
            this, SLOT(new_buffer_from_file()));

    connect(infopane, SIGNAL(evaluator_run_clicked()),
            this, SLOT(evaluator_run()));

    connect(infopane, SIGNAL(evaluator_abort_clicked()), this,
            SLOT(evaluator_abort()));

    connect(infopane, SIGNAL(action_new_side_widget_cursor()),
            this, SLOT(new_custom_widget_cursor()));
    connect(infopane, SIGNAL(action_new_side_widget_selection()),
            this, SLOT(new_custom_widget_selection()));
    connect(infopane, SIGNAL(action_new_side_widget_button()),
            this, SLOT(new_custom_widget_button()));

    // bufferpane connects

    connect(bufferpane, SIGNAL(paste_buffer_selection_to_main()),
            this, SLOT(paste_buffer_sel_up()));

    connect(bufferpane, SIGNAL(paste_whole_buffer_to_main()),
            this, SLOT(paste_whole_buffer_up()));


    splitter_buffers->addWidget(hv);
    //splitter_buffers->addWidget(buffer_tabs);
    bufferpane->insert_buffer_tab_widget(tabwidget_buffers);
    splitter_buffers->addWidget(bufferpane);

    splitter_hexview->addWidget(splitter_buffers);
    splitter_hexview->addWidget(infopane);

    splitter_hexview->setSizes(QList<int>() << 670 << 160);

    splitter_buffers->setCollapsible(1, false);


    splitter_hexview->setHandleWidth(5);
    splitter_buffers->setHandleWidth(3);


    main_layout->addWidget(splitter_hexview);

    setLayout(main_layout);
    setWindowIcon(QIcon(":/icons/hex_buffer_t_16.png"));

    setWindowTitle("Hex View - Target");

    buffers_box_toggled(false);

    listbox_stack = infopane->get_navi_stack_listbox();
    setup_toolbar();

    set_detached(false);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(255,255,255));
    setPalette(pal);

    script_processor = new rv_script_processor(this);
    script_processor->engine->setProcessEventsInterval(100);
    script_processor->set_output_textbox(infopane->get_evaluator_output_box());
    script_processor->set_hex_editor(this);

    //connect(script_processor, SIGNAL(processor_thread_terminated(QScriptValue)),
    //        this, SLOT(evaluator_thread_finished(QScriptValue)));

}


void rv_hex_editor::set_target(re_target *target)
{
    t = target;
    hv->set_target(t);
    hv->set_buffer_name("Target");
}


void rv_hex_editor::display_cursor_info(re_addr_t offs)
{
    QString str;
    unsigned int  val;
    unsigned char b;
    re_addr_t va, file_offset;
    char buf[128];

    file_offset = offs;

    // offs
    infopane->set_cursor_offset_hex(re_core::dword_to_hex_string(offs));

    str = QString::number(offs);
    infopane->set_cursor_offset_dec(str);

    // VA
    if(t->file->b_file_offset_to_va(va, file_offset)) {        
        infopane->set_cursor_offset_va(re_core::dword_to_hex_string(va));
    } else {
        infopane->set_cursor_offset_va("n/a");
    }

    // val
    if(t->file->b_get_byte_at_file_offset(b, file_offset)) {
        ::snprintf(buf, 128, "h: 0x%02X\ns: %d\nu: %u", b, (char)b, b);
        infopane->set_cursor_byte(QString::fromAscii(buf));
    } else {
        infopane->set_cursor_byte("n/a");
    }

    if(t->file->b_get_word_at_file_offset(val, file_offset)) {
        ::snprintf(buf, 128, "h: 0x%04X\ns: %d\nu: %u", val, (int)val, val);
        infopane->set_cursor_word(QString::fromAscii(buf));
    } else {
        infopane->set_cursor_word("n/a\nn/a\nn/a\n");
    }

    if(t->file->b_get_dword_at_file_offset(val, file_offset)) {
        ::snprintf(buf, 128, "h: 0x%08X\ns: %d\nu: %u", val, (int)val, val);
        infopane->set_cursor_dword(QString::fromAscii(buf));
    } else {
        infopane->set_cursor_dword("n/a\nn/a\nn/a\n");
    }
}

void rv_hex_editor::display_selection_info(re_addr_t from, re_addr_t to)
{
    QString hex1, dec1, va1;
    QString hex2, dec2, va2;
    QString sz;
    char buf[128];
    re_addr_t va;

    ::snprintf(buf, 128, "0x%08X", from);
    hex1 = QString::fromAscii(buf);
    dec1 = QString::number(from);

    if(t->file->b_file_offset_to_va(va, from)) {
        ::snprintf(buf, 128, "0x%08X (va)", va);
        va1 = QString::fromAscii(buf);
    } else {
        va1 = "n/a";
    }

    ::snprintf(buf, 128, "0x%08X", to);
    hex2 = QString::fromAscii(buf);
    dec2 = QString::number(to);
    if(t->file->b_file_offset_to_va(va, to)) {
        ::snprintf(buf, 128, "0x%08X (va)", va);
        va2 = QString::fromAscii(buf);
    } else {
        va2 = "n/a";
    }

    sz ="0x"  + QString::number(to - from + 1, 16) + ", " +
            QString::number(to-from + 1);

    infopane->set_selection_info(hex1, dec1, va1, hex2, dec2, va2, sz);


}

void rv_hex_editor::clear_selection_info()
{
    infopane->set_selection_info("n/a","n/a","n/a","n/a","n/a","n/a","n/a");
}

void rv_hex_editor::buffers_box_toggled(bool checked)
{
    bufferpane->setShown(checked);
    if(tabwidget_buffers->count()) {
        bufferpane->enable_buffers(true);
    }
}

bool rv_hex_editor::new_buffer_from_main_selection()
{
    unsigned char *b;
    struct rv_hv_selection sel;

    if(!hv->get_selection(sel)) {
        QMessageBox mb(this);
        mb.setText("Nothing selected!");
        mb.setIcon(QMessageBox::Information);
        mb.exec();
        return false;
    }

    rv_hex_edit *hv_buffer = new rv_hex_edit(0, this);

    if(!(b = (unsigned char *)malloc(sel.len))) return false;
    ::memcpy(b, hv->get_data_ptr() + sel.start, sel.len);
    hv_buffer->set_data_buffer(b, sel.len);

    hv_buffer->set_buffer_name((QString)"Buffer "
                               + QString::number(tabwidget_buffers->count()));

    add_buffer_to_tabs(hv_buffer);

    buffers << hv_buffer;

    return true;
}

void rv_hex_editor::new_target_buffer()
{
    rv_hex_edit *hv_buffer = new rv_hex_edit(0, this);
    hv_buffer->set_target(t);

    hv_buffer->set_buffer_name((QString)"Target View "
                               + QString::number(tabwidget_buffers->count()));

    add_buffer_to_tabs(hv_buffer);
}

void rv_hex_editor::add_buffer_to_tabs(rv_hex_edit *hv_buffer)
{
    int tabnum;
    QIcon *icon;

    if(hv_buffer->is_target_buffer)    icon = new QIcon(":/icons/hex_buffer_t.png");
    else if(hv_buffer->is_file_buffer) icon = new QIcon(":/icons/hex_buffer_f.png");
    else                        icon = new QIcon(":/icons/hex_buffer_b.png");

    tabnum = tabwidget_buffers->addTab(hv_buffer, *icon, hv_buffer->get_name());

    tabwidget_buffers->setCurrentIndex(tabnum);
    bufferpane->enable_buffers(true);

    connect(hv_buffer, SIGNAL(cursor_offset_changed_to(re_addr_t)),
            this, SLOT(do_offset_changed_forward(re_addr_t)));

    connect(hv_buffer, SIGNAL(selection_changed(re_addr_t,re_addr_t)),
            this, SLOT(do_selection_changed_forward(re_addr_t,re_addr_t)));
}

void rv_hex_editor::buffer_rename(QString name)
{
    rv_hex_edit *hv_buffer;
    if(!tabwidget_buffers->count()) return;

    tabwidget_buffers->setTabText(tabwidget_buffers->currentIndex(), name);
    hv_buffer = (rv_hex_edit *) tabwidget_buffers->widget(tabwidget_buffers->currentIndex());
    if(!hv_buffer) return;

    hv_buffer->set_buffer_name(name);
}

void rv_hex_editor::new_buffer_from_file()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Binary"), "", tr("All files (*.*);;"));


    if(fileName.isNull()) return;
    QMessageBox mb(this);

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly )) {
        mb.setText("Error opening file!");
        mb.setIcon(QMessageBox::Critical);
        mb.exec();
        return;
    }

    QByteArray bytes;
    bytes = file.readAll();

    if(bytes.size() != file.size()) {
        mb.setText("Error reading file!");
        mb.setIcon(QMessageBox::Critical);
        mb.exec();
        return;
    }


    // --

    unsigned char *b;

    if(!(b = (unsigned char *)malloc(bytes.size()))) {
        mb.setText("Error allocating RAM for file!");
        mb.setIcon(QMessageBox::Critical);
        mb.exec();
        return;
    }
    ::memcpy(b, bytes.data(), bytes.size());

    rv_hex_edit *hv_buffer = new rv_hex_edit(0, this);
    hv_buffer->set_data_buffer(b, bytes.size());
    hv_buffer->set_isfile_flag();
    hv_buffer->set_buffer_name(file.fileName());
    add_buffer_to_tabs(hv_buffer);
    file.close();

    buffers << hv_buffer;
}

void rv_hex_editor::close_buffer_tab(int idx)
{
    if(idx >= tabwidget_buffers->count()) return;

    rv_hex_edit *hv_buffer;

    hv_buffer = (rv_hex_edit *) tabwidget_buffers->widget(idx);
    if(!hv_buffer) return;
    if(!hv->is_target_buffer) {
        free(hv_buffer->get_data_ptr()); // TODO !!!!! WHAT IF HAS CLIP SET?
                                         // GET_DATA_PTR returns CURRENT_DATA_PTR!!
    }

    tabwidget_buffers->removeTab(idx);
    buffers.removeOne(hv_buffer);

    if(!tabwidget_buffers->count()) bufferpane->enable_buffers(false);
}

void rv_hex_editor::paste_buffer_sel_up()
{
    rv_hex_edit *hv_buffer;
    struct rv_hv_selection sel;

    hv_buffer = (rv_hex_edit *) tabwidget_buffers->widget(tabwidget_buffers->currentIndex());
    if(!hv_buffer) return;

    if(!hv_buffer->get_selection(sel)) return;

    hv->paste_foreign_selection(sel);
}

void rv_hex_editor::paste_whole_buffer_up()
{
    rv_hex_edit *hv_buffer;
    struct rv_hv_selection sel;

    hv_buffer = (rv_hex_edit *) tabwidget_buffers->widget(tabwidget_buffers->currentIndex());
    if(!hv_buffer) return;

    sel.start = 0;
    sel.end = hv_buffer->get_data_len() - 1;
    sel.len = hv_buffer->get_data_len();
    sel.hv = hv_buffer;

    hv->paste_foreign_selection(sel);
}

void rv_hex_editor::new_00_buffer()
{
    unsigned char *b;
    unsigned int len = 0;
    QMessageBox mb(this);

    bool ok;
    QString text;

    while (text.isEmpty() || (len == 0)) {
        text = QInputDialog::getText(this, tr("Create a new buffer filled with zeroes"),
                                                     tr("Buffer length in bytes (C Syntax: (you can use 0x for hex)):"), QLineEdit::Normal,
                                                     "1024", &ok);
        if(!ok) return;
        len = text.toUInt(&ok, 0);

        if(!ok || !len) {
            mb.setText("Please enter a valid number!");
            mb.setIcon(QMessageBox::Critical);
            mb.exec();
            len = 0;
        }
    }


    rv_hex_edit *hv_buffer = new rv_hex_edit(0, this);

    if(!(b = (unsigned char *)calloc(len, 1))) {
        mb.setText("Error allocating " + QString::number(len)
                   + "bytes of RAM");
        mb.setIcon(QMessageBox::Critical);
        mb.exec();
        return;
    }

    hv_buffer->set_data_buffer(b, len);

    hv_buffer->set_buffer_name((QString)"00 Buffer "
                               + QString::number(tabwidget_buffers->count()));

    add_buffer_to_tabs(hv_buffer);
    buffers << hv_buffer;
}

void rv_hex_editor::re_attach_clicked()
{
    emit re_attach_me(this);
}

void rv_hex_editor::set_detached(bool b)
{
    detached = b;
    pushbutton_re_attach->setShown(b);
}

void rv_hex_editor::setup_toolbar()
{
    toolbar_hex = new QToolBar("Hex-Toolz", this);
    toolbar_hex->setFloatable(false);

    QHBoxLayout *hl = new QHBoxLayout();

    QLabel *l = new QLabel("", this);
    l->setPixmap(QPixmap(":/icons/hex_tools.png"));

    hl->addWidget(l);

    toolbar_hex->addSeparator();
    toolbar_hex->addAction(hv->action_copy_selection);
    toolbar_hex->addAction(hv->action_paste_selection);
    toolbar_hex->addSeparator();

    toolbar_hex->addAction(hv->action_fill_selection);
    toolbar_hex->addSeparator();

    toolbar_hex->addAction(hv->action_undo);
    toolbar_hex->addAction(hv->action_redo);

    tool_groupbox = new QGroupBox(this);

    pushbutton_re_attach = new QPushButton(QIcon(":/icons/re_attach.png"), "", tool_groupbox);
    connect(pushbutton_re_attach, SIGNAL(clicked()),
            this, SLOT(re_attach_clicked()));
    pushbutton_re_attach->setFlat(true);
    pushbutton_re_attach->setToolTip("Attach to main window");

    hl->addWidget(toolbar_hex);

    //QHBoxLayout *hl_stack = new QHBoxLayout();
    pushbutton_stack_f = new QPushButton(QIcon(":/icons/go_forward.png"), "",this);
    pushbutton_stack_b = new QPushButton(QIcon(":/icons/go_backward.png"), "", this);
    pushbutton_stack_f->setFlat(true);
    pushbutton_stack_b->setFlat(true);
    pushbutton_stack_f->setContentsMargins(0,0,0,0);
    pushbutton_stack_b->setContentsMargins(0,0,0,0);
    pushbutton_stack_f->setToolTip("go forward in stack");
    pushbutton_stack_b->setToolTip("go backwards in stack");

    connect(hv, SIGNAL(cursor_offset_changed_to(re_addr_t)),
            this, SLOT(stack_push(re_addr_t)));

    connect(pushbutton_stack_b, SIGNAL(clicked()),
            this, SLOT(stack_go_back()));

    connect(pushbutton_stack_f, SIGNAL(clicked()),
            this, SLOT(stack_go_forward()));

    connect(infopane, SIGNAL(navi_stack_goto(int)),
            this, SLOT(stack_goto_list_idx(int)));
    connect(infopane, SIGNAL(clear_navi_stack_clicked()),
            this, SLOT(stack_clear()));

    navi_index = 0;
    stack_push(0);

    hl->addWidget(pushbutton_stack_b);
    hl->addWidget(pushbutton_stack_f);

    hl->addStretch();

    hl->addWidget(pushbutton_re_attach);
    tool_groupbox->setLayout(hl);
    tool_groupbox->setFixedHeight(30);
    hl->setMargin(0);
    hl->setContentsMargins(4,0,4,0);

    main_layout->insertWidget(0, tool_groupbox);
}


void rv_hex_editor::stack_go_back()
{
    if(!navi_stack.count()) return;
    if(navi_index >= (navi_stack.count()-1)) return;
    navi_index++;
    stack_repaint();
    hv->set_shadow_cursor(navi_stack.at((navi_stack.count()-1) - navi_index));
}

void rv_hex_editor::stack_go_forward()
{
    if(!navi_stack.count()) return;
    if(!navi_index) return;
    navi_index--;
    stack_repaint();
    hv->set_shadow_cursor(navi_stack.at((navi_stack.count()-1) - navi_index));
}

re_addr_t rv_hex_editor::stack_pop()
{
    return 0;
}

void rv_hex_editor::stack_push(re_addr_t offs)
{
    if(navi_index)
        for(int i=0; i<navi_index; i++) {
            navi_stack.pop();
            listbox_stack->takeItem(0);
        }
    navi_stack.push(offs);
    listbox_stack->insertItem(0, re_core::dword_to_hex_string(offs));
    navi_index=0;
    stack_repaint();
}

void rv_hex_editor::stack_repaint()
{
    for(int i=0; i<listbox_stack->count();i++) {
        listbox_stack->item(i)->setIcon(QIcon(":/icons/empty16_16.png"));
        listbox_stack->item(i)->setFont(QFont("Liberation Mono", 9, QFont::Normal));
    }
    listbox_stack->item(navi_index)->setIcon(QIcon(":/icons/stack_forward.png"));
    listbox_stack->item(navi_index)->setFont(QFont("Liberation Mono", 9,  QFont::Bold));
}

void rv_hex_editor::stack_clear()
{
    navi_stack.clear();
    listbox_stack->clear();
    navi_index = 0;
    stack_push(0);
    stack_repaint();
}

void rv_hex_editor::stack_goto_list_idx(int idx)
{
   if(navi_stack.count() && ((navi_stack.count()-1-idx) >=0))
   hv->set_shadow_cursor(navi_stack.at((navi_stack.count()-1) - idx));
}

rv_hex_edit *rv_hex_editor::get_buffer_by_name(QString name)
{
    rv_hex_edit *hv_buffer = 0;
    if(!tabwidget_buffers->count()) return 0;

    for(int i=0; i<tabwidget_buffers->count(); i++) {
        if(((rv_hex_edit *)tabwidget_buffers->widget(i))->buffer_name == name)
        hv_buffer = (rv_hex_edit *)tabwidget_buffers->widget(i);
    }

    return hv_buffer;
}

void rv_hex_editor::evaluator_run_in_thread()
{
    if(script_processor->engine->isEvaluating()) {
        QMessageBox msg;
        msg.setText("Previous script still running!");
        msg.exec();
        return;
    }

    QString script_text = (infopane->get_evaluator_input_box())->toPlainText();

    QScriptSyntaxCheckResult res1 =
            script_processor->engine->checkSyntax(script_text);

    if(res1.state() != QScriptSyntaxCheckResult::Valid) {
        (infopane->get_evaluator_output_box())->appendPlainText(
        (QString) "rv|script syntax error at line "
                    + QString::number(res1.errorLineNumber())
                    + ", column "
                    + QString::number(res1.errorColumnNumber())
                    + ": " + res1.errorMessage());
        return;
    }

    infopane->get_evaluator_abort_button()->setEnabled(true);
    infopane->get_evaluator_run_button()->setEnabled(false);
    script_processor->run_script_in_thread(script_text);
}

void rv_hex_editor::evaluator_thread_finished(QScriptValue result)
{
    infopane->get_evaluator_abort_button()->setEnabled(false);
    infopane->get_evaluator_run_button()->setEnabled(true);

    (infopane->get_evaluator_output_box())->setPlainText(
                result.toString());

    if (result.isError()) {
             (infopane->get_evaluator_output_box())->appendPlainText(
                    (QString)"rv|script error"
                                  +
                                   QString::fromLatin1("%0: %1")
                                   .arg(result.property("lineNumber").toInt32())
                                   .arg(result.toString()));
             return;
         }
}

void rv_hex_editor::evaluator_abort()
{
    script_processor->engine->abortEvaluation();
    (infopane->get_evaluator_output_box())->setPlainText("ABORTED!");
}

void rv_hex_editor::evaluator_run()
{
    if(script_processor->engine->isEvaluating()) {
        QMessageBox msg;
        msg.setText("Previous script still running!");
        msg.exec();
        return;
    }

    QString script_text = (infopane->get_evaluator_input_box())->toPlainText();

    QScriptSyntaxCheckResult res1 =
            script_processor->engine->checkSyntax(script_text);

    if(res1.state() != QScriptSyntaxCheckResult::Valid) {
        (infopane->get_evaluator_output_box())->appendPlainText(
        (QString) "rv|script syntax error at line "
                    + QString::number(res1.errorLineNumber())
                    + ", column "
                    + QString::number(res1.errorColumnNumber())
                    + ": " + res1.errorMessage());
        return;
    }

    infopane->get_evaluator_abort_button()->setEnabled(true);
    infopane->get_evaluator_run_button()->setEnabled(false);
    QScriptValue result = script_processor->engine->evaluate(script_text);
    infopane->get_evaluator_abort_button()->setEnabled(false);
    infopane->get_evaluator_run_button()->setEnabled(true);

    (infopane->get_evaluator_output_box())->appendPlainText(result.toString());

    if (result.isError()) {
             (infopane->get_evaluator_output_box())->appendPlainText(
                    (QString)"rv|script error"
                                  +
                                   QString::fromLatin1("%0: %1")
                                   .arg(result.property("lineNumber").toInt32())
                                   .arg(result.toString()));
             return;
         }
}

void rv_hex_editor::new_custom_widget_cursor()
{
    new_cusom_widget(RV_CW_CURSOR_CONNECTED);
}

void rv_hex_editor::new_custom_widget_selection()
{
    new_cusom_widget(RV_CW_SELECTION_CONNECTED);
}

void rv_hex_editor::new_custom_widget_button()
{
    new_cusom_widget(RV_CW_BUTTON_CONNECTED);
}

void rv_hex_editor::new_cusom_widget(int mode)
{
    bool ok;
    QString name = QInputDialog::getText(this, "New custom sideinfo widget",
                    "Enter the widget's name: ", QLineEdit::Normal,
                    "Widget" + QString::number(infopane->custom_widgets.count()),
                    &ok);
    if (!ok || name.isEmpty()) return;

    foreach (rv_sideinfo_custom_widget *tmp, infopane->custom_widgets) {
        if(tmp->title() == name) {
            name += "_1";
        }
    }

    rv_sideinfo_custom_widget* widget = new rv_sideinfo_custom_widget(infopane);
    widget->set_mode(mode);
    widget->setTitle(name);
    widget->setCheckable(true);
    widget->setChecked(true);
    infopane->add_custom_widget(name, widget);

    if(mode & RV_CW_CURSOR_CONNECTED)
        connect(this, SIGNAL(offset_changed_forward(rv_hex_edit*,re_addr_t)),
            widget, SLOT(cursor_offset_changed_to(rv_hex_edit*,re_addr_t)));

    if(mode & RV_CW_SELECTION_CONNECTED)
        connect(this, SIGNAL(selection_changed_forward(rv_hex_edit*,re_addr_t,re_addr_t)),
                widget, SLOT(selection_changed(rv_hex_edit*,re_addr_t,re_addr_t)))    ;
}

void rv_hex_editor::do_offset_changed_forward(re_addr_t offset)
{
    script_processor->set_current_buffer((rv_hex_edit *)sender());
    emit offset_changed_forward((rv_hex_edit *)sender(), offset);
}

void rv_hex_editor::do_selection_changed_forward(re_addr_t from, re_addr_t to)
{
    script_processor->set_current_buffer((rv_hex_edit *)sender());
    emit selection_changed_forward((rv_hex_edit *)sender(), from, to);
}

void rv_hex_editor::change_orientation()
{
    if(splitter_buffers->orientation() == Qt::Vertical)
        splitter_buffers->setOrientation(Qt::Horizontal);
    else
        splitter_buffers->setOrientation(Qt::Vertical);
}

void rv_hex_editor::set_cursor_to_va(re_addr_t va)
{
    hv->set_cursor_to_va(va);
}
