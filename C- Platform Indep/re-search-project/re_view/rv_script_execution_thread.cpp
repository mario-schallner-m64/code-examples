#include "rv_script_execution_thread.h"

rv_script_execution_thread::rv_script_execution_thread(QScriptEngine *eng,
                                                       const QString &script,
                                                       QObject *parent) :
    QThread(parent)
{
    engine = eng;
    my_script = script;
}

rv_script_execution_thread::~rv_script_execution_thread()
{
}

void rv_script_execution_thread::run()
{
    QScriptValue result = engine->evaluate(my_script);
    emit thread_terminated(result);
}



