#ifndef RE_ADDRESS_SPACE_H
#define RE_ADDRESS_SPACE_H

#include "re_types.h"
#include "re_logger.h"

#include <QObject>

class re_file;

struct re_addr_desc {
    re_addr_t   va;
	re_addr_t	offset;
    char        desc;

    re_addr_desc *prev;
    re_addr_desc *next;
};

class RE_CORESHARED_EXPORT re_address_space : public QObject
{
    Q_OBJECT
public:
    explicit re_address_space(QObject *parent = 0);
    ~re_address_space();

    void init(re_file *f, struct re_addr_desc **imap, re_logger *l);

    // linked list functions
    re_addr_desc *list_insert(re_addr_t va, char desc);
    bool list_remove(re_addr_t);
    void clear();
    re_addr_desc *first() const;
    re_addr_desc *last() const;
    re_addr_desc *current() const;
    re_addr_desc *goto_next();
    re_addr_desc *goto_first();

    // address_space functions
    re_addr_desc *insert_va(re_addr_t va, char desc);
    re_addr_desc *insn_at_va(re_addr_t va);
    re_addr_desc *desc_at_va(re_addr_t va);
    //bool va_valid(re_addr_t va);
    bool remove_va(re_addr_t va);
    re_addr_t count() const;

    // cloned
    void set_cloned(bool yesno);
    bool is_cloned();
    re_addr_desc *find_desc_at_va(re_addr_t va);

    re_addr_desc  **image_map; // public for fast access ? hmm. dirty! ;)


signals:
    void msg(const int level, const QString S);

public slots:

private:
    re_file *file;
    re_logger *logger;

    // -- list data
    re_addr_desc    *ad_first;
    re_addr_desc    *ad_last;
    re_addr_desc    *last_pos;

    re_addr_t       cnt;

    bool            am_clone;
};

#endif // RE_ADDRESS_SPACE_H
