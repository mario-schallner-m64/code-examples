#ifndef RE_SCRIPTLIB_GLOBAL_H
#define RE_SCRIPTLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RE_SCRIPTLIB_LIBRARY)
#  define RE_SCRIPTLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RE_SCRIPTLIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RE_SCRIPTLIB_GLOBAL_H
