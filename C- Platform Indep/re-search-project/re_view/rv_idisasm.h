#ifndef RV_IDISASM_H
#define RV_IDISASM_H

#include <QtGui>
#include <QWidget>
#include <QFile>
#include <QRegExp>
#include <QInputDialog>
#include <re_core.h>
#include <rv_splitter.h>
#include <rv_script_processor.h>
#include <rv_disasm_view.h>
#include <rv_console_edit.h>
#include <rv_tag_processor.h>


class rv_idisasm;

struct rv_idisasm_cmd_function {
    QString name;
    QString descr;
    QString help;
    bool (rv_idisasm::*f)(QString, QStringList);
};

struct rv_idisasm_subview {
    rv_disasm_view   *dv;
    re_address_space *address_space;
};

class rv_idisasm : public QWidget
{
    Q_OBJECT
public:
    explicit rv_idisasm(QTabWidget *attach_to = 0, QWidget *parent = 0);

signals:
    void re_attach_me(rv_idisasm *me);

public slots:
    void set_target(re_target *target);
    void set_detached(bool b);
    void re_attach_clicked();
    void goto_va(re_addr_t va);
    void process_console_cmd(QString cmd);
    void setup_toolbar();

    void stack_go_forward();
    void stack_go_back();
    void stack_push(re_addr_t);
    re_addr_t stack_pop();
    void stack_repaint();
    void stack_clear();
    void stack_goto_list_idx(int idx);

    void tabwidget_va_dblclicked(int r, int c);

    void update_last_pos(re_addr_t va);

    void setup_infopane();

    void show_cursor_info(re_addr_t va);

    void disassembly_changed();
    void pre_disassembly_changes();
    void post_disassembly_change();

    // --- CONSOLE -----
    void setup_console(rv_console_edit *c);

    bool idisasm_cmd_help(QString line, QStringList args);
    bool idisasm_cmd_clear(QString line, QStringList args);
    bool idisasm_cmd_history(QString line, QStringList args);

    bool idisasm_cmd_d(QString line, QStringList args);
    bool idisasm_cmd_b(QString line, QStringList args);
    bool idisasm_cmd_tag(QString line, QStringList args);

    bool idisasm_cmd_subview(QString line, QStringList args);

    void biggify_console(bool yesno);

    bool get_va_or_sym_va(re_addr_t &va, QString arg);

    // --- SUB VIEWS ---
    rv_disasm_view *new_filter_view(QString name="new");
    rv_disasm_view *new_target_view(QString name="new");

    void change_orientation();
    void close_sub_view(int idx);
    void view_changed_disasm_addr_space();
    void set_active_dv(rv_disasm_view *view);
    void create_new_filter_view();
    void create_new_target_view();

    rv_disasm_view *get_visible_filter_view();
    void disasm_ctf_into_filter_view(re_addr_t va);
    void disasm_linear_into_filter_view(re_addr_t va);

    void paste_into_new_filter();
    void paste_ctf_into_new_filter();

private:
    re_target       *t;
    rv_disasm_view  *dv;
    re_addr_t        d_va;
    QList<struct rv_idisasm_subview> subviews;
    rv_disasm_view  *active_dv;

    // console stuff
    rv_console_edit *console;
    QHash<QString, struct rv_idisasm_cmd_function> console_cmd_functions;

    QVBoxLayout     *main_layout;

    rv_splitter     *splitter_main;   // |
    rv_splitter     *splitter_disasm; // - between disassembly and sub views

    QPushButton     *pushButton_re_attach;
    QTabWidget      *tabwidget_attach_to;
    bool            detached;

    QToolBar        *toolbar_idisasm;

    QGroupBox       *tool_groupbox;
    QToolButton     *toolbutton_edit;

    rv_script_processor *script_processor;

    // infopane
    QWidget         *infopane;
    QList<QGroupBox *> infopane_widgets;

    // cursor info
    QLabel          *label_cursor_info;

    // navi stack
    QStack<re_addr_t> navi_stack;
    QPushButton     *pushbutton_stack_f;
    QPushButton     *pushbutton_stack_b;
    QListWidget     *listbox_stack;
    int             navi_index;
    QPushButton     *pushbutton_stack_clear;
    re_addr_t       last_pos_va;

    // tags
    rv_tag_processor *tag_processor;
    QTableWidget    *TW_yel;
    QTableWidget    *TW_blu;
    QTableWidget    *TW_red;
    QTableWidget    *TW_grn;
    QTableWidget    *TW_blk;
    QTableWidget    *TW_ANY;

    QTabWidget      *tabwidget_tags;

    // sub-/Clone-Views Stuff
    QTabWidget      *tabwidget_sub_views;
    rv_splitter     *splitter_disasm_console;
    QPushButton     *pushbutton_subviews_chg_orientation;

    QPushButton     *pushbutton_new_filter_view;
    QPushButton     *pushbutton_new_target_view;

    // copy / paste
    struct rv_idisasm_selection selection;
};

#endif // RV_IDISASM_H
