#include "re_disassembler.h"

re_disassembler::re_disassembler(QObject *parent) :
    QObject(parent)
{
    file = 0;
    address_space = 0;
    logger = 0;

    opt_auto_comments_on = false;
}

re_disassembler::~re_disassembler()
{
    if(address_space) delete address_space;
}

void re_disassembler::init(re_file *f, re_address_space *as, re_logger *l)
{
    file = f; address_space = as, logger = l;

    connect(this, SIGNAL(msg(int, QString)),
            logger, SLOT(log_event(int, QString)));

    emit msg(RE_LOG_DEBUG_3, "re_disassembler::init()");

    x86_init(opt_none, NULL, 0);
}

bool re_disassembler::disassemble_linear_cnt(re_addr_t va_start, re_addr_t insn_count)
{
    re_addr_t i = 0;
    x86_insn_t insn;

    re_file_segment seg;
    re_addr_t offset = file->va_to_segment_offset(&seg, va_start);
    re_addr_t va_tmp = va_start;

    insn.size =1;
    while((i < insn_count) && (offset < seg.file_size) && insn.size) {
        x86_disasm(seg.data_ptr, seg.file_size, seg.va_start, offset, &insn );
        if(insn.size && x86_insn_is_valid(&insn))
            address_space->insert_va(va_tmp, insn.size);
        offset += insn.size;
        va_tmp += insn.size;
        i++;
        
        x86_oplist_free(&insn);
    }

    return true;
}

bool re_disassembler::disassemble_linear_range(re_addr_t va_start, re_addr_t va_end)
{
    re_addr_t i = 0;
    x86_insn_t insn;

    re_file_segment seg;
    re_addr_t offset = file->va_to_segment_offset(&seg, va_start);
    re_addr_t va_tmp = va_start;

    insn.size =1;
    while((va_tmp <= va_end) && (offset < seg.file_size) && insn.size) {
        x86_disasm(seg.data_ptr, seg.file_size, seg.va_start, offset, &insn );
        if(insn.size && x86_insn_is_valid(&insn))
            address_space->insert_va(va_tmp, insn.size);
        offset += insn.size;
        va_tmp += insn.size;
        i++;

        x86_oplist_free(&insn);
    }

    return true;
}

bool re_disassembler::print_disassembly(QString &rstr)
{
    if(!address_space->count()) return false;

    re_addr_desc *adesc = address_space->goto_first();
    if(!adesc) return false;

    char tmp_buf[4096];

    while(adesc) {
        // print xrefs
        if(va_xrefs_mmap.contains(adesc->va)) {
            rstr.append("xrefs: ");
            QList<struct re_xref> xrlist = va_xrefs_mmap.values(adesc->va);


            for(int i=0; i < xrlist.count(); ++i)
                rstr.append(QString::number(xrlist.at(i).va_src, 16) + " ");
            rstr.append("\n");
        }
        // va in functions
        if(file->va_functions_map.contains(adesc->va) &&
           file->va_functions_map.value(adesc->va)) {
            rstr.append("\n" +
                QString::fromAscii(file->va_functions_map.value(adesc->va))
                        +":\n");
        }
	
        // va in symbols
        if(file->va_symbols_map.contains(adesc->va) &&
           file->va_symbols_map.value(adesc->va)) {
            rstr.append(
                QString::fromAscii(file->va_symbols_map.value(adesc->va))
                        +":\n");
        }

        // va in imports
        if(file->va_imports_map.contains(adesc->va) &&
           file->va_imports_map.value(adesc->va)) {
            rstr.append(
                QString::fromAscii(file->va_imports_map.value(adesc->va))
                        +":\n");
        }

        // print hidden insn
        if(hidden_insns_hash.contains(adesc->va)) {
            rstr.append("!hidden! in " + 
            QString::number(hidden_insns_hash.value(adesc->va), 16)
            + "\n");
        }

        if(adesc->desc < RE_IMG_IS_INSN) {
            if(format_insn_textline(adesc->va, tmp_buf, 4095))
                rstr.append(QString::fromAscii(tmp_buf));
        }		

        adesc = address_space->goto_next();
    }

    return true;
}

bool re_disassembler::format_insn_textline(re_addr_t va, char *rbuf, int maxchar)
{
    x86_insn_t insn;
    re_addr_t tmp_offset;
    re_file_segment seg;
    char tmp_buf[1024];
    char line[2048];
    char space_str[80];
    int pos = 0, i;

    rbuf[0] = 0x0;

    tmp_offset = file->va_to_segment_offset(&seg, va);
    if(tmp_offset == BAD_ADDR) return false;

    x86_disasm(seg.data_ptr, seg.file_size, seg.va_start, tmp_offset, &insn);    
    if(!insn.size) return false;


    // start to print
    sprintf(line, "%08x ", file->va_to_file_offset(va)); pos += 9;

    sprintf(tmp_buf, "%08x ", va);
    strcat(line, tmp_buf);
    pos += 9;
    
    for(i = 0; i < insn.size; i++) {
        sprintf(tmp_buf, "%02x ", insn.bytes[i]); pos +=3;
        strcat(line, tmp_buf);
    }

    // tab
    for(i = 0; i < 50 - pos; i++) space_str[i]=' ';
    space_str[i] = 0x0;
    strcat(line, space_str);   

    format_insn_string_lookups(&insn, tmp_buf, 1023);
    strcat(line, tmp_buf);
    strcat(line, "\n");
    
    strcat(rbuf, line);
    
    x86_oplist_free(&insn);
    
    return true;
}

