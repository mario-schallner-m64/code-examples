#include "re_logger.h"

re_logger::re_logger(QObject *parent) :
    QObject(parent)
{
    log_file = 0;
    log_stream = 0;
    debug_level = 0;

    use_stdout = true;
}

bool re_logger::open_log_file(const QString &filename)
{
    log_file = new QFile(filename);

    if (!log_file->open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    log_stream = new QTextStream(log_file);

    return true;
}

void re_logger::set_debug_level(int level)
{
    debug_level = level;
}

void re_logger::log_message(const QString &msg, const QString &header) const
{
    if(!header.isEmpty()) {
        if(log_stream) {
            *log_stream << header << " " << msg << "\n";
            //log_stream->flush();
        }

        if(use_stdout) fprintf(stdout, "%s",
                               (header + " " + msg + "\n").toAscii().data());
    }
    else {
        if(log_stream) {
            *log_stream << msg << "\n";
            //log_stream->flush();
        }

        if(use_stdout) fprintf(stdout, "%s",(msg + "\n").toAscii().data());
    }
}

void re_logger::flush_stream() const
{
    if(log_stream) log_stream->flush();
}

void re_logger::log_info(const QString &msg) const
{
    log_message(msg, "[info   ]");
}

void re_logger::log_warning(const QString &msg) const
{
    log_message(msg, "[warning]");
}

void re_logger::log_error(const QString &msg) const
{
    log_message(msg, "[error  ]"); flush_stream();
}

void re_logger::log_debug(const QString &msg, int level) const
{
    if(level > debug_level) return;
    log_message(msg, "[debug|" + QString::number(level) + "]");
}

void re_logger::log_event(const int level, const QString &msg) const
{
    switch(level) {
    case RE_LOG_NORMAL:
        log_info(msg);
        break;

    case RE_LOG_WARNING:
        log_warning(msg);
        break;

    case RE_LOG_ERROR:
        log_error(msg);
        break;

    case RE_LOG_DEBUG_1:
    case RE_LOG_DEBUG_2:
    case RE_LOG_DEBUG_3:
        log_debug(msg, level);
        break;

    default:
        log_info(msg);
        break;
    }
}
