#ifndef RV_MAINWIN_H
#define RV_MAINWIN_H

#include <QMainWindow>

#include <QtGui>

#include <re_core.h>
#include <rv_global.h>
#include <rv_hex_edit.h>
#include <rv_hex_editor.h>
#include <rv_idisasm.h>
#include <rv_progress_dialog.h>

// threading
#include <rv_disasm_thread.h>
#include <rv_file_analyze_thread.h>

// ELF specific
#include <rv_elfhdr_model.h>
#include <rv_elf_phdr_model.h>
#include <rv_elf_shdr_model.h>

// dialogs
#include <rv_about_dialog.h>

// tab specific
#include <rv_script_code_editor.h>
#include <rv_script_highlighter.h>


namespace Ui {
    class rv_mainwin;
}

class rv_mainwin : public QMainWindow
{
    Q_OBJECT

public:
    explicit rv_mainwin(QWidget *parent = 0);
    ~rv_mainwin();

signals:
    void global_goto_va(re_addr_t);

public slots:
    bool open_file();
    void file_opened();
    void log_msg(int level, const QString &msg);
    void exit_review();

    void populate_file_info();
    void populate_imports();
    void display_imports();
    void populate_symbols();
    void display_symbols();
    void populate_functions();
    void display_functions();
    void populate_strings();
    void display_strings();

    void find_import();
    void find_next_import();
    void find_function();
    void find_next_function();
    void find_symbol();
    void find_next_symbol();
    void find_string();
    void find_next_string();

    // new windows stuff
    void new_hexview_window();

    // tabs stuff

    void new_hexview(bool show_after_create = true);
    void detach_to_new_win_clicked();
    void attach_hexview(rv_hex_editor *hv);

    void new_disasm_view(bool show_after_create = true);
    void attach_disasmview(rv_idisasm *dv);

    void tab_idx_changed(int idx);


    void tab_close_request(int);
    void close_hexview(int);
    void close_main_tab(int);
    void rewrite_hexview_names();
    void rewrite_disasmview_names();

    void show_overview_tab();
    void show_acss_tab();
    void show_wildasm_tab();
    void show_re_script_tab();
    void show_notes_tab();
    void show_log_tab();

    // menu actions
    void set_fullscreen(bool yesno);
    void show_about_dialog();

    // disassembling
    void run_full_analysis();
    void disasm_thread_finished();
    void disasm_progress_status(int);
    void status_msg(int, QString);

    // navigation handling
    void va_receiver(re_addr_t va);
    void symbolic_tab_clicked(int r, int c);


private:

    void setup_mainwin_tabs();
    void setup_re_script_tab();
    void create_overview();

    Ui::rv_mainwin *ui;
    QPushButton *pushButton_detach;


    re_file     *f;
    re_logger   *l;
    re_target   *t;

    int         found_pos_import;
    int         found_pos_function;
    int         found_pos_string;
    int         found_pos_symbol;

    QList<rv_hex_editor *> hex_views;
    QList<rv_idisasm *> disasm_views;
    QWidget *tab_overview;
    QWidget *tab_acss;
    QWidget *tab_wildasm;
    QWidget *tab_re_script;
    QWidget *tab_notes;
    QWidget *tab_log;

    rv_progress_dialog *progress_dialog;

    rv_about_dialog *about_dialog;

    rv_script_code_editor *re_script_editor;
};

#endif // RV_MAINWIN_H
