#include "rv_elf_phdr_model.h"

rv_elf_phdr_model::rv_elf_phdr_model(QObject *parent) : QAbstractTableModel(parent)
{
    f = 0;
}

Qt::ItemFlags rv_elf_phdr_model::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column()==1) {
        flags |= Qt::ItemIsSelectable;
        flags |= Qt::ItemIsEditable;
    }
    //return (Qt::ItemIsDragEnabled|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    return flags;
}

QVariant rv_elf_phdr_model::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() ) return QVariant();
    if(!f) return QVariant();

    QString S;
    char buf[4096];
    buf[0] = 0;

    // setup program header
    unsigned long int    ph_offs;
    Elf32_Phdr          *phdr;

    ph_offs =  (unsigned long int)f->ELF_Hdr->e_phoff        +
                ( (unsigned long int)f->ELF_Hdr->e_phentsize *
                  (unsigned long int) index.row()              );

    if(ph_offs > (f->stat_struct.st_size - f->ELF_Hdr->e_phentsize )) return S.fromAscii("bogusbogus");

    phdr = ((Elf32_Phdr *) ((char *)f->image + ph_offs));

    if( role == Qt::TextAlignmentRole ) {
        if((index.column()==0) || (index.column()==7) )
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

    // -- p_type --
    if( index.column() == 1) {
        if ( role == Qt::DisplayRole ) {
            if(phdr->p_type == PT_NULL) return S.fromAscii("PT_NULL");
            if(phdr->p_type == PT_LOAD) return S.fromAscii("PT_LOAD");
            if(phdr->p_type == PT_DYNAMIC) return S.fromAscii("PT_DYNAMIC");
            if(phdr->p_type == PT_INTERP) {
                sprintf(buf, "PT_INTERP (%s)",
                    (char *)f->image + phdr->p_offset);
                return S.fromAscii(buf);
            }
            if(phdr->p_type == PT_NOTE) return S.fromAscii("PT_NOTE");
            if(phdr->p_type == PT_SHLIB) return S.fromAscii("PT_SHLIB");
            if(phdr->p_type == PT_PHDR) return S.fromAscii("PT_PHDR");
            if(phdr->p_type == PT_TLS) return S.fromAscii("PT_TLS");

            if(phdr->p_type == PT_GNU_EH_FRAME) return S.fromAscii("PT_GNU_EH_FRAME");
            if(phdr->p_type == PT_GNU_STACK) return S.fromAscii("PT_GNU_STACK");
            if(phdr->p_type == PT_GNU_RELRO) return S.fromAscii("PT_GNU_RELRO");
            if(phdr->p_type == PT_SUNWBSS) return S.fromAscii("PT_SUNWBSS");
            if(phdr->p_type == PT_SUNWSTACK) return S.fromAscii("PT_SUNWSTACK");

            if(phdr->p_type >= PT_LOOS && phdr->p_type <= PT_HIOS
                     ) return S.fromAscii("OS specific");

            if(phdr->p_type >= PT_LOPROC && phdr->p_type <= PT_HIPROC
                     ) return S.fromAscii("processor specific");

            return QVariant();
        }
        if ( role == Qt::ForegroundRole ) {
            if(phdr->p_type == PT_NULL) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_type == PT_LOAD) return QColor(0x40, 0x40, 0x80);
            if(phdr->p_type == PT_INTERP) return QColor(0x40, 0x80, 0x40);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
        if ( role == Qt::FontRole ) {
            if(phdr->p_type == PT_LOAD) return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
            return QFont (MY_FONT, MY_FONT_SIZE, 0);
        }
    }

    // -- p_offset --
    if( index.column() == 2) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", phdr->p_offset);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_offset) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- p_vaddr --
    if( index.column() == 3) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x - %08x", phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_vaddr) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);
            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- p_paddr --
    if( index.column() == 4) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", phdr->p_paddr);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_paddr) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);

            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- p_filesz --
    if( index.column() == 5) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", phdr->p_filesz);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_filesz) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);

            return QColor(0x20, 0x20, 0x20);
        }
    }

    // -- p_memsz --
    if( index.column() == 6) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", phdr->p_memsz);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_memsz) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_memsz < phdr->p_filesz) return QColor(0x80, 0x40, 0x40);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);

            return QColor(0x20, 0x20, 0x20);
        }
        if ( role == Qt::FontRole ) {
            if(phdr->p_memsz != phdr->p_filesz) return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
            return QFont (MY_FONT, MY_FONT_SIZE, 0);
        }
    }



    // -- sh_flags --
    if( index.column() == 7) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            if(phdr->p_flags & PF_R) strcat(buf, "R");
            else strcat(buf, "-");

            if(phdr->p_flags & PF_W) strcat(buf, "W");
            else strcat(buf, "-");

            if(phdr->p_flags & PF_X) strcat(buf, "X");
            else strcat(buf, "-");

            if(phdr->p_flags & PF_MASKOS) strcat(buf, ", OS specific");
            if(phdr->p_flags & PF_MASKPROC) strcat(buf, ", processor specific");


            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_flags) return QColor(0xa0, 0xa0, 0xa0);

            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);

            if(phdr->p_flags & PF_X) return QColor(0x80, 0x40, 0x40);
            if(phdr->p_flags & PF_W)     return QColor(0x80, 0x40, 0x40);
            if(phdr->p_flags & PF_R)     return QColor(0x40, 0x80, 0x40);

            return QColor(0x20, 0x20, 0x20);
        }

        if ( role == Qt::FontRole ) {
            if(phdr->p_flags & PF_X) return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
            return QFont (MY_FONT, MY_FONT_SIZE, 0);
        }
    }

    // -- p_align --
    if( index.column() == 8) {
        if ( role == Qt::DisplayRole ) {
            buf[0]=0;

            sprintf(buf, "%08x", phdr->p_align);
            return S.fromAscii(buf);
        }

        if ( role == Qt::ForegroundRole ) {
            if(!phdr->p_align) return QColor(0xa0, 0xa0, 0xa0);
            if(phdr->p_type == PT_PHDR) return QColor(0xa0, 0xa0, 0xa0);

            return QColor(0x20, 0x20, 0x20);
        }
    }

    if ( role == Qt::FontRole ) {
        return QFont (MY_FONT, MY_FONT_SIZE, 0);
    }

    return QVariant();
}

QVariant rv_elf_phdr_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole ) return QVariant();
    if( orientation == Qt::Horizontal ) {
        if( section == 0 ) return "#";
        if( section == 1 ) return "p_type";
        if( section == 2 ) return "p_offset";
        if( section == 3 ) return "p_vaddr";
        if( section == 4 ) return "p_paddr";
        if( section == 5 ) return "p_filesz";
        if( section == 6 ) return "p_memsz";
        if( section == 7 ) return "p_flags";
        if( section == 8 ) return "p_align";
    }

    return QVariant();
}


int rv_elf_phdr_model::rowCount(const QModelIndex &) const
{
    if(f) return f->ELF_Hdr->e_phnum;
    else return 0;
}

int rv_elf_phdr_model::columnCount(const QModelIndex &) const
{
    return 9;
}

void rv_elf_phdr_model::setFile(re_file *file)
{
    f = file;
}
