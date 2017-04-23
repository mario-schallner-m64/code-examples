#ifndef RE_DISASSEMBLER_H
#define RE_DISASSEMBLER_H

#include "re_address_space.h"
#include "re_logger.h"
#include "re_file.h"
#include "libdis/libdis.h"
#include "re_acss/re_acss_compiler.h"


#include <QObject>
#include <QList>

#define STRNCAT( buf, str, len ) do {   				\
	int _i = strlen(str), _blen = strlen(buf), _len = len - 1;  	\
	if ( len ) {							\
        	strncat( buf, str, _len );  				\
		if ( _len <= _i ) {					\
			buf[_blen+_len] = '\0';				\
			len = 0;					\
		} else {						\
			len -= _i;					\
		}							\
	}								\
} while( 0 )

#define re_optype_is_memory( optype )                           \
	( optype == op_immediate    || optype == op_relative_near || \
      optype == op_relative_far || optype == op_absolute ||      \
      optype == op_offset || optype == op_expression)

#define RE_DISASM_STR_DISP_LEN 30

struct re_acss_result
{
    re_addr_t   va_start;
    re_addr_t   va_end;
    int         insn_count;
    int         byte_count;
};

struct re_xref
{
    re_addr_t   va_src;
    re_addr_t   va_dst;

    char        t;      // "type" : c all, j mp, b ranche, r ead, w rite...
};

class RE_CORESHARED_EXPORT re_disassembler : public QObject
{
    Q_OBJECT
public:
    explicit re_disassembler(QObject *parent = 0);
    ~re_disassembler();

    void init(re_file *f, re_address_space *as, re_logger *l);

    // disassembly functions
    bool disassemble_linear_cnt(re_addr_t va_start, re_addr_t insn_count);
    bool disassemble_linear_range(re_addr_t va_start, re_addr_t va_end);
    bool disassemble_flow_from(re_addr_t va);
    bool disassemble_flow_recursive(re_addr_t va_start, int &reclevel);
    int disassemble_single(re_addr_t offset, x86_insn_t *insn);
    int disassemble_single_va_into_space(re_addr_t va, x86_insn_t *insn);
    bool full_analysis();

    // acss functions
    bool search_deep_acss(
        re_acss_compiler *compiler, re_acss_result *res,
        re_addr_t &va_from, re_addr_t va_until, re_file_segment *seg);

    bool search_deep_acss_exp(
        re_acss_compiler *compiler, re_acss_result *res,
        re_addr_t &va_from, re_addr_t va_until, re_file_segment *seg);

    bool search_deep_acss_block(
        re_acss_compiler *compiler, re_acss_result *res,
        re_addr_t &va_from, re_addr_t va_until, re_file_segment *seg);

    QHash<re_addr_t, re_acss_result> *search_deep_all(
        re_acss_compiler *compiler,
        int maxresults = 0);


    // disassembly output/formating functions
    void set_autocomment_on(bool v);
    bool format_insn_textline(re_addr_t va, char *rbuf, int maxchar);
    bool format_insn_textline_foffset(re_addr_t foffset, char *rbuf, int maxchar);
    bool format_insn_string(x86_insn_t *insn, char *rbuf, int len);
    bool format_insn_string_lookups(x86_insn_t *insn, char *rbuf, int len, bool tabs=false);
    bool print_disassembly(QString &rstr);

    void format_mnemonic(x86_insn_t *insn, char *buf, int len);
    void format_operands(x86_insn_t *insn, char *buf, int len);

    bool op1_lookup_addr_n_format(x86_insn_t *insn, re_addr_t &op_addr, char *op_str);
    bool op2_lookup_addr_n_format(x86_insn_t *insn, re_addr_t &op_addr, char *op_str);
    bool op3_format(x86_insn_t *insn, char *op_str);

    // helper funcs
    re_addr_t get_op_absolute_address(x86_op_t *op, x86_insn_t *insn); // or 0
    bool format_operand_lookups(x86_op_t *op, x86_insn_t *insn, char *rbuf,
                                char *comments, int len);   

    re_addr_t follow_control_flow(x86_insn_t *insn); // or 0
    bool stop_control_flow(x86_insn_t *insn);

    // no getter/setter functions yet -> public. dirty! ;)
    QMultiMap <re_addr_t, struct re_xref> va_xrefs_mmap;
    QHash<re_addr_t, re_addr_t> datablocks_hash; // offs start, len
    QHash<re_addr_t, re_addr_t> hidden_insns_hash;

    void add_xref(re_addr_t dst, x86_insn_t *insn, char t);
    void add_xref(re_addr_t dst, x86_insn_t *insn, char t, int reclevel);

signals:
    void msg(const int level, const QString S);
    void progress_changed_pct(int pct);

    void disassembly_changed();

public slots:
    void    re_process_address_space();
    void    remove_datablocks();
    void    set_address_space(re_address_space *as);
    re_address_space *get_address_space();

private:
    re_file *file;
    re_address_space *address_space;
    re_logger *logger;

    bool opt_auto_comments_on;
};

#endif // RE_DISASSEMBLER_H
