#ifndef RV_HEX_EDITOR_INFOPANE_H
#define RV_HEX_EDITOR_INFOPANE_H

#include <QWidget>
#include <QtGui>
#include <rv_sideinfo_listbox.h>
#include <rv_sideinfo_custom_widget.h>
#include <rv_script_highlighter.h>

namespace Ui {
    class rv_hex_editor_infopane;
}



class rv_hex_editor_infopane : public QWidget
{
    Q_OBJECT

public:
    explicit rv_hex_editor_infopane(QWidget *parent = 0);
    ~rv_hex_editor_infopane();
    void setup_side_infos();
    QListWidget *get_navi_stack_listbox();
    QPlainTextEdit *get_evaluator_input_box();
    QPlainTextEdit *get_evaluator_output_box();

    QPushButton *get_evaluator_run_button();
    QPushButton *get_evaluator_abort_button();
    QList<rv_sideinfo_custom_widget *> custom_widgets;


signals:
    void buffers_toggled(bool);
    void copy_selection_into_new_buffer();
    void buffer_rename(QString);
    void new_target_buffer();
    void load_file_into_new_buffer_clicked();
    void new_00_buffer_clicked();

    void clear_navi_stack_clicked();
    void navi_stack_goto(int pos);

    void evaluator_run_clicked();
    void evaluator_abort_clicked();

    void action_new_side_widget_cursor();
    void action_new_side_widget_selection();
    void action_new_side_widget_button();

public slots:
    void set_cursor_offset_hex(QString str);
    void set_cursor_offset_dec(QString str);
    void set_cursor_offset_va(QString str);
    void set_cursor_byte(QString str);
    void set_cursor_word(QString str);
    void set_cursor_dword(QString str);

    void set_selection_info(QString start_hex,
                            QString start_dec,
                            QString start_va,
                            QString end_hex,
                            QString end_dec,
                            QString end_va,
                            QString sz);

    // connected
    void show_hide_info_item(QListWidgetItem *li);

    void redo_layout(bool b);
    void redo_layout();

    void add_custom_widget(QString name, rv_sideinfo_custom_widget *widget);
    void widget_height_changed(rv_sideinfo_custom_widget *w,  int h);

private:
    Ui::rv_hex_editor_infopane *ui;

    QHash<QString, QWidget*> info_items;
    QHash<QString, int>      info_items_widths;
    QHash <QString, int>     info_items_heights;
    rv_sideinfo_listbox *listWidget_side_items;

    QHBoxLayout* main_layout;
    QVBoxLayout* main_v_layout;
    QToolBar *toolbar_sideitems;
    QToolButton *toolbutton_new_sidewidgets;

    rv_script_highlighter *highlighter;
    QFont   font_title;
    QFont   font_monospace;
    QFont   font_standard;
};

#endif // RV_HEX_EDITOR_INFOPANE_H
