#ifndef RV_DISASM_VIEW_H
#define RV_DISASM_VIEW_H

#include <QWidget>

#include <QtGui>
#include <QAbstractScrollArea>
#include <QPainter>
#include <rv_hex_edit.h>
#include <rv_disasm_xrefs_widget.h>
#include <QStack>
#include <rv_tooltip_widget.h>
#include <rv_tooltip_xrefs.h>
#include <rv_tag_processor.h>
#include <rv_disasm_view_options_widget.h>
#include <rv_tag_cell_editor.h>
#include <re_core.h>
#include <html_processor.h>

#define RV_DV_LMARGIN 5


enum RV_DV_MODES {
    DV_MODE_TARGET,
    DV_MODE_FILTER
};

struct rv_disasm_line {
    int     nr;
    int     y;
    int     height;
    bool    had_line;
    bool    prev_had_line;
    int     op_len;
};

class rv_disasm_view;

struct rv_idisasm_selection {
    re_addr_desc *start;
    re_addr_desc *end;
    rv_disasm_view *dv;
    bool        active;
};

class rv_disasm_view : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit rv_disasm_view(QTabWidget *attach_to = 0, QWidget *parent = 0);
    void set_navi_stack(QStack<re_addr_t> *s);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool event(QEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);

signals:
    void first_paint();
    void repaintme();
    void va_clicked(re_addr_t va);
    void cursor_changed_to(re_addr_t va);
    void jmped_from(re_addr_t va);
    void disassembly_finished();
    void received_focus();

    void do_ctf_disasm_filter_view(re_addr_t va);
    void do_linear_disasm_filter_view(re_addr_t va);

    void paste_to_new_filter_requested();
    void paste_ctf_to_new_filter_requested();

public slots:
    void set_target(re_target *target);
    void set_address_space(re_address_space *s);
    re_address_space *get_address_space();
    void set_name(QString s);
    QString get_name();
    void recalc_geometry();

    void scrollbar_value_changed(int new_v);
    bool get_scrollbar_pos(re_addr_t va, int &val);

    void pre_disassembly_change();
    void post_disassembly_change();
    void disassembly_changed();
    void do_repaint();
    void first_paint_updates();

    void scroll_to(re_addr_t va);
    void scroll_n_cursor_to(re_addr_t va);
    void cursor_up();
    void cursor_down();
    re_addr_desc *get_mouse_desc();
    re_addr_desc *get_cursor_desc();

    void set_tag_processor(rv_tag_processor *tp);
    void disassemble_ctf_from_va(re_addr_t va);
    void disassemble_linear_from_va(re_addr_t va);

    void set_mode(int m);
    int get_mode();
    void set_have_idisasm(bool b);

    void set_show_offsets_(bool b);
    void set_show_bytes_cnt_(int i);
    void show_hide_options();
    void set_show_offsets(bool b);
    void set_show_bytes_cnt(int i);

    void tag_editor_finished();

    // CTX menu
    void rmb_clicked();
    void setup_menu_actions();

    void ctf_disasm_here_filter();
    void ctf_disasm_dest_filter();

    void linear_disasm_here_filter();
    void linear_disasm_dest_filter();

    void set_idisasm_selection(struct rv_idisasm_selection *sel);
    bool get_selection();

    void menu_copy();

    void menu_paste();
    void menu_paste_ctf();

    void menu_delete();
    void menu_select_all();
    void menu_clear_selection();

    // dump
    void menu_dump_sel_to_html();
    void menu_dump_view_to_html();

    void dump_range_to_html(re_addr_desc *start, re_addr_desc *end);

    QString html_header();
    QString hex_dump(re_addr_t offset, int len);
    void translatePrintableChars(unsigned char *dataptr, int length, QString &S);



