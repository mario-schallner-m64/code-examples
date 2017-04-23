#include "rv_mainwin.h"
#include "ui_rv_mainwin.h"


rv_mainwin::rv_mainwin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::rv_mainwin)
{
    ui->setupUi(this);

    f = 0;
    t = 0;
    l = new re_logger();
    about_dialog = 0;

    ui->textEdit_startup->setHidden(false);
//    QString tmp = ui->textEdit_startup->toHtml();
//    tmp.replace("__VERSION__", RV_VERSION);
//    ui->textEdit_startup->setHtml(tmp);


    ui->scrollArea_overview->setHidden(true);

    // connect menu actions: File
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exit_review()));

    connect(ui->actionOpen_Binary, SIGNAL(triggered()),
            this, SLOT(open_file()));

    // connect menu actions: Display
    connect(ui->actionImports, SIGNAL(triggered()),
            this, SLOT(display_imports()));

    connect(ui->actionSymbols, SIGNAL(triggered()),
            this, SLOT(display_symbols()));

    connect(ui->actionNew_Hexview, SIGNAL(triggered()),
            this, SLOT(new_hexview()));

    connect(ui->actionNew_Disassembly, SIGNAL(triggered()),
            this, SLOT(new_disasm_view()));

    // connect menu actions: new windows
    connect(ui->actionNew_Hexview_Window, SIGNAL(triggered()),
            this, SLOT(new_hexview_window()));


    //                                              display tabs:
    connect(ui->actionOverview, SIGNAL(triggered()),
            this, SLOT(show_overview_tab()));
    connect(ui->actionACSS, SIGNAL(triggered()),
            this, SLOT(show_acss_tab()));
    connect(ui->actionWILDASM, SIGNAL(triggered()),
            this, SLOT(show_wildasm_tab()));
    connect(ui->actionEditor, SIGNAL(triggered()),
            this, SLOT(show_re_script_tab()));
    connect(ui->actionNotes, SIGNAL(triggered()),
            this, SLOT(show_notes_tab()));
    connect(ui->actionLog, SIGNAL(triggered()),
            this, SLOT(show_log_tab()));


    // connect menu actions: Disassemble
    connect(ui->actionFull_Analysis, SIGNAL(triggered()),
            this, SLOT(run_full_analysis()));

    // connect find functions
    connect(ui->lineEdit_find_import, SIGNAL(textChanged(QString)),
            this, SLOT(find_import()));
    connect(ui->lineEdit_find_import, SIGNAL(returnPressed()),
            this, SLOT(find_next_import()));
    connect(ui->pushButton_find_import, SIGNAL(clicked()),
            this, SLOT(find_next_import()));

    connect(ui->lineEdit_find_function, SIGNAL(textChanged(QString)),
            this, SLOT(find_function()));
    connect(ui->lineEdit_find_function, SIGNAL(returnPressed()),
            this, SLOT(find_next_function()));
    connect(ui->pushButton_find_function, SIGNAL(clicked()),
            this, SLOT(find_next_function()));

    connect(ui->lineEdit_find_symbol, SIGNAL(textChanged(QString)),
            this, SLOT(find_symbol()));
    connect(ui->lineEdit_find_symbol, SIGNAL(returnPressed()),
            this, SLOT(find_next_symbol()));
    connect(ui->pushButton_find_symbol, SIGNAL(clicked()),
            this, SLOT(find_next_symbol()));

    connect(ui->lineEdit_find_string, SIGNAL(textChanged(QString)),
            this, SLOT(find_string()));
    connect(ui->lineEdit_find_string, SIGNAL(returnPressed()),
            this, SLOT(find_next_string()));
    connect(ui->pushButton_find_string, SIGNAL(clicked()),
            this, SLOT(find_next_string()));

    // tab close action
    connect(ui->tabWidget_main, SIGNAL(tabCloseRequested(int)),
            this, SLOT(tab_close_request(int)));


    // connect menu actions
    connect(ui->actionShow_Fullscreen, SIGNAL(triggered(bool)),
            this, SLOT(set_fullscreen(bool)));

    connect(ui->actionAbout_re_view, SIGNAL(triggered()),
            this, SLOT(show_about_dialog()));


    // ======================================== connect navigation actions

    connect(this, SIGNAL(global_goto_va(re_addr_t)),
            this, SLOT(va_receiver(re_addr_t)));

    connect(ui->tableWidget_imports, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(symbolic_tab_clicked(int,int)));
    connect(ui->tableWidget_symbols, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(symbolic_tab_clicked(int,int)));
    connect(ui->tableWidget_strings, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(symbolic_tab_clicked(int,int)));
    connect(ui->tableWidget_functions, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(symbolic_tab_clicked(int,int)));

    // latest gui stuff

    ui->dockWidget_symbolic->hide();
    ui->tabWidget_main->setTabText(0, "Welcome");
    ui->tabWidget_main->setCurrentIndex(0);

    pushButton_detach = new QPushButton(QIcon(":/icons/detach_to_new_win.png"),
                                        "", this);
    pushButton_detach->setFlat(true);
    pushButton_detach->setEnabled(false);
    ui->tabWidget_main->setCornerWidget(pushButton_detach,
                                        Qt::TopRightCorner);

    connect(pushButton_detach, SIGNAL(clicked()),
            this, SLOT(detach_to_new_win_clicked()));

    connect(ui->tabWidget_main, SIGNAL(currentChanged(int)),
            this, SLOT(tab_idx_changed(int)));

    ui->actionSave_Workspace->setDisabled(true);
    ui->actionSave_Workspace_as->setDisabled(true);
    ui->actionWrite_Binary->setDisabled(true);
    ui->actionSave_Binary_as->setDisabled(true);
    ui->actionClose_Workspace->setDisabled(true);

    ui->actionOverview->setEnabled(false);
    ui->actionACSS->setEnabled(false);
    ui->actionWILDASM->setEnabled(false);
    ui->actionEditor->setEnabled(false);
    ui->actionNotes->setEnabled(false);
    ui->actionLog->setEnabled(false);

    ui->actionNew_Disassembly->setEnabled(false);
    ui->actionNew_Disassembly_Window->setEnabled(false);
    ui->actionNew_Hexview->setEnabled(false);
    ui->actionNew_Hexview_Window->setEnabled(false);

    ui->actionImports->setEnabled(false);
    ui->actionASCIIZ_Strings->setEnabled(false);
    ui->actionBookmarks->setEnabled(false);
    ui->actionComments->setEnabled(false);
    ui->actionCrypto_Scan->setEnabled(false);
    ui->actionDisassemble->setEnabled(false);
    ui->actionExports->setEnabled(false);
    ui->actionFileOffset->setEnabled(false);
    ui->actionFull_Analysis->setEnabled(false);
    ui->actionNames->setEnabled(false);
    ui->actionSegment_offset->setEnabled(false);
    ui->actionSymbols->setEnabled(false);
    //ui->actionTags->setEnabled(false);
    ui->actionUNICODE_Strings->setEnabled(false);
    ui->actionVa->setEnabled(false);

    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    setup_mainwin_tabs();

    ui->statusBar->showMessage((QString)"re|view " + RV_VERSION + " READY.");
}

