#include "rv_script_code_editor.h"

rv_script_code_editor::rv_script_code_editor(QWidget *parent) : QPlainTextEdit(parent)
{
    la_color_bg  = QColor(0xc0, 0xc0, 0xc0);
    la_color_fg  = QColor(0x80, 0x80, 0x80);
    la_color_fg2 = QColor(0x40, 0x40, 0x40);

    lineNumberArea = new LineNumberArea(this);

    editor_font = QFont("Liberation Mono", 8, QFont::Normal);
    setFont(editor_font);
    lineNumberArea->setFont(editor_font);

    setWindowIcon(QIcon(":/logos/re_script_run3.png"));
    setWindowTitle("Edit re|script");
    setTabStopWidth(QFontMetrics(editor_font).maxWidth() * 4);

    if(!parent) {
        setMinimumWidth(400);
        setMinimumHeight(300);
    }

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    highlighter.setDocument(document());
}

int rv_script_code_editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = RV_CE_LINENR_LPADD
            + fontMetrics().width(QLatin1Char('9')) * digits
            + RV_CE_LINENR_RPADD;

    return space;
}

void rv_script_code_editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void rv_script_code_editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void rv_script_code_editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void rv_script_code_editor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void rv_script_code_editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), la_color_bg);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(la_color_fg);
            if(blockNumber == textCursor().blockNumber())
                painter.setPen(la_color_fg2);
            painter.drawText(0, top, lineNumberArea->width() - RV_CE_LINENR_RPADD, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}


// --

//void rv_script_code_editor::setupFileActions()
//{
//    QToolBar *tb = new QToolBar(this);
//    tb->setWindowTitle("File Actions");
//    addToolBar(tb);

//    QMenu *menu = new QMenu("&File", this);
//    menuBar()->addMenu(menu);

//    QAction *a;

//    QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
//    a = new QAction( newIcon, "&New", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(QKeySequence::New);
//    connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
//    tb->addAction(a);
//    menu->addAction(a);

//    a = new QAction(QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png")),
//                    "&Open...", this);
//    a->setShortcut(QKeySequence::Open);
//    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
//    tb->addAction(a);
//    menu->addAction(a);

//    menu->addSeparator();

//    actionSave = a = new QAction(QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png")),
//                                 "&Save", this);
//    a->setShortcut(QKeySequence::Save);
//    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
//    a->setEnabled(false);
//    tb->addAction(a);
//    menu->addAction(a);

//    a = new QAction("Save &As...", this);
//    a->setPriority(QAction::LowPriority);
//    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
//    menu->addAction(a);
//    menu->addSeparator();

//#ifndef QT_NO_PRINTER
//    a = new QAction(QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png")),
//                    "&Print...", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(QKeySequence::Print);
//    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
//    tb->addAction(a);
//    menu->addAction(a);

//    a = new QAction(QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png")),
//                    "Print Preview...", this);
//    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
//    menu->addAction(a);

//    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png")),
//                    "&Export PDF...", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(Qt::CTRL + Qt::Key_D);
//    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
//    tb->addAction(a);
//    menu->addAction(a);

//    menu->addSeparator();
//#endif

//    a = new QAction("&Quit", this);
//    a->setShortcut(Qt::CTRL + Qt::Key_Q);
//    connect(a, SIGNAL(triggered()), this, SLOT(close()));
//    menu->addAction(a);
//}

//void rv_script_code_editor::setupEditActions()
//{
//    QToolBar *tb = new QToolBar(this);
//    tb->setWindowTitle("Edit Actions");
//    addToolBar(tb);
//    QMenu *menu = new QMenu("&Edit", this);
//    menuBar()->addMenu(menu);

//    QAction *a;
//    a = actionUndo = new QAction(QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png")),
//                                 "&Undo", this);
//    a->setShortcut(QKeySequence::Undo);
//    tb->addAction(a);
//    menu->addAction(a);
//    a = actionRedo = new QAction(QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png")),
//                                 "&Redo", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(QKeySequence::Redo);
//    tb->addAction(a);
//    menu->addAction(a);
//    menu->addSeparator();
//    a = actionCut = new QAction(QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png")),
//                                "Cu&t", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(QKeySequence::Cut);
//    tb->addAction(a);
//    menu->addAction(a);
//    a = actionCopy = new QAction(QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png")),
//                                 "&Copy", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(QKeySequence::Copy);
//    tb->addAction(a);
//    menu->addAction(a);
//    a = actionPaste = new QAction(QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png")),
//                                  "&Paste", this);
//    a->setPriority(QAction::LowPriority);
//    a->setShortcut(QKeySequence::Paste);
//    tb->addAction(a);
//    menu->addAction(a);
//#ifndef QT_NO_CLIPBOARD
//    if (const QMimeData *md = QApplication::clipboard()->mimeData())
//        actionPaste->setEnabled(md->hasText());
//#endif
//}

