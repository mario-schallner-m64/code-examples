#include "rv_disasm_xrefs_widget.h"

rv_disasm_xrefs_widget::rv_disasm_xrefs_widget(QWidget *parent) :
    QWidget(parent)
{
    table = new QTableWidget(this);
    my_desc = 0;

    QVBoxLayout *vl = new QVBoxLayout();

    QFont my_font = QFont("Liberation Mono", 8);
    //my_font.setBold(true);
    //my_font.setPointSize(8);

    setFont(my_font);

    l1 = new QLabel("Cross References");
    l1->setStyleSheet("color: rgb(64, 64, 64);");
    l1->setAlignment(Qt::AlignHCenter);
    my_font.setBold(true);
    l1->setFont(my_font);
    //l1->setPixmap(QPixmap(":/disassembly/xrefs_small_up_new2.png"));

    vl->addWidget(l1);
    vl->addWidget(table);
    vl->setContentsMargins(3,10,3,3);

    setLayout(vl);

    active = false;
    top_line = true;
}

void rv_disasm_xrefs_widget::set_desc(re_addr_desc *d, re_target *t)
{
    my_desc = d;
    my_target = t;
    int col_width_add = 0;

    QList<struct re_xref> xrefs = t->disassembler->va_xrefs_mmap.values(d->va);

    QTableWidgetItem *item;

    QFont f1 = QFont("Liberation Mono", 8, QFont::Bold);
    table->setFont(f1);

    table->setRowCount(xrefs.count());
    table->setColumnCount(4);
    table->verticalHeader()->setHidden(true);
    table->horizontalHeader()->setHidden(true);
    table->setAlternatingRowColors(true);

    int i=0;
    foreach(struct re_xref xref, xrefs) {
        item = new QTableWidgetItem((QString)"" + xref.t);
        item->setForeground(QBrush(QColor(0x60, 0x60, 0x80)));
        table->setItem(i, 1, item);

        item = new QTableWidgetItem(re_core::dword_to_pure_hex_string(xref.va_src));
        item->setForeground(QBrush(QColor(0x40, 0x40, 0x60)));
        table->setItem(i, 2, item);

        if(t->file->va_analyzed_functions_hash.contains(xref.va_src)) {
            item = new QTableWidgetItem(
                        t->file->va_analyzed_functions_hash.value(xref.va_src)
                        );
            table->setItem(i, 3, item);
            col_width_add = 100;
        } else
        if(t->file->va_names_hash.contains(xref.va_src)) {
            item = new QTableWidgetItem(
                        t->file->va_names_hash.value(xref.va_src)
                        );
            table->setItem(i, 3, item);
            col_width_add = 100;
        }

        if(xref.va_src < d->va)
            if((d->va - xref.va_src) < 128)
                item = new QTableWidgetItem(QIcon(":/disassembly/xr_arrow1_up.png"),"");
            else
                item = new QTableWidgetItem(QIcon(":/disassembly/xr_arrow2_up.png"),"");
        else
            if((xref.va_src - d->va) < 128)
                item = new QTableWidgetItem(QIcon(":/disassembly/xr_arrow1_down.png"), "");
            else
                item = new QTableWidgetItem(QIcon(":/disassembly/xr_arrow2_down.png"), "");
        table->setItem(i, 0, item);
        i++;
    }
    if(!col_width_add) table->setColumnCount(3);

    if(xrefs.count() == 1) l1->setText("1 xref");
    else l1->setText(QString::number(xrefs.count()) + " xrefs");

    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if(xrefs.count() <= 8) setFixedSize(120 + col_width_add,
                                        (xrefs.count()+2) * 20 - 3);
    else setFixedSize(140 + col_width_add, 200);

    if(col_width_add) setFixedHeight(height() + 30);

    connect(table, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(tbl_cell_doubleclicked(int,int)));
}

void rv_disasm_xrefs_widget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

//    p.fillRect(rect(), QColor(0xff, 0xff, 0xf8));

    QLinearGradient linearGrad(QPointF(0, 20), QPointF(20, height()));
         linearGrad.setColorAt(0, QColor(0xff, 0xff, 0xff));
         linearGrad.setColorAt(1, QColor(0xe0, 0xe0, 0xe0));

    p.fillRect(rect(), linearGrad);

    //p.fillRect(rect(), QColor(0xff, 0xff, 0xff));
    p.setPen(QPen(QBrush(QColor(0xa0, 0xa0, 0xc0).darker(120)), 1, Qt::DashLine));
    p.drawRect(0, 0, width()-1, height()-1);

    if(!top_line) {
        p.setPen(QColor(0xff, 0xf8, 0xff));
        p.drawLine(0,0, width()-1, 0);
    }

//    p.setPen(QPen(QBrush(QColor(0xa0, 0xa0, 0xc0)), 1, Qt::DotLine));
//    p.drawLine(0,0, width()-1, 0);

    QWidget::paintEvent(e);
}

void rv_disasm_xrefs_widget::tbl_cell_doubleclicked(int r, int c)
{
    QTableWidgetItem *ti;
    re_addr_t va;

    ti = table->item(r, 2);

    va = ti->text().toUInt(0, 16);

    emit va_dblclicked(va);
    emit jmp_from(my_desc->va);
}