void rv_mainwin::exit_review()
{
    exit(0);
}

rv_mainwin::~rv_mainwin()
{
    delete ui;
}

void rv_mainwin::log_msg(int level, const QString &msg) {
    QString h;

    switch(level) {
    case RE_LOG_WARNING:
        h = "[warning]";
        break;

    case RE_LOG_ERROR:
        h = "[!ERROR!]";
        break;

    case RE_LOG_DEBUG_1:
        h = "[debug|1]";
        break;
    case RE_LOG_DEBUG_2:
        h = "[debug|2]";
        break;
    case RE_LOG_DEBUG_3:
        h = "[debug|3]";
        break;

    case RE_LOG_NORMAL:
    default:
        h = "[info   ]";
        break;
    }

    ui->textEdit_log->append(h + " " + msg);
}

bool rv_mainwin::open_file()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Binary"), "", tr("All files (*.*);;"
                                    "MS-Windows Binary Files (*.exe *.dll)"));

    if(!fileName.isNull())  {
        if(f) delete f;

        f = new re_file();
        f->set_strings_scan_on_load(false);
        f->set_filename(fileName);
        connect(f, SIGNAL(msg(int,QString)), this, SLOT(log_msg(int,QString)));
        connect(f, SIGNAL(msg(int,QString)), l, SLOT(log_event(int,QString)));

        // threased opening for progress bar
        rv_file_analyze_thread *at = new rv_file_analyze_thread();
        at->set_file(f);
        connect(at, SIGNAL(finished()), this, SLOT(file_opened()));

        progress_dialog = new rv_progress_dialog();
        progress_dialog->set_main_task_msg("Opening " + f->file_name);
        progress_dialog->set_pct(0);
        progress_dialog->show();
        connect(f, SIGNAL(file_load_pct(int)),
                progress_dialog, SLOT(set_pct(int)));
        connect(f, SIGNAL(strings_scan_pct(int)),
                progress_dialog, SLOT(set_pct(int)));
        connect(f, SIGNAL(msg(int,QString)),
                progress_dialog, SLOT(set_progress_msg(int, QString)));
        //connect(t->disassembler, SIGNAL(progress_changed_pct(int)),
        //        this, SLOT(disasm_progress_status(int)));
        //connect(t->disassembler, SIGNAL(msg(int,QString)), this,
        //        SLOT(status_msg(int,QString)));

        at->start();
        return true;
    }

    else {
        ui->statusBar->showMessage("Error loading file '" + fileName
                                   + "'");
        return false;
    }
}

void rv_mainwin::file_opened()
{
    t = new re_target();
    t->init(f, l);
    connect(t->disassembler, SIGNAL(msg(int,QString)),
            this, SLOT(log_msg(int,QString)));

    ui->statusBar->showMessage("Successfully loaded file '" + f->file_name
                                + "'. Populating file info ...");

    ui->textEdit_startup->setHidden(true);
    ui->tabWidget_main->setTabText(0, "Overview");

    progress_dialog->set_main_task_msg("populating file info");
    populate_file_info();

    progress_dialog->close();

    progress_dialog->set_main_task_msg("creating new hexview");
    new_hexview(false);

    progress_dialog->set_main_task_msg("creating file overview");
    create_overview();
    ui->statusBar->showMessage((QString)"re|view " + RV_VERSION + " READY.");


    ui->actionNew_Hexview->setEnabled(true);
    ui->actionNew_Hexview_Window->setEnabled(true);

    ui->actionNew_Disassembly->setEnabled(true);

    ui->actionImports->setEnabled(true);
    ui->actionASCIIZ_Strings->setEnabled(true);
    ui->actionBookmarks->setEnabled(true);
    ui->actionComments->setEnabled(true);
    ui->actionCrypto_Scan->setEnabled(true);
    ui->actionDisassemble->setEnabled(true);
    ui->actionExports->setEnabled(true);
    ui->actionFileOffset->setEnabled(true);
    ui->actionFull_Analysis->setEnabled(true);
    ui->actionNames->setEnabled(true);
    ui->actionSegment_offset->setEnabled(true);
    ui->actionSymbols->setEnabled(true);
    //ui->actionTags->setEnabled(true);
    ui->actionUNICODE_Strings->setEnabled(true);
    ui->actionVa->setEnabled(true);
}

