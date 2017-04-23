#include <QtCore/QCoreApplication>
#include <QDate>
#include <QTime>

#include <re_core.h>

#define RE_SCAN_VERSION     (QString)"0.1"

void usage()
{
    char *usage_str = "\n\
usage:\n\
    re_scan FILE[s ...] OPTION[s...]\n\
    re_scan OPTION[s ...] FILE[s ...]\n\
        \n\
    re_scan [-d|-d[1-3]|--debug_level_[1-3]]\n\
            [-l|--logfile <logfilename>] \n\
            [-fa|--full_analysis]\n\
            [-as|--acss_scan]\n\
            [-ad|--acss_disassemble\n\
            [-acss '<acss_filter>' [-acss <> ...]]\n\
            [-af|--acss_file <acss_filter_filename> [-af <> ...]]\n\
            [-ws|--wildasm_scan]\n\
            [-wd|--wildasm_disassemble\n\
            [-wildasm '<wildasm_filter>' [-wildasm <> ...]]\n\
            [-wf|--wildasm_file <wildasm_filter_filename> [-wf <> ...]]\n\
            [-su|--scan_unicode] [-sa|--scan_ascii] \n\
            [-dl|--disassemble_linear <va_start> <insn_count>\n\
            [-df|--dissasemble_flow <va_start>]\n\
            [-pd|--print_disassembly]\n\
            <file(s ...)>\n\
\n\
    re_scan [-v|--version]\n\
    re_scan [-h|--help]\n\
\n\
options:\n\
\n\
debug/log options:\n\
    -d|--debug|--debug_level1          enable debug output (level 1)\n\
    -d1|-d2|-d3|                       enable debug output, set level to 1/2/3\n\
    --debug_level_1|--debug_level_2|\n\
    --debug_level_3   \n\
        \n\
    -l|--logfile <logfilename>         redirect output to logfile\n\
\n\
strings scan options:\n\
    -sa|--scan_ascii                   enable scan for ascii strings\n\
    -su|--scan_unicode                 enable scan for unicode strings\n\
\n\
acss options:\n\
    -acss '<acss_filter>'              specify acss filter, this option can\n\
                                       be specified multiple times\n\
        \n\
    -af|--acss_file <acss_filter_filename> specify acss filter file\n\
                                       this option can be specified multiple\n\
                                       times \n\
        \n\
    -as|--acss_search                  acss scan for filter, print resulting\n\
                                       addresses\n\
        \n\
    -ad|--acss_disassemble             acss scan for filter, disassemble\n\
                                       results (linear)\n\
\n\
wildasm options:\n\
    -wildasm '<wildasm_filter>'        specify wildasm filter, this option can\n\
                                       be specified multiple times\n\
      \n\
    -wf|--wildasm_file <wildasm_filter_filename> specify wildasm filter file\n\
                                       this option can be specified multiple\n\
                                       times \n\
      \n\
    -ws|--wildasm_search               wildasm scan for filter, print resulting\n\
                                       addresses\n\
      \n\
    -wd|--wildasm_disassemble          wildasm scan for filter, disassemble\n\
                                       results (linear)\n\
      \n\
disassembler options:\n\
    -fa|--full_analysis                full analysis\n\
    -dl|--disassemble_linear va_start insn_count \n\
                                       disassemble insn_count # of instructions\n\
                                       linear from address va_start\n\
        \n\
    -df|--disassemble_flow va_start    control-flow analysis, starting from\n\
                                       va_start\n\
       \n\
    -co|--autocomments_on              enable auto-comments of the \n\
                                       disassembling engine\n\
        \n\
    -pd|--print_disassembly            after -df or -dl: print disassembly \n\
\n\
^------------------------------------------------------------------------------^\n";

    printf("%s", usage_str);
    exit(1);
}

void version()
{
    char *version_str = "| version: re_scan 0.1 (12.2011)\n\
| written in 2011 by Mario Schallner\n\
^------------------------------------------------------------------------------^\n\
                                                       ... have a lot of fun ...\n\
";
    printf("%s", version_str);
    exit(0);
}

