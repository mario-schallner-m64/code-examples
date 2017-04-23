#include "rv_idisasm.h"

rv_idisasm::rv_idisasm(QTabWidget *attach_to, QWidget *parent) :
    QWidget(parent)
{
    tabwidget_attach_to = attach_to;
    main_layout = new QVBoxLayout(this);
    selection.active = false;

    dv = new rv_disasm_view(0, this);
    dv->set_name("main disassembly");
    dv->set_have_idisasm(true);
    dv->set_idisasm_selection(&selection);
    connect(dv, SIGNAL(jmped_from(re_addr_t)),
            this, SLOT(stack_push(re_addr_t)));
    connect(dv, SIGNAL(va_clicked(re_addr_t)),
            this, SLOT(update_last_pos(re_addr_t)));
    connect(dv, SIGNAL(disassembly_finished()),
            this, SLOT(disassembly_changed()));

    connect(dv, SIGNAL(received_focus()),
            this, SLOT(view_changed_disasm_addr_space()));

    connect(dv, SIGNAL(do_ctf_disasm_filter_view(re_addr_t)),
            this, SLOT(disasm_ctf_into_filter_view(re_addr_t)));

    connect(dv, SIGNAL(do_linear_disasm_filter_view(re_addr_t)),
            this, SLOT(disasm_linear_into_filter_view(re_addr_t)));

    connect(dv, SIGNAL(paste_ctf_to_new_filter_requested()),
            this, SLOT(paste_ctf_into_new_filter()));
    connect(dv, SIGNAL(paste_to_new_filter_requested()),
            this, SLOT(paste_into_new_filter()));

    d_va = 0;
    active_dv = dv;

    console = new rv_console_edit(this);
    setup_console(console);
    connect(console, SIGNAL(execute_cmd(QString)),
            this, SLOT(process_console_cmd(QString)));
    connect(console, SIGNAL(biggify(bool)),
            this, SLOT(biggify_console(bool)));

    script_processor = new rv_script_processor(this);
    script_processor->engine->setProcessEventsInterval(100);

    setup_infopane();
    setup_toolbar();

    splitter_main = new rv_splitter(Qt::Horizontal, this);

    splitter_disasm = new rv_splitter(Qt::Vertical, this);
    splitter_disasm->set_sub_splitter(true);
    splitter_disasm->addWidget(dv);

    tabwidget_sub_views = new QTabWidget(this);
    tabwidget_sub_views->setDocumentMode(true);
    tabwidget_sub_views->setTabsClosable(true);
    tabwidget_sub_views->setMovable(true);
    tabwidget_sub_views->setIconSize(QSize(32, 16));

    splitter_disasm->addWidget(tabwidget_sub_views);
    tabwidget_sub_views->hide();

//    pushbutton_subviews_chg_orientation = new QPushButton(QIcon(":/icons/horz_vert2.png"),
//                                                 "", this);

    pushbutton_subviews_chg_orientation =
            new QPushButton(QIcon(":/icons/chg_orientation_small2.png"),
                                                 "", this);

    connect(pushbutton_subviews_chg_orientation, SIGNAL(clicked()),
            this, SLOT(change_orientation()));

    pushbutton_subviews_chg_orientation->setToolTip("Change orientation");
    pushbutton_subviews_chg_orientation->setFlat(true);

    tabwidget_sub_views->setCornerWidget(pushbutton_subviews_chg_orientation,
                                        Qt::TopRightCorner);

    connect(tabwidget_sub_views, SIGNAL(tabCloseRequested(int)),
            this, SLOT(close_sub_view(int)));

    splitter_disasm_console = new rv_splitter(Qt::Vertical, this);
    splitter_disasm_console->addWidget(splitter_disasm);
    splitter_disasm_console->addWidget(console);
    splitter_disasm_console->setSizes(QList<int>() << 670 << 120);

    splitter_main->addWidget(splitter_disasm_console);
    splitter_main->addWidget(infopane);

    splitter_main->setSizes(QList<int>() << height() << 150);

    main_layout->addWidget(splitter_main);
    setLayout(main_layout);
    setWindowIcon(QIcon(":/icons/disasm_view2.png"));

    setWindowTitle("Disassembly View - Target");
    last_pos_va = BAD_ADDR;
    set_detached(false);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(255,255,255));
    setPalette(pal);

}

void rv_idisasm::set_target(re_target *target)
{
    t = target;
    dv->set_target(t);
}

