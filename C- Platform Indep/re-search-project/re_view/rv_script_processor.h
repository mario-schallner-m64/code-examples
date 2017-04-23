#ifndef RV_SCRIPT_PROCESSOR_H
#define RV_SCRIPT_PROCESSOR_H

#include <QObject>
#include <QtScript/QtScript>
#include <QTextEdit>
#include <QtScriptTools/QtScriptTools>
#include <re_core.h>
#include <rv_script_execution_thread.h>
#include <rv_hex_edit.h>

class rv_hex_editor;

Q_DECLARE_METATYPE(x86_insn_t)
Q_DECLARE_METATYPE(x86_op_t)
Q_DECLARE_METATYPE(x86_reg_t)

enum {
    RV_SCRIPT_FUNCTION_CORE,
    RV_SCRIPT_FUNCTION_DISASSEMBLY,
    RV_SCRIPT_FUNCTION_IO,
    RV_SCRIPT_FUNCTION_FILE_IO,
    RV_SCRIPT_FUNCTION_BUFFER
};


struct rv_script_function_desc {
    QString name;
    QString arguments;
    int     type;
    QString desc;
    QString example;
    QString example_result;
};

class rv_script_processor : public QObject
{
    Q_OBJECT
public:
    explicit rv_script_processor(QObject *parent = 0);
    ~rv_script_processor();
    QScriptEngine *engine;

public slots:
    rv_script_execution_thread *run_script_in_thread(QString &script);
    void set_output_textbox(QPlainTextEdit *outbox);
    void set_target(re_target *target);
    void set_hex_editor(rv_hex_editor *h);
    void set_current_buffer(rv_hex_edit *hv);

signals:
    void processor_thread_terminated(QScriptValue result);

private:
    void init_engine();
    rv_script_execution_thread *exec_thread;
};



#endif // RV_SCRIPT_PROCESSOR_H
