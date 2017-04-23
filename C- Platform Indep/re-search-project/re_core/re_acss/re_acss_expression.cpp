#include "re_acss_expression.h"

re_acss_expression::re_acss_expression(QObject *parent) :
    QObject(parent)
{
    count = 0;
    maxcount = 0;
    text = "";
}

void re_acss_expression::init(re_logger *l)
{
    logger = l;
    connect(this, SIGNAL(msg(int, QString)),
        logger, SLOT(log_event(int, QString)));
}

void re_acss_expression::set_text(char *exp)
{
    text = QString::fromAscii(exp);
}

char *re_acss_expression::get_text()
{
    return text.toAscii().data();
}

bool re_acss_expression::is_wildcard()
{
    if(compiled_expression.isEmpty()) return false;
    if(compiled_expression[0]->at(0).type == RE_TYPE_WILDCARD) {
        return true;
    }
}

int re_acss_expression::evaluate(x86_insn_t *current_insn)
{
    x86_op_t *op;

    int i, j, optype, negateme;
    int subexpr_match_cnt, breakme, have_matched;

    if(compiled_expression.isEmpty()) return RE_EXPR_SYNTAXERROR;

    // for insn type == ANY (to use only in wildasm or for blocks)

    if(compiled_expression[0]->at(0).type == RE_TYPE_WILDCARD) {
        return RE_EXPR_MATCH;
    }


    // else: for all subexpressions
    for(i=0; i < compiled_expression.count(); i++) {
        // for all tokens
        subexpr_match_cnt = 0;
        breakme = 0;
        for(j=0; j<compiled_expression[i]->count() && !breakme; j++) {
            have_matched = 0;
            optype = compiled_expression[i]->at(j).type;
            if( optype >= RE_TYPE_OP1_BEGIN) {
                if( (optype >= RE_TYPE_OP1_BEGIN) && (optype <= RE_TYPE_OP1_END) )
                    op = x86_operand_1st(current_insn);

                if( (optype >= RE_TYPE_OP2_BEGIN) && (optype <= RE_TYPE_OP2_END) )
                    op = x86_operand_2nd(current_insn);

                if( (optype >= RE_TYPE_OP3_BEGIN) && (optype <= RE_TYPE_OP3_END) )
                    op = x86_operand_3rd(current_insn);

                if(!op) return RE_EXPR_NOMATCH;
            }

            negateme = compiled_expression[i]->at(j).negate;

            switch(compiled_expression[i]->at(j).type) {

// -------- INSN simple compares --------
            case RE_TYPE_DWORD:
                if((*(unsigned long int *)( (unsigned long int) compiled_expression[i]->at(j).lvar + (unsigned long int) current_insn)  ==
                                    (unsigned long int) compiled_expression[i]->at(j).rval)
                        ) have_matched=1;
                else breakme = 1;
                break;

            case RE_TYPE_WORD:
                if( ((*((unsigned long int *)(
                    (unsigned long int) compiled_expression[i]->at(j).lvar +
                    (unsigned long int) current_insn)) & 0x0000ffff)
                        == (unsigned long int) compiled_expression[i]->at(j).rval)   ) have_matched=1;
                else breakme = 1;
                break;

            case RE_TYPE_BYTE:
                if( ((*((unsigned long int *)(
                    (unsigned long int) compiled_expression[i]->at(j).lvar +
                    (unsigned long int) current_insn)) & 0x000000ff)
                        == (unsigned long int) compiled_expression[i]->at(j).rval)  ) have_matched=1;
                else breakme = 1;
                break;

// -------- INSN AND compares --------
            case RE_TYPE_BYTE_MASK_AND:
                if( ((*((unsigned long int *)(
                    (unsigned long int) compiled_expression[i]->at(j).lvar +
                    (unsigned long int) current_insn)) & 0x000000ff)
                        & (unsigned long int) compiled_expression[i]->at(j).rval)  ) have_matched=1;
                else breakme = 1;
                break;


// -------- OP[123] simple compares --------
            case RE_TYPE_OP1_BYTE:
            case RE_TYPE_OP2_BYTE:
            case RE_TYPE_OP3_BYTE:
                if( ((*((unsigned long int *)(
                    (unsigned long int) compiled_expression[i]->at(j).lvar +
                    (unsigned long int) op)) & 0x000000ff)
                        == (unsigned long int) compiled_expression[i]->at(j).rval)  ) have_matched=1;
                else breakme = 1;
                break;

            case RE_TYPE_OP1_WORD:
            case RE_TYPE_OP2_WORD:
            case RE_TYPE_OP3_WORD:
                if( ((*((unsigned long int *)(
                    (unsigned long int) compiled_expression[i]->at(j).lvar +
                    (unsigned long int) op)) & 0x0000ffff)
                        == (unsigned long int) compiled_expression[i]->at(j).rval)  ) have_matched=1;
                else breakme = 1;
                break;

            case RE_TYPE_OP1_DWORD:
            case RE_TYPE_OP2_DWORD:
            case RE_TYPE_OP3_DWORD:
                if( ((*((unsigned long int *)(
                           (unsigned long int) compiled_expression[i]->at(j).lvar +
                           (unsigned long int) op)) )
                        == (unsigned long int) compiled_expression[i]->at(j).rval)  ) have_matched=1;
                else breakme = 1;
                break;

                // -------- OP[123] AND  compares --------
            case RE_TYPE_OP1_BYTE_MASK_AND:
            case RE_TYPE_OP2_BYTE_MASK_AND:
            case RE_TYPE_OP3_BYTE_MASK_AND:
                if( ((*((unsigned long int *)(
                           (unsigned long int) compiled_expression[i]->at(j).lvar +
                           (unsigned long int) op)) & 0x000000ff)
                        & (unsigned long int) compiled_expression[i]->at(j).rval) ) have_matched=1;
                else breakme = 1;
                break;


                // -------- OP[123] AND  compares --------
            case RE_TYPE_OP1_WORD_MASK_AND:
            case RE_TYPE_OP2_WORD_MASK_AND:
            case RE_TYPE_OP3_WORD_MASK_AND:
                if( ((*((unsigned long int *)(
                           (unsigned long int) compiled_expression[i]->at(j).lvar +
                           (unsigned long int) op)) & 0x0000ffff)
                        & (unsigned long int) compiled_expression[i]->at(j).rval) ) have_matched=1;
                else breakme = 1;
                break;


                // -------- OP[123] AND  compares --------
            case RE_TYPE_OP1_DWORD_MASK_AND:
            case RE_TYPE_OP2_DWORD_MASK_AND:
            case RE_TYPE_OP3_DWORD_MASK_AND:
                if( ((*((unsigned long int *)(
                           (unsigned long int) compiled_expression[i]->at(j).lvar +
                           (unsigned long int) op)))
                        & (unsigned long int) compiled_expression[i]->at(j).rval)  )  have_matched=1;
                else breakme = 1;

                break;


// removed, since wildcard should only be used as single compare -> above
//                // -------- WILDCARD --------
//            case RE_TYPE_WILDCARD:
//                have_matched=1;
//                break;

            }

            if(negateme) { have_matched = 1 - have_matched; breakme = 1 - breakme; }
            subexpr_match_cnt += have_matched;
        }

        // compare made here, multiple expressions concatenated via logical OR !
        if(subexpr_match_cnt == compiled_expression[i]->count()) return RE_EXPR_MATCH;
    }

    return RE_EXPR_NOMATCH;
}