rv_disasm_view *rv_idisasm::new_filter_view(QString name)
{
    struct rv_idisasm_subview subview;
    rv_disasm_view *dview;
    re_address_space *as;

    as = t->clone_address_space();

    dview = new rv_disasm_view(tabwidget_sub_views);
    dview->set_target(t);
    dview->set_address_space(as);
    dview->set_name(name);
    dview->set_have_idisasm(true);
    dview->set_idisasm_selection(&selection);

    connect(dview, SIGNAL(received_focus()),
            this, SLOT(view_changed_disasm_addr_space()));
    connect(dview, SIGNAL(disassembly_finished()),
            this, SLOT(disassembly_changed()));

    connect(dview, SIGNAL(cursor_changed_to(re_addr_t)),
            this, SLOT(view_changed_disasm_addr_space()));

    connect(dview, SIGNAL(cursor_changed_to(re_addr_t)),
            this, SLOT(show_cursor_info(re_addr_t)));

    if(tag_processor) {
        dview->set_tag_processor(tag_processor);

        connect(tag_processor, SIGNAL(tags_changed()),
                dview, SLOT(do_repaint()));
    }

    connect(dview, SIGNAL(paste_ctf_to_new_filter_requested()),
            this, SLOT(paste_ctf_into_new_filter()));
    connect(dview, SIGNAL(paste_to_new_filter_requested()),
            this, SLOT(paste_into_new_filter()));

    dview->set_show_offsets(false);
    dview->set_show_bytes_cnt(0);

    subview.address_space = as;
    subview.dv = dview;

    subviews << subview;

    if(tabwidget_sub_views->isHidden()) tabwidget_sub_views->show();

    int idx = tabwidget_sub_views->addTab(dview,
                                          QIcon(":/icons/disasm_view_sub_filter_t3.png"),
                                          name);

    tabwidget_sub_views->setCurrentIndex(idx);

    set_active_dv(dview);

    connect(dview, SIGNAL(do_ctf_disasm_filter_view(re_addr_t)),
            this, SLOT(disasm_ctf_into_filter_view(re_addr_t)));

    connect(dview, SIGNAL(do_linear_disasm_filter_view(re_addr_t)),
            this, SLOT(disasm_linear_into_filter_view(re_addr_t)));
}

rv_disasm_view *rv_idisasm::new_target_view(QString name)
{
    struct rv_idisasm_subview subview;
    rv_disasm_view *dview;

    dview = new rv_disasm_view(tabwidget_sub_views);
    dview->set_target(t);
    dview->set_name(name);
    dview->set_have_idisasm(true);
    dview->set_idisasm_selection(&selection);

    connect(dview, SIGNAL(received_focus()),
            this, SLOT(view_changed_disasm_addr_space()));
    connect(dview, SIGNAL(disassembly_finished()),
            this, SLOT(disassembly_changed()));

    connect(dview, SIGNAL(cursor_changed_to(re_addr_t)),
            this, SLOT(view_changed_disasm_addr_space()));

    connect(dview, SIGNAL(cursor_changed_to(re_addr_t)),
            this, SLOT(show_cursor_info(re_addr_t)));

    connect(dview, SIGNAL(paste_ctf_to_new_filter_requested()),
            this, SLOT(paste_ctf_into_new_filter()));
    connect(dview, SIGNAL(paste_to_new_filter_requested()),
            this, SLOT(paste_into_new_filter()));

    if(tag_processor) {
        dview->set_tag_processor(tag_processor);

        connect(tag_processor, SIGNAL(tags_changed()),
            dview, SLOT(do_repaint()));
    }

    dview->set_show_offsets(false);
    dview->set_show_bytes_cnt(0);

    subview.address_space = t->address_space;
    subview.dv = dview;

    subviews << subview;

    if(tabwidget_sub_views->isHidden()) tabwidget_sub_views->show();

    int idx = tabwidget_sub_views->addTab(dview,
                                          QIcon(":/icons/disasm_view_t_32_16.png"),
                                          name);

    tabwidget_sub_views->setCurrentIndex(idx);

    set_active_dv(dview);

    connect(dview, SIGNAL(do_ctf_disasm_filter_view(re_addr_t)),
            this, SLOT(disasm_ctf_into_filter_view(re_addr_t)));
    connect(dview, SIGNAL(do_linear_disasm_filter_view(re_addr_t)),
            this, SLOT(disasm_linear_into_filter_view(re_addr_t)));
}

void rv_idisasm::re_attach_clicked()
{
    emit re_attach_me(this);
}

void rv_idisasm::set_detached(bool b)
{
    detached = b;
    pushButton_re_attach->setShown(b);
}

void rv_idisasm::goto_va(re_addr_t va)
{
    if(!active_dv) return;
    active_dv->scroll_n_cursor_to(va);
    if(active_dv == dv) last_pos_va = va; // TODO
}

