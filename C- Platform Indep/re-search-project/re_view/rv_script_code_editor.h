#ifndef RV_SCRIPT_CODE_EDITOR_H
#define RV_SCRIPT_CODE_EDITOR_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>

#include <rv_script_highlighter.h>

#define RV_CE_LINENR_RPADD 3
#define RV_CE_LINENR_LPADD 3

class LineNumberArea;


class rv_script_code_editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    rv_script_code_editor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);


private:
    QWidget *lineNumberArea;

    QColor      la_color_bg;
    QColor      la_color_fg;
    QColor      la_color_fg2;
    QFont       editor_font;

    rv_script_highlighter highlighter;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(rv_script_code_editor *editor) : QWidget(editor) {
        e = editor;
    }

    QSize sizeHint() const {
        return QSize(e->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        e->lineNumberAreaPaintEvent(event);
    }

private:
    rv_script_code_editor *e;
};

#endif // RV_SCRIPT_CODE_EDITOR_H


