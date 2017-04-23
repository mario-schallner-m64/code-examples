#include "re_file.h"

re_file::re_file(QObject *parent) :
    QObject(parent)
{
    image = 0;
    max_ptr = 0;
    va_entry_point = BAD_ADDR;
    va_custom_ep   = BAD_ADDR;
    file_type = RE_FILE_TYPE_UNKNOWN;
    status = RE_FILE_STATUS_NONE;
    filesize = 0;
    strings_scan_on_load = true;

    // ELF Specific
    ELF_Hdr = 0;
}

re_file::~re_file()
{
    if(image) free(image);
}

void re_file::msg_helper(int level, const QString &message)
{
    emit msg(level, message);
}

void re_file::pct_helper(int pct)
{
    emit file_load_pct(pct);
}


bool re_file::load_file(const QString &name)
{
    // get file attributes (size, ...)
    emit msg(RE_LOG_DEBUG_1, "re_file: loading file '" + name + "'");

    int fd;
    if( (  (fd = open(name.toAscii().data(), O_RDONLY)) == -1)
            || (fstat(fd, &stat_struct)         == -1) ) {
        emit msg(RE_LOG_ERROR, "unable to stat file: '" + name + "'");
        return false;
    }
    close(fd);

    filesize = stat_struct.st_size;

    // load, and not map file
    image       = (unsigned char *) calloc(stat_struct.st_size, 1);
    if ( image == (void*) -1 ) {
        emit msg(RE_LOG_ERROR, "unable to calloc file buffer");
        return false;
    }
    FILE *f = fopen(name.toAscii().data(), "rb");
    if(!f) {
        emit msg(RE_LOG_ERROR, "unable to fopen file'" + name + "'");
        return false;
    }
    if( fread(image, 1, stat_struct.st_size, f) !=
            (unsigned long) stat_struct.st_size ) {
        emit msg(RE_LOG_ERROR, "unable to read file into ram");
        return false;
    }
    fclose(f);

    // for quick pointer sanity checks
    max_ptr = (char *)image + stat_struct.st_size;

    file_name = name;
    status = RE_FILE_STATUS_LOADED;
    emit msg(RE_LOG_NORMAL, "successfully loaded file: '" + name + "'");

    int analyze_result;
    switch(file_type = get_file_type()) {

    case RE_FILE_TYPE_ELF32:
        emit msg(RE_LOG_NORMAL, "detected file type: elf32");
        analyze_result = re_file_elf32::analyze_file(this);
        break;

    case RE_FILE_TYPE_PE32:
        emit msg(RE_LOG_NORMAL, "detected file type: pe32");
        analyze_result = re_file_pe32::analyze_file(this);
        break;

    default:
        file_type = RE_FILE_TYPE_RAW32;
        emit msg(RE_LOG_NORMAL, "detected file type: raw32");
        analyze_result = re_file_raw32::analyze_file(this);
        break;
    }

    if(strings_scan_on_load) {
        scan_ascii_strings();
        scan_unicode_strings();
    }

    if(!analyze_result) emit msg(RE_LOG_WARNING, "unable to analyze this file");
    else status = RE_FILE_STATUS_ANALYZED;

    return true;
}

bool re_file::file_loaded() const
{
    if(image) return true;
    return false;
}

int re_file::get_file_type()
{
    if(re_file_elf32::check_file_type(this)) return RE_FILE_TYPE_ELF32;
    if(re_file_pe32::check_file_type(this)) return RE_FILE_TYPE_PE32;

    return RE_FILE_TYPE_UNKNOWN;
}

// ---

re_addr_t re_file::file_offset_to_va(re_addr_t offset) const
{
    int segcount = segments.count();
    if(!segcount) return offset;        // for raw files

    for(int i=0; i < segcount; i++)
        if( (segments.at(i).file_offset <= offset) &&
           ((segments.at(i).file_offset + segments.at(i).file_size) >= offset))

            return offset-segments.at(i).file_offset + segments.at(i).va_start;

    return BAD_ADDR;
}

re_addr_t re_file::va_to_file_offset(re_addr_t va) const
{
    int segcount = segments.count();

    if(!segcount)  {
        // for raw files
        if (va < (re_addr_t)stat_struct.st_size) return va;
        else return BAD_ADDR;
    }

    for(int i=0; i < segcount; i++)
        if((segments.at(i).va_start <= va) && (segments.at(i).va_end >= va))
            return va - segments.at(i).va_start + segments.at(i).file_offset;

    return BAD_ADDR;
}

