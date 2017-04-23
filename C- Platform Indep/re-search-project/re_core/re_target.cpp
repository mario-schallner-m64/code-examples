#include "re_target.h"

re_target::re_target(QObject *parent) :
    QObject(parent)
{
    file = 0;
    image_map = 0;
    logger = 0;
    disassembler = 0;
}

re_target::~re_target()
{
    if(file) delete file;
    if(disassembler) delete disassembler;
    if(address_space) delete address_space;
    if(image_map) delete image_map;
}

bool re_target::init(re_file *f, re_logger *l)
{
    file = f; logger = l;

    connect(this, SIGNAL(msg(int, QString)),
        logger, SLOT(log_event(int, QString)));

    emit msg(RE_LOG_DEBUG_3, "re_target::init()");
    image_map = (re_addr_desc **)
            calloc(file->stat_struct.st_size * sizeof(re_addr_desc *), 1);
    if ( image_map == (void*)-1 ){
        emit msg(RE_LOG_ERROR, "unable to malloc image_map");
        return false;
    }
    emit msg(RE_LOG_DEBUG_3, "re_target::init() - image map created");

    address_space = new re_address_space();
    address_space->init(file, image_map, logger);

    disassembler = new re_disassembler();
    disassembler->init(file, address_space, logger);

    return true;
}

re_address_space *re_target::clone_address_space()
{
    re_address_space *clone;

    clone = new re_address_space();
    clone->init(file, image_map, logger);
    clone->set_cloned(true);

    return clone;
}
