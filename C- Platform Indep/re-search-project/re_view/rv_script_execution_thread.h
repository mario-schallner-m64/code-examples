#ifndef RV_SCRIPT_EXECUTION_THREAD_H
#define RV_SCRIPT_EXECUTION_THREAD_H

#include <QThread>
#include <QtScript/QtScript>
#include <QtScriptTools/QtScriptTools>

class rv_script_execution_thread : public QThread
{
    Q_OBJECT
public:
    explicit rv_script_execution_thread(QScriptEngine *eng, const QString &script, QObject *parent = 0);
    ~rv_script_execution_thread();

protected:
    void run();

signals:
    void thread_terminated(QScriptValue);

public slots:

private:
    QScriptEngine   *engine;
    QString         my_script;
};

#endif // RV_SCRIPT_EXECUTION_THREAD_H