bool re_disassembler::format_insn_textline_foffset(re_addr_t foffset, char *rbuf, int maxchar)
{
    x86_insn_t insn;
    re_addr_t tmp_offset, tmp_va;
    re_file_segment seg;
    char tmp_buf[1024];
    char line[2048];
    char space_str[80];
    int pos = 0, i;

    rbuf[0] = 0x0;

    if(!file->b_file_offset_to_segment_offset(seg, tmp_offset, foffset)) {
        snprintf(rbuf, maxchar, "invalid offset!");
        return false;
    }

    x86_disasm(seg.data_ptr, seg.file_size, seg.va_start, tmp_offset, &insn);
    if(!insn.size) {
        snprintf(rbuf, maxchar, "invalid instruction!");
        return false;
    }


    // start to print
    sprintf(line, "%08x ", foffset); pos += 9;
    if(!file->b_file_offset_to_va(tmp_va, foffset)) {
        sprintf(tmp_buf, "        ");
    } else sprintf(tmp_buf, "%08x ", tmp_va);

    strcat(line, tmp_buf);
    pos += 9;

    for(i = 0; i < insn.size; i++) {
        sprintf(tmp_buf, "%02x ", insn.bytes[i]); pos +=3;
        strcat(line, tmp_buf);
    }

    // tab
    for(i = 0; i < 50 - pos; i++) space_str[i]=' ';
    space_str[i] = 0x0;
    strcat(line, space_str);

    format_insn_string_lookups(&insn, tmp_buf, 1023);
    strcat(line, tmp_buf);
    strcat(line, "\n");

    strcat(rbuf, line);

    x86_oplist_free(&insn);

    return true;
}

bool re_disassembler::format_insn_string(x86_insn_t *insn, char *rbuf, int len) 
{
    rbuf[0] = 0;
    char str[MAX_OP_STRING];
    x86_op_t *src, *dst;

    /* INTEL STYLE: mnemonic dest, src, imm */
    STRNCAT( rbuf, insn->prefix_string, len );
    STRNCAT( rbuf, insn->mnemonic, len );
    STRNCAT( rbuf, " ", len );

    /* dest */
    if ( (dst = x86_operand_1st( insn )) && !(dst->flags & op_implied) ) {
        x86_format_operand( dst, str, MAX_OP_STRING, intel_syntax);
        STRNCAT( rbuf, str, len );
    }

    /* src */
    if ( (src = x86_operand_2nd( insn )) ) {
        if ( !(dst->flags & op_implied) ) {
            STRNCAT( rbuf, ", ", len );
        }
        x86_format_operand( src, str, MAX_OP_STRING, intel_syntax);
        STRNCAT( rbuf, str, len );
    }

    /* imm */
    if ( x86_operand_3rd( insn )) {
        STRNCAT( rbuf, ", ", len );
        x86_format_operand( x86_operand_3rd( insn ), 
                           str, MAX_OP_STRING, intel_syntax);
        STRNCAT( rbuf, str, len );
    }
    
    return true;
}

bool re_disassembler::format_insn_string_lookups(x86_insn_t *insn, char *rbuf, int len, bool tabs)
{
    rbuf[0] = 0;
    char tmp_str[1024];
    char tmp_comment_str[1024]; tmp_comment_str[0] = 0x0;
    char *tmp_comment = 0;
    char space_str[80];

    if(opt_auto_comments_on == true) tmp_comment = (char *)&tmp_comment_str;

    x86_op_t *dst, *src, *op;

    // INTEL STYLE: mnemonic dest, src, imm
    STRNCAT(rbuf, insn->prefix_string, len );
    STRNCAT(rbuf, insn->mnemonic, len );
    if(tabs) STRNCAT(rbuf, "\t", len );
    else STRNCAT(rbuf, " ", len );

    // dest
    if ( (dst = x86_operand_1st( insn )) && !(dst->flags & op_implied) ) {
        format_operand_lookups(dst, insn, tmp_str, tmp_comment, len);
        STRNCAT( rbuf, tmp_str, len );
    }

    // src
    if ( (src = x86_operand_2nd( insn )) ) {
        if ( !(dst->flags & op_implied) ) {
                STRNCAT( rbuf, ", ", len );
        }
        format_operand_lookups(src, insn, tmp_str, tmp_comment, len);
        STRNCAT( rbuf, tmp_str, len );
    }

    // imm
    if ( (op = x86_operand_3rd( insn )) ) {
        STRNCAT( rbuf, ", ", len );
        x86_format_operand( op, tmp_str, MAX_OP_STRING, intel_syntax);
        STRNCAT( rbuf, tmp_str, len );
    }
    
    if(opt_auto_comments_on && tmp_comment[0])  {
        int pos = strlen(rbuf), i;
        for(i = 0; i < 40 - pos; i++) space_str[i]=' ';
        space_str[i] = 0x0;
        STRNCAT( rbuf, space_str, len);
        STRNCAT( rbuf, "; ", len );
        STRNCAT( rbuf, tmp_comment, len );
    }

    return true;
}

re_addr_t re_disassembler::get_op_absolute_address(x86_op_t *op, x86_insn_t *insn)
{
    re_addr_t addr = 0;
    
    switch(op->type) {
    case op_immediate:       
        addr+= op->data.dword;
        break;
        
    case op_relative_near:
        addr = insn->addr + op->data.sbyte + insn->size;
        break;

    case op_relative_far:
        if ( op->datatype == op_word )
             addr += insn->addr + op->data.sword  + insn->size;
        else addr += insn->addr + op->data.sdword + insn->size;
        break;
        
    case op_absolute:
        if (op->datatype == op_descr16) 
             addr += op->data.absolute.offset.off16;
        else addr += op->data.absolute.offset.off32;
        break;

    case op_offset:
        addr+= op->data.sdword;
        break;

    case op_expression:
        addr+= op->data.expression.disp;
        break;
        
    default:
        addr = 0;
    }
    
    return addr;
}


