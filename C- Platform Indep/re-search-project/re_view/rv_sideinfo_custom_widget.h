#ifndef RV_SIDEINFO_CUSTOM_WIDGET_H
#define RV_SIDEINFO_CUSTOM_WIDGET_H

#include <QGroupBox>
#include <QtGui>
#include <QTimer>
#include <re_core.h>
#include <rv_hex_edit.h>
#include <rv_script_processor.h>
#include <rv_script_highlighter.h>
#include <rv_script_code_editor.h>
#include <rv_slider.h>

enum {
    RV_CW_NONE = 0,
    RV_CW_CURSOR_CONNECTED = 1,
    RV_CW_SELECTION_CONNECTED = 2,
    RV_CW_BUTTON_CONNECTED = 4
};

class rv_sideinfo_custom_widget : public QGroupBox
{
    Q_OBJECT
public:
    explicit rv_sideinfo_custom_widget(QWidget *parent = 0);
    ~rv_sideinfo_custom_widget();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);


signals:
    void my_height_changed(rv_sideinfo_custom_widget *w, int height);

public slots:
    void cursor_offset_changed_to(rv_hex_edit *hv, re_addr_t offset);
    void selection_changed(rv_hex_edit *hv, re_addr_t from, re_addr_t to);
    QPlainTextEdit *get_text_edit_outbox();
    void toggle_edit_mode(bool show);
    void abort_execution();
    void editor_enlarge();
    void editor_sync();
    void set_mode(int m);
    void execute(rv_hex_edit *hv);
    void execute();
    void try_execute_with_last_hv();
    void unset_draw_up_down();
    void slider_released();
    void change_height(int h);
    int get_height();

private:
    rv_script_code_editor *text_box_script;
    QPlainTextEdit *text_box_output;
    rv_script_code_editor *text_box_large;
    rv_script_processor *sp;
    QLabel label_offset_d, label_offset_v, label_pic1, label_pic2, label_pic3;

    re_addr_t my_received_offset;

    QPushButton *pushbutton_abort;
    QPushButton *pushbutton_editor_enlarge;
    QPushButton *pushbutton_run;
    QCheckBox *checkbox_details;

    int mode;
    rv_hex_edit *last_hv;
    QPixmap     lil_up_down;

    bool        draw_up_down;
    QTimer      t1;
    rv_slider   *slider;
    QPoint      p;

    int         my_height;
    bool        have_height;
};

#endif // RV_SIDEINFO_CUSTOM_WIDGET_H
