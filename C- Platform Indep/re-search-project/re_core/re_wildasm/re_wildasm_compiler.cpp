#include "re_wildasm_compiler.h"

re_wildasm_compiler::re_wildasm_compiler(QObject *parent) :
    QObject(parent)
{
}

void re_wildasm_compiler::init(re_logger *l)
{
    logger = l;
    connect(this, SIGNAL(msg(int, QString)),
        logger, SLOT(log_event(int, QString)));
}

void re_wildasm_compiler::set_filter(char *exp)
{
    text = QString::fromAscii(exp);
}

char *re_wildasm_compiler::get_text()
{
    return text.toAscii().data();
}

bool re_wildasm_compiler::compile()
{
    compiled_lines = 0;
    acss_filter.clear();

    if(text.isEmpty()) {
        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile: filter is empty");
        return false;
    }

    QString line, acss_line;
    QTextStream wstream(&text);

    while(!wstream.atEnd()) {
        line = wstream.readLine();
        acss_line.clear();

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile: parsing line: '" + line + "'");

        if(compile_line_to_acss(line, acss_line)) {
            compiled_lines++;
            emit msg(RE_LOG_DEBUG_3,
                "re_wildasm_compiler::compile: successfully compiled line to: '"
                     + acss_line + "'");
            acss_filter+= acss_line ;
        }
    }

    if(!acss_filter.isEmpty())  {
        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile: successfully compiled to acss: '"
                 + acss_filter + "'");
        return true;
    }

    return false;
}

bool re_wildasm_compiler::compile_line_to_acss(const QString &line, QString &acss_line)
{
    acss_line.clear();
    if(line.isEmpty())  return false; // emty line

    QString insn_str, op1_str, op2_str, my_line, my_acss_line;
    int mincount=1, maxcount=1;
    bool parsed_line = false;

    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("^\\s*;");
    if(rx.indexIn(line, 0)  != -1) return false; // comment only

    rx.setPattern("(.+);");
    if(rx.indexIn(line, 0)  != -1) {
        my_line = rx.cap(1);
    } else my_line = line;

    // -- 2 operands line with count
    rx.setPattern("\\s*(\\*|\\b.+\\b)\\s+(\\*|\\b.+\\b)\\s*,\\s*(\\*|\\b.+\\b)\\s*\\(\\s*([0123456789]+)\\s*-\\s*([0123456789]+)\\s*\\)");
    if(rx.indexIn(my_line, 0)  != -1) {
        insn_str = rx.cap(1);
        op1_str  = rx.cap(2);
        op2_str  = rx.cap(3);
        mincount = rx.cap(4).toInt();
        maxcount = rx.cap(5).toInt();

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile_line_to_acss: 2 args insn: '"
             + insn_str
             + "' op1: '"
             + op1_str
             + "' op2: '"
             + op2_str
             + "'"
             + (QString)" min: " + QString::number(mincount)
             +          " max: " + QString::number(maxcount)
                 );
        parsed_line = true;
    }

    // -- 1 operand1 line with count
    rx.setPattern("\\s*(\\*|\\b.+\\b)\\s+(\\*|\\b.+\\b)\\s*\\(\\s*([0123456789]+)\\s*-\\s*([0123456789]+)\\s*\\)");
    if(!parsed_line && rx.indexIn(my_line, 0)  != -1) {
        insn_str = rx.cap(1);
        op1_str  = rx.cap(2);
        op2_str.clear();
        mincount = rx.cap(3).toInt();
        maxcount = rx.cap(4).toInt();

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile_line_to_acss: 1 arg insn: '"
             + insn_str
             + "' op1: '"
             + op1_str + "'"
                 + (QString)" min: " + QString::number(mincount)
                 +          " max: " + QString::number(maxcount)
                 );
        parsed_line = true;
    }

    // -- 0 operand1 line with count
    rx.setPattern("\\s*(\\*|\\b.+\\b)\\s*\\(\\s*([0123456789]+)\\s*-\\s*([0123456789]+)\\s*\\)");
    if(!parsed_line && rx.indexIn(my_line, 0)  != -1) {
        insn_str = rx.cap(1);
        op1_str.clear();
        op2_str.clear();
        mincount = rx.cap(2).toInt();
        maxcount = rx.cap(3).toInt();

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile_line_to_acss: insn: '"
             + insn_str + "'"
                 + (QString)" min: " + QString::number(mincount)
                 +          " max: " + QString::number(maxcount)
                 );
    }


    // -- 2 operands line
    rx.setPattern("\\s*(\\*|\\b.+\\b)\\s+(\\*|\\b.+\\b)\\s*,\\s*(\\*|\\b.+\\b)");
    if(rx.indexIn(my_line, 0)  != -1) {
        insn_str = rx.cap(1);
        op1_str  = rx.cap(2);
        op2_str  = rx.cap(3);

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile_line_to_acss: 2 args insn: '"
             + insn_str
             + "' op1: '"
             + op1_str
             + "' op2: '"
             + op2_str
             + "'"
                 );
        parsed_line = true;
    }

    // -- 1 operand1 line
    rx.setPattern("\\s*(\\*|\\b.+\\b)\\s+(\\*|\\b.+\\b)");
    if(!parsed_line && rx.indexIn(my_line, 0)  != -1) {
        insn_str = rx.cap(1);
        op1_str  = rx.cap(2);
        op2_str.clear();

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile_line_to_acss: 1 arg insn: '"
             + insn_str
             + "' op1: '"
             + op1_str + "'"
                 );
        parsed_line = true;
    }

    // -- 0 operand1 line
    rx.setPattern("\\s*(\\*|\\b.+\\b)");
    if(!parsed_line && rx.indexIn(my_line, 0)  != -1) {
        insn_str = rx.cap(1);
        op1_str.clear();
        op2_str.clear();

        emit msg(RE_LOG_DEBUG_3,
            "re_wildasm_compiler::compile_line_to_acss: insn: '"
             + insn_str + "'"
                 );
    }

    // do it
    if(compile_wildasm_to_acss(insn_str, op1_str, op2_str, my_acss_line,
                               mincount, maxcount)) {
        acss_line += my_acss_line;
        return true;
    }

    return false;
}