bool re_disassembler::format_operand_lookups(x86_op_t *op, x86_insn_t *insn,
                                             char *rbuf, char *comments, int len)
{
    re_addr_t op_addr;
    char str[1024];
    char *s;

    rbuf[0] = 0;
    if(comments) comments[0] = 0;
    int have_printed = 0;

    if(re_optype_is_memory(op->type)) {
        op_addr = get_op_absolute_address(op, insn);

        if(op_addr) {
            if(file->va_functions_map.contains(op_addr) &&
                    file->va_functions_map.value(op_addr)) {
                STRNCAT( rbuf, file->va_functions_map.value(op_addr), len );

                if(comments) {
                sprintf(str, "f:%s=%08x ", file->va_functions_map.value(op_addr),
                        op_addr);
                STRNCAT( comments, str, len ); }
                have_printed = 1;
            } else

            if(file->va_symbols_map.contains(op_addr) &&
                    file->va_symbols_map.value(op_addr)) {
                STRNCAT( rbuf, file->va_symbols_map.value(op_addr), len );

                if(comments) {
                sprintf(str, "s:%s=%08x ", file->va_symbols_map.value(op_addr),
                        op_addr);
                STRNCAT( comments, str, len ); }
                have_printed = 1;
            } else

            if(file->va_imports_map.contains(op_addr) &&
                    file->va_imports_map.value(op_addr)) {
                STRNCAT( rbuf, file->va_imports_map.value(op_addr), len );

                if(comments) {
                sprintf(str, "i:%s=%08x", file->va_imports_map.value(op_addr),
                         op_addr);
                STRNCAT( comments, str, len ); }
                have_printed = 1;
            } else

            if(file->va_cstrings_hash.contains(op_addr) &&
                    (s = file->va_cstrings_hash.value(op_addr))) {
                snprintf(str, RE_DISASM_STR_DISP_LEN + 2, "\"%s\"", s);

                for(int i=0; i < RE_DISASM_STR_DISP_LEN + 2; i++)
                    if( (str[i] == 0x0d) ||
                        (str[i] == 0x0a) ||
                        (str[i] == 0x09)    ) (str[i] = ' ');

                if(strlen(s) > RE_DISASM_STR_DISP_LEN)
                    STRNCAT( str, "...\"", len );
                STRNCAT( rbuf, str, len );

                if(comments) {
                sprintf(str, "str va = %08x", op_addr);
                STRNCAT( comments, str, len ); }
                have_printed = 1;
            }

            if(op->type == op_expression) {
                unsigned long val;
                x86_format_operand( op, str, MAX_OP_STRING, intel_syntax);
                STRNCAT( rbuf, str, len );

                if(comments) {
                val = file->get_dword_at_va(op->data.expression.disp);
                if(val != BAD_ADDR && op->data.expression.disp) {
                    sprintf(str, "[%08x]=%08x ", op->data.expression.disp, val);
                    STRNCAT( comments, str, len );
                }}
                have_printed = 1;
            }
        } // op_addr
    }

    // if nothing applies
    if(!have_printed) {
        x86_format_operand( op, str, MAX_OP_STRING, intel_syntax);
        STRNCAT( rbuf, str, len );
    }

    return true;
}

re_addr_t re_disassembler::follow_control_flow(x86_insn_t *insn)
{
    if(insn->type != insn_jmp   &&
       insn->type != insn_jcc   &&
       insn->type != insn_call  &&
       insn->type != insn_callcc) return 0;

    x86_op_t *op = x86_operand_1st(insn);
    re_addr_t addr = 0;

    switch(op->type) {
    case op_immediate:
        addr+= op->data.dword;
        break;

    case op_relative_near:
        addr = insn->addr + op->data.sbyte + insn->size;
        break;

    case op_relative_far:
        if ( op->datatype == op_word )
             addr += insn->addr + op->data.sword  + insn->size;
        else addr += insn->addr + op->data.sdword + insn->size;
        break;

    case op_absolute:
        if (op->datatype == op_descr16)
             addr += op->data.absolute.offset.off16;
        else addr += op->data.absolute.offset.off32;
        break;

    case op_offset:
        addr+= op->data.sdword;
        break;

    case op_expression:
        addr+= file->get_dword_at_va(op->data.expression.disp);
        if(addr == BAD_ADDR) addr = 0;
        break;
    
    default:
        addr = 0;
    }

    if(file->va_to_file_offset(addr) != BAD_ADDR) return addr;

    return 0;
}



bool re_disassembler::stop_control_flow(x86_insn_t *insn)
{
    if(insn->type == insn_jmp)          return true;
    if(insn->type == insn_return)       return true;
    if(insn->type == insn_invalid_op)   return true;
    if(insn->type == insn_iret)         return true;
    if(insn->type == insn_halt)         return true;

    return false;
}

