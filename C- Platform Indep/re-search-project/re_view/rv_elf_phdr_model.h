#ifndef RV_ELF_PHDR_MODEL_H
#define RV_ELF_PHDR_MODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <QFont>
#include <re_core.h>


#define MY_FONT         "Consolas"
#define MY_FONT_SIZE    8


class rv_elf_phdr_model : public QAbstractTableModel
{
public:
    rv_elf_phdr_model(QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setFile(re_file *file);

private:
    re_file *f;
};

#endif // RV_ELF_PHDR_MODEL_H
