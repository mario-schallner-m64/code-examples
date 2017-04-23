#ifndef RE_CORE_GLOBAL_H
#define RE_CORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RE_CORE_LIBRARY)
#  define RE_CORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define RE_CORESHARED_EXPORT Q_DECL_IMPORT
#endif

#define BAD_ADDR    0xffffffff

enum {
    RE_LOG_DEBUG_1 = 1,
	RE_LOG_DEBUG_2,
	RE_LOG_DEBUG_3,
    RE_LOG_NORMAL,
    RE_LOG_WARNING,
    RE_LOG_ERROR
};

// image_map entries
enum {
    RE_IMG_NONE         =  0x00,
    RE_IMG_IS_INSN      =  0x10, // 00 ... 0f -> insn size
    RE_IMG_EXTRALINE    =  0x20  // or ... extraline
};

#endif // RE_CORE_GLOBAL_H
