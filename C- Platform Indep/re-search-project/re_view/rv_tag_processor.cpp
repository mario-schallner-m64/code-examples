#include "rv_tag_processor.h"

rv_tag_processor::rv_tag_processor(QObject *parent) :
    QObject(parent)
{
    TW_yel = 0;
    TW_blu = 0;
    TW_red = 0;
    TW_grn = 0;
    TW_blk = 0;

    icon_tag_yel = QIcon(":/icons/bookmark_small.png");
    icon_tag_blu = QIcon(":/icons/bookmark_blu_small.png");
    icon_tag_red = QIcon(":/icons/bookmark_red_small.png");
    icon_tag_grn = QIcon(":/icons/bookmark_grn_small.png");
    icon_tag_blk = QIcon(":/icons/bookmark_black_small.png");
}

void rv_tag_processor::set_tag_va(unsigned long va, int tag_nr, QString s)
{
    QHash<unsigned long, QString> *hash = 0;

    switch(tag_nr) {
    case RV_TAG_YEL:
        hash = &qh_Addr_Tag_yel;
        break;

    case RV_TAG_BLU:
        hash = &qh_Addr_Tag_blu;
        break;

    case RV_TAG_RED:
        hash = &qh_Addr_Tag_red;
        break;

    case RV_TAG_GRN:
        hash = &qh_Addr_Tag_grn;
        break;

    case RV_TAG_BLK:
        hash = &qh_Addr_Tag_blk;
        break;
    }

    if(!hash) return;

    hash->insert(va, s);

    fill_tag_table_contents(tag_nr);

    emit tags_changed();
}

void rv_tag_processor::set_tag_tablewidget(int tag_nr, QTableWidget *tw)
{
    QTableWidget **TW = 0;

    switch(tag_nr) {
    case RV_TAG_YEL:
        TW = &TW_yel;
        break;

    case RV_TAG_BLU:
        TW = &TW_blu;
        break;

    case RV_TAG_RED:
        TW = &TW_red;
        break;

    case RV_TAG_GRN:
        TW = &TW_grn;
        break;

    case RV_TAG_BLK:
        TW = &TW_blk;
        break;

    case RV_TAG_ANY:
        TW = &TW_ANY;
        break;
    }


    if(!TW) return;

    *TW = tw;
}

bool rv_tag_processor::has_va_any_tag(unsigned long va)
{
    return qh_Addr_Tag_yel.contains(va) ||
            qh_Addr_Tag_blu.contains(va) ||
            qh_Addr_Tag_red.contains(va) ||
            qh_Addr_Tag_grn.contains(va) ||
            qh_Addr_Tag_blk.contains(va);
}

bool rv_tag_processor::has_va_tag(unsigned long va, int tag_nr)
{
    QHash<unsigned long, QString> *hash = 0;

    switch(tag_nr) {
    case RV_TAG_YEL:
        hash = &qh_Addr_Tag_yel;
        break;

    case RV_TAG_BLU:
        hash = &qh_Addr_Tag_blu;
        break;

    case RV_TAG_RED:
        hash = &qh_Addr_Tag_red;
        break;

    case RV_TAG_GRN:
        hash = &qh_Addr_Tag_grn;
        break;

    case RV_TAG_BLK:
        hash = &qh_Addr_Tag_blk;
        break;
    }

    if(!hash) return false;

    return hash->contains(va);
}


