#include "re_file_raw32.h"
#include "re_file.h"

re_file_raw32::re_file_raw32(QObject *parent) :
    QObject(parent)
{
}

bool re_file_raw32::check_file_type(re_file *f)
{
    return true;
}

bool re_file_raw32::analyze_file(re_file *f)
{
    f->msg_helper(RE_LOG_NORMAL, "raw32: starting to analyze");

    // entry point
    f->va_entry_point = 0;
    f->msg_helper(RE_LOG_NORMAL, "entry point at 0x00000000");

    re_file_segment tmp_segment;
    tmp_segment.va_start   = 0;
    tmp_segment.va_end     = f->stat_struct.st_size;
    tmp_segment.file_offset = 0;
    tmp_segment.file_size   = f->stat_struct.st_size;
    tmp_segment.mem_size    = f->stat_struct.st_size;
    tmp_segment.flags = RE_SEGMENT_LOAD;
    tmp_segment.struct_ptr  = (char *)f->image;
    tmp_segment.data_ptr    = f->image;
    f->segments.append(tmp_segment);
    f->msg_helper(RE_LOG_DEBUG_2, "+ adding pheader #0"
            + (QString)" (0x" + QString::number(tmp_segment.va_start, 16)
            + " - 0x" + QString::number(tmp_segment.va_end, 16) + ")"
           );

    return true;
}
