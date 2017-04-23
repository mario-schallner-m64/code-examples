#ifndef RV_HEX_EDIT_H
#define RV_HEX_EDIT_H

#include <QtGui>
#include <QAbstractScrollArea>
#include <QPainter>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QUndoStack>
#include <QAction>
#include <QMenu>

#include <re_core.h>

#define MARGIN_LEFT 3
#define MARGIN_TOP  3

struct rv_hv_color_range {
    re_addr_t   start;
    re_addr_t   end;
    QColor      bgcolor;
    QColor      txtcolor;
    QString     desc;
};

struct rv_hv_cellgeometry {
    int width;
    int height;
    int txt_width;
    int txt_height;
    int bg_width;
    int bg_height;
};

struct rv_hv_cursor {
    re_addr_t   offset;
    int         row;
    int         col;
    int         x;
    int         y;
    bool        on_screen;
    int         screen_row;
};

//class rv_hex_edit;

struct rv_hv_selection {
    re_addr_t   start;
    re_addr_t   end;
    re_addr_t   len;  // for convenience

    class rv_hex_edit *hv;
};

class rv_hex_edit : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit rv_hex_edit(QTabWidget *attach_to = 0, QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void contextMenuEvent(QContextMenuEvent *event);

signals:
    void cursor_offset_changed_to(re_addr_t offs);
    void selection_changed(re_addr_t from, re_addr_t to);
    void selection_cleared();

    // emit data_changed modelindex(0,0), modelindex(rowcount, clolcount)
    // if model is set - telling a view to update whole contents
    void data_changed(QModelIndex, QModelIndex);

public slots:
    // ---- SETTER SLOTS ----
    // ( use either set_target
    //   or set_data_buffer
    //   to be able to view content ! )

    void set_target(re_target *target);
    void set_data_buffer(unsigned char *buf, re_addr_t len);
    void set_isfile_flag();

    void set_buffer_name(QString name);
    void set_buffer_ifilename(QString name);
    void set_buffer_ofilename(QString name);

    void set_clipping_region(re_addr_t offs, re_addr_t len, bool use_va = false);
    void set_model(QAbstractItemModel *m);

    void set_can_paste(bool yesno);

    // ---- ACTION RECEIVCERS ---
    // they push to the undostack
    void ar_copy_selection();
    void ar_paste();
    void ar_fill_selection();

    void recalc_scrollbars();
    int get_total_height();
    int get_total_width();
    void handle_click(int x, int y);

    // cursors
    // set cursor_offset, x, y, row from coodrinates: ie mouseevent
    void set_cursor_data(struct rv_hv_cursor *c, int x, int y);
    // set cursor x, y, on_scren, ... from offset
    void set_cursor_data(struct rv_hv_cursor *c, re_addr_t offs);

    void editor_enter_pressed();
    void set_cellgeometry(int w, int h, int bw, int bh, int tw, int th);
    void init_std_geometry();

    void paint_cell(QPainter *painter, int x, int y);
    void paint_infoline(QPainter *painter);
    void paint_range_desc(QPainter *painter, int i);
    void paint_grid(QPainter *painter);
    void paint_hdr_col(QPainter *painter, QRect boundr);
    void paint_asc_col(QPainter *painter, int y);

    void repaint_cursor(struct rv_hv_cursor *old_pos,
                        struct rv_hv_cursor *new_pos);
    void set_shadow_cursor(int x, int y);
    void set_single_range(QModelIndex i);

    void handle_selection();

    bool paste_foreign_selection(struct rv_hv_selection p_sel);

    void do_repaint();

    void set_shadow_cursor(re_addr_t offs);
    void set_cursor_to_offs(re_addr_t offs);
    void set_cursor_to_va(re_addr_t va);



public:
    // --- GETTER Functions ---
    bool            get_selection(struct rv_hv_selection &sel);

    unsigned char   *get_data_ptr();
    re_addr_t       get_data_len();
    re_addr_t       get_current_offset();  // this is the clipping offset! use cursor_offset!!
    re_addr_t       get_cursor_offset();


    QString         get_name();

    bool            get_can_paste();

    // --- SETTER Functions ---    // color range stuff
    void add_color_range(re_addr_t   start,
                         re_addr_t   end,
                         QColor      bgcolor,
                         QColor      txtcolor,
                         QString     desc);
    void add_color_range(struct rv_hv_color_range cr);

    void set_single_range(int i);
    void set_selection(struct rv_hv_selection sel);



public:
    // wuuuhuuu w/o getter/setter functions!!!
    // and uninitialized!!!!!!!!!!!!
    // only used by undo commands!!!!
    struct      rv_hv_selection selection_paste;
    unsigned    char *selection_paste_data;


    bool        show_col_ascii;
    bool        show_infoline;
    bool        show_grid;

    bool        is_target_buffer;
    bool        is_file_buffer;
    bool        have_data;

    QString     buffer_name;
    QString     buffer_ifilename;
    QString     buffer_ofilename;

    QString     info_txt;

    // -- colors --
    // color cell
    QColor color_hex_vals;
    QColor color_bg1_vals;
    QColor color_bg2_vals;
    QColor color_asc_vals;
    QColor color_asc_vals2;

    // color cursor
    QColor color_cursor;
    QColor color_cursor2;
    QColor color_shadow_cursor;

    // color grid
    QColor color_grid;
    QColor color_grid2;

    // color info
    QColor color_grid_info;
    QColor color_bg_info;
    QColor color_txt_info;

    // color headers
    QColor color_bg_hdr;
    QColor color_txt_hdr;
    QColor color_bg_hdr_selected;
    QColor color_shadow_bg_hdr_selected;

    // colors selection
    QColor selection_bgcolor;
    QColor selection_txtcolor;

    QList<struct rv_hv_color_range> color_ranges;
    int single_range;

    QMenu       *context_menu;


    QAction     *action_undo;
    QAction     *action_redo;
    QAction     *action_copy_selection;
    QAction     *action_paste_selection;
    QAction     *action_fill_selection;
    QAction     *action_edit_at_cursor;

private:

    void    setup_actions();
    void    setup_menus();

    re_target       *t;
    unsigned char   *original_dataptr;
    re_addr_t       original_len;

    // part of file/buffer (clip)
    unsigned char   *current_dataptr;
    re_addr_t       current_offset;
    re_addr_t       current_data_len;


    // gui stuff
    re_addr_t   lines_count;    // lines to view in total
    int         num_lines;      // lines visible
    struct      rv_hv_cellgeometry cell_geometry;
    int         header_offs_width; // including content space!
    int         header_col_height; // including content space!
    int         hdr_content_space; // subtract from header width/height!!!
    int         info_line_height;
    int         lmargin_col_ascii;
    int         line_height;

    // cursor stuff
    struct rv_hv_cursor cursor;
    struct rv_hv_cursor old_cursor;
    struct rv_hv_cursor shadow_cursor;
    struct rv_hv_cursor old_shadow_cursor;

    bool        have_focus;

    // edit stuff
    QLineEdit *editor;

    // selection
    rv_hv_cursor tmp_cursor;
    re_addr_t   selection_start;
    re_addr_t   selection_end;
    bool        selecting;
    bool        have_selection;
    bool        have_color_ranges;
    bool        can_paste;
    struct      rv_hv_selection selection;

    QAbstractItemModel *model;

    // kbd status stuff
    bool        shift_down;

    QUndoStack  *undo_stack;
    QMenu       *menu_selection;
    QTabWidget    *tabwidget_attach_to;

    bool use_va;
};

#endif // RV_HEX_EDIT_H
