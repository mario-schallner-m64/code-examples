#include "rv_elf_shdr_model.h"

rv_elf_shdr_model::rv_elf_shdr_model(QObject *parent) : QAbstractTableModel(parent)
{
    f = 0;
}

Qt::ItemFlags rv_elf_shdr_model::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column()==1) {
        flags |= Qt::ItemIsSelectable;
        flags |= Qt::ItemIsEditable;
    }
    //return (Qt::ItemIsDragEnabled|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    return flags;
}

QVariant rv_elf_shdr_model::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() ) return QVariant();
    if(!f) return QVariant();

    QString S;
    char buf[4096];
    char *s;

    buf[0] = 0;

    // setup section
    unsigned long int    sh_offs;
    Elf32_Shdr          *shdr;

    sh_offs =  (unsigned long int)f->ELF_Hdr->e_shoff        +
                ( (unsigned long int)f->ELF_Hdr->e_shentsize *
                  (unsigned long int) index.row()              );

    if(sh_offs > (f->stat_struct.st_size - f->ELF_Hdr->e_shentsize )) return S.fromAscii("bogusbogus");

    shdr = ((Elf32_Shdr *) ((char *)f->image + sh_offs));

    if( role == Qt::TextAlignmentRole ) {
        if((index.column()==0) || (index.column()==3) )
        return int( Qt::AlignCenter | Qt::AlignVCenter );
        return int( Qt::AlignLeft | Qt::AlignVCenter );
    }

    // -- # --
    if( index.column() == 0) {
        if ( role == Qt::DisplayRole ) {
            sprintf(buf, "%02x", index.row());
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            return QColor(0x20, 0x20, 0x20);
        }

        if ( role == Qt::FontRole ) {
            return QFont (MY_FONT, MY_FONT_SIZE, 0);
        }
    }

    // -- sh_name --
    if( index.column() == 1) {
        if(!sh_stringtable) return S.fromAscii("no sh strtable section");

        s = sh_stringtable + shdr->sh_name;

        if ( role == Qt::DisplayRole ) {
            if(!*s) return S.fromAscii("<NULL>");
            return S.fromAscii(s);
        }

        if ( role == Qt::FontRole ) {
            return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
        }
        if ( role == Qt::ForegroundRole ) {
            if(!*s) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_type --
    if( index.column() == 2) {
        if ( role == Qt::DisplayRole ) {
            if(shdr->sh_type == SHT_NULL) return S.fromAscii("SHT_NULL");
            if(shdr->sh_type == SHT_PROGBITS) return S.fromAscii("SHT_PROGBITS");
            if(shdr->sh_type == SHT_SYMTAB) return S.fromAscii("SHT_SYMTAB");
            if(shdr->sh_type == SHT_STRTAB) return S.fromAscii("SHT_STRTAB");
            if(shdr->sh_type == SHT_RELA) return S.fromAscii("SHT_RELA");
            if(shdr->sh_type == SHT_HASH) return S.fromAscii("SHT_HASH");
            if(shdr->sh_type == SHT_DYNAMIC) return S.fromAscii("SHT_DYNAMIC");
            if(shdr->sh_type == SHT_NOTE) return S.fromAscii("SHT_NOTE");
            if(shdr->sh_type == SHT_NOBITS) return S.fromAscii("SHT_NOBITS");
            if(shdr->sh_type == SHT_REL) return S.fromAscii("SHT_REL");
            if(shdr->sh_type == SHT_SHLIB) return S.fromAscii("SHT_SHLIB");
            if(shdr->sh_type == SHT_DYNSYM) return S.fromAscii("SHT_DYNSYM");
            if(shdr->sh_type == SHT_INIT_ARRAY) return S.fromAscii("SHT_INIT_ARRAY");
            if(shdr->sh_type == SHT_FINI_ARRAY) return S.fromAscii("SHT_FINI_ARRAY");
            if(shdr->sh_type == SHT_PREINIT_ARRAY) return S.fromAscii("SHT_PREINIT_ARRAY");
            if(shdr->sh_type == SHT_GROUP) return S.fromAscii("SHT_GROUP");
            if(shdr->sh_type == SHT_SYMTAB_SHNDX) return S.fromAscii("SHT_SYMTAB_SHNDX");
            if(shdr->sh_type == SHT_LOOS) return S.fromAscii("SHT_LOOS");
            if(shdr->sh_type == SHT_GNU_ATTRIBUTES) return S.fromAscii("SHT_GNU_ATTRIBUTES");
            if(shdr->sh_type == SHT_GNU_HASH) return S.fromAscii("SHT_GNU_HASH");
            if(shdr->sh_type == SHT_GNU_LIBLIST) return S.fromAscii("SHT_GNU_LIBLIST");
            if(shdr->sh_type == SHT_CHECKSUM) return S.fromAscii("SHT_CHECKSUM");
            if(shdr->sh_type == SHT_LOSUNW) return S.fromAscii("SHT_LOSUNW");
            if(shdr->sh_type == SHT_SUNW_move) return S.fromAscii("SHT_SUNW_move");
            if(shdr->sh_type == SHT_SUNW_COMDAT) return S.fromAscii("SHT_SUNW_COMDAT");
            if(shdr->sh_type == SHT_SUNW_syminfo) return S.fromAscii("SHT_SUNW_syminfo");
            if(shdr->sh_type == SHT_GNU_verdef) return S.fromAscii("SHT_GNU_verdef");
            if(shdr->sh_type == SHT_GNU_verneed) return S.fromAscii("SHT_GNU_verneed");
            if(shdr->sh_type == SHT_GNU_versym) return S.fromAscii("SHT_GNU_versym");
            if(shdr->sh_type == SHT_HISUNW) return S.fromAscii("SHT_HISUNW");
            if(shdr->sh_type == SHT_HIOS) return S.fromAscii("SHT_HIOS");
            if(shdr->sh_type == SHT_LOPROC) return S.fromAscii("SHT_LOPROC");
            if(shdr->sh_type == SHT_HIPROC) return S.fromAscii("SHT_HIPROC");
            if(shdr->sh_type == SHT_LOUSER) return S.fromAscii("SHT_LOUSER");
            if(shdr->sh_type == SHT_HIUSER) return S.fromAscii("SHT_HIUSER");

            return QVariant();
        }
        if ( role == Qt::ForegroundRole ) {
            if(shdr->sh_type == SHT_NULL) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_flags --
    if( index.column() == 3) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            if(shdr->sh_flags & SHF_ALLOC) strcat(buf, "A");
            else strcat(buf, "-");

            if(shdr->sh_flags & SHF_WRITE) strcat(buf, "W");
            else strcat(buf, "-");

            if(shdr->sh_flags & SHF_EXECINSTR) strcat(buf, "X");
            else strcat(buf, "-");

            if(shdr->sh_flags & SHF_MERGE) strcat(buf, ", SHF_MERGE");
            if(shdr->sh_flags & SHF_STRINGS) strcat(buf, ", SHF_STRINGS");
            if(shdr->sh_flags & SHF_INFO_LINK) strcat(buf, ", SHF_INFO_LINK");
            if(shdr->sh_flags & SHF_LINK_ORDER) strcat(buf, ", SHF_LINK_ORDER");
            if(shdr->sh_flags & SHF_OS_NONCONFORMING) strcat(buf, ", SHF_OS_NONCONFORMING");
            if(shdr->sh_flags & SHF_GROUP) strcat(buf, ", SHF_GROUP");
            if(shdr->sh_flags & SHF_TLS) strcat(buf, ", SHF_TLS");
            if(shdr->sh_flags & SHF_MASKOS) strcat(buf, ", SHF_MASKOS");
            if(shdr->sh_flags & SHF_MASKPROC) strcat(buf, ", SHF_MASKPROC");
            if(shdr->sh_flags & SHF_ORDERED) strcat(buf, ", SHF_ORDERED");
            if(shdr->sh_flags & SHF_EXCLUDE) strcat(buf, ", SHF_EXCLUDE");

            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_flags) return QColor(0xa0, 0xa0, 0xa0);

            if(shdr->sh_flags & SHF_EXECINSTR) return QColor(0x80, 0x40, 0x40);
            if(shdr->sh_flags & SHF_WRITE)     return QColor(0x80, 0x40, 0x40);
            if(shdr->sh_flags & SHF_ALLOC)     return QColor(0x40, 0x80, 0x40);

            return QColor(0x20, 0x20, 0x20);
        }

        if ( role == Qt::FontRole ) {
            if(shdr->sh_flags & SHF_EXECINSTR) return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
            return QFont (MY_FONT, MY_FONT_SIZE, 0);
        }
    }

    // -- sh_addr --
    if( index.column() == 4) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x - %08x", shdr->sh_addr, shdr->sh_addr + shdr->sh_size);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_addr) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_offset --
    if( index.column() == 5) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", shdr->sh_offset);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_offset) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_size --
    if( index.column() == 6) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", shdr->sh_size);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_size) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_link --
    if( index.column() == 7) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", shdr->sh_link);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_link) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_info --
    if( index.column() == 8) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", shdr->sh_info);
            return S.fromAscii(buf);
        }
        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_info) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_addralign --
    if( index.column() == 9) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", shdr->sh_addralign);
            return S.fromAscii(buf);
        }
        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_addralign) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- sh_entsize --
    if( index.column() == 10) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", shdr->sh_entsize);
            return S.fromAscii(buf);
        }
        if ( role == Qt::ForegroundRole ) {
            if(!shdr->sh_entsize) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    if ( role == Qt::ForegroundRole ) {
        //if( index.row() == 4 ) return QColor(0x80, 0x40, 0x40);
        if( index.column() == 1 ) return QColor(0x40, 0x40, 0x40);
        return QColor(0x20, 0x20, 0x20);
    }

    if ( role == Qt::FontRole ) {
        if( index.column() == 1 ) return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
        return QFont (MY_FONT, MY_FONT_SIZE, 0);
    }
    return QVariant();
}