void rv_tag_processor::fill_tag_table_contents(int tag_nr)
{
    QHash<unsigned long, QString> *hash = 0;
    QTableWidget *TW = 0;
    QTableWidgetItem *TWI = 0;
    QIcon *icon = 0;
    unsigned long va;

    switch(tag_nr) {
    case RV_TAG_YEL:
        hash = &qh_Addr_Tag_yel; TW = TW_yel; icon = &icon_tag_yel;
        break;

    case RV_TAG_BLU:
        hash = &qh_Addr_Tag_blu; TW = TW_blu; icon = &icon_tag_blu;
        break;

    case RV_TAG_RED:
        hash = &qh_Addr_Tag_red; TW = TW_red; icon = &icon_tag_red;
        break;

    case RV_TAG_GRN:
        hash = &qh_Addr_Tag_grn; TW = TW_grn; icon = &icon_tag_grn;
        break;

    case RV_TAG_BLK:
        hash = &qh_Addr_Tag_blk; TW = TW_blk; icon = &icon_tag_blk;
        break;
    }

    if(!hash || !TW) return;

    TW->clear();
    QStringList table_header;
    table_header << "va" << "...";
    TW->setRowCount(hash->count());
    TW->setColumnCount(2);
    TW->setHorizontalHeaderLabels(table_header);
    TW->setSelectionBehavior(QAbstractItemView::SelectRows);

    for(int i=0; i < hash->count(); i++) {
        va = hash->keys().at(i);
        TWI = new QTableWidgetItem(*icon, QString::number(va, 16));
        TW->setItem(i, 0, TWI);
        TWI = new QTableWidgetItem(hash->values().at(i));
        TW->setItem(i, 1, TWI);
    }

    TW->sortItems(0);
    TW->resizeColumnsToContents();
    TW->resizeRowsToContents();

    fill_any_tag_table();
}

void rv_tag_processor::fill_any_tag_table()
{
    QHash<unsigned long, QString> *hash = 0;
    QTableWidgetItem *TWI = 0;
    QIcon *icon = 0;
    unsigned long va;
    int i, j = 0;

    TW_ANY->clear();
    TW_ANY->setRowCount(
                qh_Addr_Tag_yel.count() +
                qh_Addr_Tag_blu.count() +
                qh_Addr_Tag_red.count() +
                qh_Addr_Tag_grn.count() +
                qh_Addr_Tag_blk.count()
                        );

    QStringList table_header;
    table_header << "va" << "...";
    TW_ANY->setColumnCount(2);
    TW_ANY->setHorizontalHeaderLabels(table_header);
    TW_ANY->setSelectionBehavior(QAbstractItemView::SelectRows);


//    case RV_TAG_YEL:
        hash = &qh_Addr_Tag_yel; icon = &icon_tag_yel;
        for(i=0; i < hash->count(); i++) {
            va = hash->keys().at(i);
            TWI = new QTableWidgetItem(*icon, QString::number(va, 16));
            TW_ANY->setItem(i + j, 0, TWI);
            TWI = new QTableWidgetItem(hash->values().at(i));
            TW_ANY->setItem(i + j, 1, TWI);
        }
        j += hash->count();


//    case RV_TAG_BLU:
        hash = &qh_Addr_Tag_blu; icon = &icon_tag_blu;
        for(i=0; i < hash->count(); i++) {
            va = hash->keys().at(i);
            TWI = new QTableWidgetItem(*icon, QString::number(va, 16));
            TW_ANY->setItem(i + j, 0, TWI);
            TWI = new QTableWidgetItem(hash->values().at(i));
            TW_ANY->setItem(i + j, 1, TWI);
        }
        j += hash->count();

//    case RV_TAG_RED:
        hash = &qh_Addr_Tag_red; icon = &icon_tag_red;
        for(i=0; i < hash->count(); i++) {
            va = hash->keys().at(i);
            TWI = new QTableWidgetItem(*icon, QString::number(va, 16));
            TW_ANY->setItem(i + j, 0, TWI);
            TWI = new QTableWidgetItem(hash->values().at(i));
            TW_ANY->setItem(i + j, 1, TWI);
        }
        j += hash->count();

//    case RV_TAG_GRN:
        hash = &qh_Addr_Tag_grn; icon = &icon_tag_grn;
        for(i=0; i < hash->count(); i++) {
            va = hash->keys().at(i);
            TWI = new QTableWidgetItem(*icon, QString::number(va, 16));
            TW_ANY->setItem(i + j, 0, TWI);
            TWI = new QTableWidgetItem(hash->values().at(i));
            TW_ANY->setItem(i + j, 1, TWI);
        }
        j += hash->count();

//    case RV_TAG_BLK:
        hash = &qh_Addr_Tag_blk; icon = &icon_tag_blk;
        for(i=0; i < hash->count(); i++) {
            va = hash->keys().at(i);
            TWI = new QTableWidgetItem(*icon, QString::number(va, 16));
            TW_ANY->setItem(i + j, 0, TWI);
            TWI = new QTableWidgetItem(hash->values().at(i));
            TW_ANY->setItem(i + j, 1, TWI);
        }
        j += hash->count();

        TW_ANY->sortItems(0);
        TW_ANY->resizeColumnsToContents();
        TW_ANY->resizeRowsToContents();
}