bool re_disassembler::disassemble_flow_recursive(re_addr_t va_start, int &reclevel)
{
    re_addr_t va_tmp, offset_tmp, va_branch_to;
    re_file_segment seg;
    x86_insn_t insn;
    int stop = 0;
    va_tmp = va_start;
    re_addr_desc *my_adr_desc;

    x86_op_t *dst, *src;
    re_addr_t op_va;


    offset_tmp = file->va_to_segment_offset(&seg, va_tmp);

    if(offset_tmp == BAD_ADDR) {
        emit msg(RE_LOG_DEBUG_3,
            "re_disassembler::disassemble_flow_recursive(): invalid address: "
             +QString::number(va_start, 16) + " - level: "
             +QString::number(reclevel)) ;
        return false;
    }

    if(address_space->insn_at_va(va_start)) {
        emit msg(RE_LOG_DEBUG_3,
            "re_disassembler::disassemble_flow_recursive(): allredy been at "
             +QString::number(va_start, 16) + " - level: "
             +QString::number(reclevel)) ;
        return false;
    }

    reclevel++;

    if(reclevel > 1000) {
        emit msg(RE_LOG_WARNING,
            "re_disassembler::disassemble_flow_recursive(): gone too deep:"
             +QString::number(va_start, 16) + " - level: "
             +QString::number(reclevel));
        goto stop_disasm_false;
    }

    emit msg(RE_LOG_DEBUG_3,
        "re_disassembler::disassemble_flow_recursive(): starting at "
         +QString::number(va_start, 16) + " - level: "
         +QString::number(reclevel)
         +"in seg: " + QString::number(seg.va_start, 16)
         +" - " + QString::number(seg.va_end, 16)


             ) ;

    while(!stop) {
      // emit msg(RE_LOG_NORMAL,
        // "re_disassembler::disassemble_flow_recursive(): in block at "
         // +QString::number(va_tmp, 16) + " - level: "
         // +QString::number(reclevel) + "(" + QString::number(va_start, 16) + ")") ;
                
        x86_disasm(seg.data_ptr, seg.file_size, seg.va_start, offset_tmp, &insn);
        if(insn.size && x86_insn_is_valid(&insn)) {
            // insert and check for overlap with prev -> hidden_insn
            my_adr_desc = address_space->insert_va(va_tmp, insn.size);
            
            // only for insn
                // check prev insn overlap
//                if( (my_adr_desc->prev != address_space->first()) && (my_adr_desc->prev->desc < RE_IMG_IS_INSN) &&
//                    ((my_adr_desc->prev->va + my_adr_desc->prev->desc) > va_tmp) ) {
//                    emit msg(RE_LOG_WARNING,
//                    "re_disassembler::disassemble_flow_recursive(): current insn va 0x"
//                     +QString::number(va_tmp, 16) + " conflicts with previous insn at 0x"
//                     +QString::number(my_adr_desc->prev->va, 16));
//                    hidden_insns_hash.insert(va_tmp, my_adr_desc->prev->va);
//                }

//                // check next insn overlap
//                while( (my_adr_desc->next != address_space->last()) && (my_adr_desc->next->desc < RE_IMG_IS_INSN) &&
//                    ((va_tmp + insn.size) > my_adr_desc->next->va) ) {
//                    emit msg(RE_LOG_WARNING,
//                    "re_disassembler::disassemble_flow_recursive(): current insn va 0x"
//                     +QString::number(va_tmp, 16) + " conflicts with next insn at 0x"
//                     +QString::number(my_adr_desc->next->va, 16));
//                    hidden_insns_hash.insert(my_adr_desc->next->va, va_tmp);
//                    my_adr_desc = my_adr_desc->next;
//                }
        }
        else goto stop_disasm_false;

        // recurse here
        if(va_branch_to = follow_control_flow(&insn)) {
            add_xref(va_branch_to, &insn, 'X', reclevel);
            emit msg(RE_LOG_DEBUG_3,
            "re_disassembler::disassemble_flow_recursive(): *** recursing into "
                 +QString::number(va_branch_to, 16)
                 + " from: "
                 +QString::number(va_tmp, 16)
                 + " - level: "
                 +QString::number(reclevel)) ;
            disassemble_flow_recursive(va_branch_to, reclevel);            
        }

        // check string data ref, sym ref
        if(dst = x86_operand_1st(&insn)) {
            if(op_va = get_op_absolute_address(dst, &insn)) {
                if(file->va_cstrings_hash.contains(op_va)) {
                        add_xref(op_va, &insn, 'w');
                }

                if(file->va_symbols_map.contains(op_va)) {
                        add_xref(op_va, &insn, 'w');
                }
            }
        }

        // check string data ref, sym ref
        if(src = x86_operand_2nd(&insn)) {
            if(op_va = get_op_absolute_address(src, &insn)) {
                if(file->va_cstrings_hash.contains(op_va)) {
                   add_xref(op_va, &insn, 'r');
                }

                if(file->va_symbols_map.contains(op_va)) {
                    add_xref(op_va, &insn, 'r');
                }
            }
        }

        if(stop_control_flow(&insn)) {
            stop = 1;
        }

        offset_tmp += insn.size;
        va_tmp += insn.size;
        x86_oplist_free(&insn);
                
        if(offset_tmp  >= seg.file_size) goto stop_disasm_false;

        if(address_space->insn_at_va(va_tmp)) {
            emit msg(RE_LOG_DEBUG_3,
                "re_disassembler::disassemble_flow_recursive(): allredy been at "
                 +QString::number(va_tmp, 16) + " - level: "
                 +QString::number(reclevel)) ;
            stop = 1;
        }
    }

    emit msg(RE_LOG_DEBUG_3,
        "re_disassembler::disassemble_flow_recursive(): stopping at "
         +QString::number(va_tmp, 16) + " - level: "
         +QString::number(reclevel)) ;

    reclevel --;
    return true;

stop_disasm_false:
    x86_oplist_free(&insn);
    emit msg(RE_LOG_DEBUG_3,
        "re_disassembler::disassemble_flow_recursive(): stopping at "
         +QString::number(va_tmp, 16) + " - level: "
         +QString::number(reclevel)) ;

    reclevel --;
    return false;
}

bool re_disassembler::disassemble_flow_from(re_addr_t va_start)
{
    int reclevel = 0;
    return disassemble_flow_recursive(va_start, reclevel);
}

int re_disassembler::disassemble_single(re_addr_t offset, x86_insn_t *insn)
{
    re_file_segment seg;
    re_addr_t seg_offs;
    if(!file->b_file_offset_to_segment_offset(seg, seg_offs, offset)) return 0;

    x86_disasm( seg.data_ptr, seg.file_size, seg.va_start, seg_offs, insn );

    return insn->size;
}

int re_disassembler::disassemble_single_va_into_space(re_addr_t va, x86_insn_t *insn)
{
    if(!insn) return 0;
    if(address_space->insn_at_va(va)) return 0;

    re_addr_t offs;
    if(!file->b_va_to_file_offset(offs, va)) return 0;

    disassemble_single(offs, insn);

    if(insn->size && x86_insn_is_valid(insn)) {
        address_space->insert_va(va, insn->size);
    }

    return insn->size;
}


void re_disassembler::set_autocomment_on(bool v)
{
    opt_auto_comments_on = v;
}

bool re_disassembler::search_deep_acss(
    re_acss_compiler *compiler, re_acss_result *my_result,
    re_addr_t &va_from,
    re_addr_t va_until,
    re_file_segment *seg)
{
    if(!compiler->is_block())
          return search_deep_acss_exp(compiler, my_result,
                                      va_from, va_until, seg);
    else {
        emit msg(RE_LOG_DEBUG_1, "compiler block state: block size = "
                 + QString::number(compiler->block_size)
                 + " threshold = "
                 + QString::number(compiler->threshold) + "%"
                 );
        return search_deep_acss_block(compiler, my_result,
                                      va_from, va_until, seg);
    }
}

