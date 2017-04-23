#include "rv_elfhdr_model.h"

rv_elfhdr_model::rv_elfhdr_model(QObject *parent) : QAbstractTableModel(parent)
{
    f = 0;
}


Qt::ItemFlags rv_elfhdr_model::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column()==1) {
        flags |= Qt::ItemIsSelectable;
        flags |= Qt::ItemIsEditable;
    }
    //return (Qt::ItemIsDragEnabled|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    return flags;
}

QVariant rv_elfhdr_model::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() ) return QVariant();

    if(!f) return QVariant();

    QString S;
    char buf[4096], buf2[256];
    unsigned int i, j;

    if( role == Qt::TextAlignmentRole ) {
            if( index.column() == 0) return int( Qt::AlignLeft | Qt::AlignVCenter );
            if( index.column() == 1) return int( Qt::AlignLeft | Qt::AlignVCenter );
    }

    // -- HEADER COL --
    if( index.column() == 0) {
        if ( role == Qt::DisplayRole ) {
            if( index.row() == 0) return "e_ident";
            if( index.row() == 1) return "e_type";
            if( index.row() == 2) return "e_machine";
            if( index.row() == 3) return "e_version";
            if( index.row() == 4) return "e_entry";
            if( index.row() == 5) return "e_phoff";
            if( index.row() == 6) return "e_shoff";
            if( index.row() == 7) return "e_flags";
            if( index.row() == 8) return "e_ehsize";
            if( index.row() == 9) return "e_phentsize";
            if( index.row() ==10) return "e_phnum";
            if( index.row() ==11) return "e_shentsize";
            if( index.row() ==12) return "e_shnum";
            if( index.row() ==13) return "e_shstrndx";
            if( index.row() ==14) return "Complete Hexdump";
        }
        if ( role == Qt::ForegroundRole ) {
            return QColor(0x20, 0x20, 0x40);
        }
        if ( role == Qt::FontRole ) {
            return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
        }
    }

    // -- VALUE COL --
    if( index.column() == 1) {
        if ( role == Qt::DisplayRole || role == Qt::EditRole) {

            // -- E_IDENT --
            if( index.row() == 0) {
                buf[0] = 0x0;
                for(i=0; i< 8; i++) {
                    sprintf(buf2, "%02x ", f->ELF_Hdr->e_ident[i]);
                    strcat(buf, buf2);
                }
                strcat(buf, "\n");
                for(i=8; i< 16; i++) {
                    sprintf(buf2, "%02x ", f->ELF_Hdr->e_ident[i]);
                    strcat(buf, buf2);
                }
                return S.fromAscii(buf);
            }

            // -- E_TYPE --
            if( index.row() == 1) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_type);
                return S.fromAscii(buf);
            }

            // -- E_MACHINE --
            if( index.row() == 2) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_machine);
                return S.fromAscii(buf);
            }

            // -- E_VERSION --
            if( index.row() == 3) {
                sprintf(buf, "%08x", f->ELF_Hdr->e_version);
                return S.fromAscii(buf);
            }

            // -- E_ENTRY --
            if( index.row() == 4) {
                sprintf(buf, "%08x", f->ELF_Hdr->e_entry);
                return S.fromAscii(buf);
            }

            // -- E_PHOFF --
            if( index.row() == 5) {
                sprintf(buf, "%08x", f->ELF_Hdr->e_phoff);
                return S.fromAscii(buf);
            }

            // -- E_SHOFF --
            if( index.row() == 6) {
                sprintf(buf, "%08x", f->ELF_Hdr->e_shoff);
                return S.fromAscii(buf);
            }

            // -- E_FLAGS --
            if( index.row() == 7) {
                sprintf(buf, "%08x", f->ELF_Hdr->e_flags);
                return S.fromAscii(buf);
            }

            // -- E_EHSIZE --
            if( index.row() == 8) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_ehsize);
                return S.fromAscii(buf);
            }

            // -- E_PHENTSIZE --
            if( index.row() == 9) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_phentsize);
                return S.fromAscii(buf);
            }

            // -- E_PHNUM --
            if( index.row() == 10) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_phnum);
                return S.fromAscii(buf);
            }

            // -- E_SHENTSIZE --
            if( index.row() == 11) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_shentsize);
                return S.fromAscii(buf);
            }

            // -- E_SHNUM --
            if( index.row() == 12) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_shnum);
                return S.fromAscii(buf);
            }

            // -- E_EHSIZE --
            if( index.row() == 13) {
                sprintf(buf, "%04x", f->ELF_Hdr->e_shstrndx);
                return S.fromAscii(buf);
            }
        }



        if ( role == Qt::ForegroundRole ) {
            if( index.row() == 4 ) return QColor(0x80, 0x40, 0x40);
            return QColor(0x40, 0x40, 0x40);
        }
        if ( role == Qt::FontRole ) {
            if( index.row() == 14 ) return QFont (MY_FONT, MY_FONT_SIZE, 0);
            return QFont (MY_FONT, MY_FONT_SIZE, QFont::Bold);
        }
    }


    // -- DESC COL --
    if( index.column() == 2) {
        if ( role == Qt::DisplayRole  ) {

            buf[0] = 0x0;

            // -- E_IDENT --
            if( index.row() == 0) {
                if( f->ELF_Hdr->e_ident[EI_MAG0] == 0x7f &&
                    f->ELF_Hdr->e_ident[EI_MAG1] == 'E' &&
                    f->ELF_Hdr->e_ident[EI_MAG2] == 'L' &&
                    f->ELF_Hdr->e_ident[EI_MAG3] == 'F' )
                    sprintf(buf, "ELF, ");

                if( f->ELF_Hdr->e_ident[EI_CLASS] == 0x00 ) strcat(buf, "Invalid class, ");
                if( f->ELF_Hdr->e_ident[EI_CLASS] == 0x01 ) strcat(buf, "32 bit, ");
                if( f->ELF_Hdr->e_ident[EI_CLASS] == 0x02 ) strcat(buf, "64 bit, ");

                if( f->ELF_Hdr->e_ident[EI_DATA] == 0x00 ) strcat(buf, "Invalid data encoding, ");
                if( f->ELF_Hdr->e_ident[EI_DATA] == 0x01 ) strcat(buf, "LSB, ");
                if( f->ELF_Hdr->e_ident[EI_DATA] == 0x02 ) strcat(buf, "MSB, ");

                if( f->ELF_Hdr->e_ident[EI_VERSION] == 0x00 ) strcat(buf, "Invalid version, ");
                if( f->ELF_Hdr->e_ident[EI_VERSION] == 0x01 ) strcat(buf, "Current version, ");

                strcat(buf, "\nABI: ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 0 ) strcat(buf, "None / UNIX System V ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 1 ) strcat(buf, "HP-UX ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 2 ) strcat(buf, "NetBSD ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 3 ) strcat(buf, "Linux ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 6 ) strcat(buf, "Sun Solaris ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 7 ) strcat(buf, "IBM AIX ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 8 ) strcat(buf, "SGI Irix ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 9 ) strcat(buf, "FreeBSD ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 10 ) strcat(buf, "Compaq TRU64 UNIX ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 11 ) strcat(buf, "Novell Modesto ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 12 ) strcat(buf, "OpenBSD ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 64 ) strcat(buf, "ARM EABI ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 97 ) strcat(buf, "ARM ");
                if( f->ELF_Hdr->e_ident[EI_OSABI] == 255 ) strcat(buf, "Standalone (embedded) ");

                sprintf(buf2, "Version: %02x, ", f->ELF_Hdr->e_ident[EI_ABIVERSION]);
                strcat(buf, buf2 );

                sprintf(buf2, "padding at: %02x", f->ELF_Hdr->e_ident[EI_PAD]);
                strcat(buf, buf2 );
                return S.fromAscii(buf);
            }

            // -- E_TYPE --
            if( index.row() == 1) {
                if( f->ELF_Hdr->e_type == 0x00 ) sprintf(buf, "<none>");
                if( f->ELF_Hdr->e_type == 0x01 ) sprintf(buf, "Relocatable file");
                if( f->ELF_Hdr->e_type == 0x02 ) sprintf(buf, "Executable file");
                if( f->ELF_Hdr->e_type == 0x03 ) sprintf(buf, "Shared object file");
                if( f->ELF_Hdr->e_type == 0x04 ) sprintf(buf, "Core file");
                if( f->ELF_Hdr->e_type >= 0xfe00  &&
                    f->ELF_Hdr->e_type <= 0xfeff
                        ) sprintf(buf, "OS specific");
                if( f->ELF_Hdr->e_type >= 0xff00) sprintf(buf, "processor specific");

                return S.fromAscii(buf);
            }

            // -- E_MACHINE --
            if( index.row() == 2) {
                if( f->ELF_Hdr->e_machine == 0x00 ) sprintf(buf, "<none>");
                if( f->ELF_Hdr->e_machine == 0x01 ) sprintf(buf, "AT&T WE 32100");
                if( f->ELF_Hdr->e_machine == 0x02 ) sprintf(buf, "SUN SPARC");
                if( f->ELF_Hdr->e_machine == 0x03 ) sprintf(buf, "Intel 80386");
                if( f->ELF_Hdr->e_machine == 0x04 ) sprintf(buf, "Motorola 68k");
                if( f->ELF_Hdr->e_machine == 0x05 ) sprintf(buf, "Motorola 88k");
                if( f->ELF_Hdr->e_machine == 0x07 ) sprintf(buf, "Intel 80860");
                if( f->ELF_Hdr->e_machine == 0x08 ) sprintf(buf, "MIPS RS3000 big endian");
                if( f->ELF_Hdr->e_machine == 0x09 ) sprintf(buf, "IBM System/370");
                if( f->ELF_Hdr->e_machine == 0x0a ) sprintf(buf, "MIPS RS3000 little endian");

                // ...
                return S.fromAscii(buf);
            }

            // -- E_VERSION --
            if( index.row() == 3) {
                if( f->ELF_Hdr->e_version == 0x00 ) sprintf(buf, "<none>");
                if( f->ELF_Hdr->e_version == 0x01 ) sprintf(buf, "Current version");
                return S.fromAscii(buf);
            }

            // -- E_ENTRY --
            if( index.row() == 4) {
                return S.fromAscii("Entry point virtual address");
            }

            if( index.row() == 5) {
                return S.fromAscii("Program header table file offset");
            }
            if( index.row() == 6) {
                return S.fromAscii("Section header table file offset");
            }
            if( index.row() == 7) {
                return S.fromAscii("Processor-specific flags (none for intel)");
            }
            if( index.row() == 8) {
                return S.fromAscii("ELF header size in bytes");
            }
            if( index.row() == 9) {
                return S.fromAscii("Program header table entry size");
            }
            if( index.row() == 10) {
                return S.fromAscii("Program header table entry count");
            }
            if( index.row() == 11) {
                return S.fromAscii("Section header table entry size");
            }
            if( index.row() == 12) {
                return S.fromAscii("Section header table entry count");
            }
            if( index.row() == 13) {
                return S.fromAscii("Section header string table index ");
            }

        }
        if ( role == Qt::ForegroundRole ) {
            if( index.row() == 4 ) return QColor(0x80, 0x40, 0x40);
            return QColor(0x80, 0x80, 0x80);
        }
        if ( role == Qt::FontRole ) {
            return QFont (MY_FONT, MY_FONT_SIZE, 0);
        }
    }

    if ( role == Qt::FontRole ) {
        return QFont (MY_FONT, MY_FONT_SIZE, 0);
    }
    return QVariant();
}

QVariant rv_elfhdr_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole ) return QVariant();
    if( orientation == Qt::Horizontal ) {
        if( section == 0 ) return QVariant();

    }
    return QVariant();
}


int rv_elfhdr_model::rowCount(const QModelIndex &) const
{
    return 14;
}

int rv_elfhdr_model::columnCount(const QModelIndex &) const
{
    return 3;
}


void rv_elfhdr_model::setFile(re_file *file)
{
    f = file;
}

