#include "re_file_pe32.h"
#include "re_file.h"

re_file_pe32::re_file_pe32(QObject *parent) :
    QObject(parent)
{
}

bool re_file_pe32::check_file_type(re_file *f)
{
    IMAGE_DOS_HEADER *DOS_Hdr;
    IMAGE_NT_HEADERS *NT_Hdr;

    if((unsigned) f->stat_struct.st_size >=
            (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) ) {
        DOS_Hdr = (IMAGE_DOS_HEADER *) f->image;
        if(DOS_Hdr->e_magic != IMAGE_DOS_SIGNATURE) return false;

        if(DOS_Hdr->e_lfanew >
                (f->stat_struct.st_size - sizeof(IMAGE_NT_HEADERS)))
            return false;

        NT_Hdr = (IMAGE_NT_HEADERS *)(f->image + DOS_Hdr->e_lfanew);
        if(NT_Hdr->Signature != IMAGE_NT_SIGNATURE) return false;

        if(NT_Hdr->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
            return false;

        if(!(NT_Hdr->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)||
           !(NT_Hdr->FileHeader.Characteristics & IMAGE_FILE_32BIT_MACHINE) )
            return false;

        return true;
    }

    return false;
}

bool re_file_pe32::analyze_file(re_file *f)
{
    IMAGE_DOS_HEADER *DOS_Hdr;
    IMAGE_NT_HEADERS *NT_Hdr;

    f->msg_helper(RE_LOG_NORMAL, "pe32: starting to analyze");

    DOS_Hdr = (IMAGE_DOS_HEADER *) f->image;
    NT_Hdr = (IMAGE_NT_HEADERS *)(f->image + DOS_Hdr->e_lfanew);

    if(!NT_Hdr->FileHeader.NumberOfSections) {
        f->msg_helper(RE_LOG_WARNING, "no sections found");
        return false;
    }

    f->msg_helper(RE_LOG_NORMAL, "entry point at 0x"
             + QString::number(NT_Hdr->OptionalHeader.AddressOfEntryPoint, 16));
    f->msg_helper(RE_LOG_DEBUG_1, "base of code: 0x"
             + QString::number(NT_Hdr->OptionalHeader.BaseOfCode, 16));
    f->msg_helper(RE_LOG_DEBUG_1, "base of data: 0x"
             + QString::number(NT_Hdr->OptionalHeader.BaseOfData, 16));
    f->msg_helper(RE_LOG_DEBUG_1, "imagebase: 0x"
             + QString::number(NT_Hdr->OptionalHeader.ImageBase, 16));

    f->va_entry_point = NT_Hdr->OptionalHeader.AddressOfEntryPoint +
                     NT_Hdr->OptionalHeader.ImageBase;

    re_file_segment tmp_segment;

    IMAGE_SECTION_HEADER *ish =
            (IMAGE_SECTION_HEADER *)((char *)NT_Hdr + sizeof(IMAGE_NT_HEADERS));

    for(int i=0; i < NT_Hdr->FileHeader.NumberOfSections; i++, ish++) {
        if( ((char *)ish + sizeof(IMAGE_SECTION_HEADER)) > f->max_ptr) break;

        tmp_segment.va_start   = NT_Hdr->OptionalHeader.ImageBase +
                ish->VirtualAddress;
        tmp_segment.va_end     = tmp_segment.va_start + ish->SizeOfRawData;
        tmp_segment.file_offset = ish->PointerToRawData;
        tmp_segment.file_size   = ish->SizeOfRawData;
        tmp_segment.mem_size    = ish->Misc.VirtualSize;
        tmp_segment.flags       = 0;
        tmp_segment.type        = ish->Characteristics;

        tmp_segment.flags = 0;
        if( (ish->Characteristics & 0x20000000 ) )
            tmp_segment.flags |= RE_SEGMENT_LOAD;

        tmp_segment.struct_ptr  = (char *)ish;
        tmp_segment.data_ptr    =  f->image + tmp_segment.file_offset;
        if(tmp_segment.data_ptr > (unsigned char *) f->max_ptr)
            tmp_segment.data_ptr = 0;

        f->msg_helper(RE_LOG_DEBUG_2, "+ adding section #" + QString::number(i)
                + " (0x" + QString::number(tmp_segment.va_start, 16)
                + " - 0x" + QString::number(tmp_segment.va_end, 16) + ")"
                + QString::fromAscii((const char *)ish->Name));

        f->segments.append(tmp_segment);
    }

    //f->va_functions_map[f->va_entry_point] = (char *) "entry_point";

    // do the fscking imports ...

    if( !NT_Hdr->OptionalHeader.DataDirectory[1].Size) {
        f->msg_helper(RE_LOG_WARNING, "zero import table size");
        return true;
    }

    re_addr_t va_import_table =
            NT_Hdr->OptionalHeader.DataDirectory[1].VirtualAddress +
            NT_Hdr->OptionalHeader.ImageBase;

    f->msg_helper(RE_LOG_DEBUG_1, "import table at 0x"
                  + QString::number(va_import_table, 16));


    IMAGE_IMPORT_DESCRIPTOR *import_descriptor =
            (IMAGE_IMPORT_DESCRIPTOR *) f->va_to_data_ptr(va_import_table);

    re_addr_t import_va, import_val_va, import_code_va;
    QString lib_name;
    f->shared_libs.clear();
    while(import_descriptor->DUMMYUNIONNAME.Characteristics &&
          import_descriptor->Name) {

        lib_name = QString::fromAscii(
                    (char *)f->va_to_data_ptr(import_descriptor->Name +
                                           NT_Hdr->OptionalHeader.ImageBase));

        f->shared_libs.append(lib_name);
        f->msg_helper(RE_LOG_DEBUG_2, "+ adding shared lib '" + lib_name + "'");

        // go through imports
        import_va = import_descriptor->DUMMYUNIONNAME.Characteristics;
        //if(!import_va) import_va = import_descriptor->FirstThunk;
        if(!import_va) break;

        IMAGE_THUNK_DATA32 *thunk_data = (IMAGE_THUNK_DATA32 *)
            f->va_to_data_ptr(import_va +  NT_Hdr->OptionalHeader.ImageBase);

        import_code_va = import_descriptor->FirstThunk +
                            NT_Hdr->OptionalHeader.ImageBase;

        char *tmp_name;
        while(thunk_data->u1.AddressOfData) {
            import_val_va = thunk_data->u1.AddressOfData;
            if( (import_val_va>>16) == 0x8000 ) {
                // ORDINAL
                tmp_name = (char *) malloc(1024);
                snprintf(tmp_name, 1023, "%x/%s",
                        import_val_va & 0x0000ffff,
                        lib_name.toAscii().data());

                f->va_imports_map.insert(import_code_va, tmp_name);
                f->msg_helper(RE_LOG_DEBUG_2, "+ adding import '"
                    + QString::fromAscii(tmp_name)
                    + "' at 0x" + QString::number(import_code_va, 16));

            } else {
                // ASCIIZ
                import_val_va = thunk_data->u1.AddressOfData +
                        NT_Hdr->OptionalHeader.ImageBase;

                f->msg_helper(RE_LOG_DEBUG_2, "+ adding import '"
                    + QString::fromAscii(
                        (const char*)f->va_to_data_ptr(import_val_va+2))
                    + "' at 0x" + QString::number(import_code_va, 16));

                f->va_imports_map.insert(import_code_va,
                    (char *)f->va_to_data_ptr(import_val_va+2));
            }

            thunk_data++;
            import_code_va+=4;
        }

        import_descriptor++;
    }

    f->va_symbols_map.insert(f->va_entry_point, "entry point");

    f->msg_helper(RE_LOG_NORMAL, "pe32: analysis finished");
    return true;
}
