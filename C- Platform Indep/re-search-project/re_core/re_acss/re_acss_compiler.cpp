#include "re_acss_compiler.h"

re_acss_compiler::re_acss_compiler(QObject *parent) :
    QObject(parent)
{
    blocked = false;
    block_size = 0;
    threshold = 0;
}

void re_acss_compiler::init(re_logger *l)
{
    logger = l;
    connect(this, SIGNAL(msg(int, QString)),
        logger, SLOT(log_event(int, QString)));

}

bool re_acss_compiler::is_block()
{
    return blocked;
}

void re_acss_compiler::set_filter(char *f)
{
    filter = QString::fromAscii(f);
}

char * re_acss_compiler::get_filter()
{
    return filter.toAscii().data();
}

bool re_acss_compiler::compile_expressions()
{
    emit msg(RE_LOG_DEBUG_3,
             "re_acss_compiler::compile_expressions(): starting");

    emit msg(RE_LOG_DEBUG_3,
             "acss filter: '" + filter + "'");

    QString exp_str, block_str;
    QRegExp rx;
    int pos = 0, count = 0, compiled = 0;
    re_acss_expression *expression;
    rx.setMinimal(TRUE);

    rx.setPattern("(block\\s*\\{\\s*size\\s*=\\s*[0123456789]+\\s*,\\s*threshold\\s*=\\s*[0123456789]+\\s*\\%\\s*;\\s*\\}\\s*;)");
    if((pos = rx.indexIn(filter) ) != -1) {
        block_str = rx.cap(1);

        // emit msg(RE_LOG_DEBUG_3, "block CAP1");

        rx.setPattern("block\\s*\\{\\s*size\\s*=\\s*([0123456789]+)\\s*,\\s*threshold\\s*=\\s*([01234567890]+)\\s*\\%\\s*;\\s*\\}\\s*;");
        if((pos = rx.indexIn(filter) ) != -1) {
            block_size = rx.cap(1).toULong();
            threshold = rx.cap(2).toUInt();

            if( (threshold > 1) && (threshold <= 100) && (block_size > 1) ) {
                blocked = true;
                emit msg(RE_LOG_DEBUG_1, "acss block mode enabled: block size ="
                         + QString::number(block_size)
                         + " threshold = "
                         + QString::number(threshold) + "%"
                         );
            }
            else {
                threshold = 0;
                block_size = 0;
                blocked = false;
                block_str.clear();
            }
        }
    }

    pos = 0;

    rx.setPattern(
                "([\\*0123456789]+\\-*[0123456789]*\\s*x\\s*insn\\s*\\{.+\\})");

    expressions.clear();

    while ( (pos = rx.indexIn(filter, pos) ) != -1) {
        exp_str = rx.cap(1);
        count++;

        if(!block_str.isEmpty()) {
            exp_str.prepend(block_str + " ");
            block_str.clear();
        }

        emit msg(RE_LOG_DEBUG_3,
                 "re_acss_compiler::compile_expressions(): found expr: '"
                 + exp_str + "'");
        emit msg(RE_LOG_DEBUG_3,
                 "re_acss_compiler::compile_expressions(): compiling ...");

        expression = new re_acss_expression();
        expression->init(logger);

        expression->set_text(exp_str.toAscii().data());
        if( expression->compile() ) {
            expressions.append(expression);
            emit msg(RE_LOG_DEBUG_3,
"re_acss_compiler::compile_expressions(): expression compiled successfully");
            compiled++;
        }
        else {
            emit msg(RE_LOG_ERROR, expression->get_error_string());
            return false;
        }

        pos += rx.matchedLength();
    }

    if(!compiled) {
        emit msg(RE_LOG_ERROR, "no valid expression found in '"
                 + filter + "'");
        return false;
    }
    emit msg(RE_LOG_DEBUG_3,
       "re_acss_compiler::compile_expressions(): # of compiled expressions: "
             +QString::number(compiled));

    emit msg(RE_LOG_DEBUG_3,
       "re_acss_compiler::compile_expressions(): done");

    return true;
}