void rv_idisasm::setup_toolbar()
{
    QHBoxLayout *hl = new QHBoxLayout();

    QLabel *l = new QLabel("", this);
    l->setPixmap(QPixmap(":/icons/hex_tools.png"));

    hl->addWidget(l);

    tool_groupbox = new QGroupBox(this);

    pushButton_re_attach = new QPushButton(QIcon(":/icons/re_attach.png"), "", this);
    connect(pushButton_re_attach, SIGNAL(clicked()),
            this, SLOT(re_attach_clicked()));
    pushButton_re_attach->setFlat(true);
    pushButton_re_attach->setToolTip("Attach to main window");


    //QHBoxLayout *hl_stack = new QHBoxLayout();
    pushbutton_stack_f = new QPushButton(QIcon(":/icons/go_forward.png"), "",this);
    pushbutton_stack_b = new QPushButton(QIcon(":/icons/go_backward.png"), "", this);

    pushbutton_stack_f->setFlat(true);
    pushbutton_stack_b->setFlat(true);

    pushbutton_stack_f->setToolTip("go forward in stack");
    pushbutton_stack_b->setToolTip("go backwards in stack");

    connect(pushbutton_stack_b, SIGNAL(clicked()),
            this, SLOT(stack_go_back()));

    connect(pushbutton_stack_f, SIGNAL(clicked()),
            this, SLOT(stack_go_forward()));



//    connect(infopane, SIGNAL(clear_navi_stack_clicked()),
//            this, SLOT(stack_clear()));

    navi_index = 0;

    hl->addWidget(pushbutton_stack_b);
    hl->addWidget(pushbutton_stack_f);

    hl->addStretch();

    hl->addWidget(pushButton_re_attach);
    tool_groupbox->setLayout(hl);
    tool_groupbox->setFixedHeight(30);
    hl->setMargin(0);
    hl->setContentsMargins(4,0,4,0);

    main_layout->insertWidget(0, tool_groupbox);
}

void rv_idisasm::stack_go_back()
{
    if(!navi_stack.count()) return;
    if(navi_index >= (navi_stack.count()-1)) return;
    navi_index++;
    stack_repaint();
    dv->scroll_n_cursor_to(navi_stack.at((navi_stack.count()-1) - navi_index));
}

void rv_idisasm::stack_go_forward()
{
    if(!navi_stack.count()) return;
    if(navi_index >=1) {
        navi_index--;
        dv->scroll_n_cursor_to(navi_stack.at((navi_stack.count()-1) - navi_index));
        stack_repaint();
    } else if(navi_index == 0) {
        navi_index--;
        if(last_pos_va != BAD_ADDR)
            dv->scroll_n_cursor_to(last_pos_va);
        stack_repaint();
    }
}

re_addr_t rv_idisasm::stack_pop()
{
    return 0;
}

void rv_idisasm::stack_push(re_addr_t offs)
{
    if(navi_index > 0)
        for(int i=0; i<navi_index; i++) {
            navi_stack.pop();
        }
    if(!navi_stack.isEmpty() && (navi_stack.top()==offs)) return;
    navi_stack.push(offs);
    navi_index=-1;
    stack_repaint();
}

void rv_idisasm::stack_repaint()
{
    if(navi_stack.isEmpty()) return;

    listbox_stack->clearSelection();
    listbox_stack->clear();

    QListWidgetItem *item;
    for(int i=0; i<navi_stack.count(); i++) {
        item = new QListWidgetItem(QIcon(":/disassembly/arrow_left_green_small.png"),
                                   re_core::dword_to_hex_string(
                                       navi_stack.at(navi_stack.count()-1-i)));
        item->setBackgroundColor(QColor(0xff, 0xff, 0xff));
        listbox_stack->insertItem(i, item);
        listbox_stack->item(i)->setFont(QFont("Liberation Mono", 9, QFont::Normal));
    }
    if(navi_index>=0) {
        //listbox_stack->item(navi_index)->setIcon(QIcon(":/icons/stack_forward.png"));
        listbox_stack->item(navi_index)->setFont(QFont("Liberation Mono", 9,  QFont::Bold));
        listbox_stack->item(navi_index)->setBackgroundColor(QColor(0xe0, 0xe0, 0xe8));
    }
}

void rv_idisasm::stack_clear()
{
    navi_stack.clear();
    listbox_stack->clear();
    navi_index = -1;
    stack_repaint();
}

void rv_idisasm::stack_goto_list_idx(int idx)
{
   if(idx < 0) return;
   if(navi_stack.count() && ((navi_stack.count()-1-idx) >=0))
   dv->scroll_n_cursor_to(navi_stack.at((navi_stack.count()-1) - idx));
}