bool re_acss_expression::compile()
{
    emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile(): starting");

    QString r_num, r_str, r_str2, r_sub_str;
    QRegExp rx;
    int pos = 0, cnt = 0;

    error_string = "";

    rx.setPattern("([\\*0123456789]+\\-*[0123456789]*)\\s*x\\s*insn\\s*\\{(.+)\\}");
    rx.setMinimal(TRUE);

    if ( rx.indexIn(text) != -1) {
        r_num = rx.cap(1);
        r_str = rx.cap(2);

        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile(): found expr: "
                 + r_num + " x " +  r_str);

        if(!process_firstpass(r_str, r_str2)) {
            error_string = "error in firstpass of '" + r_str + "'";
            return false;
        }

        r_str = r_str2;

        QRegExp rx3;
        rx3.setPattern("([0-9]+)\\-([0-9]+)");
        if(rx3.indexIn(r_num, 0)  != -1) {
            count   = rx3.cap(1).toUInt();
            maxcount= rx3.cap(2).toUInt();
        } else  {
            count = r_num.toUInt(); // this->count
            maxcount = count; // this->count
        }

        if(!count) {
            error_string = "invalid count in '" + r_num + "'";
            return false;
        }

        emit msg(
            RE_LOG_DEBUG_3,
            "re_acss_expression::compile(): AFTER FIRSTPASS: expr = " +
                   QString::number(count) + " - " +
                   QString::number(maxcount) + " x " + r_str);

        // --
        QRegExp rx2;
        cnt = 0;
        rx2.setPattern("(.+;)");
        rx2.setMinimal(TRUE);
        while ( (pos = rx2.indexIn(r_str, pos) ) != -1) {
            r_sub_str = rx2.cap(1);
            cnt++;

            emit msg(RE_LOG_DEBUG_3,
                "re_acss_expression::compile(): found sub expression: '"
                     + r_sub_str + "'");

            sub_expression_strings.append(r_sub_str);

            pos += rx2.matchedLength();
        }
        if(!cnt) {
            error_string = "no expression found in '" + r_str + "'";
            return false;
        }
    }
    // no match _x_ x insn { ... }
    else {
        error_string = "no valid expression found in '" + text + "'";
        return false;
    }

    // compile sub expressions

    int i;

    for(i=0; i < cnt; i++) {
        if(!compile_sub_expression(sub_expression_strings[i])) {
                // error_string = sub_expression_strings[i];
                // error string must be set in compile function
                return false;
        }
    }

    emit msg(RE_LOG_DEBUG_3,
             "re_acss_expression::compile(): compilation complete");
    emit msg(RE_LOG_DEBUG_3,
             "re_acss_expression::compile(): this expression has "
             + QString::number(compiled_expression.count())
             + " subexpression(s)");

    for(i=0; i < compiled_expression.count(); i++) {
        emit msg(RE_LOG_DEBUG_3,
            "re_acss_expression::compile(): subexpression " + QString::number(i)
                 + " has " + QString::number(compiled_expression[i]->count())
                 + " tokens");

    }

    return true;
}

