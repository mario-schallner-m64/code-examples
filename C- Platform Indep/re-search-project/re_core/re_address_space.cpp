#include "re_address_space.h"
#include "re_file.h"

re_address_space::re_address_space(QObject *parent) :
    QObject(parent)
{
    logger = 0;
    file = 0;
    image_map = 0;

    ad_first    = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
    ad_last     = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
    ad_first->next  = ad_last;
    ad_last->prev   = ad_first;

    ad_last->va = 0xffffffff;

    cnt = 0;
    last_pos = ad_first;

    am_clone = false;
}

re_address_space::~re_address_space()
{
    re_addr_desc *ad_tmp, *ad_next;

    ad_tmp = ad_first;

    while(ad_tmp->next) {
        ad_next = ad_tmp->next;
        free(ad_tmp);
        ad_tmp = ad_next;
    }

    free(ad_tmp);
}


void re_address_space::init(re_file *f, re_addr_desc **imap, re_logger *l)
{
    file = f; image_map = imap; logger = l;

    connect(this, SIGNAL(msg(int, QString)),
        logger, SLOT(log_event(int, QString)));

    emit msg(RE_LOG_DEBUG_3, "re_address_space::init()");
}

re_addr_desc *re_address_space::list_insert(re_addr_t va, char desc)
{
    re_addr_desc *ad_tmp, *ad_new;

    if(cnt && (va < ad_first->next->va) ) {
        // --- prepend ---
        ad_new = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
        ad_new->va   = va;
        ad_new->desc = desc;

        ad_tmp = ad_first->next;

        ad_new->next = ad_tmp;
        ad_new->prev = ad_first;

        ad_tmp->prev = ad_new;
        ad_first->next = ad_new;

        cnt++;

        last_pos = ad_new;

        return last_pos;
    }

    if(cnt && (va > ad_last->prev->va)) {
        // --- append ---
        ad_new = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
        ad_new->va   = va;
        ad_new->desc = desc;

        ad_tmp = ad_last->prev;

        ad_new->prev = ad_tmp;
        ad_new->next = ad_last;

        ad_tmp->next = ad_new;
        ad_last->prev = ad_new;

        cnt++;

        last_pos = ad_new;

        return last_pos;
    }

    // check, if we could insert after last_pos

    ad_tmp = last_pos;

    if(va < ad_tmp->va) {
        ad_tmp = ad_first;
    }

    // --- walk through whole shit ---
    // ad_tmp = ad_first;
    // no need for check of end, since above append
    while( va > ad_tmp->next->va ) ad_tmp = ad_tmp->next;
    // now we stand before next is bigger (or equal, in cloned case, we don't know)

    // when cloned and entry allready present: overwrite desc, return
    if(am_clone && (va == ad_tmp->next->va)) {
        ad_tmp = ad_tmp->next;
        ad_tmp->desc = desc;
        return ad_tmp;
    }

    ad_new = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
    ad_new->va   = va;
    ad_new->desc = desc;

    // relink now between ad_tmp and ad_tmp->next
    re_addr_desc *old_next = ad_tmp->next;

    ad_new->prev = ad_tmp;
    ad_new->next = old_next;

    old_next->prev  = ad_new;
    ad_tmp->next    = ad_new;

    cnt++;

    last_pos = ad_new;

    return ad_new;
}

void re_address_space::clear()
{
    re_addr_desc *ad_tmp, *ad_next;

    ad_tmp = ad_first;

    while(ad_tmp->next) {
        ad_next = ad_tmp->next;
        free(ad_tmp);
        ad_tmp = ad_next;
    }

    free(ad_tmp);

    // -- prepare for new usage, taken from constructor
    ad_first    = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
    ad_last     = (re_addr_desc *) calloc(sizeof(struct re_addr_desc), 1);
    ad_first->next  = ad_last;
    ad_last->prev   = ad_first;

    ad_last->va = 0xffffffff;

    cnt = 0;
    last_pos = ad_first;

    if(!am_clone)
    memset(image_map, 0, file->stat_struct.st_size * sizeof(re_addr_desc *));
}