QVariant rv_elf_shdr_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole ) return QVariant();
    if( orientation == Qt::Horizontal ) {
        if( section == 0 ) return "#";
        if( section == 1 ) return "sh_name";
        if( section == 2 ) return "sh_type";
        if( section == 3 ) return "sh_flags";
        if( section == 4 ) return "sh_addr";
        if( section == 5 ) return "sh_offset";
        if( section == 6 ) return "sh_size";
        if( section == 7 ) return "sh_link";
        if( section == 8 ) return "sh_info";
        if( section == 9 ) return "sh_addralign";
        if( section == 10 ) return "sh_entsize";
    }

    return QVariant();
}


int rv_elf_shdr_model::rowCount(const QModelIndex &) const
{
    if(f &&  (f->file_type == RE_FILE_TYPE_ELF32) ) return f->ELF_Hdr->e_shnum;
    else return 0;
}

int rv_elf_shdr_model::columnCount(const QModelIndex &) const
{
    return 11;
}


void rv_elf_shdr_model::setFile(re_file *file)
{
    f = file;
    sh_stringtable = 0;

    if(f->file_type != RE_FILE_TYPE_ELF32) {
        return;
    }

    unsigned long int    sh_offs;
    Elf32_Shdr          *shdr;

    sh_offs =  (unsigned long int)f->ELF_Hdr->e_shoff          +
                  ( (unsigned long int)f->ELF_Hdr->e_shentsize *
                    (unsigned long int)f->ELF_Hdr->e_shstrndx    );

    shdr = ((Elf32_Shdr *) ((char *)f->image + sh_offs));
    if((char *)shdr > (f->max_ptr - 20)) return;

    sh_stringtable = (char *)f->image + shdr->sh_offset;
}
