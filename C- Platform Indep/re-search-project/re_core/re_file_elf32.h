#ifndef RE_FILE_ELF32_H
#define RE_FILE_ELF32_H

#include "re_core_global.h"
#include "re_elf.h"

class re_file;

#include <QObject>

class RE_CORESHARED_EXPORT re_file_elf32 : public QObject
{
    Q_OBJECT
public:
    explicit re_file_elf32(QObject *parent = 0);

    static bool check_file_type(re_file *f);
    static bool analyze_file(re_file *f);

signals:

public slots:

private:
    static re_addr_t got2plt(re_file *f, re_addr_t got);
};

#endif // RE_FILE_ELF32_H