// returns offset into segment, not into file
re_addr_t re_file::va_to_segment_offset(re_file_segment *seg, re_addr_t va) const
{
    int segcount = segments.count();

    if(!segcount) {// for raw files: return whole file as segment
        seg->file_offset = 0;
        seg->file_size = stat_struct.st_size;
        seg->mem_size = stat_struct.st_size;
        seg->va_end = stat_struct.st_size;
        seg->va_start = 0;
        seg->struct_ptr = 0;
        seg->type = 0;
        seg->data_ptr = (unsigned char *) image;

        if(va <= (re_addr_t)stat_struct.st_size) return va;

        return BAD_ADDR;
    }

    for(int i=0; i < segcount; i++)
        if( (segments.at(i).va_start <= va) &&
                (segments.at(i).va_end >= va)
          ) {
            seg->data_ptr = segments.at(i).data_ptr;
            seg->file_offset = segments.at(i).file_offset;
            seg->file_size = segments.at(i).file_size;
            seg->flags = segments.at(i).flags;
            seg->mem_size = segments.at(i).mem_size;
            seg->va_end = segments.at(i).va_end;
            seg->va_start = segments.at(i).va_start;
            seg->struct_ptr = segments.at(i).struct_ptr;
            seg->type = segments.at(i).type;

            return va - segments.at(i).va_start;
        }

    return BAD_ADDR;
}

unsigned char *re_file::va_to_data_ptr(re_addr_t va) const
{
    re_addr_t offset = va_to_file_offset(va);
    if(offset != BAD_ADDR) return (unsigned char *)image + offset;

    return (unsigned char *) BAD_ADDR;
}

unsigned char re_file::get_byte_at_va(re_addr_t va) const
{
    re_file_segment seg;
    re_addr_t offset;

    if( (offset = va_to_segment_offset(&seg, va)) != BAD_ADDR ) {
        return *((unsigned char *)(seg.data_ptr + offset));
    }
    else return (unsigned char) BAD_ADDR;
}

unsigned long re_file::get_dword_at_va(re_addr_t va) const
{
    re_file_segment seg;
    re_addr_t offset;

    if( (offset = va_to_segment_offset(&seg, va)) != BAD_ADDR ) {
        return *((unsigned long *)(seg.data_ptr + offset));
    }
    else return BAD_ADDR;
}

unsigned char re_file::get_byte_at_offset(re_addr_t offset)
{
    if(offset >= filesize) return 0xff;

    return (unsigned char)image[offset];
}

unsigned long re_file::get_dword_at_offset(re_addr_t offset)
{
    if(offset >= filesize) return BAD_ADDR;

    unsigned long *lptr = (unsigned long *)(image + offset);
    return *lptr;
}

void re_file::scan_ascii_strings()
{
    char *c;
    char *file_end;
    char *str_start;
    char *file_start;
    int str_len;
    re_addr_t str_va;
    char *new_str;
    char tmp_str[100];

    file_start = (char *) image;
    c = file_start;

    file_end = (char *)image + stat_struct.st_size;

    emit msg(RE_LOG_NORMAL, "scanning for ascii strings ...");
    emit strings_scan_pct(0);

    re_addr_t emit_pct_at = filesize / 100;
    re_addr_t pos = 0;

    while( c < file_end) {
        str_start = c;
        str_len = 0;
        while(
		 ( ((str_start[str_len] >= 32) && (str_start[str_len] <=126)) ||
			  (str_start[str_len] == 0x0a || str_start[str_len] == 0x0d
			  || str_start[str_len] == 0x09)
		 )
            && ((str_start + str_len) < file_end)) str_len++;
        if(str_len > 3) {
            new_str = (char *) malloc(str_len + 1);
            strncpy(new_str, str_start, str_len);
            new_str[str_len] = 0x0;
            tmp_str[0] = 0x0;
            strncpy(tmp_str, new_str, 34);
            tmp_str[34] = 0x0;
            if((int)strlen(tmp_str) < str_len) strcat(tmp_str, " ...");

            str_va = file_offset_to_va(
                        (re_addr_t)str_start - (re_addr_t)file_start);

            if(str_va != BAD_ADDR) {
				va_cstrings_hash.insert(str_va, new_str);
				emit msg(RE_LOG_DEBUG_2, "+ adding string '"
						+ QString::fromAscii(tmp_str) + "' at 0x"
						+ QString::number(str_va, 16));
			} else {
				emit msg(RE_LOG_DEBUG_2, "- skipping string '"
						+ QString::fromAscii(tmp_str) + "'");				
			}
            c+=str_len;
            pos+=str_len;
            continue;
        }
        pos++;
        c++;
        if(!(pos % emit_pct_at)) emit strings_scan_pct(pos / emit_pct_at);
    }
    emit msg(RE_LOG_NORMAL, "ascii strings scan finished");
}

void re_file::scan_unicode_strings()
{
    emit msg(RE_LOG_NORMAL, "scanning for unicode strings ...");
    emit msg(RE_LOG_NORMAL, "unicode strings scan finished");
}

