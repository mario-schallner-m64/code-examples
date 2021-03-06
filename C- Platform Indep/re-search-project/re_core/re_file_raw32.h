#ifndef RE_FILE_RAW32_H
#define RE_FILE_RAW32_H

#include "re_core_global.h"

class re_file;

#include <QObject>

class RE_CORESHARED_EXPORT re_file_raw32 : public QObject
{
    Q_OBJECT
public:
    explicit re_file_raw32(QObject *parent = 0);

    static bool check_file_type(re_file *f);
    static bool analyze_file(re_file *f);

signals:

public slots:

};

#endif // RE_FILE_RAW32_H
