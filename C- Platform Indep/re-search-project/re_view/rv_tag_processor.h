#ifndef RV_TAG_PROCESSOR_H
#define RV_TAG_PROCESSOR_H

#include <QObject>
#include <QHash>
#include <QTableWidget>
#include <QIcon>

#define RV_TAG_YEL  1
#define RV_TAG_BLU  2
#define RV_TAG_RED  4
#define RV_TAG_GRN  8
#define RV_TAG_BLK  16
#define RV_TAG_ANY  32

class rv_tag_processor : public QObject
{
    Q_OBJECT
public:
    explicit rv_tag_processor(QObject *parent = 0);

    void set_tag_va(unsigned long va, int tag_nr, QString s ="");
    void set_tag_tablewidget(int tag_nr, QTableWidget *tw);

    bool has_va_any_tag(unsigned long va);
    bool has_va_tag(unsigned long va, int tag);

    int  get_va_tag_value(unsigned long va);
    void set_va_tag_value(unsigned long va, int val);
    void clear_tag_va(unsigned long va, int tag_nr);

    void fill_tag_table_contents(int tag);
    void fill_any_tag_table();
    void clear_all_tags(int tag);
    void clear_all_tags();

signals:
    void tags_changed();

public slots:

private:
    QHash<unsigned long, QString> qh_Addr_Tag_yel;
    QHash<unsigned long, QString> qh_Addr_Tag_blu;
    QHash<unsigned long, QString> qh_Addr_Tag_red;
    QHash<unsigned long, QString> qh_Addr_Tag_grn;
    QHash<unsigned long, QString> qh_Addr_Tag_blk;

    QTableWidget *TW_yel;
    QTableWidget *TW_blu;
    QTableWidget *TW_red;
    QTableWidget *TW_grn;
    QTableWidget *TW_blk;
    QTableWidget *TW_ANY;

    QIcon icon_tag_yel;
    QIcon icon_tag_blu;
    QIcon icon_tag_red;
    QIcon icon_tag_grn;
    QIcon icon_tag_blk;
};


#endif // RV_TAG_PROCESSOR_H