void re_file::set_strings_scan_on_load(bool tf)
{
    strings_scan_on_load = tf;
}

void re_file::set_filename(QString fn)
{
    file_name = fn;
}

// ---- safe access functions ----

// return file offset from va
// input:  va
// output: offs, bool retval
// if va out of any bounds, function returns false
// if filetype == raw, and va corresponds to a valid offset
//                     then this offset = va is returned, true
//                     else returns false!
//
bool re_file::b_va_to_file_offset(re_addr_t &offs, re_addr_t va)
{
    int segcount = segments.count();

    if(!segcount)  {
        // for raw files
        if (va < (re_addr_t)stat_struct.st_size) {
            offs = va;
            return true;
        }
        else return false;
    }

    for(int i=0; i < segcount; i++)
        if((segments.at(i).va_start <= va) && (segments.at(i).va_end >= va)) {
            offs = va - segments.at(i).va_start + segments.at(i).file_offset;
            return true;
        }

    return false;
}

// return va from file offset
// input:  offs
// output: va, bool retval
// if offset >= filesize, function returns false
// if offset out of any segment bounds, function returns false
// if filetype == raw, always returns va = offs, true
//
bool re_file::b_file_offset_to_va(re_addr_t &va, re_addr_t offs)
{
    int segcount = segments.count();
    if(!segcount) {
        // for raw files
        if (offs < (re_addr_t)stat_struct.st_size) {
            va = offs;
            return true;
        }
        else return false;
    }

    for(int i=0; i < segcount; i++)
        if( (segments.at(i).file_offset <= offs) &&
                ((segments.at(i).file_offset + segments.at(i).file_size) >= offs)) {
            va = offs-segments.at(i).file_offset + segments.at(i).va_start;
            return true;
        }

    return false;
}

// in case of error, b = 0, retval = false
bool re_file::b_get_byte_at_file_offset(unsigned char &b, re_addr_t offs)
{
    if (offs < (re_addr_t)stat_struct.st_size) {
        b = image[offs];
        return true;
    }
    b = 0;
    return false;
}

bool re_file::b_get_dword_at_file_offset(unsigned int &dw, re_addr_t offs)
{
    if (offs <= ((re_addr_t)stat_struct.st_size-4)) {
        unsigned long *lptr = (unsigned long *)(image + offs);
        dw =  *lptr;
        return true;
    }
    dw = 0;
    return false;
}

bool re_file::b_get_word_at_file_offset(unsigned int &w, re_addr_t offs)
{
    if (offs <= ((re_addr_t)stat_struct.st_size-2)) {
        w = image[offs];
        w += image[offs + 1] << 8;
        return true;
    }
    w = 0;
    return false;
}

bool re_file::b_get_byte_at_va(unsigned char &b, re_addr_t va)
{
    re_addr_t offs;

    if(b_va_to_file_offset(offs, va)) {
        return b_get_byte_at_file_offset(b, offs);
    }

    b = 0;
    return false;
}

bool re_file::b_get_dword_at_va(unsigned int &dw, re_addr_t va)
{
    re_addr_t offs;

    if(b_va_to_file_offset(offs, va)) {
        return b_get_dword_at_file_offset(dw, offs);
    }

    dw = 0;
    return false;
}

bool re_file::b_get_word_at_va(unsigned int &w, re_addr_t va)
{
    re_addr_t offs;

    if(b_va_to_file_offset(offs, va)) {
        return b_get_word_at_file_offset(w, offs);
    }

    w = 0;
    return false;
}

// returns offset into segment, not into file
bool re_file::b_file_offset_to_segment_offset(re_addr_t &seg_offs, re_addr_t offset)
{
    int segcount = segments.count();

    if(!segcount) {// for raw files: return whole file as segment
        seg_offs = offset;
        return true;
    }

    for(int i=0; i < segcount; i++)
        if( (segments.at(i).file_offset <= offset) &&
                ((segments.at(i).file_offset + segments.at(i).file_size) >= offset)
          ) {
            seg_offs = offset - segments.at(i).file_offset;
        }

    seg_offs = 0;
    return false;
}


// returns offset into segment, not into file
bool re_file::b_file_offset_to_segment_offset(re_file_segment &seg,
                                                   re_addr_t &seg_offs,
                                                   re_addr_t offset)
{
    int segcount = segments.count();

    if(!segcount) {// for raw files: return whole file as segment
        seg_offs = offset;
        return true;
    }

    for(int i=0; i < segcount; i++)
        if( (segments.at(i).file_offset <= offset) &&
                ((segments.at(i).file_offset + segments.at(i).file_size) >= offset)
          ) {
            seg = segments.at(i);
            seg_offs = offset - segments.at(i).file_offset;
            return true;
        }

    seg_offs = 0;
    return false;
}
