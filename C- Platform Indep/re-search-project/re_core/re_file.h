#ifndef RE_FILE_H
#define RE_FILE_H

#include "re_core_global.h"
#include "re_types.h"
#include "re_elf.h"
#include "re_pe.h"

#include "re_file_elf32.h"
#include "re_file_pe32.h"
#include "re_file_raw32.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QHash>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

// File types
enum {
    RE_FILE_TYPE_UNKNOWN = 0,
    RE_FILE_TYPE_ELF32,
    RE_FILE_TYPE_PE32,
    RE_FILE_TYPE_RAW32
};

// Segment flags
enum {
                    // TODO: specify other flags
    RE_SEGMENT_LOAD = 1
};

enum {
    RE_SEGMENT_TYPE_DYNAMIC = 1
};

// status
enum {
    RE_FILE_STATUS_NONE,
    RE_FILE_STATUS_LOADED,
    RE_FILE_STATUS_ANALYZED
};

// file segment abstraction
// re_file_segments are:
// elf: file segment (phdr->)
// pe:	file section (IMAGE_SECTION_HEADER->)
// raw: whole file
struct re_file_segment {
    re_addr_t       va_start;
    re_addr_t       va_end;
    re_addr_t       file_offset;
    re_addr_t       file_size;
    re_addr_t       mem_size;
    unsigned int    flags;
    unsigned int    type;
    void            *struct_ptr;    // phdr* || IMAGE_SECTION_HEADER*
    unsigned char   *data_ptr;      // pointer into file (image + ...)
};

// file section abstraction
// re_file_sections are:
// elf: file section (shdr->)
// pe:	n/a - no correspondent structure
// raw: n/a - no correspondent structure
struct re_file_section {
    re_addr_t       va_start;
    re_addr_t       va_end;
    re_addr_t       size;
    unsigned int    flags;
    QString         name;
};

class RE_CORESHARED_EXPORT re_file : public QObject
{
    Q_OBJECT
public:
    explicit re_file(QObject *parent = 0);
    ~re_file();

    bool load_file(const QString &name);
    bool file_loaded() const;
    int get_file_type();

    void msg_helper(int level, const QString &msg);
    void pct_helper(int pct);

    re_addr_t file_offset_to_va(re_addr_t offset) const;
    re_addr_t va_to_file_offset(re_addr_t va) const;
    re_addr_t va_to_segment_offset(re_file_segment *seg, re_addr_t va) const;

    unsigned char *va_to_data_ptr(re_addr_t va) const;
    unsigned char get_byte_at_va(re_addr_t va) const;  // do not use (no validity check)
    unsigned long get_dword_at_va(re_addr_t va) const; // used in dissasembler!!

    unsigned char get_byte_at_offset(re_addr_t offset);
    unsigned long get_dword_at_offset(re_addr_t offset);

    // ---- safe access functions, use them!  ----
    bool    b_va_to_file_offset(re_addr_t &offs, re_addr_t va);
    bool    b_file_offset_to_va(re_addr_t &va, re_addr_t offs);

    bool    b_get_byte_at_file_offset(unsigned char &b, re_addr_t offs);
    bool    b_get_word_at_file_offset(unsigned int &w, re_addr_t offs);
    bool    b_get_dword_at_file_offset(unsigned int &dw, re_addr_t offs);

    bool    b_get_byte_at_va(unsigned char &b, re_addr_t va);
    bool    b_get_word_at_va(unsigned int &w, re_addr_t va);
    bool    b_get_dword_at_va(unsigned int &dw, re_addr_t va);

    bool    b_file_offset_to_segment_offset(re_addr_t &seg_offs, re_addr_t offset);
    bool    b_file_offset_to_segment_offset(struct re_file_segment &seg, re_addr_t &seg_offs, re_addr_t offset);

    // ----

    void scan_ascii_strings();
    void scan_unicode_strings();

    void set_strings_scan_on_load(bool tf);

signals:
    void msg(const int level, const QString S);
    void strings_scan_pct(int pct);
    void file_load_pct(int pct);

public slots:
    void set_filename(QString fn);

public:
    QString         file_name;
    unsigned char   *image;
    char            *max_ptr;
    struct stat     stat_struct;
    int             status;
    unsigned long   filesize;

    int             file_type;
    re_addr_t       va_entry_point;
    re_addr_t       va_custom_ep;

    QStringList     shared_libs;
    QList<re_file_segment>  segments;
    QList<re_file_section>  sections;

    QMap<re_addr_t, char *>  va_functions_map;
    QMap<re_addr_t, char *>  va_symbols_map;
    QMap<re_addr_t, char *>  va_imports_map;
    QHash<re_addr_t, char *> va_cstrings_hash;
    QHash<re_addr_t, QString> va_names_hash;
    QHash<re_addr_t, QString> va_analyzed_functions_hash;

    // ELF Specific
    Elf32_Ehdr *ELF_Hdr;

    // GUI related
    bool            strings_scan_on_load;
};

#endif // RE_FILE_H