void rv_idisasm::setup_infopane()
{
    infopane = new QWidget(this);
    infopane->setStyleSheet("color: rgb(64, 64, 64);");

    QFont my_font = QFont();
    my_font.setUnderline(true);
    my_font.setBold(true);
    QVBoxLayout *vl_main = new QVBoxLayout();
    QVBoxLayout *vl;
    QHBoxLayout *hl;
    QGroupBox *g;

    // -- cursor info --
    vl = new QVBoxLayout();
    hl = new QHBoxLayout();
    g = new QGroupBox("Cursor Info");
    g->setFont(my_font);
    label_cursor_info = new QLabel();
    hl->addWidget(label_cursor_info);

    vl->addLayout(hl);
    g->setLayout(vl);
    vl_main->addWidget(g);
    infopane_widgets << g;
    // ---

    // -- navi stack --
    vl = new QVBoxLayout();
    hl = new QHBoxLayout();

    g = new QGroupBox("Navigation Stack");
    g->setFont(my_font);

    pushbutton_stack_clear = new QPushButton("Clear");
    connect(pushbutton_stack_clear, SIGNAL(clicked()),
            this, SLOT(stack_clear()));

    hl->addStretch();
    hl->addWidget(pushbutton_stack_clear);

    listbox_stack = new QListWidget();
    listbox_stack->setAlternatingRowColors(true);

    vl->addWidget(listbox_stack);
    vl->addLayout(hl);
    g->setLayout(vl);
    vl_main->addWidget(g);
    infopane_widgets << g;
    connect(listbox_stack, SIGNAL(currentRowChanged(int)),
            this, SLOT(stack_goto_list_idx(int)));
    // --

    // -- tag processor --
    tag_processor = new rv_tag_processor(this);
    dv->set_tag_processor(tag_processor);

    QTableWidget *TW;

    TW = new QTableWidget();
    TW->verticalHeader()->hide();
    TW->setAlternatingRowColors(true);
    TW->setShowGrid(false);
    TW->setSelectionMode(QAbstractItemView::SingleSelection);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);
    TW->setEditTriggers(QAbstractItemView::NoEditTriggers);
    TW_yel = TW;

    TW = new QTableWidget();
    TW->verticalHeader()->hide();
    TW->setAlternatingRowColors(true);
    TW->setShowGrid(false);
    TW->setSelectionMode(QAbstractItemView::SingleSelection);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);
    TW->setEditTriggers(QAbstractItemView::NoEditTriggers);
    TW_blu = TW;

    TW = new QTableWidget();
    TW->verticalHeader()->hide();
    TW->setAlternatingRowColors(true);
    TW->setShowGrid(false);
    TW->setSelectionMode(QAbstractItemView::SingleSelection);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);
    TW->setEditTriggers(QAbstractItemView::NoEditTriggers);
    TW_red = TW;

    TW = new QTableWidget();
    TW->verticalHeader()->hide();
    TW->setAlternatingRowColors(true);
    TW->setShowGrid(false);
    TW->setSelectionMode(QAbstractItemView::SingleSelection);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);
    TW->setEditTriggers(QAbstractItemView::NoEditTriggers);
    TW_grn = TW;

    TW = new QTableWidget();
    TW->verticalHeader()->hide();
    TW->setAlternatingRowColors(true);
    TW->setShowGrid(false);
    TW->setSelectionMode(QAbstractItemView::SingleSelection);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);
    TW->setEditTriggers(QAbstractItemView::NoEditTriggers);
    TW_blk = TW;

    TW = new QTableWidget();
    TW->verticalHeader()->hide();
    TW->setAlternatingRowColors(true);
    TW->setShowGrid(false);
    TW->setSelectionMode(QAbstractItemView::SingleSelection);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);
    TW->setEditTriggers(QAbstractItemView::NoEditTriggers);
    TW_ANY = TW;

    tag_processor->set_tag_tablewidget(RV_TAG_YEL, TW_yel);
    tag_processor->set_tag_tablewidget(RV_TAG_GRN, TW_grn);
    tag_processor->set_tag_tablewidget(RV_TAG_RED, TW_red);
    tag_processor->set_tag_tablewidget(RV_TAG_ANY, TW_ANY);
    tag_processor->set_tag_tablewidget(RV_TAG_BLK, TW_blk);
    tag_processor->set_tag_tablewidget(RV_TAG_BLU, TW_blu);

    connect(tag_processor, SIGNAL(tags_changed()),
            dv, SLOT(do_repaint()));

    tabwidget_tags = new QTabWidget();
    tabwidget_tags->addTab(TW_ANY, QIcon(":/icons/4stars_small.png"), "");
    tabwidget_tags->addTab(TW_yel, QIcon(":/icons/bookmark_small.png"), "");
    tabwidget_tags->addTab(TW_grn, QIcon(":/icons/bookmark_grn_small.png"), "");
    tabwidget_tags->addTab(TW_red, QIcon(":/icons/bookmark_red_small.png"), "");
    tabwidget_tags->addTab(TW_blk, QIcon(":/icons/bookmark_black_small.png"), "");
    tabwidget_tags->addTab(TW_blu, QIcon(":/icons/bookmark_blu_small.png"), "");

    connect(TW_ANY, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tabwidget_va_dblclicked(int,int)));
    connect(TW_yel, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tabwidget_va_dblclicked(int,int)));
    connect(TW_blu, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tabwidget_va_dblclicked(int,int)));
    connect(TW_red, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tabwidget_va_dblclicked(int,int)));
    connect(TW_grn, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tabwidget_va_dblclicked(int,int)));
    connect(TW_blk, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tabwidget_va_dblclicked(int,int)));

    tabwidget_tags->setDocumentMode(true);

    vl = new QVBoxLayout();
    //hl = new QHBoxLayout();

    g = new QGroupBox("Tags");
    g->setFont(my_font);

    vl->addWidget(tabwidget_tags);
    //vl->addLayout(hl);
    g->setLayout(vl);
    vl_main->addWidget(g);
    infopane_widgets << g;

    // --

    // -- sub views --
    vl = new QVBoxLayout();
    hl = new QHBoxLayout();

    g = new QGroupBox("Sub views");
    g->setFont(my_font);

    pushbutton_new_filter_view = new QPushButton(
                QIcon(":/icons/disasm_view_sub_filter_plus.png"), "");
    pushbutton_new_filter_view->setIconSize(QSize(32, 16));
    pushbutton_new_target_view = new QPushButton(
                QIcon(":/icons/disasm_view_t_plus.png"), "");
    pushbutton_new_target_view->setIconSize(QSize(24, 16));

    connect(pushbutton_new_filter_view, SIGNAL(clicked()),
            this, SLOT(create_new_filter_view()));
    connect(pushbutton_new_target_view, SIGNAL(clicked()),
            this, SLOT(create_new_target_view()));

    pushbutton_new_filter_view->setToolTip("New FILTER view for target");
    pushbutton_new_target_view->setToolTip("New target view");