bool re_address_space::list_remove(re_addr_t)
{
    return true;
}


re_addr_desc *re_address_space::insert_va(re_addr_t va, char desc)
{
    re_addr_t offs;
    re_addr_desc *r = 0;

    if( (offs=file->va_to_file_offset(va)) == BAD_ADDR ) return 0;


    if(!am_clone) {
        if(!image_map[offs]) {
            r = list_insert(va, desc);
            r->offset = offs;
            image_map[offs] = r;
        }
    } else {
        // work w/o image map
        r = list_insert(va, desc);
        r->offset = offs;
    }

    emit msg(RE_LOG_DEBUG_1, "re_address_space::insert_va(0x" +
             QString::number(va, 16) + ", " +
             QString::number(desc) + ")" );

    return r;
}

re_addr_desc *re_address_space::insn_at_va(re_addr_t va)
{
    re_addr_t offs;
    re_addr_desc *r;

    if( (offs=file->va_to_file_offset(va)) == BAD_ADDR ) return 0;

    if(!am_clone) r = image_map[offs];
    else r = find_desc_at_va(va);

    if(!r) return 0;

    if(r->desc < RE_IMG_IS_INSN) return r;

    return 0;
}

re_addr_desc *re_address_space::desc_at_va(re_addr_t va)
{
    re_addr_t offs;
    re_addr_desc *r;

    if( (offs=file->va_to_file_offset(va)) == BAD_ADDR ) return 0;

    if(!am_clone) r = image_map[offs];
    else r = find_desc_at_va(va);

    if(!r) return 0;

    return r;
}


bool re_address_space::remove_va(re_addr_t va)
{
    re_addr_t offs;
    re_addr_desc *r;

    if( (offs=file->va_to_file_offset(va)) == BAD_ADDR ) return false;

    r = image_map[offs];

    if(!am_clone) {
        r = image_map[offs];
        image_map[offs] = 0;
    }
    else r = find_desc_at_va(va);

    if(!r)
        return false;

    // list_remove(va);
    re_addr_desc *r_prev = r->prev;
    re_addr_desc *r_next = r->next;
    r_prev->next = r_next;
    r_next->prev = r_prev;

    if(r == last_pos) last_pos = r_prev;

    free(r);

    cnt--;

    return true;
}

re_addr_t re_address_space::count() const
{
    return cnt;
}

re_addr_desc *re_address_space::first() const
{
    if(!cnt) return 0;
    return ad_first->next;
}

re_addr_desc *re_address_space::goto_first()
{
    if(!cnt) return 0;
    last_pos = ad_first->next;
    return last_pos;
}

re_addr_desc *re_address_space::last() const
{
    if(!cnt) return 0;
    return ad_last->prev;
}

re_addr_desc *re_address_space::current() const
{
    if(!cnt) return 0;
    return last_pos;
}

re_addr_desc *re_address_space::goto_next()
{
    if(last_pos->next == ad_last) return 0;

    last_pos = last_pos->next;
    return last_pos;
}


// imap independence for clone views

void re_address_space::set_cloned(bool yesno)
{
    // when i am a clone .... i WON'T USE the image_map !!!
    am_clone = yesno;
}


re_addr_desc *re_address_space::find_desc_at_va(re_addr_t va)
{
        re_addr_desc *ad_tmp;

        // va too small
        if(cnt && (va < ad_first->next->va) ) {
            return 0;
        }

        // va too big
        if(cnt && (va > ad_last->prev->va)) {
            return 0;
        }

        ad_tmp = ad_first;

        while( va != ad_tmp->va ) {
            ad_tmp = ad_tmp->next;
            if(ad_tmp == ad_last) break;
        }

        if(ad_tmp != ad_last) return ad_tmp;

        return 0;
}

bool re_address_space::is_cloned()
{
    return am_clone;
}
