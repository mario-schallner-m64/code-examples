#include "rv_file_analyze_thread.h"

rv_file_analyze_thread::rv_file_analyze_thread(QObject *parent) :
    QThread(parent)
{
}

rv_file_analyze_thread::~rv_file_analyze_thread()
{
}

void rv_file_analyze_thread::run()
{
    emit progress_changed_to(0);
    f->load_file(f->file_name);
    emit progress_changed_to(100);
    if(!f->strings_scan_on_load) {
        f->scan_ascii_strings();
    }
}


void rv_file_analyze_thread::set_file(re_file *file)
{
    f = file;
}
