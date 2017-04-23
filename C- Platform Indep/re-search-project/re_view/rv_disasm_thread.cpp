#include "rv_disasm_thread.h"

rv_disasm_thread::rv_disasm_thread(QObject *parent) :
    QThread(parent)
{
}

rv_disasm_thread::~rv_disasm_thread()
{
}

void rv_disasm_thread::run()
{
    emit progress_changed_to(0);
    t->disassembler->full_analysis();
    emit progress_changed_to(100);
}


void rv_disasm_thread::set_target(re_target *target)
{
    t = target;
}
