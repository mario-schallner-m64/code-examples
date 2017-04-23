#ifndef RE_WILDASM_COMPILER_H
#define RE_WILDASM_COMPILER_H

#include "re_types.h"
#include "re_logger.h"
#include "re_acss/re_acss_expression.h"
#include "../libdis/libdis.h"

#include <QObject>

class RE_CORESHARED_EXPORT re_wildasm_compiler : public QObject
{
    Q_OBJECT
public:
    explicit re_wildasm_compiler(QObject *parent = 0);

    void init(re_logger *l);

    void set_filter(char *exp);
    char *get_text();
    QString get_compiled_acss_str();
    QString get_error_string();

    bool compile();
    bool compile_line_to_acss(const QString &line, QString &acss_line);
    bool compile_wildasm_to_acss(const QString &mnem_str,
                                 const QString &op1_str,
                                 const QString &op2_str,
                                 QString &acss_line,
                                 int mincnt, int maxcnt);
                                 
    bool parse_operand(const QString &wild_op_str, int op_num,
                        QString &acss_op_str, 
                        QString &err_str);

signals:
    void msg(const int level, const QString S);

public slots:

private:
    QString text;
    QString acss_filter;
    QString error_string;

    int compiled_lines;

    re_logger *logger;
};

#endif // RE_WILDASM_COMPILER_H
