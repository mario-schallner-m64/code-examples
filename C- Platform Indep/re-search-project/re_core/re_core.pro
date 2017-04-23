#-------------------------------------------------
#
# Project created by QtCreator 2011-11-27T19:49:17
#
#-------------------------------------------------

QT       -= gui

TARGET = re_core
TEMPLATE = lib

DEFINES += RE_CORE_LIBRARY

SOURCES += re_core.cpp \
    re_logger.cpp \
    re_file.cpp \
    libdis/x86_operand_list.c \
    libdis/x86_misc.c \
    libdis/x86_insn.c \
    libdis/x86_imm.c \
    libdis/x86_format.c \
    libdis/x86_disasm.c \
    libdis/ia32_settings.c \
    libdis/ia32_reg.c \
    libdis/ia32_operand.c \
    libdis/ia32_opcode_tables.c \
    libdis/ia32_modrm.c \
    libdis/ia32_invariant.c \
    libdis/ia32_insn.c \
    libdis/ia32_implicit.c \
    re_file_elf32.cpp \
    re_file_pe32.cpp \
    re_file_raw32.cpp \
    re_target.cpp \
    re_address_space.cpp \
    re_disassembler.cpp \
    re_acss/re_acss_expression.cpp \
    re_acss/re_acss_compiler.cpp \
    re_wildasm/re_wildasm_compiler.cpp


HEADERS += re_core.h\
        re_core_global.h \
    re_logger.h \
    re_file.h \
    re_types.h \
    re_elf.h \
    re_pe.h \
    libdis/x86_operand_list.h \
    libdis/x86_imm.h \
    libdis/qword.h \
    libdis/libdis.h \
    libdis/ia32_settings.h \
    libdis/ia32_reg.h \
    libdis/ia32_operand.h \
    libdis/ia32_opcode_tables.h \
    libdis/ia32_modrm.h \
    libdis/ia32_invariant.h \
    libdis/ia32_insn.h \
    libdis/ia32_implicit.h \
    re_file_elf32.h \
    re_file_pe32.h \
    re_file_raw32.h \
    re_target.h \
    re_address_space.h \
    re_disassembler.h \
    re_acss/re_acss_expression.h \
    re_acss/re_acss_compiler.h \
    re_wildasm/re_wildasm_compiler.h

symbian {
    #Symbian specific definitions
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE0AC7BB0
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = re_core.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