bool re_acss_expression::compile_sub_expression(QString exp)
{
    emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_sub_expression(): starting");
    emit msg(RE_LOG_DEBUG_3, "for '" + exp + "'");

    QRegExp rx;
    QString lvar, rval, sEoperator;
    int pos = 0;
    int compiled_token = 0;

    QList<re_acss_cmp_token> *subExpression = new QList<re_acss_cmp_token>;
    re_acss_cmp_token token;
    token.negate = 0;

    // do it for insn
    rx.setPattern("\\s*(\\bgroup\\b|\\btype\\b|\\bsize\\b|\\baddr_size\\b|\\bop_size\\b|\\bstack_mod\\b|\\bstack_mod_val\\b|\\boperand_count\\b|\\bexplicit_count\\b|\\bprefix\\b)\\s*([\\!=]=)\\s*(\\b.+\\b)\\s*[,;]{1,}");
    rx.setMinimal(TRUE);

    while ( (pos = rx.indexIn(exp, pos) ) != -1) {
        lvar = rx.cap(1);
        rval = rx.cap(3);
        sEoperator = rx.cap(2);

        emit msg(RE_LOG_DEBUG_3,
                 "re_acss_expression::compile_sub_expression(): matched: '"
                 + lvar + "' " + sEoperator + " "
                 + "'" + rval + "'");

        if(sEoperator == "!=" ) token.negate = 1;
        else token.negate = 0;

        if(!compile_token(lvar.toAscii().data(),
                rval.toAscii().data(), &token)) {            
            delete subExpression;
            return false;
        } else subExpression->append(token);

        pos += rx.matchedLength();
        compiled_token++;
    }

    // do it for operands
    rx.setPattern("\\s*(\\bop[123]_type\\b|\\bop[123]_datatype\\b|\\bop[123]_reg_size\\b|\\bop[123]_reg_name\\b|\\bop[123]_data_dword\\b|\\bop[123]_data_word\\b|\\bop[123]_data_byte\\b)\\s*([\\!=]=)\\s*(\\b.+\\b)\\s*[,;]{1,}");
    rx.setMinimal(TRUE);

    pos = 0;
    while ( (pos = rx.indexIn(exp, pos) ) != -1) {
        lvar = rx.cap(1);
        rval = rx.cap(3);
        sEoperator = rx.cap(2);
        emit msg(RE_LOG_DEBUG_3,
                 "re_acss_expression::compile_sub_expression(): matched: '"
                 + lvar + "' " + sEoperator + " "
                 + "'" + rval + "'");


        if(sEoperator == "!=" ) token.negate = 1;
        else token.negate = 0;

        if(!compile_token(lvar.toAscii().data(),
                rval.toAscii().data(), &token)) {
            delete subExpression;
            return false;
        } else subExpression->append(token);

        pos += rx.matchedLength();
        compiled_token++;
    }


    // do it for &,|
    rx.setPattern("\\s*(\\bflags_set\\b|\\bflags_tested\\b|\\bop[123]_access\\b|\\bop[123]_flags\\b|\\bop[123]_reg_type\\b)\\s*([\\!=]=)\\s*([\\|\\&].+)\\s*[,;]{1,}");
    rx.setMinimal(TRUE);

    pos = 0;
    while ( (pos = rx.indexIn(exp, pos) ) != -1) {
        lvar = rx.cap(1);
        rval = rx.cap(3);
        sEoperator = rx.cap(2);
        emit msg(RE_LOG_DEBUG_3,
                 "re_acss_expression::compile_sub_expression(): matched: '"
                 + lvar + "' " + sEoperator + " "
                 + "'" + rval + "'");

        if(sEoperator == "!=" ) token.negate = 1;
        else token.negate = 0;

        if(!compile_token(lvar.toAscii().data(),
                rval.toAscii().data(), &token)) {
            delete subExpression;
            return false;
        } else subExpression->append(token);

        pos += rx.matchedLength();
        compiled_token++;
    }

    if(!compiled_token) {
        error_string = "error in sub expression '" + exp + "'";
        return false;
    }

    compiled_expression.append(subExpression);
    return true;
}