bool re_disassembler::search_deep_acss_block(
    re_acss_compiler *compiler, re_acss_result *my_result,
    re_addr_t &va_from,
    re_addr_t va_until, re_file_segment *seg)
{
    emit msg(RE_LOG_DEBUG_1, "acss block search: starting at: 0x"
             +QString::number(va_from, 16));

    my_result->va_start = BAD_ADDR;
    my_result->va_end = BAD_ADDR;
    my_result->insn_count = 0;
    my_result->byte_count = 0;

    re_addr_t my_va_from = va_from, block_start, last_block_end;

    bool stop = false;

    if( (va_from + compiler->block_size) > va_until) return false;

    int byte_counts, insn_counts;

    while( !stop ) {
        if( (my_va_from + compiler->block_size) > va_until ) {
            // stop here
            my_result->va_start = BAD_ADDR;
            my_result->va_end = BAD_ADDR;
            my_result->insn_count = 0;
            my_result->byte_count = 0;
            return false;
        }

        // find first occurence for block of expr or stop
        if(!search_deep_acss_exp(
                    compiler, my_result, my_va_from, va_until, seg)) {
            // stop here
            my_result->va_start = BAD_ADDR;
            my_result->va_end = BAD_ADDR;
            my_result->insn_count = 0;
            my_result->byte_count = 0;
            return false;
        }

        // reposition back to result start
        my_va_from  = my_result->va_start;
        block_start = my_result->va_start;

        // find a single block at this position
        int j = 0; byte_counts = 0; insn_counts = 0;
        while( search_deep_acss_exp(compiler,  my_result,
                                    my_va_from,
                                    block_start + compiler->block_size,
                                    seg) ) {

            emit msg(RE_LOG_DEBUG_1, "acss block: exp match at: 0x"
                     +QString::number(my_result->va_start, 16));
            insn_counts += my_result->insn_count;
            byte_counts += my_result->byte_count;

            last_block_end = my_result->va_end;

            j++;
        }

        if((byte_counts * 100 / compiler->block_size) >= compiler->threshold) {
            // found a block!

            // reposition va_from for consecutive call of func
            va_from = my_va_from;

            my_result->va_start = block_start;
            my_result->va_end = last_block_end;
            // keep va_end from last result
            my_result->byte_count = byte_counts;
            my_result->insn_count = insn_counts;

            return true;
        }
    }

    my_result->va_start = BAD_ADDR;
    my_result->va_end = BAD_ADDR;
    my_result->insn_count = 0;
    my_result->byte_count = 0;
    return false;
}

bool re_disassembler::search_deep_acss_exp(
    re_acss_compiler *compiler, re_acss_result *my_result,
    re_addr_t &va_from,
    re_addr_t va_until, re_file_segment *seg)
{
    my_result->va_start = BAD_ADDR;
    my_result->va_end = BAD_ADDR;
    my_result->insn_count = 0;
    my_result->byte_count = 0;

    int j, l, breakme, breakme_inner, have_found_1st, size;
    unsigned int k;
    unsigned int insn_count = 0, byte_count = 0;

    re_addr_t offs, max_offs, first_found_offs;

    x86_insn_t *insn;

    offs = va_from - seg->va_start;

    if(!seg->file_size) {
        emit msg(RE_LOG_DEBUG_3,
                 "stopping immediately: segment has zero size"
                 );

        return false;
    }

    max_offs = va_until - seg->va_start;

    if( (va_from < seg->va_start) || (va_until > seg->va_end) ) {
        emit msg(RE_LOG_DEBUG_3,
                 "stopping immediately: va not in seg range: " + (QString)
                   " va: " + QString::number(va_from, 16)
                 + " seg.va_start: " + QString::number(seg->va_start, 16)
                 + " va_until: " + QString::number(va_until, 16)
                 + " seg.va_end: " + QString::number(seg->va_end, 16)
                 + " max_offs: " + QString::number(max_offs, 16)
                 );

        return false;
    }

    if(compiler->expressions.isEmpty()) {
        emit msg(RE_LOG_DEBUG_3,
                 "stopping immediately: compiler has no expressions "
                 );
        return false;
    }

    // starting deep search

    insn = new x86_insn_t;
    while(offs < max_offs) {
        // position first at a valid insn
        size = x86_disasm(seg->data_ptr, seg->file_size,
                          seg->va_start, offs, insn);
        if (!size || !x86_insn_is_valid(insn)) {
            offs ++; continue;
        }

        // match first expression
        have_found_1st = 0;
        while( !have_found_1st  && (offs < max_offs)) {
            size = x86_disasm(seg->data_ptr, seg->file_size,
                              seg->va_start, offs, insn );
            if (!size || !x86_insn_is_valid(insn)) {
                offs ++; continue;
            }

            if(compiler->expressions[0]->evaluate(insn) != RE_EXPR_MATCH)
                offs++;
            else have_found_1st = 1;
        }

        // have not found first expression match -> return false
        if(offs >= max_offs ) {
            // stop here         
            my_result->va_start = BAD_ADDR;
            my_result->va_end = BAD_ADDR;
            my_result->insn_count = 0;
            my_result->byte_count = 0;
            return false;
        }

        // found 1st here, 1 time.
        first_found_offs = offs;

        j=0; k=0; l=0; breakme = 0; insn_count = 0; byte_count = 0;

        while(j < compiler->expressions.count() && !breakme) {
            k=0; breakme_inner = 0;
            while( !breakme_inner ) {
                size = x86_disasm(seg->data_ptr, seg->file_size,
                                  seg->va_start, offs, insn);
                if(!size || !x86_insn_is_valid(insn)) {
                    breakme_inner = 1; break;
                }

                if(((compiler->expressions[j]->evaluate(insn))
                                                        == RE_EXPR_MATCH) &&
                        (k < compiler->expressions[j]->maxcount) ) {
                    k++; offs +=size;
                    byte_count += size;
                    // old: if wildcard: evaluate next, and break if true
                    // now: if maxcount > 1
                    if(compiler->expressions[j]->maxcount > 1) {
                        if(j < (compiler->expressions.count()-1)) {
                            size = x86_disasm(seg->data_ptr, seg->file_size,
                                              seg->va_start, offs, insn);
                            if((compiler->expressions[j+1]->evaluate(insn))
                                    == RE_EXPR_MATCH) breakme_inner = 1;
                        }
                    }
                }
                else breakme_inner =1;
            }

            if( (k >= compiler->expressions[j]->count) &&
                    (k <= compiler->expressions[j]->maxcount) ) {
                j++;
                insn_count += k;
            }
            else breakme = 1;
        }

        if(j == compiler->expressions.count()) {
            delete insn;

            // set va_from for consecutive calls of search
            va_from = offs + seg->va_start;

            my_result->byte_count = byte_count;
            my_result->insn_count = insn_count;
            my_result->va_start = first_found_offs + seg->va_start;
            my_result->va_end = offs + seg->va_start - 1;

            return true;
        }

        //offs+=size;
       if(offs <= first_found_offs) offs++;
    }

    delete insn;

    my_result->va_start = BAD_ADDR;
    my_result->va_end = BAD_ADDR;
    my_result->insn_count = 0;
    my_result->byte_count = 0;
    return false;
}