void rv_mainwin::populate_file_info()
{
    populate_imports();
    populate_symbols();
    populate_functions();
    populate_strings();


    //ui->dockWidget_symbolic->show();
    ui->tabWidget_symbolic->setCurrentIndex(0);
}

void rv_mainwin::populate_imports()
{
    QTableWidgetItem *item;
    int i = 0;

    if(f->va_imports_map.isEmpty()) return;

    ui->tableWidget_imports->clear();
    ui->tableWidget_imports->setRowCount(f->va_imports_map.count());
    ui->tableWidget_imports->setColumnCount(2);
    ui->tableWidget_imports->verticalHeader()->setHidden(true);
    QStringList header;
    header << "VA" << "Name";
    ui->tableWidget_imports->setHorizontalHeaderLabels(header);
    ui->tableWidget_imports->setAlternatingRowColors(true);

    foreach (const re_addr_t va, f->va_imports_map.keys())  {
        item = new QTableWidgetItem(QString::number(va, 16));
        ui->tableWidget_imports->setItem(i, 0, item);
        item = new QTableWidgetItem(QIcon(":/icons/import_small.png"),
                f->va_imports_map.value(va));
        ui->tableWidget_imports->setItem(i, 1, item);
        i++;
    }

    ui->tableWidget_imports->setShowGrid(false);
    ui->tableWidget_imports->resizeColumnsToContents();
    ui->tableWidget_imports->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_imports->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void rv_mainwin::display_imports()
{
    ui->dockWidget_symbolic->show();
    ui->tabWidget_symbolic->setCurrentIndex(0);
}

void rv_mainwin::populate_symbols()
{
    QTableWidgetItem *item;
    int i = 0;

    if(f->va_symbols_map.isEmpty()) return;

    ui->tableWidget_symbols->clear();
    ui->tableWidget_symbols->setRowCount(f->va_symbols_map.count());
    ui->tableWidget_symbols->setColumnCount(2);
    ui->tableWidget_symbols->verticalHeader()->setHidden(true);
    QStringList header;
    header << "VA" << "Name";
    ui->tableWidget_symbols->setHorizontalHeaderLabels(header);
    ui->tableWidget_symbols->setAlternatingRowColors(true);

    foreach (const re_addr_t va, f->va_symbols_map.keys())  {
        item = new QTableWidgetItem(QString::number(va, 16));
        ui->tableWidget_symbols->setItem(i, 0, item);
        item = new QTableWidgetItem(QIcon(":/icons/symbol_small.png"),
                f->va_symbols_map.value(va));
        ui->tableWidget_symbols->setItem(i, 1, item);
        i++;
    }

    ui->tableWidget_symbols->setShowGrid(false);
    ui->tableWidget_symbols->resizeColumnsToContents();
    ui->tableWidget_symbols->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_symbols->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void rv_mainwin::display_symbols()
{
    ui->dockWidget_symbolic->show();
    ui->tabWidget_symbolic->setCurrentIndex(1);
}

void rv_mainwin::populate_functions()
{
    QTableWidgetItem *item;
    int i = 0;

    if(f->va_functions_map.isEmpty()) return;

    ui->tableWidget_functions->clear();
    ui->tableWidget_functions->setRowCount(f->va_functions_map.count()
                                           + f->va_analyzed_functions_hash.count());
    ui->tableWidget_functions->setColumnCount(2);
    ui->tableWidget_functions->verticalHeader()->setHidden(true);
    QStringList header;
    header << "VA" << "Name";
    ui->tableWidget_functions->setHorizontalHeaderLabels(header);
    ui->tableWidget_functions->setAlternatingRowColors(true);

    foreach (const re_addr_t va, f->va_functions_map.keys())  {
        item = new QTableWidgetItem(QString::number(va, 16));
        ui->tableWidget_functions->setItem(i, 0, item);
        item = new QTableWidgetItem(QIcon(":/icons/function_small.png"),
                f->va_functions_map.value(va));
        ui->tableWidget_functions->setItem(i, 1, item);
        i++;
    }

    foreach (const re_addr_t va, f->va_analyzed_functions_hash.keys())  {
        item = new QTableWidgetItem(QString::number(va, 16));
        ui->tableWidget_functions->setItem(i, 0, item);
        item = new QTableWidgetItem(QIcon(":/icons/function_small.png"),
                f->va_analyzed_functions_hash.value(va));
        ui->tableWidget_functions->setItem(i, 1, item);
        i++;
    }

    ui->tableWidget_functions->setShowGrid(false);
    ui->tableWidget_functions->resizeColumnsToContents();
    ui->tableWidget_functions->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_functions->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void rv_mainwin::display_functions()
{
    ui->dockWidget_symbolic->show();
    ui->tabWidget_symbolic->setCurrentIndex(2);
}

void rv_mainwin::populate_strings()
{
    QTableWidgetItem *item;
    int i = 0;

    if(f->va_cstrings_hash.isEmpty()) return;

    ui->tableWidget_strings->clear();
    ui->tableWidget_strings->setRowCount(f->va_cstrings_hash.count());
    ui->tableWidget_strings->setColumnCount(2);
    ui->tableWidget_strings->verticalHeader()->setHidden(true);
    QStringList header;
    header << "VA" << "String";
    ui->tableWidget_strings->setHorizontalHeaderLabels(header);
    ui->tableWidget_strings->setAlternatingRowColors(true);

    foreach (const re_addr_t va, f->va_cstrings_hash.keys())  {
        item = new QTableWidgetItem(QString::number(va, 16));
        ui->tableWidget_strings->setItem(i, 0, item);
        item = new QTableWidgetItem(QIcon(":/icons/strings_small.png"),
                f->va_cstrings_hash.value(va));
        ui->tableWidget_strings->setItem(i, 1, item);
        i++;
    }

    ui->tableWidget_strings->setShowGrid(false);
    ui->tableWidget_strings->resizeColumnsToContents();
    ui->tableWidget_strings->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_strings->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void rv_mainwin::display_strings()
{
    ui->dockWidget_symbolic->show();
    ui->tabWidget_symbolic->setCurrentIndex(3);
}

void rv_mainwin::find_import()
{
    QString find;

    QTableWidget *tw = ui->tableWidget_imports;
    find = ui->lineEdit_find_import->text();

    int maxpos = tw->rowCount() - 1;
    int found = 0;

    if(found_pos_import >= maxpos) found_pos_import = 0;

    QRegExp rx;
    rx.setPattern(find);
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    int i;

    for(i=found_pos_import; i < maxpos; i++) {
        if (rx.indexIn( tw->item(i,1)->text() ) != -1) {
            found_pos_import = i;
            found = 1;
            tw->selectRow(found_pos_import);
            break;
        }
    }
    if(!found) { tw->selectRow(0); found_pos_import = 0;}
}

void rv_mainwin::find_next_import()
{
    found_pos_import++;
    find_import();
}

void rv_mainwin::find_function()
{
    QString find;

    QTableWidget *tw = ui->tableWidget_functions;
    find = ui->lineEdit_find_function->text();

    int maxpos = tw->rowCount() - 1;
    int found = 0;

    if(found_pos_import >= maxpos) found_pos_function = 0;

    QRegExp rx;
    rx.setPattern(find);
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    int i;

    for(i=found_pos_function; i < maxpos; i++) {
        if (rx.indexIn( tw->item(i,1)->text() ) != -1) {
            found_pos_function = i;
            found = 1;
            tw->selectRow(found_pos_function);
            break;
        }
    }
    if(!found) { tw->selectRow(0); found_pos_function = 0;}
}

void rv_mainwin::find_next_function()
{
    found_pos_function++;
    find_function();
}

void rv_mainwin::find_symbol()
{
    QString find;

    QTableWidget *tw = ui->tableWidget_symbols;
    find = ui->lineEdit_find_symbol->text();

    int maxpos = tw->rowCount() - 1;
    int found = 0;

    if(found_pos_symbol >= maxpos) found_pos_symbol = 0;

    QRegExp rx;
    rx.setPattern(find);
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    int i;

    for(i=found_pos_symbol; i < maxpos; i++) {
        if (rx.indexIn( tw->item(i,1)->text() ) != -1) {
            found_pos_symbol = i;
            found = 1;
            tw->selectRow(found_pos_symbol);
            break;
        }
    }
    if(!found) { tw->selectRow(0); found_pos_symbol = 0;}
}

void rv_mainwin::find_next_symbol()
{
    found_pos_symbol++;
    find_symbol();
}

void rv_mainwin::find_string()
{
    QString find;

    QTableWidget *tw = ui->tableWidget_strings;
    find = ui->lineEdit_find_string->text();

    int maxpos = tw->rowCount() - 1;
    int found = 0;

    if(found_pos_string >= maxpos) found_pos_string = 0;

    QRegExp rx;
    rx.setPattern(find);
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    int i;

    for(i=found_pos_string; i < maxpos; i++) {
        if (rx.indexIn( tw->item(i,1)->text() ) != -1) {
            found_pos_string = i;
            found = 1;
            tw->selectRow(found_pos_string);
            break;
        }
    }
    if(!found) { tw->selectRow(0); found_pos_string = 0;}
}

void rv_mainwin::find_next_string()
{
    found_pos_string++;
    find_string();
}


void rv_mainwin::new_hexview(bool show_after_create)
{
    if(!f || !t || !t->file) return;
    int tab;

    rv_hex_editor *view = new rv_hex_editor(ui->tabWidget_main);
    view->set_target(t);

    tab = ui->tabWidget_main->addTab(view, QIcon(":/icons/hex_buffer_t.png"),
                                     "Disassembly View  " +
                                     QString::number(hex_views.count() + 1));
    if(show_after_create) ui->tabWidget_main->setCurrentIndex(tab);

    // remember me

    hex_views << view;
    rewrite_hexview_names();
}

void rv_mainwin::new_disasm_view(bool show_after_create)
{
    if(!f || !t || !t->file || !t->address_space || !t->address_space->image_map)
        return;
    int tab;

    rv_idisasm *view = new rv_idisasm(ui->tabWidget_main);
    view->set_target(t);

    tab = ui->tabWidget_main->addTab(view, QIcon(":/icons/disasm_view_t.png"),
                                     "Disassembly View " +
                                     QString::number(disasm_views.count() + 1));
    if(show_after_create) ui->tabWidget_main->setCurrentIndex(tab);

    if(t->address_space->count()) view->goto_va(t->file->va_entry_point);

    // remember me

    disasm_views << view;
}

void rv_mainwin::new_hexview_window()
{
    rv_hex_editor *view = new rv_hex_editor(ui->tabWidget_main);
    view->set_target(t);

    view->set_detached(true);
    view->show();
    view->resize(900, view->height());

    connect(view, SIGNAL(re_attach_me(rv_hex_editor*)),
            this, SLOT(attach_hexview(rv_hex_editor*)));
    // remember me

    hex_views << view;
}


void rv_mainwin::tab_close_request(int idx)
{
    rv_hex_editor *hv;
    rv_idisasm *tmp_dview;

    if(hv = qobject_cast<rv_hex_editor *>(ui->tabWidget_main->widget(idx))) {
        close_hexview(idx);
    }
    else
    if((tmp_dview = qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(idx)))) {
        ui->tabWidget_main->removeTab(idx);
        disasm_views.removeOne(tmp_dview);
        delete tmp_dview;
        rewrite_disasmview_names();
    }


    // treat disassembly views

    // else: treat main tabs
    else close_main_tab(idx);
}

void rv_mainwin::close_hexview(int idx)
{
    rv_hex_editor *hv;

    if(hv = qobject_cast<rv_hex_editor *>(ui->tabWidget_main->widget(idx))) {
        ui->tabWidget_main->removeTab(idx);
        hex_views.removeOne(hv);
        delete hv;
    }

    rewrite_hexview_names();
}

void rv_mainwin::close_main_tab(int idx)
{
    if(ui->tabWidget_main->widget(idx) == tab_overview) {
        if(!t) return;
        ui->actionOverview->setEnabled(true);
    }

    if(ui->tabWidget_main->widget(idx) == tab_acss)
        ui->actionACSS->setEnabled(true);

    if(ui->tabWidget_main->widget(idx) == tab_wildasm)
        ui->actionWILDASM->setEnabled(true);

    if(ui->tabWidget_main->widget(idx) == tab_re_script)
        ui->actionEditor->setEnabled(true);

    if(ui->tabWidget_main->widget(idx) == tab_notes)
        ui->actionNotes->setEnabled(true);

    if(ui->tabWidget_main->widget(idx) == tab_log)
        ui->actionLog->setEnabled(true);

    ui->tabWidget_main->removeTab(idx);
}


void rv_mainwin::rewrite_hexview_names()
{
    if(!ui->tabWidget_main->count()) return;

    rv_hex_editor *hv;
    int cnt = 1;


    for(int i=0; i<ui->tabWidget_main->count(); i++) {
        if(hv = qobject_cast<rv_hex_editor *>(ui->tabWidget_main->widget(i))) {
            ui->tabWidget_main->setTabText(i, "Hex View "
                                           + QString::number(cnt++));
        }
    }
}

void rv_mainwin::rewrite_disasmview_names()
{
    if(!ui->tabWidget_main->count()) return;

    rv_idisasm *dv;
    int cnt = 1;


    for(int i=0; i<ui->tabWidget_main->count(); i++) {
        if(dv = qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(i))) {
            ui->tabWidget_main->setTabText(i, "Disassembly View "
                                           + QString::number(cnt++));
        }
    }
}

void rv_mainwin::setup_mainwin_tabs()
{
    tab_overview    = ui->tab_overview;
    tab_acss        = ui->tab_acss;
    tab_wildasm     = ui->tab_wildasm;
    tab_re_script   = ui->tab_rescript;
    tab_notes       = ui->tab_notes;
    tab_log         =ui->tab_log;

    close_main_tab(1);
    close_main_tab(1);
    close_main_tab(1);
    close_main_tab(1);
    close_main_tab(1);

    setup_re_script_tab();
}

void rv_mainwin::setup_re_script_tab()
{
    re_script_editor = new rv_script_code_editor(this);
    tab_re_script->layout()->addWidget(re_script_editor);
}

void rv_mainwin::show_overview_tab()
{
    int tab = ui->tabWidget_main->addTab(tab_overview,
                               QIcon(":/logos/rs_general_logo_big.png"),
                               "Overview");
    ui->actionOverview->setEnabled(false);
    ui->tabWidget_main->setCurrentIndex(tab);
}

void rv_mainwin::show_acss_tab()
{
    int tab = ui->tabWidget_main->addTab(tab_acss,
                               QIcon(":/icons/acss_1_small.png"),
                               "ACSS");
    ui->actionACSS->setEnabled(false);
    ui->tabWidget_main->setCurrentIndex(tab);
}

void rv_mainwin::show_wildasm_tab()
{
    int tab = ui->tabWidget_main->addTab(tab_wildasm,
                               QIcon(":/icons/wildasm2.png"),
                               "WildASM");
    ui->actionWILDASM->setEnabled(false);
    ui->tabWidget_main->setCurrentIndex(tab);
}

void rv_mainwin::show_re_script_tab()
{
    int tab = ui->tabWidget_main->addTab(tab_re_script,
                               QIcon(":/icons/x-sys_action.png"),
                               "re|script");
    ui->actionEditor->setEnabled(false);
    ui->tabWidget_main->setCurrentIndex(tab);
}

void rv_mainwin::show_notes_tab()
{
    int tab = ui->tabWidget_main->addTab(tab_notes,
                               QIcon(":/icons/notes_4.png"),
                               "Notes");
    ui->actionNotes->setEnabled(false);
    ui->tabWidget_main->setCurrentIndex(tab);
}

void rv_mainwin::show_log_tab()
{
    int tab = ui->tabWidget_main->addTab(tab_log,
                               QIcon(":/icons/notes_2.png"),
                               "Log");
    ui->actionLog->setEnabled(false);
    ui->tabWidget_main->setCurrentIndex(tab);
}

void rv_mainwin::create_overview()
{
    if(!t || !t->file) return;

    QVBoxLayout *vlayout = new QVBoxLayout();
    QString tmp;
    QLabel *ql;
    QTableView *tv;
    rv_hex_edit *hv;

    // file info
    QTableWidget *tw_fileinfo =
            new QTableWidget(4 + t->file->shared_libs.count(), 2);
    tw_fileinfo->setAlternatingRowColors(true);
    QTableWidgetItem *ti;
    QFont stdfont;
    QFont boldfont = QFont(stdfont.defaultFamily(), stdfont.pixelSize(), QFont::Bold);
    QFont bigboldfont = QFont(stdfont.defaultFamily(), 14, QFont::Bold);

    QLabel *ql_file_info = new QLabel("File Info");
    ql_file_info->setFont(bigboldfont);

    ti = new QTableWidgetItem("Filename");
    ti->setFont(boldfont);
    tw_fileinfo->setItem(0, 0, ti);
    ti = new QTableWidgetItem(t->file->file_name);
    tw_fileinfo->setItem(0, 1, ti);

    ti = new QTableWidgetItem("Size");
    ti->setFont(boldfont);
    tw_fileinfo->setItem(1, 0, ti);
    ti = new QTableWidgetItem(QString::number(t->file->filesize) + " Bytes");
    tw_fileinfo->setItem(1, 1, ti);

    ti = new QTableWidgetItem("Type");
    ti->setFont(boldfont);
    tw_fileinfo->setItem(2, 0, ti);
    if(t->file->file_type == RE_FILE_TYPE_ELF32) tmp = "ELF32";
    if(t->file->file_type == RE_FILE_TYPE_PE32)  tmp = "PE32";
    if(t->file->file_type == RE_FILE_TYPE_RAW32) tmp = "RAW32";
    ti = new QTableWidgetItem(tmp);
    tw_fileinfo->setItem(2, 1, ti);

    ti = new QTableWidgetItem("Shared libs: ");
    ti->setFont(boldfont);
    tw_fileinfo->setItem(3, 1, ti);

    for(int i=0; i < t->file->shared_libs.count(); i++) {
        //ti = new QTableWidgetItem("Shared lib");
        //tw_fileinfo->setItem(3 + i, 0, ti);
        ti = new QTableWidgetItem(t->file->shared_libs.at(i));
        tw_fileinfo->setItem(4 + i, 1, ti);
    }

    tw_fileinfo->horizontalHeader()->setHidden(true);
    tw_fileinfo->verticalHeader()->setHidden(true);
    tw_fileinfo->resizeColumnsToContents();
    tw_fileinfo->resizeRowsToContents();
    tw_fileinfo->setFixedHeight(120 + 18 * t->file->shared_libs.count());
    tw_fileinfo->horizontalHeader()->setStretchLastSection(true);
    tw_fileinfo->verticalHeader()->setStretchLastSection(true);
    tw_fileinfo->setFrameStyle(QFrame::NoFrame);

    vlayout->addWidget(ql_file_info);
    vlayout->addWidget(tw_fileinfo);
    vlayout->addSpacing(20);

    // ELF Hdrs
    if(f->file_type == RE_FILE_TYPE_ELF32) {
        // EHDR
        ql = new QLabel("ELF Header");
        ql->setFont(bigboldfont);

        tv = new QTableView();
        rv_elfhdr_model *ehm = new rv_elfhdr_model();
        ehm->setFile(t->file);
        tv->setModel(ehm);
        tv->resizeColumnsToContents();
        tv->resizeRowsToContents();
        tv->horizontalHeader()->hide();
        tv->verticalHeader()->hide();
        tv->setFixedHeight(335);
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->setFrameStyle(QFrame::NoFrame);
        tv->setSelectionBehavior(QAbstractItemView::SelectRows);
        tv->setSelectionMode(QAbstractItemView::SingleSelection);

        vlayout->addWidget(ql);
        vlayout->addWidget(tv);
        vlayout->addSpacing(20);

        // EHDR Hexdump
        ql = new QLabel("ELF Header Hexdump");
        ql->setFont(boldfont);
        hv = new rv_hex_edit();
        hv->set_target(t);
        hv->set_model(ehm);
        hv->set_clipping_region(0, sizeof(Elf32_Ehdr));
        //hv->setFixedHeight(120);
        struct rv_hv_color_range cr;

        cr.start = 0x00;
        cr.end   = 0x0f;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_ident";
        hv->add_color_range(cr);

        cr.start = 0x10;
        cr.end   = 0x11;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_type";
        hv->add_color_range(cr);

        cr.start = 0x12;
        cr.end   = 0x13;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_machine";
        hv->add_color_range(cr);

        cr.start = 0x14;
        cr.end   = 0x17;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_version";
        hv->add_color_range(cr);

        cr.start = 0x18;
        cr.end   = 0x1b;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_entry";
        hv->add_color_range(cr);

        cr.start = 0x1c;
        cr.end   = 0x1f;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_phoff";
        hv->add_color_range(cr);

        cr.start = 0x20;
        cr.end   = 0x23;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_shoff";
        hv->add_color_range(cr);

        cr.start = 0x24;
        cr.end   = 0x27;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_flags";
        hv->add_color_range(cr);

        cr.start = 0x28;
        cr.end   = 0x29;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_ehsize";
        hv->add_color_range(cr);

        cr.start = 0x2a;
        cr.end   = 0x2b;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_phentsize";
        hv->add_color_range(cr);

        cr.start = 0x2c;
        cr.end   = 0x2d;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_phnum";
        hv->add_color_range(cr);

        cr.start = 0x2e;
        cr.end   = 0x2f;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_shentsize";
        hv->add_color_range(cr);

        cr.start = 0x30;
        cr.end   = 0x31;
        cr.bgcolor = QColor(0x70, 0x40, 0x40);
        cr.txtcolor = Qt::white;
        cr.desc = "e_shnum";
        hv->add_color_range(cr);

        cr.start = 0x32;
        cr.end   = 0x33;
        cr.bgcolor = QColor(0x40, 0x60, 0x70);
        cr.txtcolor = Qt::white;
        cr.desc = "e_shstrndx";
        hv->add_color_range(cr);

        hv->single_range = 0;
        hv->setFixedHeight(150);


        connect(tv, SIGNAL(clicked(QModelIndex)),
                hv, SLOT(set_single_range(QModelIndex)));

        connect(hv, SIGNAL(data_changed(QModelIndex,QModelIndex)),
                tv, SLOT(dataChanged(QModelIndex,QModelIndex)));

        vlayout->addWidget(ql);
        vlayout->addWidget(hv);
        vlayout->addSpacing(20);



        // PHDR
        ql = new QLabel("ELF Program Headers (Segments)");
        ql->setFont(bigboldfont);

        tv = new QTableView();
        rv_elf_phdr_model *phm = new rv_elf_phdr_model();
        phm->setFile(t->file);
        tv->setModel(phm);
        tv->resizeColumnsToContents();
        tv->resizeRowsToContents();
        tv->verticalHeader()->hide();
        tv->setFixedHeight(250);
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->setFrameStyle(QFrame::NoFrame);

        vlayout->addWidget(ql);
        vlayout->addWidget(tv);
        vlayout->addSpacing(20);

        // PHDR Hexdump
        ql = new QLabel("ELF Program Headers Hexdump");
        ql->setFont(boldfont);
        hv = new rv_hex_edit();
        hv->set_target(t);
        hv->set_clipping_region(t->file->ELF_Hdr->e_phoff,
                              t->file->ELF_Hdr->e_phnum *
                              t->file->ELF_Hdr->e_phentsize);
        hv->setFixedHeight(250);

        for(int i=0; i<t->file->segments.count(); i++) {
            cr.start = i * t->file->ELF_Hdr->e_phentsize;
            cr.end = cr.start + t->file->ELF_Hdr->e_phentsize -1;
            if(i%2) cr.bgcolor = QColor(0x40, 0x48, 0x60);
            else cr.bgcolor = QColor(0x60, 0x68, 0x80);
            cr.txtcolor = Qt::white;
            cr.desc = "program header " + QString::number(i);
            hv->add_color_range(cr);
        }
        hv->single_range = 0;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                hv, SLOT(set_single_range(QModelIndex)));

        vlayout->addWidget(ql);
        vlayout->addWidget(hv);
        vlayout->addSpacing(20);

        // SHDR
        ql = new QLabel("ELF Section Headers");
        ql->setFont(bigboldfont);

        tv = new QTableView();
        rv_elf_shdr_model *shm = new rv_elf_shdr_model();
        shm->setFile(t->file);
        tv->setModel(shm);
        tv->resizeColumnsToContents();
        tv->resizeRowsToContents();
        tv->verticalHeader()->hide();
        tv->setFixedHeight(250);
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->setFrameStyle(QFrame::NoFrame);

        vlayout->addWidget(ql);
        vlayout->addWidget(tv);
        vlayout->addSpacing(20);

        // SHDR Hexdump
        ql = new QLabel("ELF Section Headers Hexdump");
        ql->setFont(boldfont);
        hv = new rv_hex_edit();
        hv->set_target(t);
        hv->set_clipping_region(t->file->ELF_Hdr->e_shoff,
                              t->file->ELF_Hdr->e_shnum *
                              t->file->ELF_Hdr->e_shentsize);
        hv->setFixedHeight(250);

        for(int i=0; i<t->file->ELF_Hdr->e_shnum; i++) {
            cr.start = i * t->file->ELF_Hdr->e_shentsize;
            cr.end = cr.start + t->file->ELF_Hdr->e_shentsize -1;
            if(i%2) cr.bgcolor = QColor(0x40, 0x48, 0x60);
            else cr.bgcolor = QColor(0x60, 0x68, 0x80);
            cr.txtcolor = Qt::white;
            cr.desc = "section header " + QString::number(i);
            hv->add_color_range(cr);
        }


        hv->single_range = 0;

        connect(tv, SIGNAL(clicked(QModelIndex)),
                hv, SLOT(set_single_range(QModelIndex)));

        vlayout->addWidget(ql);
        vlayout->addWidget(hv);
        vlayout->addSpacing(20);

    }
    ui->scrollAreaWidgetContents->setLayout(vlayout);
    ui->scrollArea_overview->setHidden(false);
    ui->tabWidget_main->setCurrentIndex(0);
}

void rv_mainwin::run_full_analysis()
{
    rv_idisasm *dv;
    if(dv = qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(
                                           ui->tabWidget_main->currentIndex()))) {
        dv->pre_disassembly_changes();
    }


    ui->statusBar->showMessage("Running full analysis");
    t->address_space->clear();

    rv_disasm_thread *dt = new rv_disasm_thread();
    dt->set_target(t);
    connect(dt, SIGNAL(finished()), this, SLOT(disasm_thread_finished()));

    progress_dialog = new rv_progress_dialog();
    progress_dialog->set_main_task_msg("Disassembling");
    progress_dialog->set_pct(0);
    progress_dialog->show();
    connect(t->disassembler, SIGNAL(progress_changed_pct(int)),
            progress_dialog, SLOT(set_pct(int)));
    connect(t->disassembler, SIGNAL(msg(int,QString)),
            progress_dialog, SLOT(set_progress_msg(int, QString)));
    //connect(t->disassembler, SIGNAL(progress_changed_pct(int)),
    //        this, SLOT(disasm_progress_status(int)));
    //connect(t->disassembler, SIGNAL(msg(int,QString)), this,
    //        SLOT(status_msg(int,QString)));

    dt->start();
}