//    QLabel *l1 = new QLabel("Target Views: ");
//    l1->setFont(my_font);
    hl->addWidget(pushbutton_new_target_view);
    hl->addWidget(pushbutton_new_filter_view);

    hl->addStretch();

    vl->addLayout(hl);
    g->setLayout(vl);
    vl_main->addWidget(g);
    infopane_widgets << g;
    // --

    // main layout
    vl_main->addStretch();
    vl_main->setContentsMargins(6,0,6,0);

    infopane->setLayout(vl_main);

    connect(dv, SIGNAL(cursor_changed_to(re_addr_t)),
            this, SLOT(show_cursor_info(re_addr_t)));

    connect(dv, SIGNAL(cursor_changed_to(re_addr_t)),
            this, SLOT(view_changed_disasm_addr_space()));

    dv->set_navi_stack(&navi_stack);
}

void rv_idisasm::update_last_pos(re_addr_t va)
{
    last_pos_va = va;
}


// ================= CONSOLE ===================================================


void rv_idisasm::process_console_cmd(QString cmd)
{
    re_addr_t va, offset;

    QRegExp rx;

    if(cmd.isEmpty()) {
        return;
    }

    // try command functions
    rx.setPattern("^\\s*\\b(\\w+)\\b\\s*(.*)\\s*");
    if (rx.indexIn(cmd) != -1) {
        QString cmd_fname = rx.cap(1);
        QString cmd_args_s = rx.cap(2);
        QStringList cmd_args = cmd_args_s.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        foreach(QString fname, console_cmd_functions.keys()) {
            if(cmd_fname == fname) {
                QString argline = rx.cap(2);

//                bool (rv_idisasm::*f)(QString, QStringList);
//                (this->*f)(argline, QStringList());

                (this->*console_cmd_functions[fname].f)(argline, cmd_args);

                return;
            }
        }
    }

    // try va
    if(get_va_or_sym_va(va, cmd))
        if(t->file->b_va_to_file_offset(offset, va)) {
            goto_va(va);
            console->cmd_executed("moved to va: " +
                                  re_core::dword_to_hex_string(va));
            return;
        } else {
            console->print_err("invalid address: " +
                                  re_core::dword_to_hex_string(va));
            return;
        }

    // else syntax error
    console->print_err("syntax error.\nenter 'help' for a list of available commands.");
}

bool rv_idisasm::get_va_or_sym_va(re_addr_t &r_va, QString arg)
{
    QRegExp rx; QString match;

    if(arg.isEmpty()) {
        r_va = BAD_ADDR;
        return false;
    }

    // try va
    bool ok;
    re_addr_t va, offset;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPattern("^\\s*[0x]*([0-9a-f]{6,8})\\s*$");
    if (rx.indexIn(arg) != -1) {
        match = rx.cap(1);
        va =  (re_addr_t) match.toUInt(&ok, 16);
        if(ok) {
            // test va valid
            if(t->file->b_va_to_file_offset(offset, va)) {
                r_va = va;
                return true;
            } else {
                r_va = BAD_ADDR;
                return false;
            }
        } else {
            // could be a name though hm.....
        }
    }

    QString s;
    QList<char *> l;
    int i;

    // try imports
    rx.setPattern(arg);
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    l = t->file->va_imports_map.values();

    for(i=0; i < l.count(); i++) {
        s = QString::fromAscii(l.at(i));
        if(rx.indexIn(s) != -1) {
            va = t->file->va_imports_map.key(l.at(i));
            if(t->file->b_va_to_file_offset(offset, va)) {
                r_va = va;
                return true;
            } else {
                r_va = BAD_ADDR;
                return false;
            }
        }
    }

    // try symbols
    l = t->file->va_symbols_map.values();

    for(i=0; i < l.count(); i++) {
        s = QString::fromAscii(l.at(i));
        if(rx.indexIn(s) != -1) {
            va = t->file->va_symbols_map.key(l.at(i));
            if(t->file->b_va_to_file_offset(offset, va)) {
                r_va = va;
                return true;
            } else {
                r_va = BAD_ADDR;
                return false;
            }
        }
    }

    // try functions
    l = t->file->va_functions_map.values();

    for(i=0; i < l.count(); i++) {
        s = QString::fromAscii(l.at(i));
        if(rx.indexIn(s) != -1) {
            va = t->file->va_functions_map.key(l.at(i));
            if(t->file->b_va_to_file_offset(offset, va)) {
                r_va = va;
                return true;
            } else {
                r_va = BAD_ADDR;
                return false;
            }
        }
    }

    return false;
}