void rv_tag_processor::clear_all_tags(int tag_nr)
{
    QHash<unsigned long, QString> *hash = 0;
    QTableWidget *TW = 0;

    switch(tag_nr) {
    case RV_TAG_YEL:
        hash = &qh_Addr_Tag_yel; TW = TW_yel;
        break;

    case RV_TAG_BLU:
        hash = &qh_Addr_Tag_blu; TW = TW_blu;
        break;

    case RV_TAG_RED:
        hash = &qh_Addr_Tag_red; TW = TW_red;
        break;

    case RV_TAG_GRN:
        hash = &qh_Addr_Tag_grn; TW = TW_grn;
        break;

    case RV_TAG_BLK:
        hash = &qh_Addr_Tag_blk; TW = TW_blk;
        break;
    }

    if(!hash || !TW) return;

    hash->clear();
    TW->clear();
}

void rv_tag_processor::clear_all_tags()
{
    clear_all_tags(RV_TAG_YEL);
    clear_all_tags(RV_TAG_BLU);
    clear_all_tags(RV_TAG_RED);
    clear_all_tags(RV_TAG_GRN);
    clear_all_tags(RV_TAG_BLK);

    emit tags_changed();
}

int rv_tag_processor::get_va_tag_value(unsigned long va)
{
    int rv = 0;

    if(has_va_tag(va, RV_TAG_YEL)) rv += RV_TAG_YEL;
    if(has_va_tag(va, RV_TAG_RED)) rv += RV_TAG_RED;
    if(has_va_tag(va, RV_TAG_GRN)) rv += RV_TAG_GRN;
    if(has_va_tag(va, RV_TAG_BLU)) rv += RV_TAG_BLU;
    if(has_va_tag(va, RV_TAG_BLK)) rv += RV_TAG_BLK;

    return rv;
}

void rv_tag_processor::set_va_tag_value(unsigned long va, int val) // not overwriting
{
    if( val & RV_TAG_YEL ) { if( !has_va_tag(va, RV_TAG_YEL) ) set_tag_va(va, RV_TAG_YEL); }
    else clear_tag_va(va, RV_TAG_YEL);

    if( val & RV_TAG_RED ) { if( !has_va_tag(va, RV_TAG_RED) ) set_tag_va(va, RV_TAG_RED); }
    else clear_tag_va(va, RV_TAG_RED);

    if( val & RV_TAG_GRN ) { if( !has_va_tag(va, RV_TAG_GRN) ) set_tag_va(va, RV_TAG_GRN); }
    else clear_tag_va(va, RV_TAG_GRN);

    if( val & RV_TAG_BLU ) { if( !has_va_tag(va, RV_TAG_BLU) ) set_tag_va(va, RV_TAG_BLU); }
    else clear_tag_va(va, RV_TAG_BLU);

    if( val & RV_TAG_BLK ) { if( !has_va_tag(va, RV_TAG_BLK) ) set_tag_va(va, RV_TAG_BLK); }
    else clear_tag_va(va, RV_TAG_BLK);

    emit tags_changed();
}

void rv_tag_processor::clear_tag_va(unsigned long va, int tag_nr)
{

    QHash<unsigned long, QString> *hash = 0;

    switch(tag_nr) {
    case RV_TAG_YEL:
        hash = &qh_Addr_Tag_yel;
        break;

    case RV_TAG_BLU:
        hash = &qh_Addr_Tag_blu;
        break;

    case RV_TAG_RED:
        hash = &qh_Addr_Tag_red;
        break;

    case RV_TAG_GRN:
        hash = &qh_Addr_Tag_grn;
        break;

    case RV_TAG_BLK:
        hash = &qh_Addr_Tag_blk;
        break;
    }

    if(!hash) return;

    if(!hash->contains(va)) return;

    hash->remove(va);

    fill_tag_table_contents(tag_nr);
    emit tags_changed();
}