int main(int argc, char *argv[])
{
    QCoreApplication app_re_scan(argc, argv);
	re_logger   logger;
	
    QStringList files;

    QStringList acss_filter_strings;
    QStringList acss_file_names;
    QString acss_filter_string;
    QString acss_file_name;
    bool opt_acss_scan = false;
    bool opt_acss_disassemble = false;

    QStringList wildasm_filter_strings;
    QStringList wildasm_file_names;
    QString wildasm_filter_string;
    QString wildasm_file_name;
    bool opt_wildasm_scan = false;
    bool opt_wildasm_disassemble = false;

    int opt_strings_scan = 0;
    int opt_strings_scan_unicode = 0;
    re_addr_t opt_va_start = 0, opt_insn_count = 0;
    bool opt_disassemble_linear = false;
    bool opt_disassemble_flow = false;
    bool opt_print_disassembly = false;
    bool opt_autocomments_on = false;
    bool opt_full_analysis = false;

    QString opt_va_start_str;
    QHash<re_addr_t, re_acss_result> *results;
    QList<re_addr_t>va_starts;
    QString disasm_string;

    QString log_str =
            "\nre|scan " + RE_SCAN_VERSION
//"\n\033[33mre|scan\033[0m " + RE_SCAN_VERSION
+ " >-------------------------------------------------------------------\n"
+ "                                      reverse engineering binary file scanner  <\n"
+ "^-------------------------------------------------------------------------------";
    logger.log_message(log_str, "");

    QStringList my_args = app_re_scan.arguments();

	if(!my_args.isEmpty()) my_args.removeFirst();
    if(my_args.isEmpty()) usage();

	QString my_arg;
    while(!my_args.isEmpty()) {
        my_arg = my_args.takeFirst();
		
        // help, version
        if(my_arg == "-v" || my_arg == "--version") version();
        if(my_arg == "-h" || my_arg == "--help") usage();

        // scan options
        if(my_arg == "-sa" || my_arg == "--scan_ascii") {
            opt_strings_scan++; continue;
        }

        if(my_arg == "-su" || my_arg == "--scan_unicode") {
            opt_strings_scan_unicode++; continue;
        }

        // debug options
        if(my_arg == "-d" || my_arg == "-d1" ||
                my_arg == "--debug"  || my_arg == "--debug_level_1") {
            logger.set_debug_level(1); continue;
        }

        if(my_arg == "-d2" || my_arg == "--debug_level_2") {
            logger.set_debug_level(2); continue;
        }

        if(my_arg == "-d3" || my_arg == "--debug_level_3") {
            logger.set_debug_level(3); continue;
        }
		
        if(my_arg == "-l" || my_arg == "--logfile") {
            QString logfile_name;
            if(my_args.isEmpty()) usage();
            logfile_name = my_args.takeFirst();
            logger.log_info("redirecting output to logfile: '"
                    + logfile_name + "'");
            if(logger.open_log_file(logfile_name))
                        logger.log_message(log_str, "");
                            logger.log_message("| logfile: '" + logfile_name + "', "
                            + QDate::currentDate().toString()
                            + " "
                            + QTime::currentTime().toString()
                            , "");
                            logger.log_message(
    "^------------------------------------------------------------------------------^",
                            "");
            continue;
        }

        if(my_arg == "-dl" || my_arg == "--disassemble_linear") {
            if(my_args.isEmpty()) usage();
            opt_va_start_str = my_args.takeFirst();

            if(my_args.isEmpty()) usage();
            opt_insn_count = my_args.takeFirst().toULongLong();

            opt_disassemble_linear = true;
            continue;
        }

        if(my_arg == "-df" || my_arg == "--disassemble_flow") {
            if(my_args.isEmpty()) usage();
            opt_va_start_str = my_args.takeFirst();
            opt_disassemble_flow = true;
            continue;
        }

        if(my_arg == "-pd" || my_arg == "--print_disassembly") {
            opt_print_disassembly = true;
            continue;
        }

        if(my_arg == "-co" || my_arg == "--autocomments_on") {
            opt_autocomments_on = true;
            continue;
        }

        if(my_arg == "-fa" || my_arg == "--full_analysis") {
            opt_full_analysis = true;
            continue;
        }

        // -- acss --
        if(my_arg == "-ad" || my_arg == "--acss_disassemble") {         
            opt_acss_disassemble = true;
            continue;
        }

        if(my_arg == "-as" || my_arg == "--acss_scan") {
            opt_acss_scan = true;
            continue;
        }

        if(my_arg == "-acss") {
            if(my_args.isEmpty()) usage();
            acss_filter_string = my_args.takeFirst();
            acss_filter_strings.append(acss_filter_string);
            continue;
        }

        if(my_arg == "-af" || my_arg == "--acss_file") {
            if(my_args.isEmpty()) usage();
            acss_file_name = my_args.takeFirst();
            acss_file_names.append(acss_file_name);
            continue;
        }

        // -- wildasm --
        if(my_arg == "-wd" || my_arg == "--wildasm_disassemble") {
            opt_wildasm_disassemble = true;
            continue;
        }

        if(my_arg == "-ws" || my_arg == "--wildasm_scan") {
            opt_wildasm_scan = true;
            continue;
        }

        if(my_arg == "-wildasm") {
            if(my_args.isEmpty()) usage();
            wildasm_filter_string = my_args.takeFirst();
            wildasm_filter_strings.append(wildasm_filter_string);
            continue;
        }

        if(my_arg == "-wf" || my_arg == "--wildasm_file") {
            if(my_args.isEmpty()) usage();
            wildasm_file_name = my_args.takeFirst();
            wildasm_file_names.append(wildasm_file_name);
            continue;
        }

        // unknown option found
        if(my_arg.left(1) == "-") {
            logger.log_message("unknown option: '" + my_arg + "'", "");
            usage();
        }

        files.append(my_arg);
    }

    if(files.isEmpty()) usage();

    if( opt_print_disassembly &&
        !(opt_disassemble_linear || opt_disassemble_flow ||
          opt_full_analysis ) )
        usage();

    if( (!opt_acss_scan && !opt_acss_disassemble) &&
        (!acss_filter_strings.isEmpty() || !acss_file_names.isEmpty()))
        usage();

    if( (!opt_wildasm_scan && !opt_wildasm_disassemble) &&
        (!wildasm_filter_strings.isEmpty() || !wildasm_file_names.isEmpty()))
        usage();

    logger.log_info("attemting to scan " + QString::number(files.count())
        + " files");
	
    re_file *file;
    re_target *target;

    for(int i=0; i < files.count(); ++i) {
        logger.log_info("scanning file #" + QString::number(i) + ": '" +
        files.at(i) + "'");
        file = new re_file();
        QObject::connect(file, SIGNAL(msg(int, QString)), 
            &logger, SLOT(log_event(int, QString)));

        if(!file->load_file(files.at(i))) continue;
        if(opt_strings_scan) file->scan_ascii_strings();
        if(opt_strings_scan_unicode) file->scan_unicode_strings();

        target = new re_target();
        target->init(file, &logger);

        if(opt_disassemble_linear) {           
            if(opt_va_start_str == "entry_point" || opt_va_start_str=="ep")
                opt_va_start = target->file->va_entry_point;
            else opt_va_start = opt_va_start_str.toULongLong(0, 16);

            logger.log_info("disassembling linear from 0x"
                + QString::number(opt_va_start, 16)
                + ", # of insns: " + QString::number(opt_insn_count));

            target->disassembler->set_autocomment_on(opt_autocomments_on);
            target->disassembler->disassemble_linear_cnt(opt_va_start, opt_insn_count);
            logger.log_info("# of insns in address space: " +
                            QString::number(target->address_space->count()));
        }

        if(opt_disassemble_flow) {
            if(opt_va_start_str == "entry_point" || opt_va_start_str=="ep")
                opt_va_start = target->file->va_entry_point;
            else opt_va_start = opt_va_start_str.toULongLong(0, 16);

            logger.log_info("disassembling recursive flow from 0x"
                + QString::number(opt_va_start, 16));

            target->disassembler->set_autocomment_on(opt_autocomments_on);
            target->disassembler->disassemble_flow_from(opt_va_start);
            logger.log_info("# of insns in address space: " +
                            QString::number(target->address_space->count()));
        }

        if(opt_full_analysis) {
            logger.log_info("full analysis");

            target->disassembler->set_autocomment_on(opt_autocomments_on);
            target->disassembler->full_analysis();
            logger.log_info("# of insns in address space: " +
                            QString::number(target->address_space->count()));
        }

        // ==== ACSS ====
        if(opt_acss_scan || opt_acss_disassemble) {

            if(opt_full_analysis) target->address_space->clear();

            if(acss_filter_strings.isEmpty() && acss_file_names.isEmpty()) {
                logger.log_error("no acss filter specified");
                exit(4);
            }

            // read acss files
            if(!acss_file_names.isEmpty()){
                for(int i=0; i < acss_file_names.count(); i++) {

                    acss_file_name = acss_file_names.at(i);
                    acss_filter_string.clear();

                    QFile f(acss_file_name);
                    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        logger.log_error("file not found: " + acss_file_name);
                        exit(3);
                    }

                    while (!f.atEnd()) {
                        QByteArray line = f.readLine();
                        acss_filter_string.append(line);
                    }
                    acss_filter_strings.append(acss_filter_string);
                }
            }

            target->disassembler->set_autocomment_on(opt_autocomments_on);

            re_acss_compiler *acompiler = new re_acss_compiler();
            acompiler->init(&logger);

            for(int i=0; i < acss_filter_strings.count(); i++) {
                acss_filter_string = acss_filter_strings.at(i);

                acompiler->set_filter(acss_filter_string.toAscii().data());
                if(!acompiler->compile_expressions()) {
                    logger.log_error("ERROR in ACSS EXPRESSION");
                    exit(2);
                }

                results = target->disassembler->search_deep_all(acompiler);

                logger.log_info( "# of acss results: " +
                                QString::number(results->count()));

                va_starts = results->keys();

                if(opt_acss_scan) {
                    logger.log_info("acss scan:\n" + acss_filter_string);

                    if(va_starts.count()) {
                        for(int j=0; j < va_starts.count(); ++j) {
                            logger.log_info("acss result #"
                            + QString::number(j+1) + "/"
                            + QString::number(results->count())
                            +  ": 0x"
                            + QString::number(va_starts.at(j), 16)
                            + " - 0x"
                            + QString::number(
                              results->value(va_starts.at(j)).va_end, 16));

                            logger.log_info("  matched insn count: "
                            + QString::number(
                              results->value(va_starts.at(j)).insn_count)
                            + ", matched byte count: "
                            + QString::number(
                              results->value(va_starts.at(j)).byte_count)
                                            );
                        }
                    }
                }

                if(opt_acss_disassemble) {
                    logger.log_info("acss disassemble:\n" + acss_filter_string);

                    if(va_starts.count()) {
                        for(int j=0; j < va_starts.count(); ++j) {
                            logger.log_info("disassembling acss result #"
                            + QString::number(j+1) + "/"
                            + QString::number(results->count())
                            +  ": 0x"
                            + QString::number(va_starts.at(j), 16)
                            + " - 0x"
                            + QString::number(
                              results->value(va_starts.at(j)).va_end, 16));

                            logger.log_info("matched insn count: "
                            + QString::number(
                              results->value(va_starts.at(j)).insn_count)
                            + ", matched byte count: "
                            + QString::number(
                              results->value(va_starts.at(j)).byte_count)
                                            );

                            target->disassembler->disassemble_linear_range(
                                        va_starts.at(j),
                                        results->value(va_starts.at(j)).va_end);

                            logger.log_info("printing disassembly of result: \n");
                            disasm_string.clear();
                            target->disassembler->print_disassembly(disasm_string);
                            logger.log_message(disasm_string, "");
                            target->address_space->clear();
                        }
                    }
                }

                delete results;
            }
        }


        // ==== WILDASM ====
        if(opt_wildasm_scan || opt_wildasm_disassemble) {

            if(opt_full_analysis) target->address_space->clear();

            if(wildasm_filter_strings.isEmpty() && wildasm_file_names.isEmpty()) {
                logger.log_error("no wildasm filter specified");
                exit(4);
            }

            // read wild files
            if(!wildasm_file_names.isEmpty()){
                for(int i=0; i < wildasm_file_names.count(); i++) {

                    wildasm_file_name = wildasm_file_names.at(i);
                    wildasm_filter_string.clear();

                    QFile f(wildasm_file_name);
                    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        logger.log_error("file not found: " + wildasm_file_name);
                        exit(3);
                    }

                    while (!f.atEnd()) {
                        QByteArray line = f.readLine();
                        wildasm_filter_string.append(line);
                    }
                    wildasm_filter_strings.append(wildasm_filter_string);
                }
            }

            target->disassembler->set_autocomment_on(opt_autocomments_on);

            re_wildasm_compiler *wcompiler = new re_wildasm_compiler();
            wcompiler->init(&logger);

            for(int i=0; i < wildasm_filter_strings.count(); i++) {
                wildasm_filter_string = wildasm_filter_strings.at(i);

                wcompiler->set_filter(wildasm_filter_string.toAscii().data());
                if(!wcompiler->compile()) {
                    logger.log_error("ERROR in wildasm EXPRESSION");
                    exit(2);
                } 

                re_acss_compiler *acompiler = new re_acss_compiler();
                acompiler->init(&logger);

                acompiler->set_filter(wcompiler->get_compiled_acss_str().toAscii().data());

                if(!acompiler->compile_expressions()) {
                    logger.log_error("ERROR in wildasm compiled ACSS EXPRESSION");
                    exit(2);
                }

                results = target->disassembler->search_deep_all(acompiler);

                logger.log_info( "# of wildasm results: " +
                                QString::number(results->count()));

                va_starts = results->keys();

                if(opt_wildasm_scan) {
                    logger.log_info("wildasm scan:\n" 
                                    + wildasm_filter_string + "\n");
                    logger.log_debug("wildasm filter compiled to acss: '"
                        + wcompiler->get_compiled_acss_str() + "'", 1);
                    if(va_starts.count()) {
                        for(int j=0; j < va_starts.count(); ++j) {
                            logger.log_info("wildasm result #"
                            + QString::number(j+1) + "/"
                            + QString::number(results->count())
                            +  ": 0x"
                            + QString::number(va_starts.at(j), 16)
                            + " - 0x"
                            + QString::number(
                              results->value(va_starts.at(j)).va_end, 16));

                            logger.log_info("  matched insn count: "
                            + QString::number(
                              results->value(va_starts.at(j)).insn_count)
                            + ", matched byte count: "
                            + QString::number(
                              results->value(va_starts.at(j)).byte_count)
                                            );
                        }
                    }
                }

                if(opt_wildasm_disassemble) {
                    logger.log_info("wildasm disassemble:\n" 
                                    + wildasm_filter_string + "\n");
                    logger.log_debug("wildasm filter compiled to acss: '"
                        + wcompiler->get_compiled_acss_str() + "'", 1);

                    if(va_starts.count()) {
                        for(int j=0; j < va_starts.count(); ++j) {
                            logger.log_info("disassembling wildasm result #"
                            + QString::number(j+1) + "/"
                            + QString::number(results->count())
                            +  ": 0x"
                            + QString::number(va_starts.at(j), 16)
                            + " - 0x"
                            + QString::number(
                              results->value(va_starts.at(j)).va_end, 16));

                            logger.log_info("matched insn count: "
                            + QString::number(
                              results->value(va_starts.at(j)).insn_count)
                            + ", matched byte count: "
                            + QString::number(
                              results->value(va_starts.at(j)).byte_count)
                                            );

                            target->disassembler->disassemble_linear_range(
                                        va_starts.at(j),
                                        results->value(va_starts.at(j)).va_end);

                            logger.log_info("printing disassembly of result: \n");
                            disasm_string.clear();
                            target->disassembler->print_disassembly(disasm_string);
                            logger.log_message(disasm_string, "");
                            target->address_space->clear();
                        }
                    }
                }

                delete results;
            }
        }


        if(opt_print_disassembly) {
            disasm_string.clear();
            logger.log_info("printing disassembly of address space: \n");
            target->disassembler->print_disassembly(disasm_string);
            logger.log_message(disasm_string, "");
        }

        if((files.count() > 1) && (i < (files.count()-1)))
            logger.log_message(
"^------------------------------------------------------------------------------>", 
			"");
    }

    logger.log_message(
"^------------------------------------------------------------------------------^",
                        "");    
    exit(0);
}
