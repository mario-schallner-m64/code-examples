#-------------------------------------------------
#
# Project created by QtCreator 2011-11-27T18:33:05
#
#-------------------------------------------------

QT       += core gui script scripttools

TARGET = re_view
TEMPLATE = app


SOURCES +=\
        rv_mainwin.cpp \
    re_view.cpp \
    rv_elfhdr_model.cpp \
    rv_elf_phdr_model.cpp \
    rv_elf_shdr_model.cpp \
    rv_disasm_thread.cpp \
    rv_progress_dialog.cpp \
    rv_file_analyze_thread.cpp \
    rv_hex_edit.cpp \
    rv_hex_editor.cpp \
    rv_hex_editor_infopane.cpp \
    rv_about_dialog.cpp \
    rv_splitter.cpp \
    rv_splitter_handle.cpp \
    rv_hex_editor_bufferpane.cpp \
    rv_hex_edit_undocommands.cpp \
    rv_sideinfo_listbox.cpp \
    rv_script_processor.cpp \
    rv_script_execution_thread.cpp \
    rv_sideinfo_custom_widget.cpp \
    rv_script_highlighter.cpp \
    rv_script_code_editor.cpp \
    rv_slider.cpp \
    rv_disasm_view.cpp \
    rv_disasm_xrefs_widget.cpp \
    rv_idisasm.cpp \
    rv_console_edit.cpp \
    rv_tooltip_widget.cpp \
    rv_tooltip_xrefs.cpp \
    rv_tag_processor.cpp \
    rv_disasm_view_options_widget.cpp \
    rv_tag_cell_editor.cpp \
    html_processor.cpp

HEADERS  += rv_mainwin.h \
    rv_global.h \
    rv_elfhdr_model.h \
    rv_elf_phdr_model.h \
    rv_elf_shdr_model.h \
    rv_disasm_thread.h \
    rv_progress_dialog.h \
    rv_file_analyze_thread.h \
    rv_hex_edit.h \
    rv_hex_editor.h \
    rv_hex_editor_infopane.h \
    rv_about_dialog.h \
    rv_splitter.h \
    rv_splitter_handle.h \
    rv_hex_editor_bufferpane.h \
    rv_hex_edit_undocommands.h \
    rv_sideinfo_listbox.h \
    rv_script_processor.h \
    rv_script_execution_thread.h \
    rv_sideinfo_custom_widget.h \
    rv_script_highlighter.h \
    rv_script_code_editor.h \
    rv_slider.h \
    rv_disasm_view.h \
    rv_disasm_xrefs_widget.h \
    rv_idisasm.h \
    rv_console_edit.h \
    rv_tooltip_widget.h \
    rv_tooltip_xrefs.h \
    rv_tag_processor.h \
    rv_disasm_view_options_widget.h \
    rv_tag_cell_editor.h \
    html_processor.h

FORMS    += rv_mainwin.ui \
    rv_progress_dialog.ui \
    rv_hex_editor_infopane.ui \
    rv_about_dialog.ui \
    rv_hex_editor_bufferpane.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../re_core/release/ -lre_core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../re_core/debug/ -lre_core
else:unix:!symbian: LIBS += -L$$OUT_PWD/../re_core/ -lre_core

INCLUDEPATH += $$PWD/../re_core
DEPENDPATH += $$PWD/../re_core

RESOURCES += \
    resources/resources.qrc
