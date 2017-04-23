#include "rv_console_edit.h"

rv_console_edit::rv_console_edit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    my_font         = QFont("Liberation Mono", 9, QFont::Normal);
    my_font_bold    = QFont("Liberation Mono", 9, QFont::Bold);
    setFont(my_font);
    line_height = QFontMetrics(my_font).lineSpacing() + 3;
    char_width  = QFontMetrics(my_font).width('X');

    setCursorWidth(char_width);

    prompt = "rv::target> ";


    connect(this, SIGNAL(sig_enter_pressed()),
            this, SLOT(enter_pressed()));

    history_pos = 0;

    setPlainText   ("re|view disassembler console");
    appendPlainText("----------------------------");
    appendPlainText("enter 'help' for a list of available commands.\n");
    appendPlainText(prompt);
    setUndoRedoEnabled(false);
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::End);
    setTextCursor(c);

    am_big = false;
    pushButton_big_small = new QPushButton(QIcon(":/icons/lil_up.png"),"", this);
    pushButton_big_small->move(width()-pushButton_big_small->width(), 0);
    pushButton_big_small->setFlat(true);
    pushButton_big_small->show();
    connect(pushButton_big_small, SIGNAL(clicked()),
            this, SLOT(biggify_clicked()));
}

void rv_console_edit::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);
}

int rv_console_edit::get_line_height()
{
    return line_height;
}

void rv_console_edit::set_promt(QString p)
{
    prompt = p;
}

void rv_console_edit::keyPressEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Enter)
            || (event->key() == Qt::Key_Return)) {
        emit sig_enter_pressed();
        return;
    }
    switch(event->key()) {
    case Qt::Key_Up:
        history_up();
        return;
        break;
    case Qt::Key_Down:
        history_down();
        return;
        break;
    case Qt::Key_PageUp:
        return;
        break;
    case Qt::Key_PageDown:
        return;
        break;
    case Qt::Key_Left:
        if(textCursor().positionInBlock() == prompt.length()) return;
        break;
    case Qt::Key_Backspace:
        if(textCursor().positionInBlock() == prompt.length()) return;
        break;
    case Qt::Key_Home:
        QTextCursor c = textCursor();
        c.movePosition(QTextCursor::StartOfLine);
        c.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                       prompt.length());
        setTextCursor(c);
        return;
        break;
    }

    QPlainTextEdit::keyPressEvent(event);
}

void rv_console_edit::enter_pressed()
{
    QTextCursor c = textCursor();

    c.select(QTextCursor::LineUnderCursor);

    QString cmd = c.selectedText();
    c.clearSelection();
    if(cmd.left(prompt.length()) == prompt) cmd = cmd.mid(prompt.length());

    if(!cmd.isEmpty()) {
        history << cmd;
        history_pos = history.count();

        cmd_to_execute = cmd;
        emit execute_cmd(cmd);
    } else {
        appendPlainText(prompt);
    }

    c.movePosition(QTextCursor::End);
    setTextCursor(c);
}

void rv_console_edit::history_up()
{
    QString cmd;
    QTextCursor c;

    if(history.isEmpty()) return;
    if(history_pos > 0) {
        if(history_pos == history.count()) {
            // save current cmd
            c = textCursor();
            c.select(QTextCursor::LineUnderCursor);
            cmd = c.selectedText();
            c.clearSelection();
            if(cmd.left(prompt.length()) == prompt) cmd = cmd.mid(prompt.length());
            current_cmd = cmd;
        }
        history_pos--;

        cmd = history.at(history_pos);
        c = textCursor();
        c.select(QTextCursor::LineUnderCursor);
        c.removeSelectedText();
        c.insertText(prompt);
        c.insertText(cmd);
        setTextCursor(c);
    }
}

void rv_console_edit::history_down()
{
    QString cmd;
    QTextCursor c;

    if(history_pos < history.count()) {
        history_pos++;
        if(history_pos < history.count()) {
            cmd = history.at(history_pos);
        } else {
            cmd = current_cmd;
        }

        c = textCursor();
        c.select(QTextCursor::LineUnderCursor);
        c.removeSelectedText();
        c.insertText(prompt);
        c.insertText(cmd);
        setTextCursor(c);
    }
}

void rv_console_edit::cmd_executed(QString output)
{
    appendPlainText(output);
    appendPlainText(prompt);
}

void rv_console_edit::clear_box()
{
    clear();
    appendPlainText(prompt);
}

void rv_console_edit::mouseReleaseEvent(QMouseEvent *e)
{
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::End);
    setTextCursor(c);
}

void rv_console_edit::mousePressEvent(QMouseEvent *e)
{
    return;
}

void rv_console_edit::mouseDoubleClickEvent(QMouseEvent *e)
{
    return;
}

void rv_console_edit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    if(verticalScrollBar()->isVisible())
        pushButton_big_small->move(width()-
                                   pushButton_big_small->width()-
                                   verticalScrollBar()->width(), 0);
    else
        pushButton_big_small->move(width()-
                                   pushButton_big_small->width(), 0);
}

void rv_console_edit::biggify_clicked()
{
    if(am_big) {
        am_big = false;
        pushButton_big_small->setIcon(QIcon(":/icons/lil_up.png"));
        pushButton_big_small->setToolTip("enlarge console");
    } else {
        am_big = true;
        pushButton_big_small->setIcon(QIcon(":/icons/lil_down.png"));
        pushButton_big_small->setToolTip("shrink console");
    }

    emit biggify(am_big);
}

QStringList rv_console_edit::get_history()
{
    return history;
}

void rv_console_edit::clear_history()
{
    history.clear();
}

void rv_console_edit::print_at_prompt(QString s)
{
    QTextCursor t = textCursor();
    t.insertText(s);
    setTextCursor(t);
    appendPlainText(prompt);
}

void rv_console_edit::print_err(QString s)
{
    QTextCursor t = textCursor();
    QTextCharFormat f;
    f.setForeground(QColor(0x80, 0x20, 0x20));
    s.prepend("\n");
    t.insertText(s, f);
    t.setCharFormat(QTextCharFormat());
    setTextCursor(t);
    appendPlainText(prompt);
}

void rv_console_edit::print_shadow(QString s)
{
    QTextCursor t = textCursor();
    QTextCharFormat f;
    f.setForeground(QColor(0x80, 0x80, 0x80));
    s.prepend("\n");
    t.insertText(s, f);
    t.setCharFormat(QTextCharFormat());
    setTextCursor(t);
    appendPlainText(prompt);
}

void rv_console_edit::print_shadow_at_prompt(QString s)
{
    QTextCursor t = textCursor();
    QTextCharFormat f;
    f.setForeground(QColor(0x80, 0x80, 0x80));
    t.insertText(s, f);
    t.setCharFormat(QTextCharFormat());
    setTextCursor(t);
    appendPlainText(prompt);
}
