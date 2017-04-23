#include "rv_script_processor.h"

re_target       *RV_SCRIPT_target;
rv_hex_editor   *RV_SCRIPT_Hexeditor;
rv_hex_edit     *RV_SCRIPT_Current_Buffer;

rv_script_processor::rv_script_processor(QObject *parent) :
    QObject(parent)
{
    RV_SCRIPT_target = 0;
    RV_SCRIPT_Hexeditor = 0;

    engine = new QScriptEngine();
    init_engine();
}

rv_script_processor::~rv_script_processor()
{
    delete engine;
}

rv_script_execution_thread *rv_script_processor::run_script_in_thread(QString &script)
{
    rv_script_execution_thread *thread = new rv_script_execution_thread(
                engine, script, this);
    connect(thread, SIGNAL(thread_terminated(QScriptValue)),
            this, SIGNAL(processor_thread_terminated(QScriptValue)));

    thread->start(QThread::LowPriority);
    return thread;
}

void rv_script_processor::set_output_textbox(QPlainTextEdit *outbox)
{
    QScriptValue textbox = engine->newQObject(outbox);
    engine->globalObject().setProperty("TextBox", textbox);

    QScriptValue print_f =
            engine->evaluate("(function(a) { TextBox.appendPlainText(a); return 0; })");
    engine->globalObject().setProperty("print", print_f);

    QScriptValue result_f =
            engine->evaluate("(function(a) { return a; })");
    engine->globalObject().setProperty("result", result_f);


}

void rv_script_processor::set_target(re_target *target)
{
    ::RV_SCRIPT_target = target;
}

void rv_script_processor::set_hex_editor(rv_hex_editor *h)
{
    ::RV_SCRIPT_Hexeditor = h;
}

void rv_script_processor::set_current_buffer(rv_hex_edit *hv)
{
    ::RV_SCRIPT_Current_Buffer = hv;
}

// --
QHash<QString, struct rv_script_function_desc> rv_script_functions;

QScriptValue rv_script_help_functions(QScriptContext *context, QScriptEngine *engine)
{
    QString tmp_str;
    foreach(QString fname, rv_script_functions.keys()) {
        if(context->argumentCount()) {
            if(fname.contains(context->argument(0).toString())) {
                tmp_str.append(fname + "("
                           + rv_script_functions.value(fname).arguments
                           + ");\n");
            }
        } else {
            tmp_str.append(fname + "("
                       + rv_script_functions.value(fname).arguments
                       + ");\n");
        }
    }

    return tmp_str;
}

QScriptValue rv_script_help_function(QScriptContext *context, QScriptEngine *engine)
{
    if(!context->argumentCount() || (context->argumentCount() > 1)) {
        context->throwError(QScriptContext::SyntaxError,
                            "re_script syntax error: function 'help_function' needs 1 argument.");
        return 0;
    }

    QString tmp_str, fname;
    fname = context->argument(0).toString();

    if(rv_script_functions.contains(fname)) {
        struct rv_script_function_desc tmp_desc = rv_script_functions.value(fname);
        tmp_str.append(fname + "("
                       + tmp_desc.arguments
                       + ");\n\n"
                       + "Description: " + tmp_desc.desc + "\n"
                       + "Example: " + tmp_desc.example + "\n"
                       + "Example result: " + tmp_desc.example_result + "\n"
                       );
    }

    return tmp_str;
}

// ========
QScriptValue rv_script_dword_tohex(QScriptContext *context, QScriptEngine *engine)
{
    if(!context->argumentCount() || (context->argumentCount() > 1)) {
        context->throwError(QScriptContext::SyntaxError,
                            "re_script syntax error: function 'dword_to_hex' needs 1 argument.");
        return 0;
    }
    QScriptValue a = context->argument(0);
    return re_core::dword_to_hex_string(a.toUInt32());
}

QScriptValue rv_script_byte_tohex(QScriptContext *context, QScriptEngine *engine)
{
    if(!context->argumentCount() || (context->argumentCount() > 1)) {
        context->throwError(QScriptContext::SyntaxError,
                            "re_script syntax error: function 'byte_to_hex' needs 1 argument.");
        return 0;
    }
    QScriptValue a = context->argument(0);
    return re_core::byte_to_hex_string(a.toUInt16());
}

QScriptValue rv_script_word_tohex(QScriptContext *context, QScriptEngine *engine)
{
    if(!context->argumentCount() || (context->argumentCount() > 1)) {
        context->throwError(QScriptContext::SyntaxError,
                            "re_script syntax error: function 'word_to_hex' needs 1 argument.");
        return 0;
    }
    QScriptValue a = context->argument(0);
    return re_core::word_to_hex_string(a.toUInt16());
}

// --