QHash<re_addr_t, re_acss_result> *re_disassembler::search_deep_all(
    re_acss_compiler *compiler, int maxresults)
{
    char buf[1024];

    int i=0, j;
    re_addr_t va_start, va_end, va_after_found_pos;

    QHash<re_addr_t, re_acss_result> *results =
            new QHash<re_addr_t, re_acss_result>;

    re_acss_result my_result;
    re_file_segment seg;

    // -- search all loaded segments for functions --
    for(i=0; i<file->segments.count(); i++)
    {
        if(! (file->segments.at(i).flags & RE_SEGMENT_LOAD) ) continue;
        seg = file->segments.at(i);

        va_start = seg.va_start;
        va_end = seg.va_end;

        sprintf(buf, "searching deep: phdr %d (%08x - %08x) from offset %08x",
                i, va_start, va_end, seg.file_offset);
        emit msg(RE_LOG_DEBUG_3, QString::fromAscii(buf));

        j = 0;
        va_after_found_pos = va_start;

        while( search_deep_acss(compiler, &my_result,
                                va_after_found_pos, va_end,
                                &seg) ) {
            // ======== ADD HERE
            results->insert(my_result.va_start,
                            my_result);
            emit msg(RE_LOG_DEBUG_1, "acss match at: 0x"
                     +QString::number(my_result.va_start, 16));
            j++;            
            if(maxresults && (j>=maxresults))  {
                emit msg(RE_LOG_NORMAL, "breaking search at: 0x"
                         +QString::number(my_result.va_start, 16));
                break;
            }
                // to exit while loop
            sprintf(buf, "searching deep: phdr %d (%08x - %08x)",
                    i, va_after_found_pos, va_end);
            emit msg(RE_LOG_DEBUG_3, QString::fromAscii(buf));
        }

    } 

    return results;
}

bool re_disassembler::full_analysis()
{
    QList<re_addr_t> va_starts;
    
    // 1. disassemble from ep
    emit progress_changed_pct(0);
    emit msg(RE_LOG_NORMAL, "full analysis 1: ctf disassembly from ep");

    disassemble_flow_from(file->va_entry_point);

    emit progress_changed_pct(5);

    emit msg(RE_LOG_DEBUG_1, 
        "full analysis: current # of insns in address space: " +
         QString::number(address_space->count()));


    // 2. search for func prologues, disassemble results
    emit msg(RE_LOG_NORMAL,
        "full analysis 2: search for functions, disassemble them ...");

    re_acss_compiler *acompiler = new re_acss_compiler();
    acompiler->init(logger);
    
    char *filterstr =
            (char *)
            "\n\
1 x insn { type == push, op1_type == register, \n\
                         op1_reg_name == ebp; }; \n\
                                                \n\
1 x insn { type == mov, op1_type == register, op1_reg_name == ebp, \n\
                        op2_type == register, op2_reg_name == esp; };\n";

    emit msg(RE_LOG_DEBUG_1, "scanning ... acss filter: \n" +
             QString::fromAscii(filterstr));

    acompiler->set_filter(filterstr);
    emit progress_changed_pct(7);
    acompiler->compile_expressions();
    emit progress_changed_pct(10);

    QHash<re_addr_t, re_acss_result> *results = search_deep_all(acompiler);
    emit msg(RE_LOG_DEBUG_1, "# of acss results: " +
                    QString::number(results->count()));

    emit progress_changed_pct(15);

    va_starts = results->keys();

    if(!va_starts.isEmpty()) {
        for(int i=0; i < va_starts.count(); ++i) {
            emit progress_changed_pct(15 + (80.0 / va_starts.count() * i) );
            disassemble_flow_from(va_starts.at(i));
        }
    }
    emit progress_changed_pct(95);

    emit msg(RE_LOG_DEBUG_1,
        "full analysis: current # of insns in address space: " +
         QString::number(address_space->count()));
         

    // 3. disassemble from known functions/imports(file->va_functions_map)
    emit msg(RE_LOG_NORMAL, "full analysis 3: disasm known funcs");
    va_starts = file->va_functions_map.keys();
    if(!va_starts.isEmpty()) {
        for(int i=0; i < va_starts.count(); ++i) {
            disassemble_flow_from(va_starts.at(i));
        }
    }
    emit progress_changed_pct(97);

//    emit msg(RE_LOG_NORMAL, "full analysis 3: disasm imports");
//    va_starts = file->va_imports_map.keys();
//    if(!va_starts.isEmpty()) {
//        for(int i=0; i < va_starts.count(); ++i) {
//            disassemble_flow_from(va_starts.at(i));
//        }
//    }

//    emit msg(RE_LOG_NORMAL, "full analysis 4: disasm symbols");
//    va_starts = file->va_symbols_map.keys();
//    if(!va_starts.isEmpty()) {
//        for(int i=0; i < va_starts.count(); ++i) {
//            disassemble_flow_from(va_starts.at(i));
//        }
//    }

    emit progress_changed_pct(100);

    emit msg(RE_LOG_NORMAL,
        "full analysis: current # of insns in address space: " +
         QString::number(address_space->count()));

    re_process_address_space();
}