bool re_acss_expression::compile_token(char *lvar, char *rval, re_acss_cmp_token *token)
{
    QString Qlvar = QString::fromAscii(lvar);
    QString Qrval = QString::fromAscii(rval);

    int have_compiled = 0;
    unsigned long v;

    // ===================================================================================
    // insn

    // insn.group

    if(token->negate) emit msg(RE_LOG_DEBUG_3,
        "re_acss_expression::compile_token(): negating the following token:");

    if(Qlvar == "group") {
        if( !(v = get_insn_group_fromstr(Qrval)) ) {
            error_string = "unknown insn.group: " + Qrval;
            return false;
        }
        token->lvar = (char *)((unsigned long int) &(insn.group))
                    - ((unsigned long int) &insn);
        token->rval = v;
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.type
    if(Qlvar == "type") {
        if(Qrval == "ANY") {
            token->lvar = 0;
            token->rval = 0;
            token->type = RE_TYPE_WILDCARD;
            return true;
        }

        if( !(v = get_insn_type_fromstr(Qrval)) ) {
            error_string = "unknown insn.type: " + Qrval;
            return false;
        }
        token->lvar = (char *)((unsigned long int) &(insn.type))
                    - ((unsigned long int) &insn);
        token->rval = v;
        token->type = RE_TYPE_WORD;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.size
    if(Qlvar == "size") {
        token->lvar = (char *)((unsigned long int) &(insn.size))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.addr_size
    if(Qlvar == "addr_size") {
        token->lvar = (char *)((unsigned long int) &(insn.addr_size))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.op_size
    if(Qlvar == "op_size") {
        token->lvar = (char *)((unsigned long int) &(insn.op_size))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.stack_mod
    if(Qlvar == "stack_mod") {
        token->lvar = (char *)((unsigned long int) &(insn.stack_mod))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.stack_mod_val
    if(Qlvar == "stack_mod_val") {
        token->lvar = (char *)((unsigned long int) &(insn.stack_mod_val))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.operand_count
    if(Qlvar == "operand_count") {
        token->lvar = (char *)((unsigned long int) &(insn.operand_count))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.explicit_count
    if(Qlvar == "explicit_count") {
        token->lvar = (char *)((unsigned long int) &(insn.explicit_count))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.prefix
    if(Qlvar == "prefix") {
        token->lvar = (char *)((unsigned long int) &(insn.prefix))
                    - ((unsigned long int) &insn);
        token->rval = Qrval.toUInt();
        token->type = RE_TYPE_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.flags_set
    if(Qlvar == "flags_set") {
        token->lvar = (char *)((unsigned long int) &(insn.flags_set))
                    - ((unsigned long int) &insn);

        token->type = RE_TYPE_BYTE_MASK_AND; // default

        if(Qrval.contains("|")) {
            Qrval.replace('|', " ");
            token->rval = Qrval.toUInt();
            token->type = RE_TYPE_BYTE_MASK_AND;
        }

        if(Qrval.contains("&")) {
            Qrval.replace('&', " ");
            token->rval = Qrval.toUInt();
            token->type = RE_TYPE_BYTE;
        }

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // insn.flags_tested
    if(Qlvar == "flags_tested") {
        token->lvar = (char *)((unsigned long int) &(insn.flags_tested))
                    - ((unsigned long int) &insn);

        token->type = RE_TYPE_BYTE_MASK_AND; // default

        if(Qrval.contains("|")) {
            Qrval.replace('|', " ");
            token->rval = Qrval.toUInt();
            token->type = RE_TYPE_BYTE_MASK_AND;
        }

        if(Qrval.contains("&")) {
            Qrval.replace('&', " ");
            token->rval = Qrval.toUInt();
            token->type = RE_TYPE_BYTE;
        }

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }


    // ===================================================================================
    // operands

    // op[123]_type
    if((Qlvar == "op1_type") || (Qlvar == "op2_type") || Qlvar == ("op3_type")) {
        token->lvar = (char *)((unsigned long int) &(operand.type))
                    - ((unsigned long int) &operand);
        token->rval = Qrval.toUInt();
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_BYTE;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_BYTE;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_BYTE));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_datatype
    if( (Qlvar == "op1_datatype") || (Qlvar == "op2_datatype") || (Qlvar == "op3_datatype")) {
        token->lvar = (char *)((unsigned long int) &(operand.datatype))
                    - ((unsigned long int) &operand);
        token->rval = Qrval.toUInt();
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_BYTE;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_BYTE;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_BYTE));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_access
    if( (Qlvar == "op1_access") || (Qlvar == "op2_access") || (Qlvar == "op3_access")) {
        token->lvar = (char *)((unsigned long int) &(operand.access))
                    - ((unsigned long int) &operand);

        if(Qrval.contains("|")) {
            Qrval.replace('|', " ");
            token->rval = Qrval.toUInt();
            if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_BYTE_MASK_AND;
            if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_BYTE_MASK_AND;
            if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_BYTE_MASK_AND;
        }

        if(Qrval.contains("&")) {
            Qrval.replace('&', " ");
            token->rval = Qrval.toUInt();
            if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_BYTE;
            if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_BYTE;
            if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_BYTE;
        }

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_BYTE_MASK_AND));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_reg_type
    if( (Qlvar == "op1_reg_type") || (Qlvar == "op2_reg_type") || (Qlvar == "op3_reg_type")) {
        token->lvar = (char *)((unsigned long int) &(operand.data.reg.type))
                    - ((unsigned long int) &operand);

        if(Qrval.contains("|")) {
            Qrval.replace('|', " ");
            token->rval = Qrval.toUInt();
            if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_DWORD_MASK_AND;
            if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_DWORD_MASK_AND;
            if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_DWORD_MASK_AND;
        }

        if(Qrval.contains("&")) {
            Qrval.replace('&', " ");
            token->rval = Qrval.toUInt();
            if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_DWORD;
            if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_DWORD;
            if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_DWORD;
        }

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_BYTE_MASK_AND));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_datatype
    if( (Qlvar == "op1_reg_size") || (Qlvar == "op2_reg_size") || (Qlvar == "op3_reg_size")) {
        token->lvar = (char *)((unsigned long int) &(operand.data.reg.size))
                    - ((unsigned long int) &operand);
        token->rval = Qrval.toUInt();
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_BYTE;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_BYTE;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_BYTE));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_reg_name
    if( (Qlvar == "op1_reg_name") || (Qlvar == "op2_reg_name") || (Qlvar == "op3_reg_name")) {
        if( !(v = get_insn_regid_fromstr(Qrval)) ) {
            error_string = "unknown op_reg_name: " + Qrval;
            emit msg(RE_LOG_ERROR, error_string);
            return false;
        }
        token->lvar = (char *)((unsigned long int) &(operand.data.reg.id))
                    - ((unsigned long int) &operand);
        token->rval = v;
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_DWORD;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_DWORD;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_DWORD;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(v, 16));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_data_byte
    if( (Qlvar == "op1_data_byte") || (Qlvar == "op2_data_byte") || (Qlvar == "op3_data_byte")) {
        token->lvar = (char *)((unsigned long int) &(operand.data.byte))
                    - ((unsigned long int) &operand);
        if(Qrval.contains("0x")) token->rval = Qrval.toUInt(0,16);
        else token->rval = Qrval.toUInt(0,10);
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_BYTE;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_BYTE;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_BYTE;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_BYTE));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_data_word
    if( (Qlvar == "op1_data_word") || (Qlvar == "op2_data_word") || (Qlvar == "op3_data_word")) {
        token->lvar = (char *)((unsigned long int) &(operand.data.word))
                    - ((unsigned long int) &operand);
        if(Qrval.contains("0x")) token->rval = Qrval.toUInt(0,16);
        else token->rval = Qrval.toUInt(0,10);
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_WORD;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_WORD;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_WORD;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_WORD));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    // op[123]_data_dword
    if( (Qlvar == "op1_data_dword") || (Qlvar == "op2_data_dword") || (Qlvar == "op3_data_dword")) {
        token->lvar = (char *)((unsigned long int) &(operand.data.dword))
                    - ((unsigned long int) &operand);
        if(Qrval.contains("0x")) token->rval = Qrval.toUInt(0,16);
        else token->rval = Qrval.toUInt(0,10);
        if(Qlvar.contains("op1"))token->type = RE_TYPE_OP1_DWORD;
        if(Qlvar.contains("op2"))token->type = RE_TYPE_OP2_DWORD;
        if(Qlvar.contains("op3"))token->type = RE_TYPE_OP3_DWORD;

        QString D = QString("compiling Qlvar=='%1', Qrval=='%2'(%3)")
                         .arg(Qlvar).arg(Qrval).arg(QString::number(RE_TYPE_OP1_DWORD));
        emit msg(RE_LOG_DEBUG_3, "re_acss_expression::compile_token(): " + D);

        have_compiled++;
    }

    if(!have_compiled) {
        error_string += " no valid token found"; return false;
    }

    return true;
}


int re_acss_expression::get_insn_group_fromstr(QString S) {

        char *types[] = {
                (char *) ".",           // 0
                (char *) "controlflow",// 1
                (char *) "arithmetic", // 2
                (char *) "logic",      // 3
                (char *) "stack",      // 4
                (char *) "comparison", // 5
                (char *) "move",       // 6
                (char *) "string",     // 7
                (char *) "bit_manip",  // 8
                (char *) "flag_manip", // 9
                (char *) "fpu",        // 10
                (char *) ".",           // 11
                (char *) ".",           // 12
                (char *) "interrupt",  // 13
                (char *) "system",     // 14
                (char *) "other",      // 15
        };

        for(int i=0; i<16; i++) if(S == types[i]) return i;

        return 0;
}

int re_acss_expression::get_insn_type_fromstr(QString S)
{
        struct {
                char *name;
                int  value;
        } types[] = {
                /* insn_controlflow */
                { (char *) "jmp", 0x1001 },
                { (char *) "jcc", 0x1002 },
                { (char *) "call", 0x1003 },
                { (char *) "callcc", 0x1004 },
                { (char *) "return", 0x1005 },
                { (char *) "loop", 0x1006 },
                /* insn_arithmetic */
                { (char *) "add", 0x2001 },
                { (char *) "sub", 0x2002 },
                { (char *) "mul", 0x2003 },
                { (char *) "div", 0x2004 },
                { (char *) "inc", 0x2005 },
                { (char *) "dec", 0x2006 },
                { (char *) "shl", 0x2007 },
                { (char *) "shr", 0x2008 },
                { (char *) "rol", 0x2009 },
                { (char *) "ror", 0x200A },
                /* insn_logic */
                { (char *) "and", 0x3001 },
                { (char *) "or", 0x3002 },
                { (char *) "xor", 0x3003 },
                { (char *) "not", 0x3004 },
                { (char *) "neg", 0x3005 },
                /* insn_stack */
                { (char *) "push", 0x4001 },
                { (char *) "pop", 0x4002 },
                { (char *) "pushregs", 0x4003 },
                { (char *) "popregs", 0x4004 },
                { (char *) "pushflags", 0x4005 },
                { (char *) "popflags", 0x4006 },
                { (char *) "enter", 0x4007 },
                { (char *) "leave", 0x4008 },
                /* insn_comparison */
                { (char *) "test", 0x5001 },
                { (char *) "cmp", 0x5002 },
                /* insn_move */
                { (char *) "mov", 0x6001 },      /* move */
                { (char *) "movcc", 0x6002 },    /* conditional move */
                { (char *) "xchg", 0x6003 },     /* exchange */
                { (char *) "xchgcc", 0x6004 },   /* conditional exchange */
                /* insn_string */
                { (char *) "strcmp", 0x7001 },
                { (char *) "strload", 0x7002 },
                { (char *) "strmov", 0x7003 },
                { (char *) "strstore", 0x7004 },
                { (char *) "translate", 0x7005 },        /* xlat */
                /* insn_bit_manip */
                { (char *) "bittest", 0x8001 },
                { (char *) "bitset", 0x8002 },
                { (char *) "bitclear", 0x8003 },
                /* insn_flag_manip */
                { (char *) "clear_carry", 0x9001 },
                { (char *) "clear_zero", 0x9002 },
                { (char *) "clear_oflow", 0x9003 },
                { (char *) "clear_dir", 0x9004 },
                { (char *) "clear_sign", 0x9005 },
                { (char *) "clear_parity", 0x9006 },
                { (char *) "set_carry", 0x9007 },
                { (char *) "set_zero", 0x9008 },
                { (char *) "set_oflow", 0x9009 },
                { (char *) "set_dir", 0x900A },
                { (char *) "set_sign", 0x900B },
                { (char *) "set_parity", 0x900C },
                { (char *) "tog_carry", 0x9010 },
                { (char *) "tog_zero", 0x9020 },
                { (char *) "tog_oflow", 0x9030 },
                { (char *) "tog_dir", 0x9040 },
                { (char *) "tog_sign", 0x9050 },
                { (char *) "tog_parity", 0x9060 },
                /* insn_fpu */
                { (char *) "fmov", 0xA001 },
                { (char *) "fmovcc", 0xA002 },
                { (char *) "fneg", 0xA003 },
                { (char *) "fabs", 0xA004 },
                { (char *) "fadd", 0xA005 },
                { (char *) "fsub", 0xA006 },
                { (char *) "fmul", 0xA007 },
                { (char *) "fdiv", 0xA008 },
                { (char *) "fsqrt", 0xA009 },
                { (char *) "fcmp", 0xA00A },
                { (char *) "fcos", 0xA00C },
                { (char *) "fldpi", 0xA00D },
                { (char *) "fldz", 0xA00E },
                { (char *) "ftan", 0xA00F },
                { (char *) "fsine", 0xA010 },
                { (char *) "fsys", 0xA020 },
                /* insn_interrupt */
                { (char *) "int", 0xD001 },
                { (char *) "intcc", 0xD002 },    /* not present in x86 ISA */
                { (char *) "iret", 0xD003 },
                { (char *) "bound", 0xD004 },
                { (char *) "debug", 0xD005 },
                { (char *) "trace", 0xD006 },
                { (char *) "invalid_op", 0xD007 },
                { (char *) "oflow", 0xD008 },
                /* insn_system */
                { (char *) "halt", 0xE001 },
                { (char *) "in", 0xE002 },       /* input from port/bus */
                { (char *) "out", 0xE003 },      /* output to port/bus */
                { (char *) "cpuid", 0xE004 },
                /* insn_other */
                { (char *) "nop", 0xF001 },
                { (char *) "bcdconv", 0xF002 },  /* convert to or from BCD */
                { (char *) "szconv", 0xF003 },   /* change size of operand */
                { (char *) NULL, 0 }, //end
        };

        int i;

        //go thru every type in the enum
        for ( i = 0; types[i].name; i++ )
                if ( S == types[i].name ) return types[i].value;

        return 0;
}


int re_acss_expression::get_insn_operandtype_fromstr(QString S)
{
    char *types[] = {      /* mutually exclusive */
            (char *) "unused",  // = 0,          /* empty/unused operand: should never occur */
            (char *) "register", // = 1,        /* CPU register */
            (char *) "immediate", // = 2,       /* Immediate Value */
            (char *) "relative_near", // = 3,   /* Relative offset from IP */
            (char *) "relative_far", // = 4,    /* Relative offset from IP */
            (char *) "absolute", // = 5,        /* Absolute address (ptr16:32) */
            (char *) "expression", // = 6,      /* Address expression (scale/index/base/disp) */
            (char *) "offset", // = 7,          /* Offset from start of segment (m32) */
            (char *) "unknown" // = 8
    };
    for(int i=0; i<8; i++) if(S == types[i]) return i;
    return 0;
}


int re_acss_expression::get_insn_regid_fromstr(QString S)
{
    char *regs[] = {      /* mutually exclusive */
                          (char *) "none",
                          (char *) "eax",
                          (char *) "ecx",
                          (char *) "edx",
                          (char *) "ebx",
                          /* REG_ESP_INDEX */
                          (char *) "esp",
                          (char *) "ebp",
                          (char *) "esi",
                          (char *) "edi",
                          /* REG_WORD_OFFSET */
                          (char *) "ax",
                          (char *) "cx",
                          (char *) "dx",
                          (char *) "bx",
                          (char *) "sp",
                          (char *) "bp",
                          (char *) "si",
                          (char *) "di",
                          /* REG_BYTE_OFFSET */
                          (char *) "al",
                          (char *) "cl",
                          (char *) "dl",
                          (char *) "bl",
                          (char *) "ah",
                          (char *) "ch",
                          (char *) "dh",
                          (char *) "bh",
                          /* REG_MMX_OFFSET */
                          (char *) "mm0",
                          (char *) "mm1",
                          (char *) "mm2",
                          (char *) "mm3",
                          (char *) "mm4",
                          (char *) "mm5",
                          (char *) "mm6",
                          (char *) "mm7",
                          /* REG_SIMD_OFFSET */
                          (char *) "xmm0",
                          (char *) "xmm1",
                          (char *) "xmm2",
                          (char *) "xmm3",
                          (char *) "xmm4",
                          (char *) "xmm5",
                          (char *) "xmm6",
                          (char *) "xmm7",
                          /* REG_DEBUG_OFFSET */
                          (char *) "dr0",
                          (char *) "dr1",
                          (char *) "dr2",
                          (char *) "dr3",
                          (char *) "dr4",
                          (char *) "dr5",
                          (char *) "dr6",
                          (char *) "dr7",
                          /* REG_CTRL_OFFSET */
                          (char *) "cr0",
                          (char *) "cr1",
                          (char *) "cr2",
                          (char *) "cr3",
                          (char *) "cr4",
                          (char *) "cr5",
                          (char *) "cr6",
                          (char *) "cr7",
                          /* REG_TEST_OFFSET */
                          (char *) "tr0",
                          (char *) "tr1",
                          (char *) "tr2",
                          (char *) "tr3",
                          (char *) "tr4",
                          (char *) "tr5",
                          (char *) "tr6",
                          (char *) "tr7",
                          /* REG_SEG_OFFSET */
                          (char *) "es",
                          (char *) "cs",
                          (char *) "ss",
                          (char *) "ds",
                          (char *) "fs",
                          (char *) "gs",
                          /* REG_LDTR_INDEX */
                          (char *) "ldtr",
                          /* REG_GDTR_INDEX */
                          (char *) "gdtr",
                          /* REG_FPU_OFFSET */
                          (char *) "st(0)",
                          (char *) "st(1)",
                          (char *) "st(2)",
                          (char *) "st(3)",
                          (char *) "st(4)",
                          (char *) "st(5)",
                          (char *) "st(6)",
                          (char *) "st(7)",
                          /* REG_FLAGS_INDEX : 81 */
                          (char *) "eflags",
                          /* REG_FPCTRL_INDEX  : 82*/
                          (char *) "fpctrl",
                          /* REG_FPSTATUS_INDEX : 83*/
                          (char *) "fpstat",
                          /* REG_FPTAG_INDEX : 84 */
                          (char *) "fptag",
                          /* REG_EIP_INDEX : 85 */
                          (char *) "eip",
                          /* REG_IP_INDEX : 86 */
                          (char *) "ip",
                          /* REG_IDTR_INDEX : 87 */
                          (char *) "idtr",
                          /* REG_MXCSG_INDEX : SSE Control Reg : 88 */
                          (char *) "mxcsr",
                          /* REG_TR_INDEX : Task Register : 89 */
                          (char *) "tr",
            /* REG_CSMSR_INDEX : SYSENTER_CS_MSR : 90 */
            (char *) "cs_msr",
            /* REG_ESPMSR_INDEX : SYSENTER_ESP_MSR : 91 */
            (char *) "esp_msr",
            /* REG_EIPMSR_INDEX : SYSENTER_EIP_MSR : 92 */
            (char *) "eip_msr"
    };

    for(int i=0; i<92; i++) if(S == regs[i]) return i;
    return 0;
}



bool re_acss_expression::process_firstpass(QString exp, QString &exp2)
{
    QString S2, S3;
    int pos=0, flags_val = 0, prefix_val = 0, op_type = 0;
    int breakme = 0;
    exp2 = exp;
    QRegExp rx;

    // -- process flags value --
    rx.setPattern("\\s*(flags_set|flags_tested)\\s*[\\!=]=\\s*(\\(.+\\))\\s*[,;]{1,}");
    rx.setMinimal(TRUE);

    while ( ((pos = rx.indexIn(exp2, pos) ) != -1) && !breakme )  {
        S2 = rx.cap(2);
        if(S2.contains("carry_set")) flags_val |= insn_carry_set;
        if(S2.contains("zero_set")) flags_val |= insn_zero_set;
        if(S2.contains("oflow_set")) flags_val |= insn_oflow_set;
        if(S2.contains("dir_set")) flags_val |= insn_dir_set;
        if(S2.contains("sign_set")) flags_val |= insn_sign_set;
        if(S2.contains("parity_set")) flags_val |= insn_parity_set;
        if(S2.contains("carry_or_zero_set")) flags_val |= insn_carry_or_zero_set;
        if(S2.contains("zero_set_or_sign_ne_oflow")) flags_val |= insn_zero_set_or_sign_ne_oflow;
        if(S2.contains("carry_clear")) flags_val |= insn_carry_clear;
        if(S2.contains("zero_clear")) flags_val |= insn_zero_clear;
        if(S2.contains("oflow_clear")) flags_val |= insn_oflow_clear;
        if(S2.contains("dir_clear")) flags_val |= insn_dir_clear;
        if(S2.contains("sign_clear")) flags_val |= insn_sign_clear;
        if(S2.contains("parity_clear")) flags_val |= insn_parity_clear;

        if(S2.contains("|")) S3 = "|";
        if(S2.contains("&")) S3 = "&";

        exp2.replace(S2, S3 + QString::number(flags_val));
        pos+= rx.matchedLength();
    }

    // -- process prefix value --
    rx.setPattern("\\s*(prefix)\\s*[\\!=]=\\s*(.+)\\s*[,;]{1,}");
    rx.setMinimal(TRUE);
    pos = 0;
    while ( (pos = rx.indexIn(exp2, pos) ) != -1) {
        S2 = rx.cap(2);
        if(S2.contains("no_prefix")) prefix_val = insn_no_prefix;
        if(S2.contains("rep_zero")) prefix_val = insn_rep_zero;
        if(S2.contains("rep_notzero")) prefix_val = insn_rep_notzero;
        if(S2.contains("lock")) prefix_val = insn_lock;

        exp2.replace(S2, QString::number(prefix_val));
        pos+= rx.matchedLength();
    }

    // -- process op type --
    rx.setPattern("\\s*(op[123]_type)\\s*[\\!=]=\\s*(.+)\\s*[,;]{1,}");
    rx.setMinimal(TRUE);
    pos = 0; op_type = 0;
    while ( (pos = rx.indexIn(exp2, pos) ) != -1) {
        S2 = rx.cap(2);
        if(S2.contains("unused")) op_type = op_unused;
        if(S2.contains("register")) op_type = op_register;
        if(S2.contains("immediate")) op_type = op_immediate;
        if(S2.contains("relative_near")) op_type = op_relative_near;
        if(S2.contains("relative_far")) op_type = op_relative_far;
        if(S2.contains("absolute")) op_type = op_absolute;
        if(S2.contains("expression")) op_type = op_expression;
        if(S2.contains("offset")) op_type = op_offset;
        if(S2.contains("unknown")) op_type = op_unknown;

        exp2.replace(S2, QString::number(op_type));
        pos+= rx.matchedLength();
    }


    // -- process op datatype --
    rx.setPattern("\\s*(op[123]_datatype)\\s*[\\!=]=\\s*(.+)\\s*[,;]{1,}");
    rx.setMinimal(TRUE);
    pos = 0; op_type = 0;
    while ( (pos = rx.indexIn(exp2, pos) ) != -1) {
        S2 = rx.cap(2);
        if(S2.contains("byte")) op_type = op_byte;
        if(S2.contains("word")) op_type = op_word;
        if(S2.contains("dword")) op_type = op_dword;
        if(S2.contains("qword")) op_type = op_qword;
        if(S2.contains("dqword")) op_type = op_dqword;
        if(S2.contains("sreal")) op_type = op_sreal;
        if(S2.contains("dreal")) op_type = op_dreal;
        if(S2.contains("extreal")) op_type = op_extreal;
        if(S2.contains("bcd")) op_type = op_bcd;
        if(S2.contains("ssimd")) op_type = op_ssimd;
        if(S2.contains("dsimd")) op_type = op_dsimd;
        if(S2.contains("sssimd")) op_type = op_sssimd;
        if(S2.contains("sdsimd")) op_type = op_sdsimd;
        if(S2.contains("descr32")) op_type = op_descr32;
        if(S2.contains("descr16")) op_type = op_descr16;
        if(S2.contains("pdescr32")) op_type = op_pdescr32;
        if(S2.contains("pdescr16")) op_type = op_pdescr16;
        if(S2.contains("bounds16")) op_type = op_bounds16;
        if(S2.contains("bounds32")) op_type = op_bounds32;
        if(S2.contains("fpuenv16")) op_type = op_fpuenv16;
        if(S2.contains("fpuenv32")) op_type = op_fpuenv32;
        if(S2.contains("fpustate16")) op_type = op_fpustate16;
        if(S2.contains("fpustate32")) op_type = op_fpustate32;
        if(S2.contains("fpregset")) op_type = op_fpregset;
        if(S2.contains("fpreg")) op_type = op_fpreg;

        exp2.replace(S2, QString::number(op_type));
        pos+= rx.matchedLength();
    }

    // -- process op access value --
    rx.setPattern("\\s*(op[123]_access)\\s*[\\!=]=\\s*(\\(.+\\))\\s*[,;]{1,}");
    rx.setMinimal(TRUE);
    pos = 0; flags_val = 0;
    while ( (pos = rx.indexIn(exp2, pos) ) != -1) {
        S2 = rx.cap(2);
        if(S2.contains("read")) flags_val |= op_read;
        if(S2.contains("write")) flags_val |= op_write;
        if(S2.contains("execute")) flags_val |= op_execute;

        if(S2.contains("|")) S3 = "|";
        if(S2.contains("&")) S3 = "&";

        exp2.replace(S2, S3 + QString::number(flags_val));
        pos+= rx.matchedLength();
    }

    // -- process op access value --
    rx.setPattern("\\s*(op[123]_reg_type)\\s*[\\!=]=\\s*(\\(.+\\))\\s*[,;]{1,}");
    rx.setMinimal(TRUE);
    pos = 0; flags_val = 0;
    while ( (pos = rx.indexIn(exp2, pos) ) != -1) {
        S2 = rx.cap(2);
        if(S2.contains("reg_gen")) flags_val |= reg_gen;
        if(S2.contains("reg_in")) flags_val |= reg_in;
        if(S2.contains("reg_out")) flags_val |= reg_out;
        if(S2.contains("reg_local")) flags_val |= reg_local;
        if(S2.contains("reg_fpu")) flags_val |= reg_fpu;
        if(S2.contains("reg_seg")) flags_val |= reg_seg;
        if(S2.contains("reg_simd")) flags_val |= reg_simd;
        if(S2.contains("reg_sys")) flags_val |= reg_sys;
        if(S2.contains("reg_sp")) flags_val |= reg_sp;
        if(S2.contains("reg_fp")) flags_val |= reg_fp;
        if(S2.contains("reg_pc")) flags_val |= reg_pc;
        if(S2.contains("reg_retaddr")) flags_val |= reg_retaddr;
        if(S2.contains("reg_cond")) flags_val |= reg_cond;
        if(S2.contains("reg_zero")) flags_val |= reg_zero;
        if(S2.contains("reg_ret")) flags_val |= reg_ret;
        if(S2.contains("reg_src")) flags_val |= reg_src;
        if(S2.contains("reg_dest")) flags_val |= reg_dest;
        if(S2.contains("reg_count")) flags_val |= reg_count;

        if(S2.contains("|")) S3 = "|";
        if(S2.contains("&")) S3 = "&";

        exp2.replace(S2, S3 + QString::number(flags_val));
        pos+= rx.matchedLength();
    }

    return true;
}

QString re_acss_expression::get_error_string()
{
    return error_string;
}
