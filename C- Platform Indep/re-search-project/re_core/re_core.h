#ifndef RE_CORE_H
#define RE_CORE_H

#include "re_core_global.h"
#include "re_file.h"
#include "re_target.h"
#include "re_logger.h"
#include "libdis/libdis.h"
#include "re_target.h"
#include "re_acss/re_acss_compiler.h"
#include "re_wildasm/re_wildasm_compiler.h"

class RE_CORESHARED_EXPORT re_core {
public:
    re_core();

    static QString byte_to_hex_string(unsigned char b);
    static QString dword_to_hex_string(re_addr_t dw);
    static QString word_to_hex_string(re_addr_t w);

    static QString dword_to_pure_hex_string(re_addr_t dw);
};

#endif // RE_CORE_H