void rv_idisasm::setup_console(rv_console_edit *c)
{
    struct rv_idisasm_cmd_function func;

    func.name = "help";
    func.descr = "show available commands / help on specific command";
    func.help =  "help for command '" + func.name + "'\n" +
            "syntax: 'help [cmd_name(s) ...]'\n" +
            "'help' lists the short description for all available commands.\n'help cmd_name(s) ...' shows help on the specified command(s)";
    func.f = &rv_idisasm::idisasm_cmd_help;
    console_cmd_functions[func.name] = func;

    func.name = "d";
    func.descr = "disassemble";
    func.help =  "help for command '" + func.name + "'\n"
            "syntax: 'd [va]'\n" +
            "'d' starts a recursive control flow disassembly from the current cursor position.\n" +
            "'d va' starts the disassembly from the specified va.\n" +
            "example: 'd 08040506'\n" +
            "example: 'd 0x08040506'";
    func.f = &rv_idisasm::idisasm_cmd_d;
    console_cmd_functions[func.name] = func;

    func.name = "b";
    func.descr = "convert to data";
    func.help =  "help for command '" + func.name + "'\n"
            "syntax: 'b [va] [len]'";
    func.f = &rv_idisasm::idisasm_cmd_b;
    console_cmd_functions[func.name] = func;

    func.name = "tag";
    func.descr = "set a tag";
    func.help = "help for command '" + func.name + "'\n" +
            "syntax: 'tag <ygrbB> [text]'\n" +
            "y: yellow, g: green, r: red, b: blue, B: black. text is optional.";
    func.f = &rv_idisasm::idisasm_cmd_tag;
    console_cmd_functions[func.name] = func;

    func.name = "clear";
    func.descr = "clear console window";
    func.help = "help for command '" + func.name + "'\n" +
            "syntax: 'clear'\n";
    func.f = &rv_idisasm::idisasm_cmd_clear;
    console_cmd_functions[func.name] = func;

    func.name = "history";
    func.descr = "show or clear command history";
    func.help = "help for command '" + func.name + "'\n" +
            "syntax: 'history [clear]'\n" +
            "'history' shows the current command history.\n" +
            "'history clear' clears the current command history."
            ;
    func.f = &rv_idisasm::idisasm_cmd_history;
    console_cmd_functions[func.name] = func;

    func.name = "subview";
    func.descr = "create a new, empty subview";
    func.help = "help for command '" + func.name + "'\n" +
            "syntax: 'subview <name>'\n"
            ;
    func.f = &rv_idisasm::idisasm_cmd_subview;
    console_cmd_functions[func.name] = func;
}


bool rv_idisasm::idisasm_cmd_help(QString line, QStringList args)
{
    QString rs;

    if(args.isEmpty()) {
        QStringList fnames = console_cmd_functions.keys();
        fnames.sort();

        foreach(QString fname, fnames) {
            rs += fname + "\t" + console_cmd_functions[fname].descr + "\n";
        }
    } else {
        foreach(QString fname, args) {
            if(console_cmd_functions.keys().contains(fname)) {
                rs += "\n" + console_cmd_functions[fname].help + "\n";
            } else {
                rs += "\n" + fname + "\t" + "<unknown command!>" + "\n";
            }
        }
    }

    console->cmd_executed(rs);
    return true;
}

bool rv_idisasm::idisasm_cmd_clear(QString line, QStringList args)
{
    console->clear_box();
    return true;
}

bool rv_idisasm::idisasm_cmd_history(QString line, QStringList args)
{
    QString rs;

    if(args.isEmpty()) {
        int i=1;
        rs += (QString)"command history:\n" +
              "----------------\n\n";
        foreach(QString cmd, console->get_history()) {
                rs += QString::number(i++) + ") " + cmd + "\n";
        }
    } else if((args.count() == 1) && (args.at(0) == "clear") ){
        console->clear_history();
        rs = "command history cleared.";
    } else {
        console->print_err("syntax error in command 'history'.\n");
        console->cmd_executed(console_cmd_functions["history"].help);

        return false;
    }

    console->cmd_executed(rs);
    return true;
}

bool rv_idisasm::idisasm_cmd_b(QString line, QStringList args)
{
    console->cmd_executed("executed b");

    return true;
}

