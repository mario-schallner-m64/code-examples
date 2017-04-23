#ifndef RV_HEX_EDITOR_H
#define RV_HEX_EDITOR_H

#include <QtGui>
#include <QWidget>
#include <QFile>
#include <rv_hex_edit.h>
#include <rv_hex_editor_infopane.h>
#include <rv_hex_editor_bufferpane.h>
#include <re_core.h>
#include <rv_splitter.h>
#include <rv_script_processor.h>


class rv_hex_editor : public QWidget
{
    Q_OBJECT
public:
    explicit rv_hex_editor(QTabWidget *attach_to = 0, QWidget *parent = 0);

signals:
    void re_attach_me(rv_hex_editor *me);
    void offset_changed_forward(rv_hex_edit *hv, re_addr_t offset);
    void selection_changed_forward(rv_hex_edit *hv, re_addr_t from, re_addr_t to);

public slots:
    void set_target(re_target *target);
    void display_cursor_info(re_addr_t offs);
    void display_selection_info(re_addr_t from, re_addr_t to);
    void clear_selection_info();

    void buffers_box_toggled(bool checked);

    bool new_buffer_from_main_selection();
    void new_target_buffer();

    void buffer_rename(QString name);
    void new_buffer_from_file();

    void close_buffer_tab(int idx);

    void paste_buffer_sel_up();
    void paste_whole_buffer_up();
    void new_00_buffer();

    void re_attach_clicked();

    void set_detached(bool b);

    void stack_go_forward();
    void stack_go_back();
    void stack_push(re_addr_t);
    re_addr_t stack_pop();
    void stack_repaint();
    void stack_clear();
    void stack_goto_list_idx(int idx);

    rv_hex_edit *get_buffer_by_name(QString name);

    void evaluator_run_in_thread();
    void evaluator_run();
    void evaluator_thread_finished(QScriptValue result);
    void evaluator_abort();

    void new_custom_widget_cursor();
    void new_custom_widget_selection();
    void new_custom_widget_button();

    void new_cusom_widget(int mode);

    void do_offset_changed_forward(re_addr_t offset);
    void do_selection_changed_forward(re_addr_t from, re_addr_t to);

    void change_orientation();

    void set_cursor_to_va(re_addr_t va);

public:
    void setup_toolbar();

private:
    void setup_info_layout(QVBoxLayout *vl);

    void add_buffer_to_tabs(rv_hex_edit *hv_buffer);

private:
    re_target   *t;
    rv_hex_edit *hv;
    QList<rv_hex_edit *> buffers;

    QVBoxLayout *main_layout;
    rv_hex_editor_infopane *infopane;
    rv_hex_editor_bufferpane *bufferpane;

    rv_splitter   *splitter_hexview;
    rv_splitter   *splitter_buffers;

    QTabWidget    *tabwidget_buffers;
    QPushButton   *pushbutton_buffers_chg_orientation;

    QPushButton   *pushbutton_re_attach;
    QTabWidget    *tabwidget_attach_to;
    bool          detached;

    QToolBar      *toolbar_hex;

    QGroupBox     *tool_groupbox;
    QToolButton   *toolbutton_edit;

    QStack<re_addr_t> navi_stack;
    QPushButton    *pushbutton_stack_f;
    QPushButton    *pushbutton_stack_b;
    QListWidget    *listbox_stack;
    int             navi_index;

    rv_script_processor *script_processor;
    QPlainTextEdit *evaluator_input;
    QPlainTextEdit *evaluator_output;
};

#endif // RV_HEX_EDITOR_H