void re_disassembler::re_process_address_space()
{
    if(address_space->is_cloned()) {
        emit disassembly_changed();
        return;
    }

    emit msg(RE_LOG_NORMAL,
        "re_process_address_space: analyzing address space ...");
    emit msg(RE_LOG_NORMAL,
        "re_process_address_space: current # of entries in address space: " +
         QString::number(address_space->count()));

    re_addr_t block_start;
    re_addr_t block_len;
    re_addr_t current_segment_end;

    re_addr_t tmp_va;

    re_addr_t i = 0, j;

    remove_datablocks();

    // add symbols
    if(!file->va_symbols_map.isEmpty())
    for(i=0; i<file->va_symbols_map.count(); i++) {
        // insert va does nothing if allready added
        address_space->insert_va(file->va_symbols_map.keys().at(i), 'S');
        emit msg(RE_LOG_NORMAL,(QString)
            "re_process_address_space: inserting symbol at  "
                 + QString::number(file->va_symbols_map.keys().at(i), 16));

    }

    // add strings
    if(!file->va_cstrings_hash.isEmpty())
    for(i=0; i<file->va_cstrings_hash.count(); i++) {
        // insert va does nothing if allready added
        address_space->insert_va(file->va_cstrings_hash.keys().at(i), 'c');
        emit msg(RE_LOG_NORMAL,(QString)
            "re_process_address_space: inserting string at  "
                 + QString::number(file->va_cstrings_hash.keys().at(i), 16));
    }

    i = 0;
    for(j=0; j<file->segments.count(); j++) {
        if( (i>=file->segments.at(j).file_offset) &&
                (i < (file->segments.at(j).file_offset +
                 file->segments.at(j).file_size) ))
                 {
            current_segment_end =
                file->segments.at(j).file_offset +
                file->segments.at(j).file_size;
                 }
    }

    // if not in valid segment
    if(!current_segment_end) {
        current_segment_end = file->filesize -1;
    }

    while( i < file->filesize ) {
        // --- block
        if(
              (i < current_segment_end) && (!address_space->image_map[i]
              || ((address_space->image_map[i] && (address_space->image_map[i]->desc == 'c')))
              || ((address_space->image_map[i] && (address_space->image_map[i]->desc == 'S'))))

          ) {
            block_len = 0;
            current_segment_end = 0;
            block_start = i;
            for(j=0; j<file->segments.count(); j++) {
                if( (i>=file->segments.at(j).file_offset) &&
                        (i < (file->segments.at(j).file_offset +
                         file->segments.at(j).file_size) ))
                         {
                    current_segment_end =
                        file->segments.at(j).file_offset +
                        file->segments.at(j).file_size;
                         }
            }

            // if not in valid segment
            if(!current_segment_end) {
                current_segment_end = file->filesize-1;
            }

            while( (i < current_segment_end) &&
                  (!address_space->image_map[i]
                  || ((address_space->image_map[i] && (address_space->image_map[i]->desc == 'c')))
                  || ((address_space->image_map[i] && (address_space->image_map[i]->desc == 'S'))))

                 ) {
                block_len++;
                i++;
            }

            if(file->b_file_offset_to_va(tmp_va, block_start)) {
                emit msg(RE_LOG_DEBUG_1,
                    "re_process_address_space: inserting gap 'G' " +
                         QString::number(tmp_va, 16) + " len: " +
                         QString::number(block_len)
                     );

                address_space->insert_va(tmp_va, 'G');
                datablocks_hash.insert(block_start, block_len);
            } else {
                emit msg(RE_LOG_DEBUG_1,
                    "re_process_address_space: not inserting gap 'g' " +
                         QString::number(block_start, 16) + " len: " +
                         QString::number(block_len)
                     );
//                address_space->insert_va(block_start, 'g'); // uuhh uuhhhhuu
//                datablocks_hash.insert(block_start, block_len);
            }
        }
        // --- no block
        else {
            if(i>=current_segment_end) i++;
            else i+= address_space->image_map[i]->desc;
        }
    }

    emit msg(RE_LOG_NORMAL,(QString)
        "re_process_address_space: found "
             + QString::number(datablocks_hash.count())
             + " gaps in address space");
    emit msg(RE_LOG_NORMAL,
        "re_process_address_space: current # of entries in address space: " +
         QString::number(address_space->count()));
    emit disassembly_changed();
}


void re_disassembler::format_mnemonic(x86_insn_t *insn, char *buf, int len)
{
    memset(buf, 0, len);
    /* INTEL STYLE: mnemonic dest, src, imm */
    STRNCAT( buf, insn->prefix_string, len );
    STRNCAT( buf, insn->mnemonic, len );
}

void re_disassembler::format_operands(x86_insn_t *insn, char *buf, int len)
{
    char str[MAX_OP_STRING];
    x86_op_t *src, *dst;

    memset(buf, 0, len);
        /* dest */
    if ( (dst = x86_operand_1st( insn )) && !(dst->flags & op_implied) ) {
        x86_format_operand( dst, str, MAX_OP_STRING, intel_syntax);
        STRNCAT( buf, str, len );
    }

    /* src */
    if ( (src = x86_operand_2nd( insn )) ) {
        if ( !(dst->flags & op_implied) ) {
            STRNCAT( buf, ", ", len );
        }
        x86_format_operand( src, str, MAX_OP_STRING, intel_syntax);
        STRNCAT( buf, str, len );
        // FSCKING libdis "bug":
        if(buf[strlen(buf)-2] == '+' && buf[strlen(buf)-1] == ']') {
            buf[strlen(buf)-2] = 0x0;
            STRNCAT(buf, "+0x0]", len);
        }
    }

    /* imm */
    if ( x86_operand_3rd( insn )) {
        STRNCAT( buf, ", ", len );
        x86_format_operand( x86_operand_3rd( insn ),
        str, MAX_OP_STRING, intel_syntax);
        STRNCAT( buf, str, len );
    }
}