bool rv_idisasm::idisasm_cmd_d(QString line, QStringList args)
{
    if(args.isEmpty()) {
        re_addr_desc *adesc = active_dv->get_cursor_desc();

        if(!adesc) {
            console->cmd_executed("no address selected (where is the cursor?).");
            return false;
        }

        pre_disassembly_changes();
        d_va = adesc->va;
        active_dv->disassemble_ctf_from_va(adesc->va); // includes pre disasm change
        console->cmd_executed((QString)"executing control flow disassembly"
                              + "from cursor position: "
                              + re_core::dword_to_hex_string(adesc->va));
        return true;
    } else if((args.count() == 1)) {
        re_addr_t va;
        if(get_va_or_sym_va(va, args.at(0))) {
            pre_disassembly_changes();
            d_va = va;
            active_dv->disassemble_ctf_from_va(va);
            console->cmd_executed("executing control flow disassembly from va: "
                                  + re_core::dword_to_hex_string(va));
            return true;
        } else {            
            console->print_err("invalid address: "
                               + re_core::dword_to_hex_string(va));
            return false;
        }
    }

    console->print_err("syntax error in command 'd'.\n");
    console->cmd_executed(console_cmd_functions["d"].help);

    return false;
}

bool rv_idisasm::idisasm_cmd_tag(QString line, QStringList args)
{
    re_addr_desc *adesc;
    if(!(adesc = dv->get_cursor_desc())) {
        console->print_err("invalid or no address selected.");
        return false;
    }

    if(args.isEmpty()) {
        console->print_err("syntax error in command 'tag'.\n");
        console->cmd_executed(console_cmd_functions["tag"].help);
        return false;
    }

    if(args.at(0).length() > 2) {
        console->print_err("syntax error in command 'tag'.\n");
        console->cmd_executed(console_cmd_functions["tag"].help);
        return false;
    }

    unsigned long tag_val;
    QString tag_sval;
    QString tag_text;

    switch(args.at(0).at(0).toAscii()) {
    case 'y':
        tag_val = RV_TAG_YEL;
        tag_sval = "yellow";
        break;
    case 'r':
        tag_val = RV_TAG_RED;
        tag_sval = "red";
        break;
    case 'g':
        tag_val = RV_TAG_GRN;
        tag_sval = "green";
        break;
    case 'b':
        tag_val = RV_TAG_BLU;
        tag_sval = "blue";
        break;
    case 'B':
        tag_val = RV_TAG_BLK;
        tag_sval = "black";
        break;
    default:
        console->print_err("syntax error.\n");
        console->cmd_executed(console_cmd_functions["tag"].help);
        return false;
        break;
    }

    if(args.count() == 2) tag_text = args.at(1);

    tag_processor->set_tag_va(adesc->va, tag_val, tag_text);
    console->cmd_executed(tag_sval + " tag set at va " +
                          re_core::dword_to_hex_string(adesc->va));
    return true;
}

bool rv_idisasm::idisasm_cmd_subview(QString line, QStringList args)
{
    if(args.isEmpty()) {
        console->print_err("syntax error in command 'subview'.\n");
        console->cmd_executed(console_cmd_functions["subview"].help);

        return false;
    } else if((args.count() == 1)) {
        new_filter_view(args.at(0));
        console->cmd_executed("subview '" +
                              args.at(0) + "' created.\n");
        return true;
    }

    console->print_err("syntax error in command 'subview'.\n");
    console->cmd_executed(console_cmd_functions["subview"].help);

    return false;
}

// ---------

void rv_idisasm::show_cursor_info(re_addr_t va)
{
    re_addr_t offset;
    x86_insn_t insn;
    char buf[256];
    unsigned char b;

    t->file->b_va_to_file_offset(offset, va);

    t->disassembler->disassemble_single(offset, &insn);
    if(insn.size) {
        t->disassembler->format_insn_string(&insn, buf, 255);
        label_cursor_info->setText(QString::fromAscii(buf));
    } else {
        t->file->b_get_byte_at_file_offset(b, offset);
        label_cursor_info->setText("<invalid instruction>\ndb "
                                   + re_core::byte_to_hex_string(b));
    }
}

void rv_idisasm::biggify_console(bool yesno)
{
    if(yesno) {
        splitter_disasm_console->setSizes(QList<int>() << height()*0.25 << height()*0.75);

    } else {
        splitter_disasm_console->setSizes(QList<int>() << height() << 120);
    }
}

void rv_idisasm::disassembly_changed()
{
    console->print_shadow((QString)"disassembly finished");
    if(sender() == active_dv) {
        if(d_va) active_dv->scroll_n_cursor_to(d_va);
        d_va = 0;
    }
}

void rv_idisasm::pre_disassembly_changes()
{
    active_dv->pre_disassembly_change();
}

// never called
void rv_idisasm::post_disassembly_change()
{
    active_dv->post_disassembly_change();
}

void rv_idisasm::change_orientation()
{
    if(splitter_disasm->orientation() == Qt::Vertical)
        splitter_disasm->setOrientation(Qt::Horizontal);
    else
        splitter_disasm->setOrientation(Qt::Vertical);
}

void rv_idisasm::close_sub_view(int idx)
{
    if(rv_disasm_view *sdv = qobject_cast<rv_disasm_view *>(
                tabwidget_sub_views->widget(idx))) {
        tabwidget_sub_views->removeTab(idx);

        for(int i=0; i< subviews.count(); i++) {
            if(subviews.at(i).dv == sdv) {
                subviews.removeAt(i);
                break;
            }
        }

        delete sdv;
        active_dv = dv;
        t->disassembler->set_address_space(t->address_space);
        console->print_shadow("info: using target view");
    }
    if(!tabwidget_sub_views->count()) tabwidget_sub_views->hide();
}