QScriptValue rv_script_get_byte_at_offset(QScriptContext *context, QScriptEngine *engine)
{
    if(!context->argumentCount() || (context->argumentCount() > 1)) {
        context->throwError(QScriptContext::SyntaxError,
                            "re_script syntax error: function 'get_byte_at_offset' needs 1 argument.");
        return 0;
    }
    QScriptValue a = context->argument(0);

    re_addr_t offs = a.toUInt32();
    unsigned char c;

    if(!RV_SCRIPT_Current_Buffer) {
        context->throwError(QScriptContext::RangeError, (QString)
                            "re_script runtime error: function 'get_byte_at_offset': "
                            + "current buffer is not set! (is there any buffer open?)"
                            );
    }

    if(offs >= RV_SCRIPT_Current_Buffer->get_data_len()) {
        context->throwError(QScriptContext::RangeError, (QString)
                            "re_script runtime error: function 'get_byte_at_offset': "
                            +"offset " + re_core::dword_to_hex_string(offs)
                            +" ecxeeds buffer length "
                            +re_core::dword_to_hex_string(
                                RV_SCRIPT_Current_Buffer->get_data_len()
                                ));
        return 0;
    }

    c = RV_SCRIPT_Current_Buffer->get_data_ptr()[offs];

    return c;

}



// ========


void rv_script_processor::init_engine()
{
    struct rv_script_function_desc tmp_desc;

    // --
    QScriptValue dword_tohex_f = engine->newFunction(rv_script_dword_tohex);
    engine->globalObject().setProperty("dword_to_hex", dword_tohex_f);
    tmp_desc.name = "dword_to_hex";
    tmp_desc.arguments = "dword val";
    tmp_desc.type = RV_SCRIPT_FUNCTION_CORE;
    tmp_desc.desc = "returns a hex string of the given numeric dword parameter";
    tmp_desc.example = "dword_to_hex(100)";
    tmp_desc.example_result = "\"0x00000064\"";
    ::rv_script_functions[tmp_desc.name] = tmp_desc;

    QScriptValue word_tohex_f = engine->newFunction(rv_script_word_tohex);
    engine->globalObject().setProperty("word_to_hex", word_tohex_f);
    tmp_desc.name = "word_to_hex";
    tmp_desc.arguments = "word val";
    tmp_desc.type = RV_SCRIPT_FUNCTION_CORE;
    tmp_desc.desc = "returns a hex string of the given numeric word parameter";
    tmp_desc.example = "word_to_hex(100)";
    tmp_desc.example_result = "\"0x0064\"";
    ::rv_script_functions[tmp_desc.name] = tmp_desc;

    QScriptValue byte_tohex_f = engine->newFunction(rv_script_byte_tohex);
    engine->globalObject().setProperty("byte_to_hex", byte_tohex_f);
    tmp_desc.name = "byte_to_hex";
    tmp_desc.arguments = "byte val";
    tmp_desc.type = RV_SCRIPT_FUNCTION_CORE;
    tmp_desc.desc = "returns a hex string of the given numeric byte parameter";
    tmp_desc.example = "byte_to_hex(100)";
    tmp_desc.example_result = "\"0x64\"";
    ::rv_script_functions[tmp_desc.name] = tmp_desc;

    QScriptValue functions_f = engine->newFunction(rv_script_help_functions);
    engine->globalObject().setProperty("functions", functions_f);
    tmp_desc.name = "functions";
    tmp_desc.arguments = "optional: part of or full function name";
    tmp_desc.type = RV_SCRIPT_FUNCTION_CORE;
    tmp_desc.desc = "returns a string of all rv_script functions. if a parameter is given this is used as search pattern.";
    tmp_desc.example = "functions";
    tmp_desc.example_result = "list of all rv_script functions. example2: functions(\"dword\"); this returns all function names containing the string \"dword\"";
    ::rv_script_functions[tmp_desc.name] = tmp_desc;


    QScriptValue help_function_f = engine->newFunction(rv_script_help_function);
    engine->globalObject().setProperty("help_function", help_function_f);
    tmp_desc.name = "help_function";
    tmp_desc.arguments = "full function name";
    tmp_desc.type = RV_SCRIPT_FUNCTION_CORE;
    tmp_desc.desc = "returns a help string for the given function";
    tmp_desc.example = "help_function(\"dword_to_hex\")";
    tmp_desc.example_result = "returns a help string for the given function";
    ::rv_script_functions[tmp_desc.name] = tmp_desc;

    // --
    QScriptValue get_byte_at_offset_f = engine->newFunction(rv_script_get_byte_at_offset);
    engine->globalObject().setProperty("get_byte_at_offset", get_byte_at_offset_f);
    tmp_desc.name = "get_byte_at_offset";
    tmp_desc.arguments = "uint offset";
    tmp_desc.type = RV_SCRIPT_FUNCTION_BUFFER;
    tmp_desc.desc = "returns the byte at the offset of the current buffer in the hex editor";
    tmp_desc.example = "get_byte_at_offset(0x20);";
    tmp_desc.example_result = "0xf1";
    ::rv_script_functions[tmp_desc.name] = tmp_desc;
}

