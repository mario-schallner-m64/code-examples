#ifndef RV_CONSOLE_EDIT_H
#define RV_CONSOLE_EDIT_H

#include <QPlainTextEdit>
#include <QtGui>

class rv_console_edit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit rv_console_edit(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    int get_line_height();
    QStringList get_history();

signals:
    void sig_enter_pressed();
    void execute_cmd(QString);
    void biggify(bool yesno);

public slots:
    void set_promt(QString p);
    void enter_pressed();
    void history_up();
    void history_down();

    void biggify_clicked();

    void cmd_executed(QString output);
    void clear_box();
    void clear_history();

    void print_at_prompt(QString s);
    void print_err(QString s);
    void print_shadow(QString s);
    void print_shadow_at_prompt(QString s);


private:
    QFont           my_font, my_font_bold;
    int             line_height, char_width;

    QString         prompt;
    QStringList     history;
    int             history_pos;
    QString         current_cmd;

    QString         cmd_to_execute;

    QPushButton     *pushButton_big_small;
    bool            am_big;
};

#endif // RV_CONSOLE_EDIT_H