bool re_wildasm_compiler::compile_wildasm_to_acss(const QString &mnem_str, 
                                                  const QString &op1_str,
                                                  const QString &op2_str,
                                                  QString &acss_line,
                                                  int mincnt, int maxcnt)
{
    QString acss_operand_str, op_error_str;
       
    acss_line.clear();
    
    // kind of mini pseudo 1st pass:
    QString my_mnem_str = mnem_str;
    
    if(mnem_str == "ret") my_mnem_str = "return";
    
    if(re_acss_expression::get_insn_type_fromstr(my_mnem_str)) {
        acss_line += QString::number(mincnt) + "-" +
                QString::number(maxcnt);
        acss_line += " x insn { type == " + my_mnem_str + " ";
    } else {
        if(my_mnem_str == "*") {
            acss_line += QString::number(mincnt) + "-" +
                    QString::number(maxcnt);
            acss_line += " x insn { type == ANY ";
        } else {
            error_string = "invalid mnemonic type";
            acss_line.clear();
            return false;
        }
    }
    
    if(!op1_str.isEmpty())
    if(parse_operand(op1_str, 1, acss_operand_str, op_error_str)) {
        acss_line += acss_operand_str;
    } else {
        error_string = op_error_str;
        acss_line.clear();
        return false;
    }
    
    if(!op2_str.isEmpty())
    if(parse_operand(op2_str, 2, acss_operand_str, op_error_str)) {
        acss_line += acss_operand_str;
    } else {
        error_string = op_error_str;
        acss_line.clear();
        return false;
    }

    acss_line += "; }; ";

    return true;
}

bool re_wildasm_compiler::parse_operand( const QString &wild_op_str, int op_num,
                        QString &acss_op_str, 
                        QString &err_str)
{
    acss_op_str.clear();
    
    if(re_acss_expression::get_insn_regid_fromstr(wild_op_str)) {
        acss_op_str = ", op" + QString::number(op_num) + "_type == register, " +
                      "op" + QString::number(op_num) + "_reg_name == "        
                      + wild_op_str + " ";
        return true;
    }

    if(re_acss_expression::get_insn_operandtype_fromstr(wild_op_str)) {
        acss_op_str = ", op" + QString::number(op_num) + "_type == " 
                      + wild_op_str + " ";
        return true;
    }
    
    if( wild_op_str == "*") return true;
    
    return false; 
}                      


QString re_wildasm_compiler::get_compiled_acss_str()
{
    return acss_filter;
}

