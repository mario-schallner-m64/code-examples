#ifndef RE_TARGET_H
#define RE_TARGET_H

#include "re_file.h"
#include "re_address_space.h"
#include "re_logger.h"
#include "re_disassembler.h"

#include <QObject>

class RE_CORESHARED_EXPORT re_target : public QObject
{
    Q_OBJECT
public:
    explicit re_target(QObject *parent = 0);
    ~re_target();

    bool init(re_file *f, re_logger *l);

signals:
    void msg(const int level, const QString S);

public slots:

public:
    re_address_space *clone_address_space();

    re_address_space *address_space;
    re_disassembler  *disassembler;
    re_file          *file;


private:
    re_addr_desc **image_map;
    re_logger *logger;
};

#endif // RE_TARGET_H