private:
    re_target       *t;
    re_address_space *my_address_space;
    unsigned        char *buffer_data;
    int             mode;

    bool            show_offsets;
    int             show_hex_bytes_cnt;

    bool            have_idisasm;

    void            create_options_widget();
    rv_disasm_view_options_widget *options_widget;

    rv_tag_cell_editor *tag_editor;
    re_addr_desc    *tag_editor_desc;

    QString         name;

    int             num_lines_visible;
    int             line_height;
    int             char_width;

    QTabWidget      *tabwidget_attach_to;

    re_addr_desc    *top_desc;
    re_addr_desc    *lastline;
    re_addr_desc    *cursor_desc;

    int             scrollbar_current_val;
    QHash<re_addr_t, rv_hex_edit *> hex_edits;
    QHash<re_addr_t, bool> hex_edits_status;
    QList<rv_hex_edit *> hex_edits_current_to_show;
    QHash<re_addr_desc *, rv_disasm_xrefs_widget*> xrefs_widgets;

    QFont           my_font, my_font_bold;
    QFont           my_txt_font;
    QFont           my_txt_font_bold;
    QColor          bgcolor, bgcolor1, bgcolor2, bgcolor_hex;
    QColor          bgcolor_mouse;
    QColor          bgcolor_cursorline, bgcolor_cursorline2;
    QColor          bgcolor_selection, bgcolor_selection_cursor;
    QColor          txt_color_normal;
    QColor          txt_color_offset;
    QColor          txt_color_va;
    QColor          txt_color_bytes;

    QColor          txt_color_imports;
    QColor          txt_color_symbols;
    QColor          txt_color_names;
    QColor          txt_color_strings;
    QColor          txt_color_functions;

    QColor          dis_color_line_dashed;
    QColor          dis_color_line_solid;
    QColor          dis_color_string;
    QColor          dis_bgcolor_string;

    QColor          dis_color_insn;
    QColor          dis_color_insn_call;
    QColor          dis_color_insn_jmp_ret;
    QColor          dis_color_insn_jcc;
    QColor          dis_color_insn_cmp;
    QColor          dis_color_insn_interrupt;
    QColor          dis_color_insn_stack;
    QColor          dis_color_insn_nop;


    QPixmap         pm_data_marker, pm_data_marker2;
    QPixmap         pm_follow_controlflow, pm_ret_address;
    QPixmap         pm_hex;
    QPixmap         pm_imports, pm_symbols, pm_functions, pm_strings, pm_names;
    QPixmap         pm_comments, pm_xr1, pm_xr2;
    QPixmap         pm_tag_y, pm_tag_r, pm_tag_g, pm_tag_b, pm_tag_B;

    QIcon           ico_show_options, ico_hide_options;
    QPushButton     *pushbutton_show_hide_options;

    int             col_offset;
    int             col_extras;
    int             col_va;
    int             col_bytes;
    int             col_mnemonic;
    int             col_operands;
    int             col_comments;


    bool            am_painting;
    bool            unpainted;
    QList<struct rv_disasm_line> disasm_lines;

    QStack<re_addr_t> *navi_stack;

    // pre disassembly change stuff
    bool            have_saved_vas;
    re_addr_t       saved_va;
    re_addr_t       saved_va_top;
    bool            freeze;

    rv_tooltip_widget *tooltip_widget;
    QPoint          mouse_pos;
    QPoint          mouse_global_pos;
    bool            mouse_active;
    re_addr_desc    *mouse_desc;
    struct rv_disasm_line mouse_line;
    bool            repaint_mouse;
    bool            mouse_clear;
    bool            mouse_have_dest;
    re_addr_t       mouse_va_dest;
    re_addr_t       mouse_va_here;

    int             max_op_len;

    rv_tooltip_xrefs *tooltip_label_xrefs;
    rv_tag_processor *tag_processor;

    // Menu actions
    QAction         *menu_action_copy;

    QAction         *menu_action_ctf_here_to_filter;
    QAction         *menu_action_ctf_dest_to_filter;
    QAction         *menu_action_linear_here_to_filter;
    QAction         *menu_action_linear_dest_to_filter;

    QAction         *menu_action_paste;
    QAction         *menu_action_ctf_paste;

    QAction         *menu_action_paste_to_new_filter;
    QAction         *menu_action_ctf_paste_to_new_filter;

    QAction         *menu_action_show_hide_offsets;

    QAction         *menu_action_delete;
    QAction         *menu_action_select_all;
    QAction         *menu_action_clear_selection;

    QAction         *menu_action_dump_sel_to_html;
    QAction         *menu_action_dump_view_to_html;

    QHash<QAction *, QString> menu_action_strings;

    // Selection
    bool            have_selection;
    re_addr_desc    *selection_start;
    re_addr_desc    *selection_end;

    struct rv_idisasm_selection *selection;
};


#endif // RV_DISASM_VIEW_H
