#ifndef RE_ACSS_EXPRESSION_H
#define RE_ACSS_EXPRESSION_H

#include "re_types.h"
#include "re_logger.h"
#include "../libdis/libdis.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QRegExp>

enum { RE_EXPR_MATCH, RE_EXPR_NOMATCH,
       RE_EXPR_SYNTAXERROR,
       RE_EXPR_COMPILE_OK
};

enum {
    // --- insn ---
    RE_TYPE_DWORD,
    RE_TYPE_WORD,
    RE_TYPE_BYTE,
    RE_TYPE_BYTE_MASK_AND,
    RE_TYPE_BYTE_MASK_OR,
    RE_TYPE_WORD_MASK_AND,
    RE_TYPE_WORD_MASK_OR,
    RE_TYPE_DWORD_MASK_AND,
    RE_TYPE_DWORD_MASK_OR,
    RE_TYPE_WILDCARD,

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // attention!! all RE_TYPE_OP* must be past here
    // because rval is op... check is done as
    //      if( optype >= RE_TYPE_OP1_BYTE) { ...
    // in re_acss_expression::evaluate
    // for speed reasons
    // and all OP1, OP2, OP3 values must be togehter!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // --- operands ---

    RE_TYPE_OP1_BEGIN,
    RE_TYPE_OP1_BYTE, RE_TYPE_OP1_WORD, RE_TYPE_OP1_DWORD,
    RE_TYPE_OP1_BYTE_MASK_AND, RE_TYPE_OP1_BYTE_MASK_OR,
    RE_TYPE_OP1_WORD_MASK_AND, RE_TYPE_OP1_WORD_MASK_OR,
    RE_TYPE_OP1_DWORD_MASK_AND, RE_TYPE_OP1_DWORD_MASK_OR,
    RE_TYPE_OP1_END,

    RE_TYPE_OP2_BEGIN,
    RE_TYPE_OP2_BYTE, RE_TYPE_OP2_WORD, RE_TYPE_OP2_DWORD,
    RE_TYPE_OP2_BYTE_MASK_AND, RE_TYPE_OP2_BYTE_MASK_OR,
    RE_TYPE_OP2_WORD_MASK_AND, RE_TYPE_OP2_WORD_MASK_OR,
    RE_TYPE_OP2_DWORD_MASK_AND, RE_TYPE_OP2_DWORD_MASK_OR,
    RE_TYPE_OP2_END,

    RE_TYPE_OP3_BEGIN,
    RE_TYPE_OP3_BYTE, RE_TYPE_OP3_WORD, RE_TYPE_OP3_DWORD,
    RE_TYPE_OP3_BYTE_MASK_AND, RE_TYPE_OP3_BYTE_MASK_OR,
    RE_TYPE_OP3_WORD_MASK_AND, RE_TYPE_OP3_WORD_MASK_OR,
    RE_TYPE_OP3_DWORD_MASK_AND, RE_TYPE_OP3_DWORD_MASK_OR,
    RE_TYPE_OP3_END

};


struct re_acss_cmp_token {
    void *lvar;
    unsigned long rval;
    int type;
    int negate;
};

class RE_CORESHARED_EXPORT re_acss_expression : public QObject
{
    Q_OBJECT
public:
    explicit re_acss_expression(QObject *parent = 0);

    void init(re_logger *l);

    void set_text(char *exp);
    char *get_text();
    int evaluate(x86_insn_t *insn);

    bool compile();
    bool compile_sub_expression(QString exp); //
    bool compile_token(char *lvar, char *rval, re_acss_cmp_token *token);
    QString get_error_string();
    bool is_wildcard();

    // helpers
static    int get_insn_operandtype_fromstr( QString S );
static    int get_insn_type_fromstr(QString S);
static    int get_insn_group_fromstr(QString S);
static    int get_insn_regid_fromstr(QString S);
    bool process_firstpass(QString exp, QString &exp2);

    unsigned int count; // _5_ x insn { ... }
    unsigned int maxcount;

signals:
    void msg(const int level, const QString S);

public slots:

private:
    QString text;       // "5 x insn { ... }"
    QList<QString> sub_expression_strings;
    QList<QList<re_acss_cmp_token>*> compiled_expression;
    x86_insn_t  insn;
    x86_op_t    operand;
    QString error_string;

    re_logger *logger;
};

#endif // RE_ACSS_EXPRESSION_H
