#include "re_file_elf32.h"
#include "re_file.h"

re_file_elf32::re_file_elf32(QObject *parent) :
    QObject(parent)
{
}

bool re_file_elf32::check_file_type(re_file *f)
{
    Elf32_Ehdr *ehdr;
    if((unsigned) f->stat_struct.st_size >= sizeof(Elf32_Ehdr)) {
        ehdr = (Elf32_Ehdr *) f->image;

        if(     (ehdr->e_ident[EI_MAG0]  == ELFMAG0)    &&
                (ehdr->e_ident[EI_MAG1]  == ELFMAG1)    &&
                (ehdr->e_ident[EI_MAG2]  == ELFMAG2)    &&
                (ehdr->e_ident[EI_MAG3]  == ELFMAG3)    &&
                (ehdr->e_ident[EI_CLASS] == ELFCLASS32) ) {
            f->ELF_Hdr = ehdr;
            return true;
        }
    }

    return false;
}

bool re_file_elf32::analyze_file(re_file *f)
{
    Elf32_Ehdr  *ehdr = (Elf32_Ehdr*) f->image;
    Elf32_Phdr  *phdr = 0;
    Elf32_Dyn   *dyn  = 0;
    Elf32_Sym   *sym  = 0;

    Elf32_Phdr  *phdr_dynamic = 0;
    Elf32_Sym   *sym_tab = 0;
    char        *sym_strtab = 0;
    Elf32_Rel   *jmp_rel = 0;
    int          symnum = 0;
    unsigned int  sym_ent = 0, pltrel_size = 0, foffset = 0;
    re_addr_t jmp_rel_va = 0, symtab_va = 0, symstrtab_va = 0;

    f->msg_helper(RE_LOG_NORMAL, "elf32: starting to analyze");
    f->pct_helper(00);

    if(!ehdr->e_phnum) {
        // TODO: check this out
        // (must be .o file or so)
        f->msg_helper(RE_LOG_WARNING, "no pheaders found!");
        return false;
    }

    int i;
    char *symbol_name;

    // entry point
    f->va_entry_point = ehdr->e_entry;
    f->msg_helper(RE_LOG_NORMAL, "entry point at 0x"
                  + QString::number(ehdr->e_entry, 16));

    // process program headers
    if( ((char *)(phdr = (Elf32_Phdr *) (f->image + ehdr->e_phoff))) >
                 (f->max_ptr - (sizeof(Elf32_Phdr))) ) {
        f->msg_helper(RE_LOG_WARNING, "invalid phdr pointer!");
        return true;
    }

    re_file_segment tmp_segment;

    for(i=0; i < ehdr->e_phnum; i++, phdr++) {
        if( (char *)(phdr) > (f->max_ptr - sizeof(Elf32_Phdr))) {
            f->msg_helper(RE_LOG_WARNING, "pheader #" +
                          QString::number(i) + " struct out of bounds!");
            continue;
        }
        tmp_segment.va_start   = phdr->p_paddr;
        tmp_segment.va_end     = phdr->p_paddr + phdr->p_filesz;

        tmp_segment.file_offset = phdr->p_offset;
        if(tmp_segment.file_offset +  phdr->p_filesz
                > (re_addr_t) f->stat_struct.st_size ) {
            f->msg_helper(RE_LOG_WARNING, "pheader #" +
                          QString::number(i) + " data out of bounds!");
        }

        tmp_segment.file_size   = phdr->p_filesz;
        tmp_segment.mem_size    = phdr->p_memsz;

        //tmp_segment.flags       = phdr->p_flags; // TODO: use or not use flags
        tmp_segment.flags = 0;
        if(phdr->p_type == PT_LOAD)
            tmp_segment.flags |= RE_SEGMENT_LOAD;

        tmp_segment.type = 0;
        if(phdr->p_type == PT_DYNAMIC)
            tmp_segment.type = RE_SEGMENT_TYPE_DYNAMIC;

        tmp_segment.struct_ptr  = (char *)phdr;
        tmp_segment.data_ptr    = f->image + phdr->p_offset;

        if(tmp_segment.data_ptr > (unsigned char *)f->max_ptr) {
            tmp_segment.data_ptr = 0;
            f->msg_helper(RE_LOG_WARNING, "pheader #" +
                          QString::number(i) + " data out of bounds!");
            continue;
        }

        f->segments.append(tmp_segment);
        f->msg_helper(RE_LOG_DEBUG_2, "+ adding pheader #" + QString::number(i)
                + " (0x" + QString::number(tmp_segment.va_start, 16)
                + " - 0x" + QString::number(tmp_segment.va_end, 16) + ")"
               );
    }

    f->pct_helper(10);


    // locating dynamic section
    for(i=0; i < f->segments.count(); i++)
        if(f->segments.at(i).type == RE_SEGMENT_TYPE_DYNAMIC) {
            phdr_dynamic = (Elf32_Phdr *) f->segments.at(i).struct_ptr;

            f->msg_helper(RE_LOG_DEBUG_1, "dynamic segment found at 0x"
                          + QString::number(phdr_dynamic->p_vaddr, 16));
            break;
    }

    if(!phdr_dynamic) {
        f->msg_helper(RE_LOG_WARNING, "no dynamic segment found");
        return true;
    }
    f->msg_helper(RE_LOG_DEBUG_1, "processing dynamic segment");

    // -- locate DT_STRTAB, DT_SYMTAB --
    // set  SymTab, SymStrtab, sym_ent,
    //      jmp_rel, pltrel_size
    f->msg_helper(RE_LOG_DEBUG_1, "processing dynamic section entries");
    for(dyn = (Elf32_Dyn *) (f->image+phdr_dynamic->p_offset); 1 ; dyn++) {

        if( (char *) dyn > (f->max_ptr - sizeof(Elf32_Dyn))) break;

        if(dyn->d_tag == DT_SYMTAB) {
            symtab_va = dyn->d_un.d_ptr;
            continue;
        }
        if(dyn->d_tag == DT_STRTAB) {
            symstrtab_va = dyn->d_un.d_val;
            continue;
        }
        if(dyn->d_tag == DT_SYMENT) {
            sym_ent = dyn->d_un.d_val;
            continue;
        }
        if(dyn->d_tag == DT_JMPREL) {
            jmp_rel_va = dyn->d_un.d_val;
            continue;
        }
        if(dyn->d_tag == DT_PLTRELSZ) {
            pltrel_size = dyn->d_un.d_val;
            continue;
        }

        if(!dyn->d_tag) break;

    }

    f->pct_helper(20);


    if( !symtab_va || !symstrtab_va || !sym_ent ) {
        f->msg_helper(RE_LOG_WARNING, "incomplete dynamic information found");
        return true;
    }

    // :vaddr -> offset
    // :SymTab, SymStrTab, jmp_rel
    for(i=0; i < f->segments.count(); i++) {
        // SymTab
        if( (f->segments.at(i).va_start <= symtab_va) &&
                (f->segments.at(i).va_end >= symtab_va) ) {
            foffset = symtab_va - f->segments.at(i).va_start +
                    f->segments.at(i).file_offset;
            sym_tab = (Elf32_Sym *) (f->image + foffset);
        }

        // SymStrTab
        if( (f->segments.at(i).va_start <= symstrtab_va) &&
                (f->segments.at(i).va_end >= symstrtab_va) ) {
            foffset = symstrtab_va - f->segments.at(i).va_start +
                    f->segments.at(i).file_offset;
            sym_strtab = (char *)f->image + foffset;
        }

        // jmp_rel
        if( (f->segments.at(i).va_start <= jmp_rel_va) &&
                (f->segments.at(i).va_end >= jmp_rel_va) ) {
            foffset = jmp_rel_va - f->segments.at(i).va_start +
                   f->segments.at(i).file_offset;
            jmp_rel = (Elf32_Rel *) (f->image + foffset);
        }
    }

    f->pct_helper(30);


    if(sym_strtab) {
        // -- run through dynamic table for imported libs --
        QString lib_name;
        f->shared_libs.clear();
        f->msg_helper(RE_LOG_DEBUG_1, "processing dynamic libraries");
        for(dyn=(Elf32_Dyn*)(f->image+phdr_dynamic->p_offset); 1 ; dyn++) {
            if(dyn->d_tag == DT_NEEDED) {
                if(sym_strtab + dyn->d_un.d_val < f->max_ptr) {
                    lib_name = QString::fromAscii(sym_strtab + dyn->d_un.d_val);
                    f->shared_libs.append(lib_name);
        f->msg_helper(RE_LOG_DEBUG_2, "+ adding shared lib '" + lib_name + "'");
                } else break;
            }

            if(!dyn->d_tag) break;
            if((char *) dyn->d_tag > f->max_ptr) break;
        }
    }

    f->pct_helper(40);


    if( !sym_tab || !sym_strtab || !sym_ent ) {
        f->msg_helper(RE_LOG_WARNING, "error in dynamic info");
    }

    // --- processing dynamic symbols: functions --
    f->msg_helper(RE_LOG_DEBUG_1, "processing dynamic symbol table");
    symnum =  (sym_strtab - (char *)sym_tab ) / sym_ent;
    // strtab follows symtab, usuallY       // TODO: add extra check ???
    f->msg_helper(RE_LOG_DEBUG_1, QString::number(symnum)
                  + " symbols in table");

    if(!symnum) goto plt_n_jmp_rel;

    for ( i = 0; i < symnum; i++ ) {
        sym = &sym_tab[i];
        symbol_name = &sym_strtab[sym->st_name];

        if ( (ELF32_ST_TYPE( sym->st_info )) == STT_FUNC ){
        // -- function
        // offset = sym->st_value;
            if ( sym->st_shndx ) {
                // named function
                if(sym->st_value) {
                    f->va_functions_map[sym->st_value] = symbol_name;
                    f->msg_helper(RE_LOG_DEBUG_2, "+ adding function '"
                        + QString::fromAscii(symbol_name) + "' at 0x"
                        + QString::number(sym->st_value, 16));
                }
            } else {
                // unnamed function
                if(sym->st_value) {
                    f->va_functions_map[sym->st_value] = 0;
                    f->msg_helper(RE_LOG_DEBUG_2, "+ adding unnamed function at 0x"
                        + QString::number(sym->st_value, 16));
                }
            }
        }
        else {
        // -- other symbols
            if ( sym->st_shndx ) {
            // named
                if(sym->st_value) {
                    f->va_symbols_map[sym->st_value] = symbol_name;
                    f->msg_helper(RE_LOG_DEBUG_2, "+ adding symbol '"
                        + QString::fromAscii(symbol_name) + "' at 0x"
                        + QString::number(sym->st_value, 16));
                } else {
                // unnamed
                    f->va_symbols_map[sym->st_value] = 0;
                    f->msg_helper(RE_LOG_DEBUG_2, "+ adding symbol '"
                        + QString::fromAscii(symbol_name) + "' at 0x"
                        + QString::number(sym->st_value, 16));
                }
            }
        }
    }



    // -- plt & jmp_rel fixup --------------------------------------------------
plt_n_jmp_rel:
    f->pct_helper(80);

    f->msg_helper(RE_LOG_DEBUG_1,
        "processing JMP_REL relocation entries in plt table");

    Elf32_Rel *rel; char *tmp_string;
    for(i=0, rel= (Elf32_Rel *)jmp_rel;
        (unsigned int) i < pltrel_size / sizeof(Elf32_Rel);
        i++, rel++ ) {
        if(!rel) continue;
        sym = &sym_tab[ELF32_R_SYM(rel->r_info)];
        symbol_name = &sym_strtab[sym->st_name];

        if( symbol_name ) {
            tmp_string = (char *) malloc(strlen(symbol_name) + 10);
            sprintf(tmp_string, "%s@PLT", symbol_name);
        }
        else tmp_string = (char *) "";

        f->va_symbols_map[rel->r_offset] = tmp_string;
        f->msg_helper(RE_LOG_DEBUG_2, "+ adding symbol '"
            + QString::fromAscii(tmp_string) + "' at 0x"
            + QString::number(rel->r_offset, 16));

        f->va_imports_map[got2plt(f, rel->r_offset)] = symbol_name;
        f->msg_helper(RE_LOG_DEBUG_2, "+ adding import '"
            + QString::fromAscii(symbol_name) + "' at 0x"
            + QString::number(rel->r_offset, 16));

    }

    // -- add entrypoint to functions --
    //f->va_functions_map[f->va_entry_point] = (char *) "entry_point";
    f->va_symbols_map.insert(f->va_entry_point, "entry point");

    f->msg_helper(RE_LOG_NORMAL, "elf32: analysis finished");
    f->pct_helper(100);

    return true;
}


re_addr_t re_file_elf32::got2plt(re_file *f, re_addr_t got)
{
    Elf32_Phdr *prog_hdr;
    re_addr_t r =0;

    for(int i=0; i < f->segments.count(); i++) {
        prog_hdr = (Elf32_Phdr *) f->segments.at(i).struct_ptr;
        if ( (got >= prog_hdr->p_vaddr) &&
                (got <= (prog_hdr->p_vaddr + prog_hdr->p_filesz))) {
            r = got -
            (prog_hdr->p_vaddr - prog_hdr->p_offset);
            break;
        }
    }

    // uuh, very portable ;)   ......... -> TODO:
    if( r && (r > 6) ) return ( *(re_addr_t *)((char*)f->image+r) ) - 6;

    else return 0;
}
