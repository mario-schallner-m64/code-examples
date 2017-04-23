#include "re_core.h"


re_core::re_core()
{
}

QString re_core::byte_to_hex_string(unsigned char b)
{
    char buf[8];

    snprintf(buf, 8, "0x%02X", b);
    return QString::fromAscii(buf);
}

QString re_core::dword_to_hex_string(re_addr_t dw)
{
    char buf[16];

    snprintf(buf, 16, "0x%08X", dw);
    return QString::fromAscii(buf);
}

QString re_core::word_to_hex_string(re_addr_t w)
{
    char buf[16];

    snprintf(buf, 16, "0x%04X", w & 0x0000ffff);
    return QString::fromAscii(buf);
}

QString re_core::dword_to_pure_hex_string(re_addr_t dw)
{
    char buf[16];

    snprintf(buf, 16, "%08X", dw);
    return QString::fromAscii(buf);
}