bool re_disassembler::op1_lookup_addr_n_format(x86_insn_t *insn, re_addr_t &op_addr, char *op_str)
{
    x86_op_t *dst;

    op_addr = 0;
    op_str[0] = 0x0;

    /* dest */
    if ( (dst = x86_operand_1st( insn )) && !(dst->flags & op_implied) ) {
        x86_format_operand( dst, op_str, MAX_OP_STRING, intel_syntax);

        if(re_optype_is_memory(dst->type))
            op_addr = get_op_absolute_address(dst, insn);

        return true;
    }

    return false;
}

bool re_disassembler::op2_lookup_addr_n_format(x86_insn_t *insn, re_addr_t &op_addr, char *op_str)
{
    char str[MAX_OP_STRING];
    x86_op_t *dst, *src;

    op_addr = 0;
    op_str[0] = 0x0;

    dst = x86_operand_1st( insn );

    /* dest */
    if ( (src = x86_operand_2nd( insn )) ) {
        if ( !(dst->flags & op_implied) ) {
            strncat(op_str, ", ", 32 );
        }
        x86_format_operand( src, str, MAX_OP_STRING, intel_syntax);
        strncat( op_str, str, 32 );
        // FSCKING libdis "bug":
        if(op_str[strlen(op_str)-2] == '+' && op_str[strlen(op_str)-1] == ']') {
            op_str[strlen(op_str)-2] = 0x0;
            strncat(op_str, "+0x0]", 32);
        }

        if(re_optype_is_memory(src->type))
            op_addr = get_op_absolute_address(src, insn);

        return true;
    }

    return false;
}

bool re_disassembler::op3_format(x86_insn_t *insn, char *op_str)
{
    char str[MAX_OP_STRING];
    op_str[0] = 0x0;

    /* imm */
    if ( x86_operand_3rd( insn )) {
        strncat( op_str, ", ", 32 );
        x86_format_operand( x86_operand_3rd( insn ),
        str, MAX_OP_STRING, intel_syntax);
        strncat( op_str, str, 32 );

        return true;
    }

    return false;
}

void re_disassembler::remove_datablocks()
{
    if(address_space->is_cloned()) return;

    re_addr_t va, offset;

    if(!datablocks_hash.isEmpty()) {
        foreach(offset, datablocks_hash.keys()) {
            if(file->b_file_offset_to_va(va, offset)) {
                if(!address_space->remove_va(va)) {
                    emit msg(RE_LOG_ERROR,
                        "remove datablock: REMOVE ERROR!!!!! at offs "
                             +QString::number(offset, 16)
                             +" va "
                             +QString::number(va));
                } else {
//                    emit msg(RE_LOG_NORMAL,
//                        "removed datablock: at offs "
//                             +QString::number(offset, 16)
//                             +" va "
//                             +QString::number(va, 16));
                }
            }
            else {
                emit msg(RE_LOG_ERROR,
                    "remove datablock: offs to va ERROR!!!!! at offs "
                         +QString::number(offset, 16));
            }
        }
        datablocks_hash.clear();
    }

}

void re_disassembler::add_xref(re_addr_t dst, x86_insn_t *insn, char t) {
    struct re_xref xref;
    char buf[9];

    if(va_xrefs_mmap.contains(dst)) {
        QList<struct re_xref> xrefs = va_xrefs_mmap.values(dst);

        // check if allready present
        if(!xrefs.isEmpty()) {
            foreach(struct re_xref xr_tmp, xrefs) {
                if(xr_tmp.va_src == insn->addr) return;
            }
        }
    }

    // add here
    xref.va_src = insn->addr;
    xref.va_dst = dst;
    xref.t      = t;

    if(t=='X') {
        switch(insn->type) {
        case insn_jmp:
            xref.t = 'J';
            if(!file->va_functions_map.contains(dst)) {
                sprintf(buf, "%08X", dst);
                file->va_names_hash.insert(dst, "L_" + QString::fromAscii(buf));
            }
            break;

        case insn_jcc:
            xref.t = 'b';
            break;

        case insn_call:
            xref.t = 'C';
            if(!file->va_functions_map.contains(dst)) {
                sprintf(buf, "%08X", dst);
                file->va_analyzed_functions_hash.insert(dst, "F_" + QString::fromAscii(buf));
            }
            break;

        case insn_callcc:
            xref.t = 'c';
            break;

        default:
            break;
        }

    }

    va_xrefs_mmap.insert(dst, xref);
}

void re_disassembler::add_xref(re_addr_t dst, x86_insn_t *insn, char t, int reclevel)
{
    struct re_xref xref;
    char buf[9];

    if(va_xrefs_mmap.contains(dst)) {
        QList<struct re_xref> xrefs = va_xrefs_mmap.values(dst);

        // check if allready present
        if(!xrefs.isEmpty()) {
            foreach(struct re_xref xr_tmp, xrefs) {
                if(xr_tmp.va_src == insn->addr) return;
            }
        }
    }

    // add here
    xref.va_src = insn->addr;
    xref.va_dst = dst;
    xref.t      = t;

    if(t=='X') {
        switch(insn->type) {
        case insn_jmp:
            xref.t = 'J';
            if(!file->va_functions_map.contains(dst)) {
                sprintf(buf, "%08X", dst);
                file->va_names_hash.insert(dst, "L_"
                                           + QString::fromAscii(buf)
                                           + "_" + QString::number(reclevel));
            }
            break;

        case insn_jcc:
            xref.t = 'b';
            break;

        case insn_call:
            xref.t = 'C';
            if(!file->va_functions_map.contains(dst)) {
                sprintf(buf, "%08X", dst);
                file->va_analyzed_functions_hash.insert(dst, "F_"
                                                        + QString::fromAscii(buf)
                                                        + "_" + QString::number(reclevel));
            }
            break;

        case insn_callcc:
            xref.t = 'c';
            break;

        default:
            break;
        }

    }

    va_xrefs_mmap.insert(dst, xref);
}

void re_disassembler::set_address_space(re_address_space *as)
{
    address_space = as;
}

re_address_space *re_disassembler::get_address_space()
{
    return address_space;
}