void rv_mainwin::disasm_thread_finished()
{
    ui->statusBar->showMessage("Disassembly completed.");
    progress_dialog->close();
    populate_functions();
    rv_idisasm *dv;
    if(dv = qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(ui->tabWidget_main->currentIndex()))) {
        //dv->post_disassembly_change();
        dv->goto_va(t->file->va_entry_point);
    }
}

void rv_mainwin::disasm_progress_status(int pct)
{
    ui->statusBar->showMessage("Disassembling: "
                               + QString::number(pct)
                               + "%"
                               );
}

void rv_mainwin::status_msg(int level, QString msg) {
    if(level == RE_LOG_NORMAL) {
        ui->statusBar->showMessage(msg);
    }
}

void rv_mainwin::set_fullscreen(bool yesno)
{
    if(yesno) showFullScreen();
    else showNormal();
}

void rv_mainwin::show_about_dialog()
{
    if(about_dialog) about_dialog->show();
    else {
        about_dialog = new rv_about_dialog();
        about_dialog->show();
    }
}

void rv_mainwin::detach_to_new_win_clicked()
{
    rv_hex_editor *hv;
    rv_idisasm *dv;

    int index = ui->tabWidget_main->currentIndex();

    if(hv = qobject_cast<rv_hex_editor *>(ui->tabWidget_main->widget(index))) {
        ui->tabWidget_main->removeTab(index);
        hv->setParent(0);
        hv->move(this->x() + 50, this->y() + 50);
        hv->set_detached(true);
        hv->resize(900, 700);

        connect(hv, SIGNAL(re_attach_me(rv_hex_editor*)),
                this, SLOT(attach_hexview(rv_hex_editor*)));

        hv->show();
        rewrite_hexview_names();
        return;
    }

    if(dv = qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(index))) {
        ui->tabWidget_main->removeTab(index);
        dv->setParent(0);
        dv->move(this->x() + 50, this->y() + 50);
        dv->set_detached(true);
        dv->resize(900, 700);

        connect(dv, SIGNAL(re_attach_me(rv_idisasm*)),
                this, SLOT(attach_disasmview(rv_idisasm*)));

        dv->show();
        rewrite_disasmview_names();
        return;
    }
}

