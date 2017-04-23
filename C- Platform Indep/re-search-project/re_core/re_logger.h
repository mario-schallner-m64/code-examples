#ifndef RE_LOGGER_H
#define RE_LOGGER_H

#include "re_core_global.h"

#include <QObject>
#include <QFile>
#include <QTextStream>

class RE_CORESHARED_EXPORT re_logger : public QObject
{
    Q_OBJECT
public:
    explicit re_logger(QObject *parent = 0);

public slots:

    // other classes should only connect to log_event()
    void log_event(const int level, const QString &msg) const;

    // or to log_debug();
    void log_debug(const QString &msg, int level) const;
    void set_debug_level(int level);

    // do not use these unless you really need it
    void log_message(const QString &msg, const QString &header) const;
    void log_info(const QString &msg) const;
    void log_warning(const QString &msg) const;
    void log_error(const QString &msg) const;
    void flush_stream() const;

    bool open_log_file(const QString &filename);

private:
    QFile       *log_file;
    QTextStream *log_stream;
    int         debug_level;

    bool        use_stdout;
};

#endif // RE_LOGGER_H