void rv_idisasm::view_changed_disasm_addr_space()
{
    rv_disasm_view *sdv = qobject_cast<rv_disasm_view *>(sender());
    if(!dv) return; // uuh in this case

    if(active_dv == sdv) return;

    active_dv = sdv;

    t->disassembler->set_address_space(sdv->get_address_space());

    if(sdv == dv) {
        // console->cmd_executed("info: using target view");
        console->set_promt(   "rv::target> ");
        console->print_shadow_at_prompt("( switched view )");
    } else {
//        console->cmd_executed("info: using view '" +
//                              sdv->get_name() + "'");
        console->set_promt(   "rv::" + sdv->get_name() + "> ");
        console->print_shadow_at_prompt("( switched view )");
    }
}

void rv_idisasm::set_active_dv(rv_disasm_view *view)
{
    if(active_dv == view) return;

    active_dv = view;
    t->disassembler->set_address_space(view->get_address_space());

    if(view == dv) {
        // console->cmd_executed("info: using target view");
        console->set_promt(   "rv::target> ");
        console->print_shadow_at_prompt("( switched view )");
    } else {
//        console->cmd_executed("info: using view '" +
//                              sdv->get_name() + "'");
        console->set_promt(   "rv::" + view->get_name() + "> ");
        console->print_shadow_at_prompt("( switched view )");
    }
}

void rv_idisasm::create_new_filter_view()
{
    bool ok;
    QString text;

    while (text.isEmpty()) {
        text = QInputDialog::getText(this, tr("Create a new FILTER view from target"),
                                                     tr("Name:"), QLineEdit::Normal,
                                                     "", &ok);
        if(!ok) return;
    }

    new_filter_view(text);
}

void rv_idisasm::create_new_target_view()
{
    bool ok;
    QString text;

    while (text.isEmpty()) {
        text = QInputDialog::getText(this, tr("Create a new TARGET view from target"),
                                                     tr("Name:"), QLineEdit::Normal,
                                                     "", &ok);
        if(!ok) return;
    }

    new_target_view(text);
}

void rv_idisasm::tabwidget_va_dblclicked(int r, int c)
{
        QTableWidgetItem *ti;
        re_addr_t va;

        ti = ((QTableWidget *)sender())->item(r, 0);

        va = ti->text().toUInt(0, 16);

        if(active_dv) active_dv->scroll_n_cursor_to(va);
}

rv_disasm_view *rv_idisasm::get_visible_filter_view()
{
    rv_disasm_view *dview;

    if(dview = (rv_disasm_view*) tabwidget_sub_views->widget(tabwidget_sub_views->currentIndex())) {
        if(dview->get_mode() == DV_MODE_FILTER) return dview;
    }

    return 0;
}

void rv_idisasm::disasm_ctf_into_filter_view(re_addr_t va)
{
    rv_disasm_view *dview;

    // if called from filter subview
    if(dview = qobject_cast<rv_disasm_view*>(sender())) {
        if(dview->get_mode() == DV_MODE_FILTER) {
            new_filter_view("ctf " + re_core::dword_to_pure_hex_string(va));
        }
    }

    // if called from main or any other target subview
    if(!(dview = get_visible_filter_view())) {
        new_filter_view("ctf " + re_core::dword_to_pure_hex_string(va));
    } else {
        set_active_dv(dview);
    }

    active_dv->disassemble_ctf_from_va(va); // includes pre disasm change
    active_dv->scroll_n_cursor_to(va);
}

void rv_idisasm::disasm_linear_into_filter_view(re_addr_t va)
{
    rv_disasm_view *dview;

    // if called from filter subview
    if(dview = qobject_cast<rv_disasm_view*>(sender())) {
        if(dview->get_mode() == DV_MODE_FILTER) {
            new_filter_view("ctf " + re_core::dword_to_pure_hex_string(va));
        }
    }

    // if called from main or any other target subview
    if(!(dview = get_visible_filter_view())) {
        new_filter_view("linear " + re_core::dword_to_pure_hex_string(va));
    } else {
        set_active_dv(dview);
    }

    active_dv->disassemble_linear_from_va(va); // includes pre disasm change
    active_dv->scroll_n_cursor_to(va);
}


void rv_idisasm::paste_ctf_into_new_filter()
{
    if(!selection.active) return;

    new_filter_view("ctf paste " +
                    re_core::dword_to_pure_hex_string(selection.start->va) +
                    " - " +
                    re_core::dword_to_pure_hex_string(selection.end->va)
                    );

    active_dv->menu_paste_ctf();
}

void rv_idisasm::paste_into_new_filter()
{
    if(!selection.active) return;

    new_filter_view("paste " +
                    re_core::dword_to_pure_hex_string(selection.start->va) +
                    " - " +
                    re_core::dword_to_pure_hex_string(selection.end->va)
                    );

    active_dv->menu_paste();
}