void rv_mainwin::attach_hexview(rv_hex_editor *he)
{
    int tab;

    rv_hex_editor *view = he;

    tab = ui->tabWidget_main->addTab(view, QIcon(":/icons/hex_buffer_t.png"),
                                     "Hexview " +
                                     QString::number(hex_views.count() + 1));
    ui->tabWidget_main->setCurrentIndex(tab);

    hex_views << view;

    view->set_detached(false);
    rewrite_hexview_names();
}

void rv_mainwin::attach_disasmview(rv_idisasm *dv)
{
    int tab;

    rv_idisasm *view = dv;

    tab = ui->tabWidget_main->addTab(view, QIcon(":/icons/disasm_view_t.png"),
                                     "Disassembly View " +
                                     QString::number(disasm_views.count() + 1));
    ui->tabWidget_main->setCurrentIndex(tab);

    disasm_views << view;

    view->set_detached(false);
    rewrite_disasmview_names();
}

void rv_mainwin::tab_idx_changed(int idx)
{
    if(qobject_cast<rv_hex_editor *>(ui->tabWidget_main->widget(idx))) {
        pushButton_detach->setEnabled(true);
    } else if(qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(idx))) {
        pushButton_detach->setEnabled(true);
    }
    else pushButton_detach->setEnabled(false);
}


void rv_mainwin::symbolic_tab_clicked(int r, int c)
{
    QTableWidgetItem *ti;
    re_addr_t va;

    ti = ((QTableWidget *)sender())->item(r, 0);

    va = ti->text().toUInt(0, 16);

    emit global_goto_va(va);
}

void rv_mainwin::va_receiver(re_addr_t va)
{
    rv_hex_editor *hv;
    rv_idisasm *tmp_dview;

    if(hv = qobject_cast<rv_hex_editor *>(ui->tabWidget_main->widget(
                                          ui->tabWidget_main->currentIndex()))) {
        hv->set_cursor_to_va(va);
    }
    else if(
        (tmp_dview = qobject_cast<rv_idisasm *>(ui->tabWidget_main->widget(
                                                ui->tabWidget_main->currentIndex())))) {
        tmp_dview->goto_va(va);
    }
}
