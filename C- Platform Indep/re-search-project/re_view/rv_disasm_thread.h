#ifndef RV_DISASM_THREAD_H
#define RV_DISASM_THREAD_H

#include <QThread>
#include <re_target.h>

class rv_disasm_thread : public QThread
{
    Q_OBJECT
public:
    explicit rv_disasm_thread(QObject *parent = 0);
    ~rv_disasm_thread();
    void set_target(re_target *target);

protected:
    void run();

signals:
    void progress_changed_to(int p);

private:
    re_target *t;
};

#endif // RV_DISASM_THREAD_H
