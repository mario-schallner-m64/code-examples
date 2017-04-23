#ifndef RV_FILE_ANALYZE_THREAD_H
#define RV_FILE_ANALYZE_THREAD_H

#include <QThread>
#include <re_file.h>

class rv_file_analyze_thread : public QThread
{
    Q_OBJECT
public:
    explicit rv_file_analyze_thread(QObject *parent = 0);
    ~rv_file_analyze_thread();
    void set_file(re_file *file);

    protected:
    void run();

signals:
    void progress_changed_to(int p);

private:
    re_file     *f;

};

#endif // RV_FILE_ANALYZE_THREAD_H
