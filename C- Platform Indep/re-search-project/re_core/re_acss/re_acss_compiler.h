#ifndef RE_ACSS_COMPILER_H
#define RE_ACSS_COMPILER_H


#include <QObject>
#include <QList>
#include <QMap>
#include <QHash>
#include <QRegExp>

#include "re_acss_expression.h"
#include "re_logger.h"

enum {
        RE_ACSS_SEARCH_UNDEF,
        RE_ACSS_SEARCH_IN_PROGRESS,
        RE_ACSS_SEARCH_FINISHED
};

enum {
        RE_ACSS_SEARCH_DISASSEMBLY,
        RE_ACSS_SEARCH_DEEP,
};

class RE_CORESHARED_EXPORT re_acss_compiler : public QObject
{
    Q_OBJECT
public:
    explicit re_acss_compiler(QObject *parent = 0);

    void init(re_logger *l);
    void set_filter(char *f);
    char *get_filter();
    bool compile_expressions();
    bool is_block();

    QList<re_acss_expression *> expressions;    
    re_addr_t block_size;
    int threshold;
    bool blocked;

private:
    QString filter;

    re_logger *logger;

signals:
    void msg(const int level, const QString S);

public slots:

};

#endif // RE_ACSS_COMPILER_H
