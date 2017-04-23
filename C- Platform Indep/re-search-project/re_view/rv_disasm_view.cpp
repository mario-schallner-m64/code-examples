#include "rv_disasm_view.h"


rv_disasm_view::rv_disasm_view(QTabWidget *attach_to, QWidget *parent) :
    QAbstractScrollArea(parent)
{
    setMouseTracking(true);
    tabwidget_attach_to = attach_to;
    t = 0;

    buffer_data = 0;
    freeze = false;
    navi_stack = 0;
    mouse_active = true;
    mouse_desc = 0;
    repaint_mouse = false;
    max_op_len = 0;
    tag_processor = 0;
    tag_editor_desc = 0;
    have_idisasm = false;
    have_selection = false;
    mouse_va_dest = BAD_ADDR;
    mouse_va_here = BAD_ADDR;
    selection = 0;

    top_desc = 0; cursor_desc = 0; lastline = 0;
    scrollbar_current_val = 0;
    am_painting = false;

    my_txt_font = QFont("Arial", 9);
    my_txt_font_bold = my_txt_font;
    my_txt_font_bold.setBold(true);

    my_font         = QFont("Liberation Mono", 9, QFont::Normal);
    my_font_bold    = QFont("Liberation Mono", 9, QFont::Bold);       
    setFont(my_font);
    line_height = QFontMetrics(my_font).lineSpacing() + 3;
    char_width  = QFontMetrics(my_font).width('X');
    num_lines_visible = viewport()->height() / line_height;

    pm_data_marker  = QPixmap(":/disassembly/data_small_new.png");
    pm_data_marker2 = QPixmap(":/disassembly/data_small_new2.png");
    pm_follow_controlflow = QPixmap(":/disassembly/arrow_follow_ctf.png");
    pm_ret_address = QPixmap(":/disassembly/arrow_left_green_small.png");
    pm_functions = QPixmap(":/disassembly/function_small.png");
    pm_symbols = QPixmap(":/disassembly/symbol_small.png");
    pm_imports = QPixmap(":/disassembly/import_small.png");
    pm_strings = QPixmap(":/disassembly/strings_small.png");
    pm_comments = QPixmap(":/disassembly/comment_small.png");
    pm_names = QPixmap(":/disassembly/names_small.png");

    pm_xr1 = QPixmap(":/disassembly/xrefs_small_new2.png");
    pm_xr2 = QPixmap(":/disassembly/xrefs_small_up_new2.png");

    pm_tag_y = QPixmap(":/icons/bookmark_small.png");
    pm_tag_r = QPixmap(":/icons/bookmark_red_small.png");
    pm_tag_g = QPixmap(":/icons/bookmark_grn_small.png");
    pm_tag_b = QPixmap(":/icons/bookmark_blu_small.png");
    pm_tag_B = QPixmap(":/icons/bookmark_black_small.png");

    ico_show_options = QPixmap(":/disassembly/disasm_view_options_show.png");
    ico_hide_options = QPixmap(":/disassembly/disasm_view_options_hide.png");
    pushbutton_show_hide_options = new QPushButton(this);
    pushbutton_show_hide_options->setIconSize(QSize(20, 16));
    pushbutton_show_hide_options->setIcon(ico_show_options);
    pushbutton_show_hide_options->setFlat(true);
    pushbutton_show_hide_options->move(viewport()->width() -
                                      pushbutton_show_hide_options->width() -
                                      3,
                                      0
                                      );
    pushbutton_show_hide_options->show();

    connect(pushbutton_show_hide_options, SIGNAL(clicked()),
            this, SLOT(show_hide_options()));


    set_mode(DV_MODE_TARGET);
    show_offsets = true;
    show_hex_bytes_cnt = 10;

    // columns
    set_show_offsets_(true);
    set_show_bytes_cnt_(10);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(scrollbar_value_changed(int)));

    connect(this, SIGNAL(first_paint()), this, SLOT(first_paint_updates()));

    connect(this, SIGNAL(repaintme()), this, SLOT(do_repaint()));

    unpainted = true;
    have_saved_vas = false;

    tooltip_widget = 0;
    tooltip_label_xrefs = new rv_tooltip_xrefs(this);
    tooltip_label_xrefs->setFixedHeight(line_height);
    tooltip_label_xrefs->hide();

    create_options_widget();
    tag_editor = new rv_tag_cell_editor(this);
    connect(tag_editor, SIGNAL(editingFinished()),
            this, SLOT(tag_editor_finished()));
    tag_editor->hide();

    setup_menu_actions();
}

void rv_disasm_view::set_target(re_target *target)
{
    t = target;
    if(!t->address_space) return;

    my_address_space = t->address_space;

    verticalScrollBar()->setRange(0, my_address_space->count());
    top_desc = my_address_space->first();
    connect(t->disassembler, SIGNAL(disassembly_changed()),
            this, SLOT(disassembly_changed()));
}

void rv_disasm_view::set_address_space(re_address_space *s)
{
    // DO THIS ONLY ON CREATION OF THE VIEW!!!!
    my_address_space = s;
    verticalScrollBar()->setRange(0, my_address_space->count());
    top_desc = my_address_space->first();

    //
    set_mode(DV_MODE_FILTER);
}

re_address_space *rv_disasm_view::get_address_space()
{
    return my_address_space;
}

void rv_disasm_view::recalc_geometry()
{
    num_lines_visible = viewport()->height() / line_height;
    verticalScrollBar()->setPageStep(num_lines_visible - 1);        
}

void rv_disasm_view::resizeEvent(QResizeEvent *)
{
    recalc_geometry();
    pushbutton_show_hide_options->move(viewport()->width() -
                                      pushbutton_show_hide_options->width() -
                                      3,
                                      0
                                      );
    options_widget->move(viewport()->width() - options_widget->width() -
                         pushbutton_show_hide_options->width() - 5,
                         0);

}

void rv_disasm_view::paintEvent(QPaintEvent *event)
{
    if(!t && !buffer_data) return;
    if(!my_address_space) return;
    if(!my_address_space->count()) return;

    // start the dance
    if(freeze) return;
    if(am_painting) return;

    am_painting = true;

    if(!top_desc) top_desc = my_address_space->first();
    if(!cursor_desc) cursor_desc = top_desc;

    re_addr_desc *current_desc = top_desc;
    rv_disasm_line m_line = mouse_line;

    char line_asc[1024];
    QString line;
    int current_y = 0;
    int current_line_height, current_line_height_total;
    x86_insn_t tmp_insn;
    int op_len;

    int odd_even = verticalScrollBar()->value() % 2;

    QPainter painter(viewport());

    if(!repaint_mouse) {
        disasm_lines.clear();
        hex_edits_current_to_show.clear();
        max_op_len = 0;
    }

    bool had_line;
    bool prev_had_line = false;
    int col_comment;

    int start_line, stop_line;

    start_line = 0; stop_line = num_lines_visible;
    if(repaint_mouse) {
        start_line = m_line.nr;
        stop_line = m_line.nr+1;
        current_desc = mouse_desc;
        current_y = m_line.y;
        odd_even = (verticalScrollBar()->value() + m_line.nr) % 2;
        prev_had_line = m_line.prev_had_line;
    }

    for(int i=start_line; i<stop_line; i++) {
        line = "";        
        current_line_height = 0;
        current_line_height_total = 0;
        had_line = false;

        odd_even = 1 - odd_even;
        if(odd_even) bgcolor = bgcolor1;
        else         bgcolor = bgcolor2;

        if(current_desc == cursor_desc) bgcolor = bgcolor_cursorline;

        if(have_selection) {
            if((current_desc->va >= selection_start->va) &&
                    (current_desc->va <= selection_end->va)) {
                bgcolor = bgcolor_selection;
                if(current_desc == cursor_desc) bgcolor = bgcolor_selection_cursor;
            }
        }

        if((current_desc == mouse_desc) && !mouse_clear) bgcolor = bgcolor_mouse;

        if(current_desc->va == t->file->va_entry_point) {
            // ENTRY POINT
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );

            painter.setFont(my_txt_font_bold);
            painter.setPen(txt_color_normal);
            painter.drawText(RV_DV_LMARGIN,
                             current_y + line_height - 3 - 1,
                             "entry point:");
            current_y+=line_height;
            current_line_height_total += line_height;
        } else if(t->file->va_imports_map.contains(current_desc->va)) {
            // IMPORT
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );

            painter.setFont(my_txt_font_bold);
            painter.setPen(txt_color_imports);
            painter.drawText(RV_DV_LMARGIN + 3 * char_width,
                             current_y + line_height - 3 - 1,
                             QString::fromAscii(
                                 t->file->va_imports_map.value(
                                     current_desc->va))
                             +":");
            painter.drawPixmap(RV_DV_LMARGIN ,
                               current_y+1,
                               pm_imports);

            current_y+=line_height;
            current_line_height_total += line_height;
        } else if(t->file->va_functions_map.contains(current_desc->va)) {
            // ELF functions
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );
            painter.setPen(QPen(QBrush(dis_color_line_solid), 1, Qt::SolidLine));
            painter.drawLine(0,
                             current_y-1,
                             viewport()->width(),
                             current_y-1);

            painter.setFont(my_txt_font_bold);
            painter.setPen(txt_color_functions);
            painter.drawText(RV_DV_LMARGIN + 3 * char_width,
                             current_y + line_height - 3 - 1,
                             QString::fromAscii(t->file->va_functions_map.value(
                                     current_desc->va))
                             +":");
            painter.drawPixmap(RV_DV_LMARGIN ,
                               current_y+1,
                               pm_functions);

            current_y+=line_height;
            current_line_height_total += line_height;
        }else if(t->file->va_analyzed_functions_hash.contains(current_desc->va)) {
            // analyzed_functions
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );
            painter.setPen(QPen(QBrush(dis_color_line_solid), 1, Qt::SolidLine));
            painter.drawLine(0,
                             current_y-1,
                             viewport()->width(),
                             current_y-1);

            painter.setFont(my_txt_font_bold);
            painter.setPen(txt_color_functions);
            painter.drawText(RV_DV_LMARGIN + 3 * char_width,
                             current_y + line_height - 3 - 1,
                             t->file->va_analyzed_functions_hash.value(
                                     current_desc->va)
                             +":");
            painter.drawPixmap(RV_DV_LMARGIN ,
                               current_y+1,
                               pm_functions);

            current_y+=line_height;
            current_line_height_total += line_height;
        } else if(t->file->va_names_hash.contains(current_desc->va)) {
            // NAMES
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );

            painter.setFont(my_txt_font_bold);
            painter.setPen(txt_color_names);
            painter.drawText(RV_DV_LMARGIN + 3 * char_width,
                             current_y + line_height - 3 - 1,
                             t->file->va_names_hash.value(
                                     current_desc->va)
                             +":");
            painter.drawPixmap(RV_DV_LMARGIN ,
                               current_y+1,
                               pm_names);

            current_y+=line_height;
            current_line_height_total += line_height;
        } else if(t->file->va_symbols_map.contains(current_desc->va)) {
            //SYMBOL
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );

            // --- OFFSET ---
            if(show_offsets) {
            painter.setPen(txt_color_offset);
            painter.setFont(my_font);
            painter.drawText(RV_DV_LMARGIN + col_offset * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->offset));
            }

            // --- VA ---
            painter.setPen(txt_color_va);
            painter.setFont(my_font_bold);
            painter.drawText(RV_DV_LMARGIN + col_va * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->va));

            if(t->disassembler->va_xrefs_mmap.contains(current_desc->va)) {
                painter.drawPixmap(RV_DV_LMARGIN + (col_va - 3) * char_width + 3,
                                   current_y + 3,
                                   pm_xr1
                                   );
            }

            painter.setFont(my_txt_font_bold);
            painter.setPen(txt_color_symbols);
            painter.drawText(RV_DV_LMARGIN + (col_bytes + 3) * char_width,
                             current_y + line_height - 3 - 1,
                             QString::fromAscii(
                                 t->file->va_symbols_map.value(
                                     current_desc->va))
                             );
            painter.drawPixmap(RV_DV_LMARGIN + (col_bytes) * char_width ,
                               current_y+1,
                               pm_symbols);

            current_y+=line_height;
            current_line_height_total += line_height;

//            if(!repaint_mouse) {
//                struct rv_disasm_line l;
//                l.nr = i;
//                l.height = current_line_height_total;
//                l.y = current_y;
//                l.had_line = had_line;
//                l.prev_had_line = prev_had_line;
//                disasm_lines << l;
//            }
        }

        if(current_desc->desc == 'G') {           
            re_addr_t buf_len = t->disassembler->datablocks_hash.value(current_desc->offset);
            re_addr_t buf_va = current_desc->va;

            // create hexbox, set dimensions
            if(!repaint_mouse)
            if(!hex_edits.contains(buf_va)) {
                hex_edits[buf_va] = new rv_hex_edit(0, this);
                hex_edits[buf_va]->setAttribute(Qt::WA_DeleteOnClose);
                hex_edits[buf_va]->set_target(t);
                hex_edits[buf_va]->set_clipping_region(current_desc->offset,
                                                       buf_len, true);
                hex_edits[buf_va]->setFixedWidth(
                            hex_edits[buf_va]->get_total_width() + 25);

                if(buf_len <= (16*8))
                    hex_edits[buf_va]->setFixedHeight(
                                    hex_edits[buf_va]->get_total_height()
                                );
                else hex_edits[buf_va]->setFixedHeight(8*20 + 40);

                hex_edits_status[buf_va] = false;
            }

            if(hex_edits_status[buf_va]) {
                // show hexbox
                current_line_height = hex_edits[buf_va]->height() +
                        line_height + 2 + 2;

                if(current_desc != cursor_desc) bgcolor = bgcolor_hex;
                if(repaint_mouse && !mouse_clear) bgcolor = bgcolor_mouse;
                painter.fillRect(0,
                                 current_y,
                                 viewport()->width(),
                                 current_line_height,
                                 bgcolor);
                painter.setPen(QPen(QBrush(dis_color_line_solid), 1, Qt::SolidLine));
                painter.drawLine(0,
                                 current_y - 1,
                                 viewport()->width(),
                                 current_y - 1);

                painter.setPen(QPen(QBrush(dis_color_line_solid), 1, Qt::SolidLine));
                painter.drawLine(0,
                                 current_y + current_line_height-1,
                                 viewport()->width(),
                                 current_y + current_line_height-1);

                painter.drawPixmap(RV_DV_LMARGIN + col_bytes * char_width,
                                   current_y, pm_data_marker2);

                hex_edits[buf_va]->move(RV_DV_LMARGIN + col_va * char_width,
                                        current_y + line_height + 2);
                if(!repaint_mouse)
                hex_edits_current_to_show << hex_edits[buf_va];

                had_line = true;

            } else {
                // show data marker

                current_line_height = line_height;

                if(current_desc != cursor_desc) bgcolor = bgcolor_hex;
                if(repaint_mouse && !mouse_clear) bgcolor = bgcolor_mouse;
                painter.fillRect(0,
                                 current_y,
                                 viewport()->width(),
                                 current_line_height,
                                 bgcolor);
                painter.setPen(QPen(QBrush(dis_color_line_solid), 1, Qt::SolidLine));
                painter.drawLine(0,
                                 current_y - 1,
                                 viewport()->width(),
                                 current_y - 1);

                painter.setPen(QPen(QBrush(dis_color_line_solid), 1, Qt::SolidLine));
                painter.drawLine(0,
                                 current_y + current_line_height-1,
                                 viewport()->width(),
                                 current_y + current_line_height-1);

                painter.drawPixmap(RV_DV_LMARGIN + col_bytes * char_width,
                                   current_y, pm_data_marker);

                had_line = true;
            }

            // draw data block text (always)

            // offset
            if(show_offsets) {
            painter.setPen(txt_color_offset);
            painter.setFont(my_font);
            painter.drawText(RV_DV_LMARGIN + col_offset * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->offset));
            }

            painter.setPen(txt_color_va);
            painter.setFont(my_font_bold);
            painter.drawText(RV_DV_LMARGIN + col_va * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->va));


            painter.setPen(txt_color_normal);
            painter.setFont(my_font_bold);
            line =  QString::number(buf_len);
            if(buf_len > 1) line += " bytes"; else line+= " byte";

            painter.drawText(RV_DV_LMARGIN + col_bytes *char_width + pm_data_marker.width() + 5,
                             current_y + line_height - 3 - 1, line);
            painter.setFont(my_font);

            current_y+= current_line_height;
            current_line_height_total += current_line_height;
//            if(!repaint_mouse)
//            line_heights << current_line_height_total;

        } else if(current_desc->desc == 'g') {
            line = "Gap 'g': OFFSET: "
                    + re_core::dword_to_pure_hex_string(current_desc->offset)
                    + " len: "
                    + QString::number(
                        t->disassembler->datablocks_hash.value(current_desc->offset)
                        );
//            painter.drawText(5, current_y + line_height - 3, line);
//            line_heights << line_height;
//            current_y += line_height;
        } else if(current_desc->desc == 'S') {
            // handled ABOVE !!
        } else if((current_desc->desc == 'c')) {
            // TREATING STRINGS COMPLETELY HERE and then in insn
            //SYMBOL
            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );

            // --- OFFSET ---
            if(show_offsets) {
            painter.setPen(txt_color_offset);
            painter.setFont(my_font);
            painter.drawText(RV_DV_LMARGIN + col_offset * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->offset));
            }

            // --- VA ---
            painter.setPen(txt_color_va);
            painter.setFont(my_font_bold);
            painter.drawText(RV_DV_LMARGIN + col_va * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->va));

            if(t->disassembler->va_xrefs_mmap.contains(current_desc->va)) {
                painter.drawPixmap(RV_DV_LMARGIN + (col_va - 3) * char_width + 3,
                                   current_y + 3,
                                   pm_xr1
                                   );
            }

            painter.setFont(my_txt_font);
            painter.setPen(txt_color_strings);
            painter.drawText(RV_DV_LMARGIN + (col_bytes + 4) * char_width,
                             current_y + line_height - 3 - 1,
                             "\"" + QString::fromAscii(
                                 t->file->va_cstrings_hash.value(
                                     current_desc->va))
                             +"\"");
            painter.drawPixmap(RV_DV_LMARGIN + (col_bytes) * char_width,
                               current_y+1,
                               pm_strings);

            current_y+=line_height;
            current_line_height_total += line_height;
//            if(!repaint_mouse)
//            line_heights << current_line_height_total;
        } else {

            // ==== draw insn ====

            t->disassembler->disassemble_single(current_desc->offset, &tmp_insn);

            if(!tmp_insn.size) {
                // WEIRD ERROR IN THIS CASE
                return;
            }

            current_line_height = line_height;

            painter.fillRect(0,
                             current_y,
                             viewport()->width(),
                             line_height,
                             bgcolor
                             );

            // --- OFFSET ---
            if(show_offsets) {
            painter.setPen(txt_color_offset);
            painter.setFont(my_font);
            painter.drawText(RV_DV_LMARGIN + col_offset * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->offset));
            }

            // --- VA ---
            painter.setPen(txt_color_va);
            painter.setFont(my_font_bold);
            painter.drawText(RV_DV_LMARGIN + col_va * char_width,
                             current_y + line_height - 3 - 1,
                             re_core::dword_to_pure_hex_string(current_desc->va));
            // va
            if(t->disassembler->va_xrefs_mmap.contains(current_desc->va)) {
                painter.drawPixmap(RV_DV_LMARGIN + (col_va - 3) * char_width + 3,
                                   current_y + 3,
                                   pm_xr1
                                   );
            }

            // --- BYTES ---
            int tmp_cnt = tmp_insn.size;
            QString s_hex_append;
            if(tmp_cnt > show_hex_bytes_cnt) {
                tmp_cnt = show_hex_bytes_cnt;
                s_hex_append = "...";
            }
            if(show_hex_bytes_cnt) {
                for(int i=0; i < (tmp_cnt); i++) {
                    sprintf(line_asc, "%02X ", tmp_insn.bytes[i]);
                    line += QString::fromAscii(line_asc);
                }
                line += s_hex_append;
                painter.setPen(txt_color_bytes);
                painter.setFont(my_font);
                painter.drawText(RV_DV_LMARGIN + col_bytes * char_width,
                                 current_y + line_height - 3 - 1,
                                 line);
            }

            // --- MNEMONIC ---
            painter.setFont(my_font_bold);

            if(tmp_insn.type == insn_call)
                painter.setPen(dis_color_insn_call);

            else if((tmp_insn.type == insn_return) || (tmp_insn.type == insn_jmp))
                painter.setPen(dis_color_insn_jmp_ret);

            else if((tmp_insn.type == insn_jcc) || (tmp_insn.type == insn_callcc))
                painter.setPen(dis_color_insn_jcc);

            else if((tmp_insn.type == insn_test) || (tmp_insn.type == insn_cmp)
                    || (tmp_insn.type == insn_bittest))
                painter.setPen(dis_color_insn_cmp);

            else if(tmp_insn.group == insn_interrupt)
                painter.setPen(dis_color_insn_interrupt);

            else if(tmp_insn.group == insn_stack)
                painter.setPen(dis_color_insn_stack);

            else if(tmp_insn.type == insn_nop) {
                painter.setPen(dis_color_insn_nop);
                painter.setFont(my_font);
            }

            else painter.setPen(dis_color_insn);

            t->disassembler->format_mnemonic(&tmp_insn, line_asc, 256);
            painter.drawText(RV_DV_LMARGIN + col_mnemonic * char_width,
                             current_y + line_height - 3 - 1,
                             QString::fromAscii(line_asc));

            // --- OPERANDS ---

            //painter.setPen(dis_color_insn);
            painter.setFont(my_font);
            col_comment = 0;

            // do the lookups ourselves ...
            re_addr_t op_addr;
            op_len = 0;
            bool modified_operand;

            QString op_txt;

            // ... dest ...
            modified_operand = false;
            op_txt = "";            

            if (t->disassembler->op1_lookup_addr_n_format(
                        &tmp_insn, op_addr, line_asc)  ) {
                if(op_addr) {
                    if(t->disassembler->follow_control_flow(&tmp_insn)) {
                        painter.drawPixmap(RV_DV_LMARGIN + (col_operands -2)
                                           * char_width -2,
                                           current_y,
                                           pm_follow_controlflow);
                        painter.setFont(my_font_bold);
                    }

                    if(t->file->va_imports_map.contains(op_addr)) {
                        op_txt = t->file->va_imports_map.value(op_addr);
                        painter.setFont(my_font_bold);

//                        painter.fillRect(RV_DV_LMARGIN + col_operands * char_width,
//                                         current_y,
//                                         line.length() * char_width,
//                                         line_height,
//                                         dis_bgcolor_string
//                                         );


                        painter.drawText(RV_DV_LMARGIN + (col_operands)
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         op_txt);
                        modified_operand = true;
                    }

                    else if(t->file->va_symbols_map.contains(op_addr)){
                        op_txt = t->file->va_symbols_map.value(op_addr);
                        painter.setFont(my_font_bold);

//                        painter.fillRect(RV_DV_LMARGIN + col_operands * char_width,
//                                         current_y,
//                                         line.length() * char_width,
//                                         line_height,
//                                         dis_bgcolor_string
//                                         );


                        painter.drawText(RV_DV_LMARGIN + (col_operands)
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         op_txt);
                        modified_operand = true;
                    }

                    else if(t->file->va_functions_map.contains(op_addr)){
                        op_txt = t->file->va_functions_map.value(op_addr);
                        painter.setFont(my_font_bold);

//                        painter.fillRect(RV_DV_LMARGIN + col_operands * char_width,
//                                         current_y,
//                                         line.length() * char_width,
//                                         line_height,
//                                         dis_bgcolor_string
//                                         );


                        painter.drawText(RV_DV_LMARGIN + (col_operands)
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         op_txt);
                        modified_operand = true;
                    }

                    else if(t->file->va_cstrings_hash.contains(op_addr)){
                        op_txt = QString::fromAscii(
                                    t->file->va_cstrings_hash.value(op_addr));
                        if(op_txt.length() > 30) op_txt = op_txt.left(30) + " ...";

                        painter.setFont(my_font);

                        if(current_desc != cursor_desc)
                            painter.fillRect(RV_DV_LMARGIN + (col_operands + 3)
                                         * char_width,
                                         current_y + 1,
                                         op_txt.length() * char_width,
                                         line_height - 2,
                                         dis_bgcolor_string
                                         );

                        painter.setPen(txt_color_normal);
                        painter.drawText(RV_DV_LMARGIN + (col_operands)
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         "\"");

                        painter.setPen(dis_color_string);
                        painter.drawText(RV_DV_LMARGIN + (col_operands + 1)
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         op_txt);
                        painter.setPen(txt_color_normal);
                        painter.drawText(RV_DV_LMARGIN + (col_operands + 1
                                                          + op_txt.length())
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         "\"");
                        modified_operand = true;
                    }

                    if(!modified_operand) {
                        op_txt = QString::fromAscii(line_asc);

                        painter.drawText(RV_DV_LMARGIN + (col_operands)
                                         * char_width,
                                         current_y + line_height - 3 - 1,
                                         op_txt);
                    }
                } else {
                    op_txt = QString::fromAscii(line_asc);
                    painter.setFont(my_font);

                    painter.drawText(RV_DV_LMARGIN + (col_operands)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     op_txt);
                }
                op_len = op_txt.length();
            }


            // ... src ...
            modified_operand = false;
            op_txt = "";
            //painter.setPen(dis_color_insn);


            if (t->disassembler->op2_lookup_addr_n_format(
                        &tmp_insn, op_addr, line_asc)  ) {
                if(t->file->va_imports_map.contains(op_addr)) {
                    op_txt += t->file->va_imports_map.value(op_addr);
                    painter.setFont(my_font_bold);

//                        painter.fillRect(RV_DV_LMARGIN + col_operands * char_width,
//                                         current_y,
//                                         line.length() * char_width,
//                                         line_height,
//                                         dis_bgcolor_string
//                                         );


                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     op_txt);
                    modified_operand = true;
                }
                else if(t->file->va_symbols_map.contains(op_addr)){
                    op_txt += t->file->va_symbols_map.value(op_addr);
                    painter.setFont(my_font_bold);

//                        painter.fillRect(RV_DV_LMARGIN + col_operands * char_width,
//                                         current_y,
//                                         line.length() * char_width,
//                                         line_height,
//                                         dis_bgcolor_string
//                                         );


                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     op_txt);
                    modified_operand = true;
                }

                else if(t->file->va_functions_map.contains(op_addr)){
                    op_txt += t->file->va_functions_map.value(op_addr);
                    painter.setFont(my_font_bold);

//                        painter.fillRect(RV_DV_LMARGIN + col_operands * char_width,
//                                         current_y,
//                                         line.length() * char_width,
//                                         line_height,
//                                         dis_bgcolor_string
//                                         );


                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     op_txt);
                    modified_operand = true;
                }

                else if(t->file->va_cstrings_hash.contains(op_addr)){
                    op_txt = QString::fromAscii(
                                t->file->va_cstrings_hash.value(op_addr));
                    if(op_txt.length() > 30) op_txt = op_txt.left(30) + " ...";
                    if(current_desc != cursor_desc)
                        painter.fillRect(RV_DV_LMARGIN + (col_operands + op_len + 3) * char_width,
                                     current_y + 1,
                                     op_txt.length() * char_width,
                                     line_height - 2,
                                     dis_bgcolor_string
                                     );

                    painter.setPen(txt_color_normal);
                    painter.setFont(my_font);
                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     ", \"");

                    painter.setPen(dis_color_string);
                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len + 3)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     op_txt);

                    painter.setPen(txt_color_normal);
                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len
                                                      + 3 + op_txt.length())
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     "\"");
                    modified_operand = true;
                }



                if(!modified_operand) {
                    op_txt = QString::fromAscii(line_asc);
                    //painter.setPen(dis_color_insn);
                    painter.setFont(my_font);

                    painter.drawText(RV_DV_LMARGIN + (col_operands + op_len)
                                     * char_width,
                                     current_y + line_height - 3 - 1,
                                     op_txt);
                }
                op_len += op_txt.length();
            }

            // ... imm ...
            if(t->disassembler->op3_format(&tmp_insn, line_asc)) {
                op_txt = QString::fromAscii(line_asc);
                //painter.setPen(dis_color_insn);

                painter.drawText(RV_DV_LMARGIN + (col_operands + op_len)
                                 * char_width,
                                 current_y + line_height - 3 - 1,
                                 op_txt);
                op_len += op_txt.length();
            }

            // COMMENTS N STUFF
            col_comment = col_operands + op_len + 1;
            // if(col_comment < col_comments) col_comment = col_comments;
            if(navi_stack && navi_stack->contains(current_desc->va)) {
                painter.drawPixmap(RV_DV_LMARGIN + col_comment * char_width,
                                   current_y, pm_ret_address);
            }

            // ------ /first line of total current line ---

            current_y += current_line_height;
            current_line_height_total+= current_line_height;

            if(t->file->va_cstrings_hash.contains(current_desc->va)) {
                        // TREATING STRINGS COMPLETELY HERE AGAIN
                        // SYMBOL
                        painter.fillRect(0,
                                         current_y,
                                         viewport()->width(),
                                         line_height,
                                         bgcolor
                                         );

                        // --- OFFSET ---
                        if(show_offsets) {
                        painter.setPen(txt_color_offset);
                        painter.setFont(my_font);
                        painter.drawText(RV_DV_LMARGIN + col_offset * char_width,
                                         current_y + line_height - 3 - 1,
                                         re_core::dword_to_pure_hex_string(current_desc->offset));
                        }

                        // --- VA ---
                        painter.setPen(txt_color_va);
                        painter.setFont(my_font_bold);
                        painter.drawText(RV_DV_LMARGIN + col_va * char_width,
                                         current_y + line_height - 3 - 1,
                                         re_core::dword_to_pure_hex_string(current_desc->va));

                        if(t->disassembler->va_xrefs_mmap.contains(current_desc->va)) {
                            painter.drawPixmap(RV_DV_LMARGIN + (col_va - 3) * char_width + 3,
                                               current_y + 3,
                                               pm_xr1
                                               );
                        }

                        painter.setFont(my_txt_font);
                        painter.setPen(txt_color_strings);
                        painter.drawText(RV_DV_LMARGIN + (col_bytes + 4) * char_width,
                                         current_y + line_height - 3 - 1,
                                         "\"" + QString::fromAscii(
                                             t->file->va_cstrings_hash.value(
                                                 current_desc->va))
                                         +"\"");
                        painter.drawPixmap(RV_DV_LMARGIN + (col_bytes) * char_width,
                                           current_y+1,
                                           pm_strings);

                        current_y+=line_height;
                        current_line_height_total += line_height;
            }

            if(tag_processor) {
            int tag_cnt = 0;
            if(tag_processor->has_va_tag(current_desc->va, RV_TAG_YEL)) {
                painter.drawPixmap(RV_DV_LMARGIN + col_extras * char_width
                                   + tag_cnt * 16,
                                   current_y - line_height, pm_tag_y);
                tag_cnt++;
            }
            if(tag_processor->has_va_tag(current_desc->va, RV_TAG_GRN)) {
                painter.drawPixmap(RV_DV_LMARGIN + col_extras * char_width
                                   + tag_cnt * 16,
                                   current_y - line_height, pm_tag_g);
                tag_cnt++;
            }
            if(tag_processor->has_va_tag(current_desc->va, RV_TAG_BLU)) {
                painter.drawPixmap(RV_DV_LMARGIN + col_extras * char_width
                                   + tag_cnt * 16,
                                   current_y - line_height, pm_tag_b);
                tag_cnt++;
            }
            if(tag_processor->has_va_tag(current_desc->va, RV_TAG_RED)) {
                painter.drawPixmap(RV_DV_LMARGIN + col_extras * char_width
                                   + tag_cnt * 16,
                                   current_y - line_height, pm_tag_r);
                tag_cnt++;
            }
            if(tag_processor->has_va_tag(current_desc->va, RV_TAG_BLK)) {
                painter.drawPixmap(RV_DV_LMARGIN + col_extras * char_width
                                   + tag_cnt * 16,
                                   current_y - line_height, pm_tag_B);
                tag_cnt++;
            }
            }

//            if(tag_editor_desc && (current_desc == tag_editor_desc)) {
//                tag_editor->move(RV_DV_LMARGIN + col_extras * char_width,
//                                 current_y - line_height);
//            }

            // --- LINES ---
            if((tmp_insn.type == insn_jmp) ||
                    (tmp_insn.type == insn_return) ||
                    (tmp_insn.type == insn_halt) ) {
                painter.setPen(QPen(QBrush(dis_color_line_dashed), 1, Qt::DashLine));
                painter.drawLine(0,
                                 current_y -1,
                                 viewport()->width(),
                                 current_y -1);

                had_line = true;
            }


//            current_y += current_line_height;
//            current_line_height_total+= current_line_height;
//            if(!repaint_mouse)
//            line_heights << current_line_height_total;
        }

        if(!repaint_mouse) {
            struct rv_disasm_line l;
            l.nr = i;
            l.height = current_line_height_total;
            l.y = current_y - current_line_height_total;
            l.had_line = had_line;
            l.prev_had_line = prev_had_line;
            l.op_len = op_len;
            disasm_lines << l;
        }

        if(op_len > max_op_len) max_op_len = op_len;

        // ******* CURSOR DECORATION ************
        if(current_desc == cursor_desc) {
//            painter.setPen(QPen(QBrush(bgcolor_cursorline2), 1, Qt::DotLine));
//            painter.drawRect(0,
//                             current_y - current_line_height_total,
//                             viewport()->width()-1,
//                             current_line_height_total-2);

//            if(prev_had_line) {
//                painter.setPen(bgcolor_cursorline);
//                painter.drawLine(0,
//                                 current_y - current_line_height_total,
//                                 viewport()->width(),
//                                 current_y - current_line_height_total);
//            }
//            if(had_line) {
//                painter.setPen(bgcolor_cursorline);
//                painter.drawLine(0,
//                                 current_y -2,
//                                 viewport()->width(),
//                                 current_y   -2);

//            }
        } else if((current_desc == mouse_desc) && !mouse_clear) {
//            painter.setPen(QPen(QBrush(bgcolor_mouse.darker(120)), 1, Qt::DotLine));
//            painter.drawRect(0,
//                             current_y - current_line_height_total +1,
//                             viewport()->width()-1,
//                             current_line_height_total-3);

//            if(!odd_even) {
//                painter.setPen(QColor(0xff, 0xff, 0xff));
//                painter.drawRect(0,
//                                 current_y - current_line_height_total +1,
//                                 viewport()->width()-1,
//                                 current_line_height_total-3);
//            }
        }


        // ******** XREFS WIDGETS ***********

        rv_disasm_xrefs_widget *w;

        if(xrefs_widgets.contains(current_desc)) {
            w = xrefs_widgets.value(current_desc);
            if(w->active) {
                w->move(RV_DV_LMARGIN + (col_va-3) *char_width, current_y);
                w->show();
            }
        }

        // === N E X T ===

        prev_had_line = had_line;

        if(current_y > viewport()->height()) break;

        if(current_desc == my_address_space->last()) break;

        current_desc = current_desc->next;
    }

    if(repaint_mouse) {
        repaint_mouse = false;
        am_painting = false;
        return;
    }

    lastline = current_desc->prev;

    am_painting = false;
    if(unpainted) {
        unpainted = false;
        emit first_paint();
    }
}

void rv_disasm_view::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        rmb_clicked();
        return;
    }

    if(!t || !my_address_space->count()) return;
    if(!mouse_desc) return;
    int x = event->x();

    re_addr_desc *current_desc = mouse_desc;
    rv_disasm_line current_line = mouse_line;

    // handle selection
    if(event->modifiers() & Qt::ShiftModifier) {
        if(mouse_desc == cursor_desc) return;
        if(mouse_desc->va > cursor_desc->va) {
            selection_start = cursor_desc;
            selection_end = mouse_desc;
            have_selection = true;
        } else {
            selection_end = cursor_desc;
            selection_start = mouse_desc;
            have_selection = true;
        }

        emit do_repaint();

        return;
    }

    if(hex_edits_status.contains(current_desc->va)) {
        if(hex_edits_status[current_desc->va]) hex_edits_status[current_desc->va] = false;
        else hex_edits_status[current_desc->va] = true;
        emit repaintme();
    }

    // insn controlflow -> addr operand clicked
    if((x > (RV_DV_LMARGIN + (col_operands - 2) *char_width)) &&
            (x <= (RV_DV_LMARGIN + (col_operands + current_line.op_len) * char_width))) {

        x86_insn_t tmp_insn;
        re_addr_t va_dest;

        t->disassembler->disassemble_single(current_desc->offset, &tmp_insn);
        if(tmp_insn.size && (va_dest = t->disassembler->follow_control_flow(&tmp_insn))) {
            scroll_n_cursor_to(va_dest);
            emit va_clicked(va_dest);
            emit jmped_from(current_desc->va);
            return;
        }
    }

    // xrefs
    if(   (x >= (RV_DV_LMARGIN + (col_va - 3) * char_width + 3))
       && (x <= (RV_DV_LMARGIN + (col_va + 8) * char_width))
      ) {
        rv_disasm_xrefs_widget *w;
        if(t->disassembler->va_xrefs_mmap.contains(current_desc->va)) {
            if(!xrefs_widgets.contains(current_desc)) {
                w = new rv_disasm_xrefs_widget(this);
                w->set_desc(current_desc, t);
                xrefs_widgets.insert(current_desc, w);
                w->active = true;
                connect(w, SIGNAL(va_dblclicked(re_addr_t)),
                        this, SLOT(scroll_n_cursor_to(re_addr_t)));
                connect(w, SIGNAL(va_dblclicked(re_addr_t)),
                        this, SIGNAL(va_clicked(re_addr_t)));
                connect(w, SIGNAL(jmp_from(re_addr_t)),
                        this, SIGNAL(jmped_from(re_addr_t)));
            } else {
                w = xrefs_widgets.value(current_desc);
                if(w->active) {
                    w->active = false;
                    w->hide();
                } else {
                    w->active = true;
                }
            }

            emit repaintme();
        }
    }

    // stars
    if((cursor_desc == current_desc) && (x >=
            (RV_DV_LMARGIN + col_extras * char_width))
            && (x <=
            (RV_DV_LMARGIN + col_extras * char_width + (16*5)))
            && tag_processor && tag_editor && mouse_desc) {
        tag_editor_desc = mouse_desc;
        tag_editor->set_va(mouse_desc->va);
        int y = mouse_line.y + mouse_line.height - tag_editor->height();
        tag_editor->move(RV_DV_LMARGIN + col_extras * char_width,
                         y);
        tag_editor->show();
    }

    if(cursor_desc != current_desc) {
        cursor_desc = current_desc;
        emit cursor_changed_to(cursor_desc->va);
        emit repaintme();
    }
}

void rv_disasm_view::contextMenuEvent(QContextMenuEvent *e)
{
    if(!mouse_desc) {
        // only paste possible
        QMenu *ctx_menu = new QMenu("CTX");

        if(selection && selection->active) {
            if(mode == DV_MODE_FILTER) ctx_menu->addAction(menu_action_paste);
            if(mode == DV_MODE_FILTER) ctx_menu->addAction(menu_action_ctf_paste);
        }
        if(my_address_space && my_address_space->count()) {
            ctx_menu->addSeparator();

            menu_action_show_hide_offsets->setCheckable(true);
            menu_action_show_hide_offsets->setChecked(show_offsets);
            ctx_menu->addAction(menu_action_show_hide_offsets);
        }

        ctx_menu->exec(e->globalPos());
        return;

    } else rmb_clicked();
}

void rv_disasm_view::rmb_clicked()
{
    if(!mouse_desc) return;

    QMenu *ctx_menu = new QMenu("CTX");
    x86_insn_t tmp_insn;
    mouse_va_dest = BAD_ADDR;
    mouse_have_dest = false;
    mouse_va_here = mouse_desc->va;

    t->disassembler->disassemble_single(mouse_desc->offset, &tmp_insn);
    if(tmp_insn.size && (mouse_va_dest = t->disassembler->follow_control_flow(&tmp_insn))) {
        mouse_have_dest = true;
    }

    // ----------------------------------------

    // CTF FROM HERE TO FILTER
    menu_action_ctf_here_to_filter->setText(
                menu_action_strings[menu_action_ctf_here_to_filter] +
                " (" +
                re_core::dword_to_pure_hex_string(mouse_desc->va) +
                ")"
                );
    ctx_menu->addAction(menu_action_ctf_here_to_filter);

    // CTF FROM DEST TO FILTER
    if(mouse_have_dest) {
        menu_action_ctf_dest_to_filter->setText(
                    menu_action_strings[menu_action_ctf_dest_to_filter] +
                    " (" +
                    re_core::dword_to_pure_hex_string(mouse_va_dest) +
                    ")"
                    );
        ctx_menu->addAction(menu_action_ctf_dest_to_filter);
    }

    ctx_menu->addSeparator();


    // LINEAR FROM HERE TO FILTER
    menu_action_linear_here_to_filter->setText(
                menu_action_strings[menu_action_linear_here_to_filter] +
                " (" +
                re_core::dword_to_pure_hex_string(mouse_desc->va) +
                ")"
                );
    ctx_menu->addAction(menu_action_linear_here_to_filter);

    // LINEAR FROM DEST TO FILTER
    if(mouse_have_dest) {
        menu_action_linear_dest_to_filter->setText(
                    menu_action_strings[menu_action_linear_dest_to_filter] +
                    " (" +
                    re_core::dword_to_pure_hex_string(mouse_va_dest) +
                    ")"
                    );
        ctx_menu->addAction(menu_action_linear_dest_to_filter);
    }

    ctx_menu->addSeparator();

    // ----------------------------------------

    if(my_address_space->count()) ctx_menu->addAction(menu_action_select_all);

    if(have_selection) {
        ctx_menu->addAction(menu_action_copy);
        ctx_menu->addAction(menu_action_delete);
        ctx_menu->addAction(menu_action_clear_selection);
        ctx_menu->addSeparator();
    }


    if(selection && selection->active) {
        if(mode == DV_MODE_FILTER) ctx_menu->addAction(menu_action_paste);
        if(mode == DV_MODE_FILTER) ctx_menu->addAction(menu_action_ctf_paste);
        if(mode == DV_MODE_FILTER) ctx_menu->addSeparator();

        if(mode == DV_MODE_TARGET) ctx_menu->addAction(menu_action_paste_to_new_filter);
        if(mode == DV_MODE_TARGET) ctx_menu->addAction(menu_action_ctf_paste_to_new_filter);
    }

    ctx_menu->addSeparator();

    menu_action_show_hide_offsets->setCheckable(true);
    menu_action_show_hide_offsets->setChecked(show_offsets);
    ctx_menu->addAction(menu_action_show_hide_offsets);

    // dump stuff

    ctx_menu->addSeparator();

    if(have_selection) {
        ctx_menu->addAction(menu_action_dump_sel_to_html);
    }

    if(my_address_space->count()) {
        ctx_menu->addAction(menu_action_dump_view_to_html);
    }

    ctx_menu->exec(mouse_global_pos);
}


void rv_disasm_view::disassembly_changed()
{
    if(t->disassembler->get_address_space() != my_address_space) return;
    freeze = true;
    post_disassembly_change();
}

void rv_disasm_view::do_repaint()
{
    mouse_desc = 0;

    if(!hex_edits.isEmpty()) {
        foreach(rv_hex_edit *hv, hex_edits)
            hv->hide();
    }

    if(!xrefs_widgets.isEmpty()) {
        foreach(rv_disasm_xrefs_widget *w, xrefs_widgets)
            w->hide();
    }

    viewport()->repaint();

    if(!hex_edits_current_to_show.isEmpty()) {
        foreach(rv_hex_edit *hv, hex_edits_current_to_show)
            hv->show();
    }
}

void rv_disasm_view::first_paint_updates()
{
    if(!hex_edits_current_to_show.isEmpty()) {
        foreach(rv_hex_edit *hv, hex_edits_current_to_show)
            hv->show();
    }
    mouse_active = false;
}

void rv_disasm_view::scrollbar_value_changed(int new_v)
{
    if(!t && !buffer_data) return;
    if(!my_address_space) return;
    if(!my_address_space->count()) return;

    if(new_v > scrollbar_current_val) {
        while(scrollbar_current_val < new_v) {
            scrollbar_current_val++;
            if(top_desc != my_address_space->last())
            top_desc = top_desc->next;
        }
    } else     if(new_v < scrollbar_current_val) {
        while(scrollbar_current_val > new_v) {
            scrollbar_current_val--;
            if(top_desc != my_address_space->first())
            top_desc = top_desc->prev;
        }
    }

    if(tooltip_widget) {
        if(tooltip_widget) delete tooltip_widget;
        tooltip_widget = 0;
    }

    if(!freeze) do_repaint();
}

void rv_disasm_view::scroll_to(re_addr_t va)
{
    re_addr_t tmp_offs;
    if(!top_desc) return;

    if(va == top_desc->va) return;

    if(!t->file->b_va_to_file_offset(tmp_offs, va)) return;

    int my_scroll_pos = scrollbar_current_val;
    re_addr_desc *tmp_desc = top_desc;

    if(va < tmp_desc->va)
    while ( (va < tmp_desc->va) && my_scroll_pos) {
            tmp_desc = tmp_desc->prev;
            my_scroll_pos--;
    }
    else while ( (va > tmp_desc->va) &&
                (my_scroll_pos < verticalScrollBar()->maximum())) {
        tmp_desc = tmp_desc->next;
        my_scroll_pos++;
    }

    verticalScrollBar()->setValue(my_scroll_pos);
}

bool rv_disasm_view::get_scrollbar_pos(re_addr_t va, int &val)
{
    re_addr_desc *tmp_desc = my_address_space->first();

    for(int i=0; i<my_address_space->count(); i++) {
        if(va == tmp_desc->va) {
            val = i;
            return true;
        }
        tmp_desc = tmp_desc->next;
    }

    val = 0;
    return false;
}

void rv_disasm_view::cursor_down() {
    if(cursor_desc != my_address_space->last()) {
        cursor_desc = cursor_desc->next;
        // if oldpos was last line
        if(cursor_desc->prev == lastline) {
            scroll_to(top_desc->next->va);
            do_repaint();
        } else {
            viewport()->repaint();
        }
    }
    emit cursor_changed_to(cursor_desc->va);
}

void rv_disasm_view::cursor_up() {
    if(cursor_desc != my_address_space->first()) {
        cursor_desc = cursor_desc->prev;
        // if oldpos was first line
        if(cursor_desc->next == top_desc) {
            scroll_to(top_desc->prev->va);
            do_repaint();
        } else {
            viewport()->repaint();
        }
    }
    emit cursor_changed_to(cursor_desc->va);
}

void rv_disasm_view::keyPressEvent(QKeyEvent *event)
{
    if(!t || !my_address_space->count()) return;

    re_addr_t va = 0;
    if(cursor_desc) va = cursor_desc->va;

    switch(event->key()) {
    case Qt::Key_Down:
        if(event->modifiers() == Qt::ShiftModifier) {
            if(!have_selection) {
                selection_start = cursor_desc;
                cursor_down();
                selection_end = cursor_desc;
                have_selection = true;
            } else {
                cursor_down();
                selection_end = cursor_desc;
                if(selection_start->va > selection_end->va) {
                    selection_end = selection_start;
                    selection_start = cursor_desc;
                }
            }
            emit do_repaint();
        }
        else cursor_down();
        break;
    case Qt::Key_Up:
        if(event->modifiers() == Qt::ShiftModifier) {
            if(!have_selection) {
                selection_end = cursor_desc;
                cursor_up();
                selection_start = cursor_desc;
                have_selection = true;
            } else {
                cursor_up();
                selection_start = cursor_desc;
                if(selection_start->va > selection_end->va) {
                    selection_end = selection_start;
                    selection_start = cursor_desc;
                }
            }
            emit do_repaint();
        }
        else cursor_up();
        break;
    case Qt::Key_D:
        pre_disassembly_change();
        disassemble_ctf_from_va(va);
        break;
    case Qt::Key_Delete:
        if(have_selection) menu_delete();
        break;
    case Qt::Key_PageUp:
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
        break;
    case Qt::Key_PageDown:
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
        break;
    case Qt::Key_Home:
        if(event->modifiers() == Qt::ControlModifier) {
            verticalScrollBar()->setValue(0);
        }
        break;
    case Qt::Key_End:
        if(event->modifiers() == Qt::ControlModifier) {
            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        }
        break;

    case Qt::Key_Escape:
        menu_clear_selection();
        break;

    case Qt::Key_E:
        if(event->modifiers() == Qt::ControlModifier) {
            scroll_n_cursor_to(t->file->va_entry_point);
        }

    case Qt::Key_A:
        if(event->modifiers() == Qt::ControlModifier) {
            menu_select_all();
        }

    case Qt::Key_C:
        if(event->modifiers() == Qt::ControlModifier) {
            menu_copy();
        }

    }
}

void rv_disasm_view::disassemble_ctf_from_va(re_addr_t va)
{    
    // pre_disassembly_change(); // done by idisasm
    t->disassembler->disassemble_flow_from(va);
    t->disassembler->re_process_address_space();
}

void rv_disasm_view::pre_disassembly_change()
{
    freeze = true;

    disconnect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(scrollbar_value_changed(int)));

    if(my_address_space->count()) {
        if(cursor_desc && top_desc) {
        saved_va        = cursor_desc->va;
        saved_va_top    = top_desc->va;
        have_saved_vas = true;
        }
    } else have_saved_vas = false;

    if(!hex_edits.isEmpty()) {
        foreach(rv_hex_edit *hv, hex_edits) {           
            if(hv) {
                delete hv;
            }
        }        
        hex_edits.clear();
    }
    hex_edits_current_to_show.clear();
    hex_edits_status.clear();

    // only if i am main view:
    if(!my_address_space->is_cloned())
    t->disassembler->remove_datablocks();

    if(tooltip_widget) {
        tooltip_widget->hide();
        delete tooltip_widget;
        tooltip_widget = 0;
    }

    if(!xrefs_widgets.isEmpty())
    foreach(rv_disasm_xrefs_widget *w, xrefs_widgets.values()) {
        if(w) {
            delete w;
        }
        xrefs_widgets.clear();
    }

    mouse_active = true;
    mouse_desc = 0;

    tooltip_label_xrefs->hide();
    scrollbar_current_val = 0;
}

void rv_disasm_view::post_disassembly_change()
{
    if(t->disassembler->get_address_space() != my_address_space)
        return;

    verticalScrollBar()->setRange(0, my_address_space->count());
    verticalScrollBar()->setPageStep(num_lines_visible - 1);
    verticalScrollBar()->setValue(0);

    if(my_address_space->count()) top_desc = my_address_space->first();
    else top_desc = 0;
    cursor_desc = top_desc;
    scrollbar_current_val = 0;

    if(have_saved_vas) {
        cursor_desc = my_address_space->desc_at_va(saved_va);
        top_desc = my_address_space->desc_at_va(saved_va_top);
        if(!top_desc) top_desc = my_address_space->first();
        if(!cursor_desc) cursor_desc = top_desc;
        have_saved_vas = false;
        int v;
        get_scrollbar_pos(top_desc->va, v);
        verticalScrollBar()->setValue(v);
        scrollbar_current_val = v;
    }
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(scrollbar_value_changed(int)));

    disasm_lines.clear();

    freeze = false;
    mouse_active = false;
    emit repaintme();
    emit disassembly_finished();
}

void rv_disasm_view::scroll_n_cursor_to(re_addr_t va)
{
    if(!top_desc) return;
    mouse_desc = 0;
    mouse_active = true;
//    freeze=true;
    scroll_to(va);
    re_addr_desc *tmp_cursor_desc = top_desc;

    re_addr_desc *tmp_desc = top_desc;

    for(int i=0; i < num_lines_visible/4; i++) {
        if(tmp_desc == my_address_space->first()) break;
        tmp_desc = tmp_desc->prev;
    }
    cursor_desc = tmp_cursor_desc;
    scroll_to(tmp_desc->va);
    viewport()->repaint();
    emit cursor_changed_to(va);
    mouse_active = false;
}

void rv_disasm_view::set_navi_stack(QStack<re_addr_t> *s)
{
    navi_stack = s;
}

bool rv_disasm_view::event(QEvent *event)
{
    if(event->type() == QEvent::ToolTip) {
        int x = mouse_pos.x();

        if(!mouse_desc) return true;

        re_addr_desc *current_desc = mouse_desc;
        rv_disasm_line current_line = mouse_line;

        emit cursor_changed_to(current_desc->va);

        // COL VA
        if((x >= (RV_DV_LMARGIN + col_va * char_width ))
            && (x <= (RV_DV_LMARGIN + (col_va+8)*char_width ))
                ) {
            if(t->disassembler->va_xrefs_mmap.contains(current_desc->va)) {
                tooltip_label_xrefs->setText("# xrefs: " + QString::number(
                            t->disassembler->va_xrefs_mmap.values(current_desc->va).count())
                            );
                tooltip_label_xrefs->move(RV_DV_LMARGIN
                                          + (col_va + 3) * char_width,
                                          current_line.y + current_line.height
                                          - line_height*2);
                tooltip_label_xrefs->show();
            }
            return true;
        }


        if(tooltip_widget) return true;

        // COL_OPERANDS
        if((x >= (RV_DV_LMARGIN + col_operands*char_width ))
            && (x <= (RV_DV_LMARGIN + (col_operands + current_line.op_len)*char_width ))
                ) {

            x86_insn_t tmp_insn;
            char buf[1024];
            re_addr_t va_dest = BAD_ADDR;
            re_addr_t offs_dest;
            QString tooltext;

            int tmp_x;
            int tmp_y;
            unsigned char c;

            t->disassembler->disassemble_single(current_desc->offset, &tmp_insn);

            if(va_dest = t->disassembler->follow_control_flow(&tmp_insn)) {
                if(t->file->b_va_to_file_offset(offs_dest, va_dest)) {
                    for(int i=0; i<50; i++) {
                        t->disassembler->disassemble_single(offs_dest, &tmp_insn);
                        if(tmp_insn.size) {
                            t->disassembler->format_insn_string_lookups(&tmp_insn,
                                                                      buf, 1024, true);
                            tooltext += re_core::dword_to_pure_hex_string(tmp_insn.addr)
                                + "  " + QString::fromAscii(buf) + "\n";
                            offs_dest += tmp_insn.size;
                        } else {
                            tooltext += re_core::dword_to_pure_hex_string(tmp_insn.addr)
                                + "  " + "db " + re_core::byte_to_hex_string(
                                        t->file->b_get_byte_at_file_offset(c, offs_dest));
                            offs_dest++;
                        }
                    }
                }

                tooltip_widget = new rv_tooltip_widget(this);
                tooltip_widget->setText(tooltext);

                tooltip_widget->setFixedHeight(15 * line_height);
                tooltip_widget->is_above = false;

                tmp_x = RV_DV_LMARGIN + (col_operands + max_op_len + 1)
                        * char_width;
                if((tmp_x + tooltip_widget->width()) > viewport()->width())
                    tmp_x = viewport()->width() - tooltip_widget->width()+1;

                for(int i=0; i<(15-2); i++) {
                    tmp_y = current_line.y + current_line.height;
                    tooltip_widget->is_above = false;
                    if((tmp_y + tooltip_widget->height() ) > viewport()->height()) {
                        tmp_y = current_line.y - tooltip_widget->height();
                        tooltip_widget->is_above = true;
                    }
                    if(tmp_y > 0) break;
                    tooltip_widget->setFixedHeight(
                                tooltip_widget->height()-line_height);
                }

                tooltip_widget->move(tmp_x, tmp_y);
                tooltip_widget->show();
                tooltip_widget->setFocus();
                return true;
            }


        }
    }

    return QAbstractScrollArea::event(event);
}

void rv_disasm_view::mouseMoveEvent(QMouseEvent *e)
{
    if(mouse_active) return;

    mouse_active = true;

    QRect r;

    if(tooltip_widget) {
        int xa = RV_DV_LMARGIN + col_operands * char_width;
        int w = tooltip_widget->width();
        if(tooltip_widget->x() > xa)
            w = tooltip_widget->x() - xa + tooltip_widget->width();

        if(!tooltip_widget->is_above) {
            r = QRect(// tooltip_widget->x()-10,
                    xa,
                    tooltip_widget->y() - line_height,
                    w,
                    //tooltip_widget->height()+ 2 * line_height -1
                    tooltip_widget->height() + line_height
                    );
        } else {
            r = QRect(// tooltip_widget->x()-10,
                    xa,
                    tooltip_widget->y()-1,
                    w,
                    //tooltip_widget->height()+ 2 * line_height -1
                    tooltip_widget->height() + line_height +1
                    );
        }


        if(!r.contains(e->pos())) {
            if(tooltip_widget) delete tooltip_widget;
            tooltip_widget = 0;
            viewport()->repaint();
        }
    }


    if(!tooltip_label_xrefs->isHidden()) tooltip_label_xrefs->hide();

    mouse_pos = e->pos();
    mouse_global_pos = e->globalPos();

    int y = mouse_pos.y();
    int line;
    re_addr_desc *current_desc = top_desc;

    for(line=0; line<disasm_lines.count(); line++) {
        if((y >= disasm_lines[line].y) &&
                (y < (disasm_lines[line].y + disasm_lines[line].height))) break;
        current_desc = current_desc->next;
    }

    if(line >= disasm_lines.count()) {
        mouse_active = false;
        return;
    }

    if(mouse_desc != current_desc) {
        if(mouse_desc) {
            mouse_clear = true;
            repaint_mouse = true;
            QRegion r0(0, mouse_line.y+1,viewport()->width(), mouse_line.height-2);
            viewport()->repaint(r0);
        }

        mouse_clear = false;
        mouse_desc = current_desc;
        mouse_line = disasm_lines[line];
        repaint_mouse = true;
        QRegion r1(0, mouse_line.y+1,viewport()->width(), mouse_line.height-2);
        viewport()->repaint(r1);
    }

    if(tag_editor_desc && (tag_editor_desc != mouse_desc)) {
        tag_editor->hide();
    }

    mouse_active = false;
    //QAbstractScrollArea::mouseMoveEvent(e);
}

re_addr_desc *rv_disasm_view::get_mouse_desc()
{
    int y = mouse_pos.y();
    int line;
    re_addr_desc *current_desc = top_desc;

    for(line=0; line<disasm_lines.count(); line++) {
        if((y >= disasm_lines[line].y) &&
                (y < (disasm_lines[line].y + disasm_lines[line].height))) break;
        current_desc = current_desc->next;
    }

    return current_desc;
}

re_addr_desc *rv_disasm_view::get_cursor_desc()
{
    return cursor_desc;
}

void rv_disasm_view::set_tag_processor(rv_tag_processor *tp)
{
    tag_processor = tp;
    tag_editor->set_tag_processor(tag_processor);
}

void rv_disasm_view::focusInEvent(QFocusEvent *)
{
    emit received_focus();
}

void rv_disasm_view::set_name(QString s)
{
    name = s;
}

QString rv_disasm_view::get_name()
{
    return name;
}

void rv_disasm_view::set_mode(int m)
{
    mode = m;

    switch(mode) {
    case DV_MODE_FILTER:
        bgcolor1                        = QColor(0xff, 0xff, 0xff);
        bgcolor2                        = QColor(0xf0, 0xf8, 0xf4);
        bgcolor_mouse                   = QColor(0xe8, 0xe8, 0xe8);

        bgcolor_hex                     = QColor(0xd0, 0xd4, 0xd8);
        bgcolor_cursorline              = QColor(0xff, 0xff, 0xdd);
        bgcolor_cursorline2 = bgcolor_cursorline.darker(120);

        bgcolor_selection               = QColor(0xc4, 0xe0, 0xe8);
        bgcolor_selection_cursor        = QColor(0xf0, 0xf0, 0xcc);

        txt_color_normal                = QColor(0x40, 0x40, 0x40);
        txt_color_offset                = QColor(0x80, 0x80, 0x80);
        txt_color_va                    = QColor(0x40, 0x40, 0x40);
        txt_color_bytes                 = QColor(0x80, 0x80, 0x80);

        txt_color_imports               = QColor(0x40, 0x60, 0x40);
        txt_color_symbols               = QColor(0x40, 0x40, 0x60);
        txt_color_names                 = QColor(0x80, 0x80, 0x80);
        txt_color_functions             = QColor(0x60, 0x40, 0x40);
        txt_color_strings               = QColor(0x40, 0x40, 0x40);

        dis_color_insn                  = QColor(0x40, 0x40, 0x60);
        dis_color_insn_call             = QColor(0x50, 0x50, 0x90);
        dis_color_insn_jmp_ret          = QColor(0x20, 0x20, 0x20);
        dis_color_insn_jcc              = QColor(0x60, 0x40, 0x40);
        dis_color_insn_cmp              = QColor(0x60, 0x40, 0x40);
        dis_color_insn_interrupt        = QColor(0x80, 0x20, 0x20);
        dis_color_insn_stack            = QColor(0x40, 0x40, 0x40);
        dis_color_insn_nop              = QColor(0x80, 0x80, 0x80);

        dis_color_line_dashed           = QColor(0xa0, 0xa0, 0xc0);
        dis_color_line_solid            = bgcolor_hex.darker(110);
        dis_color_string                = QColor(0xff, 0, 0);
        dis_bgcolor_string              = QColor(0xff, 0xf0, 0xf0);

        break;
    case DV_MODE_TARGET:
    default:
        bgcolor1                        = QColor(0xff, 0xff, 0xff);
        bgcolor2                        = QColor(0xf0, 0xf4, 0xf8);
        bgcolor_mouse                   = QColor(0xe8, 0xe8, 0xe8);

        bgcolor_hex                     = QColor(0xd0, 0xd4, 0xd8);
        bgcolor_cursorline              = QColor(0xff, 0xff, 0xdd);
        bgcolor_cursorline2 = bgcolor_cursorline.darker(120);

        bgcolor_selection               = QColor(0xc4, 0xe0, 0xe8);
        bgcolor_selection_cursor        = QColor(0xf0, 0xf0, 0xcc);

        txt_color_normal                = QColor(0x40, 0x40, 0x40);
        txt_color_offset                = QColor(0x80, 0x80, 0x80);
        txt_color_va                    = QColor(0x40, 0x40, 0x40);
        txt_color_bytes                 = QColor(0x80, 0x80, 0x80);

        txt_color_imports               = QColor(0x40, 0x60, 0x40);
        txt_color_symbols               = QColor(0x40, 0x40, 0x60);
        txt_color_names                 = QColor(0x80, 0x80, 0x80);
        txt_color_functions             = QColor(0x60, 0x40, 0x40);
        txt_color_strings               = QColor(0x40, 0x40, 0x40);

        dis_color_insn                  = QColor(0x40, 0x40, 0x60);
        dis_color_insn_call             = QColor(0x50, 0x50, 0x90);
        dis_color_insn_jmp_ret          = QColor(0x20, 0x20, 0x20);
        dis_color_insn_jcc              = QColor(0x60, 0x40, 0x40);
        dis_color_insn_cmp              = QColor(0x60, 0x40, 0x40);
        dis_color_insn_interrupt        = QColor(0x80, 0x20, 0x20);
        dis_color_insn_stack            = QColor(0x40, 0x40, 0x40);
        dis_color_insn_nop              = QColor(0x80, 0x80, 0x80);

        dis_color_line_dashed           = QColor(0xa0, 0xa0, 0xc0);
        dis_color_line_solid            = bgcolor_hex.darker(110);
        dis_color_string                = QColor(0xff, 0, 0);
        dis_bgcolor_string              = QColor(0xff, 0xf0, 0xf0);

        break;
    }
}

int rv_disasm_view::get_mode()
{
    return mode;
}

void rv_disasm_view::set_show_offsets_(bool b)
{
    show_offsets = b;

    // columns
    if(show_offsets) {
        col_offset = 0;
        col_extras = col_offset + 9;
        col_va = col_extras + 14; // +13 for xrefs markers, other markers
        col_bytes = col_va + 9;
        col_mnemonic = col_bytes + 3 * (show_hex_bytes_cnt + 1) + 1;
        col_operands = col_mnemonic + 12;
        col_comments = col_operands + 20;
    } else {
        col_offset = 0;
        col_extras = 0;  // HEEEERE !!
        col_va = col_extras + 14; // +13 for xrefs markers, other markers
        col_bytes = col_va + 9;
        col_mnemonic = col_bytes + 3 * (show_hex_bytes_cnt + 1) + 1;
        col_operands = col_mnemonic + 12;
        col_comments = col_operands + 20;

    }

    emit repaintme();
}

// called from "outside" ide idisasm
void rv_disasm_view::set_show_offsets(bool b)
{
    if(options_widget) {
        options_widget->checkbox->setChecked(b);
        set_show_offsets_(b);
    }
}

void rv_disasm_view::set_show_bytes_cnt_(int i)
{
    show_hex_bytes_cnt = i;
    set_show_offsets_(show_offsets); // recalcs columns allready.
}

void rv_disasm_view::set_show_bytes_cnt(int i)
{
    options_widget->spinbox->setValue(i);
    set_show_bytes_cnt_(i);
}


void rv_disasm_view::create_options_widget()
{
    options_widget = new rv_disasm_view_options_widget(this);

    options_widget->checkbox->setChecked(show_offsets);
    connect(options_widget->checkbox, SIGNAL(toggled(bool)),
            this, SLOT(set_show_offsets_(bool)));

    options_widget->spinbox->setValue(show_hex_bytes_cnt);
    connect(options_widget->spinbox, SIGNAL(valueChanged(int)),
            this, SLOT(set_show_bytes_cnt_(int)));

    options_widget->setFixedSize(250, 50);

    options_widget->move(2, 2);
    options_widget->hide();
}

void rv_disasm_view::show_hide_options()
{
    if(options_widget->isHidden()) {
        options_widget->move(viewport()->width() - options_widget->width() -
                             pushbutton_show_hide_options->width() - 5,
                             0);
        options_widget->show();
        pushbutton_show_hide_options->setIcon(ico_hide_options);
    } else {
        options_widget->hide();
        pushbutton_show_hide_options->setIcon(ico_show_options);
    }
}

void rv_disasm_view::mouseDoubleClickEvent(QMouseEvent *e)
{
//    if((mouse_pos.x() >=
//            (RV_DV_LMARGIN + col_extras * char_width))
//            && (mouse_pos.x() <=
//            (RV_DV_LMARGIN + col_extras * char_width + (16*5)))
//            && tag_processor && tag_editor && mouse_desc) {
//        tag_editor_desc = mouse_desc;
//        tag_editor->set_va(mouse_desc->va);
//        int y = mouse_line.y + mouse_line.height - tag_editor->height();
//        tag_editor->move(RV_DV_LMARGIN + col_extras * char_width,
//                         y);
//        tag_editor->show();
//    }
    have_selection = false;
    emit do_repaint();
}

void rv_disasm_view::tag_editor_finished()
{
    tag_editor->hide();
    tag_editor_desc = 0;
}

void rv_disasm_view::set_have_idisasm(bool b)
{
    have_idisasm = b;
}

void rv_disasm_view::setup_menu_actions()
{
    menu_action_ctf_here_to_filter = new QAction("Ctf disasm from here into (current/new) filter view", this);
    menu_action_ctf_dest_to_filter = new QAction("Ctf disasm from destination into (current/new) filter view", this);

    menu_action_linear_here_to_filter = new QAction("Linear disasm from here into (current/new) filter view", this);
    menu_action_linear_dest_to_filter = new QAction("Linear disasm from destination into (current/new) filter view", this);

    connect(menu_action_ctf_here_to_filter, SIGNAL(triggered()),
            this, SLOT(ctf_disasm_here_filter()));
    connect(menu_action_ctf_dest_to_filter, SIGNAL(triggered()),
            this, SLOT(ctf_disasm_dest_filter()));

    connect(menu_action_linear_here_to_filter, SIGNAL(triggered()),
            this, SLOT(linear_disasm_here_filter()));
    connect(menu_action_linear_dest_to_filter, SIGNAL(triggered()),
            this, SLOT(linear_disasm_dest_filter()));

    // menu action strings that can change (ie display addresses later, ...)
    menu_action_strings[menu_action_ctf_here_to_filter] =
            "Ctf disassemble from here into (current/new) filter view";

    menu_action_strings[menu_action_ctf_dest_to_filter] =
            "Ctf disassemble from destination into (current/new) filter view";

    menu_action_strings[menu_action_linear_here_to_filter] =
            "Linear disassemble from here into (current/new) filter view";

    menu_action_strings[menu_action_linear_dest_to_filter] =
            "Linear disassemble from destination into (current/new) filter view";


    // -- PASTE --

    menu_action_paste = new QAction(QIcon(":/icons/edit-paste.png"),
                "Paste here", this);
    menu_action_ctf_paste = new QAction(QIcon(":/icons/edit-paste.png"),
                "Ctf paste here", this);

    connect(menu_action_paste, SIGNAL(triggered()),
            this, SLOT(menu_paste()));
    connect(menu_action_ctf_paste, SIGNAL(triggered()),
            this, SLOT(menu_paste_ctf()));

    menu_action_paste_to_new_filter = new QAction(QIcon(":/icons/edit-paste.png"),
                "Paste into new filter view", this);
    menu_action_ctf_paste_to_new_filter = new QAction(QIcon(":/icons/edit-paste.png"),
                "Ctf paste into new filter view", this);

    connect(menu_action_paste_to_new_filter, SIGNAL(triggered()),
            this, SIGNAL(paste_to_new_filter_requested()));
    connect(menu_action_ctf_paste_to_new_filter, SIGNAL(triggered()),
            this, SIGNAL(paste_ctf_to_new_filter_requested()));


    // -- COPY, ... SELECTION ---

    menu_action_copy = new QAction(QIcon(":/icons/edit-copy.png"), "Copy <CTRL>+<C>", this);
    connect(menu_action_copy, SIGNAL(triggered()),
            this, SLOT(menu_copy()));

    menu_action_select_all = new QAction("Select all <CTRL>+<A>", this);
    connect(menu_action_select_all, SIGNAL(triggered()),
            this, SLOT(menu_select_all()));

    menu_action_clear_selection = new QAction("Clear selection (unselect) <ESC>", this);
    connect(menu_action_clear_selection, SIGNAL(triggered()),
            this, SLOT(menu_clear_selection()));

    menu_action_delete = new QAction(QIcon(":/icons/package-purge.png"),
                 "Delete <DEL>", this);
    connect(menu_action_delete, SIGNAL(triggered()),
            this, SLOT(menu_delete()));

    // -- DUMP ... ---

    menu_action_dump_sel_to_html = new QAction("Dump selection to HTML", this);
    connect(menu_action_dump_sel_to_html, SIGNAL(triggered()),
            this, SLOT(menu_dump_sel_to_html()));

    menu_action_dump_view_to_html = new QAction("Dump view to HTML", this);
    connect(menu_action_dump_view_to_html, SIGNAL(triggered()),
            this, SLOT(menu_dump_view_to_html()));

    // -----------------

    menu_action_show_hide_offsets = new QAction("Show offsets", this);
    connect(menu_action_show_hide_offsets, SIGNAL(toggled(bool)),
            this, SLOT(set_show_offsets(bool)));

}

void rv_disasm_view::ctf_disasm_here_filter()
{
    emit do_ctf_disasm_filter_view(mouse_va_here);
}

void rv_disasm_view::ctf_disasm_dest_filter()
{
    emit do_ctf_disasm_filter_view(mouse_va_dest);
}

void rv_disasm_view::linear_disasm_here_filter()
{
    emit do_linear_disasm_filter_view(mouse_va_here);
}

void rv_disasm_view::linear_disasm_dest_filter()
{
    emit do_linear_disasm_filter_view(mouse_va_dest);
}

bool rv_disasm_view::get_selection()
{
    if(have_selection) {
        selection->start = selection_start;
        selection->end   = selection_end;
        selection->dv    = this;
        selection->active= true;
        return true;
    }

    return false;
}

void rv_disasm_view::menu_copy()
{
    if(selection) get_selection();
}

void rv_disasm_view::set_idisasm_selection(rv_idisasm_selection *sel)
{
    selection = sel;
}

void rv_disasm_view::menu_paste()
{
    if(!selection || !selection->active) return;
    x86_insn_t insn;

    pre_disassembly_change();

    re_addr_desc *current = selection->start;

    while(current != selection->end) {
        t->disassembler->disassemble_single_va_into_space(current->va, &insn);
        current = current->next;
    }
    if(current == selection->end)
        t->disassembler->disassemble_single_va_into_space(current->va, &insn);

    t->disassembler->re_process_address_space();
}

void rv_disasm_view::menu_paste_ctf()
{
    if(!selection || !selection->active) return;

    pre_disassembly_change();

    re_addr_desc *current = selection->start;

    while(current != selection->end) {
        disassemble_ctf_from_va(current->va);
        current = current->next;
    }
    if(current == selection->end)
        disassemble_ctf_from_va(current->va);

    t->disassembler->re_process_address_space();
}

void rv_disasm_view::disassemble_linear_from_va(re_addr_t va)
{
    x86_insn_t insn;

    pre_disassembly_change();

    re_addr_t current = va;

    while(t->disassembler->disassemble_single_va_into_space(current, &insn)) {
        current += insn.size;
    }

    t->disassembler->re_process_address_space();
}

void rv_disasm_view::menu_delete()
{
    have_selection = false;

    pre_disassembly_change();

    re_addr_desc *current = selection_start;
    re_addr_desc *next;

    if((selection_start == my_address_space->first())
            && (selection_end == my_address_space->last())) {
        my_address_space->clear();
        have_saved_vas = false;
        t->disassembler->re_process_address_space();
        return;
    }

    while(current != selection_end) {
        next = current->next;
        my_address_space->remove_va(current->va);
        current = next;
    }
    if(current == selection_end)
        my_address_space->remove_va(current->va);

    t->disassembler->re_process_address_space();
}

void rv_disasm_view::menu_select_all()
{
    if(!my_address_space->count()) return;
    selection_start = my_address_space->first();
    selection_end = my_address_space->last();
    have_selection = true;
    emit do_repaint();
}

void rv_disasm_view::menu_clear_selection()
{
    if(have_selection) {
        have_selection = false;
        emit do_repaint();
    }
}


void rv_disasm_view::menu_dump_sel_to_html()
{
    if(have_selection)
    dump_range_to_html(selection_start, selection_end);
}

void rv_disasm_view::menu_dump_view_to_html()
{
    dump_range_to_html(my_address_space->first(), my_address_space->last());
}

void rv_disasm_view::dump_range_to_html(re_addr_desc *start, re_addr_desc *end)
{
    QString html_out = "";
    bool    stop = false;
    int odd_even = 0;
    char buf[1024];

    re_addr_desc *current = start;

    html_out = html_header();

    QString line, span_str, style_addon_str, tr_str;
    x86_insn_t insn;

    while (!stop) {
        odd_even = 1 - odd_even;
        line = "";
        style_addon_str = "";
        tr_str = "";

        t->disassembler->disassemble_single(current->offset, &insn);

        if(current->desc == 'G') tr_str += "<tr class='hex_box'";
        else {
            if(odd_even) tr_str += "<tr class='asm_odd_row'";
            else tr_str += "<tr class='asm_even_row'";
        }


        // SYMBOLS, ... -----------------------------


        if(current->va == t->file->va_entry_point) {
            // ENTRY POINT
            line += "><td colspan=6><b>entry point:</b></td></tr>";
        } else if(t->file->va_imports_map.contains(current->va)) {
            // IMPORT
            line += "><td colspan=6><img src='import_small.png' /><span class='imports'>" +
                    QString::fromAscii(
                        t->file->va_imports_map.value(
                            current->va))
                    +":" +
                    "</span></td></tr>\n";

        } else if(t->file->va_functions_map.contains(current->va)) {
            // ELF functions
            line += " id='solid_line_top'>";
            line += "<td colspan=6><img src='function_small.png' /><span class='functions'>" +
                    QString::fromAscii(t->file->va_functions_map.value(
                                             current->va))
                                     +":"+
                    "</span></td></tr>\n";
        } else if(t->file->va_analyzed_functions_hash.contains(current->va)) {
            // analyzed_functions
            line += " id='solid_line_top'>";
            line += "<td colspan=6><img src='function_small.png' /><span class='functions'>" +
                    t->file->va_analyzed_functions_hash.value(
                            current->va)
                    +":" +
                    "</span></td></tr>\n";
        } else if(t->file->va_names_hash.contains(current->va)) {
            // NAMES
            line += "><td colspan=6><img src='names_small.png' /><span class='names'>" +
                    t->file->va_names_hash.value(
                            current->va)
                    +":" +
                    "</span></td></tr>\n";

        } else if(t->file->va_symbols_map.contains(current->va)) {
            line += "><td colspan=6><img src='symbol_small.png' /><span class='symbols'>" +
                    QString::fromAscii(
                        t->file->va_symbols_map.value(
                            current->va)) + ":" +
                    "</span></td></tr>\n";
        }

        // TR HANDLING
        // line pre processing

        if((insn.type == insn_jmp) ||
                (insn.type == insn_return) ||
                (insn.type == insn_halt) ) {
            if(current->next && (current->next->desc != 'G')
                    && (!t->file->va_functions_map.contains(current->next->va))
                    &&
                    (!t->file->va_analyzed_functions_hash.contains(current->next->va)
                     || t->file->va_imports_map.contains(current->next->va))

                    )
            style_addon_str = "id='dash_line_bottom'";
            if(current->desc == 'G') style_addon_str = "";
        }

        if(!line.isEmpty()) {
            if(current->desc == 'G') line = "<tr class='hex_box'" + line;
            else {
                if(odd_even) line = "<tr class='asm_odd_row'" + line;
                else line = "<tr class='asm_even_row'" + line;
            }
        }

        if(!style_addon_str.isEmpty())
            tr_str = tr_str +  " " + style_addon_str + ">\n";
        else tr_str+= ">";
        line = line + tr_str;

        // OFFSET ---

        line+="<td class='col_offset'>";
        if(show_offsets) line+=re_core::dword_to_pure_hex_string(current->offset);
        line+="</td>\n";

        // XREFS, ...
        line+="<td class='col_extras'>";
        if(t->disassembler->va_xrefs_mmap.contains(current->va)) {

            int my_cnt = t->disassembler->va_xrefs_mmap.values(current->va).count();
            if(my_cnt>8) my_cnt = 8;
            line += (QString) "<div " +
                    "id='xrefs_" + re_core::dword_to_pure_hex_string(current->va) + "' " +
                    "style=\"visibility: hidden; position: absolute;\">\n" +


                    "<select class='xrefs' " +
                    "id='s_xrefs_" + re_core::dword_to_pure_hex_string(current->va) + "' " +
                     + " multiple='multiple' size=" + QString::number(my_cnt) +
                    + " onClick=\"goto_xref('" +
                    "s_xrefs_" + re_core::dword_to_pure_hex_string(current->va)
                    + "');\""
                    " >\n";
            int opt_oe = 0;
            foreach(re_xref xr, t->disassembler->va_xrefs_mmap.values(current->va)) {
                opt_oe = 1 - opt_oe;
                line+= (QString)"<option ";
                        if(opt_oe) line += "class='odd'";
                        else line += "class='even'";
                        line += (QString) " value='" + re_core::dword_to_pure_hex_string(xr.va_src) + "'" +
                        ">" + xr.t + " " +
                        re_core::dword_to_pure_hex_string(xr.va_src)
                        + "</option>\n";
            }

            line+="</select></div>";

            line += (QString)"<img src='xrefs_small_new2.png' class='xrefs' " +
                    "onClick=\"show_hide_xrefs('xrefs_"
                    + re_core::dword_to_pure_hex_string(current->va)
                    + "');\""
                    + " />";
        }
        line+="</td>\n";

        // VA ----

        line+="<td class='col_va' id='" + re_core::dword_to_pure_hex_string(current->va) + "'><a name='";
        line+=re_core::dword_to_pure_hex_string(current->va);
        line+="'></a>";
        line+=re_core::dword_to_pure_hex_string(current->va);

        line+="</td>\n";

        // HEX BOXES ----------------------------------------------

        if(current->desc == 'G') {
            re_addr_t buf_len = t->disassembler->datablocks_hash.value(current->offset);
            int rows = buf_len / 16 + 1 + 1;
            if(rows > 16) rows = 16;
            line+="<td colspan=3 class='hex_box'>";
            line+="<img id='img_hexdump_" + re_core::dword_to_pure_hex_string(current->va)
                    + "' src=\"data_small_new.png\" onClick=\"show_hide_hex_box('hexdump_";
            line+=re_core::dword_to_pure_hex_string(current->va);
            line+="');\" />" + QString::number(buf_len) + " Bytes";
            line += "</td></tr>";

            line += "<tr class='hex_box'>\n";
            line += "<td colspan=6 class='center'>";
            line += "<textarea id='hexdump_" + re_core::dword_to_pure_hex_string(current->va) + "' ";
            line += "cols=78 rows=" + QString::number(rows);
            line += " style=\"display: none;\"";
            line += ">";
            line +=hex_dump(current->offset, buf_len);
            line +="</textarea></td>\n";
        }
        else if(insn.size) {
            // BYTES ---
            line+="<td class='col_bytes'>";
            if(show_hex_bytes_cnt) {
                int tmp_cnt = insn.size;
                QString s_hex_append;
                if(tmp_cnt > show_hex_bytes_cnt) {
                    tmp_cnt = show_hex_bytes_cnt;
                    s_hex_append = "...";
                }
                if(show_hex_bytes_cnt) {
                    for(int i=0; i < (tmp_cnt); i++) {
                        sprintf(buf, "%02X ", insn.bytes[i]);
                        line += QString::fromAscii(buf);
                    }
                    line += s_hex_append;
                }
            }
            line+="</td>\n";

            // MNEMONIC ----
            span_str = "<span>";
            if(insn.type == insn_call)
                span_str = "<span class='insn_call'>";

            else if((insn.type == insn_return) || (insn.type == insn_jmp))
                span_str = "<span class='insn_jmp_ret'>";

            else if((insn.type == insn_jcc) || (insn.type == insn_callcc))
                span_str = "<span class='insn_jcc'>";

            else if((insn.type == insn_test) || (insn.type == insn_cmp)
                    || (insn.type == insn_bittest))
                span_str = "<span class='insn_cmp'>";

            else if(insn.group == insn_interrupt)
                span_str = "<span class='insn_interrupt'>";

            else if(insn.group == insn_stack)
                span_str = "<span class='insn_stack'>";

            else if(insn.type == insn_nop)
                span_str = "<span class='insn_nop'>";

            t->disassembler->format_mnemonic(&insn, buf, 256);
            line+="<td class='col_insn'>" + span_str +
                    QString::fromAscii(buf) + "</span></td>\n";

            // OPERANDS -----
            re_addr_t op_addr;
            bool modified_operand;

            QString op_txt, op_txt2="";

            // ... dest ...
            modified_operand = false;
            op_txt.clear();

            if (t->disassembler->op1_lookup_addr_n_format(
                        &insn, op_addr, buf)  ) {
                if(op_addr) {
                    if(t->disassembler->follow_control_flow(&insn)) {
                        op_txt = "<a class='dest_va' href=\"#" +
                                re_core::dword_to_pure_hex_string(op_addr) +
                                "\"><img src=\"arrow_follow_ctf.png\"/>";
                    }

                    if(t->file->va_imports_map.contains(op_addr)) {
                        op_txt += t->file->va_imports_map.value(op_addr);
                        modified_operand = true;
                    }

                    else if(t->file->va_symbols_map.contains(op_addr)){
                        op_txt += t->file->va_symbols_map.value(op_addr);
                        modified_operand = true;
                    }
                    else if(t->file->va_functions_map.contains(op_addr)){
                        op_txt += t->file->va_functions_map.value(op_addr);

                        modified_operand = true;
                    }

                    else if(t->file->va_cstrings_hash.contains(op_addr)){
                        op_txt+= QString::fromAscii(
                                    t->file->va_cstrings_hash.value(op_addr));
                        if(op_txt.length() > 30) op_txt = op_txt.left(30) + " ...";

                        op_txt.prepend("<span class='strings'>\"");
                        op_txt.append("</span>");

                        modified_operand = true;
                    }

                    if(!modified_operand) {
                        op_txt += QString::fromAscii(buf);

                    }
                } else {
                    op_txt += QString::fromAscii(buf);

                }

                if(t->disassembler->follow_control_flow(&insn)) {
                    op_txt += "</a>";
                }

            }


            // ... src ...
            modified_operand = false;

            if (t->disassembler->op2_lookup_addr_n_format(
                        &insn, op_addr, buf)  ) {
                if(op_addr) {
                   if(t->file->va_imports_map.contains(op_addr)) {
                        op_txt2 += (QString)"<font style='font-weight: bold;'>" +
                                t->file->va_imports_map.value(op_addr) +
                                "</font>"
                                ;
                        modified_operand = true;
                    }

                    else if(t->file->va_symbols_map.contains(op_addr)){
                        op_txt2 += (QString)"<font style='font-weight: bold;'>" +
                                t->file->va_symbols_map.value(op_addr) +
                                "</font>"
                                ;
                        modified_operand = true;
                    }
                    else if(t->file->va_functions_map.contains(op_addr)){
                        op_txt2 += (QString)"<font style='font-weight: bold;'>" +
                                t->file->va_functions_map.value(op_addr) +
                                "</font>"
                                ;

                        modified_operand = true;
                    }

                    else if(t->file->va_cstrings_hash.contains(op_addr)){
                        op_txt2 += QString::fromAscii(
                                    t->file->va_cstrings_hash.value(op_addr));
                        if(op_txt2.length() > 30) op_txt2 = op_txt2.left(30) + " ...";

                        op_txt2.prepend("<span class='strings'>\"");
                        op_txt2.append("</span>");

                        modified_operand = true;
                    }

                    if(!modified_operand) {
                        op_txt2 += QString::fromAscii(buf);

                    }
                } else {
                    op_txt2 += QString::fromAscii(buf);

                }

                op_txt += op_txt2;
            }

            // ... imm ...
            if(t->disassembler->op3_format(&insn, buf)) {
                op_txt += QString::fromAscii(buf);
            }

            line+="<td class='col_operands'>";
            if(!op_txt.isEmpty()) {
                op_txt.prepend(span_str);
                line+=op_txt + "</span>";
            }
            line+="</td>\n";
        }


        line+="</tr>\n\n";

        html_out += line;

        if (current == end) stop=true;

        current = current->next;
    }

    html_out.append("</table></body></html>");

    QString fileName = QFileDialog::getSaveFileName(this,
                        "Save File",
                        "",
                        "HTML File (*.html)");

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
             return;

     QTextStream out(&file);
     out << html_out;
}


QString rv_disasm_view::hex_dump(re_addr_t offset, int len)
{
    unsigned int i, j;
    unsigned char c;

    QString S, S2, s_ret;

    unsigned char *d = (unsigned char *)t->file->image + offset;

    for(i = 0; i < ( (len / 16) + 1); i++) {
        S = "";
        S.append(QString::number(t->file->file_offset_to_va(offset) + i * 16, 16) + " : ");
        j = 0;
        while( (j < 16) && ( (i * 16 + j) < len) ) {
            c = *(d + i * 16 + j);
            if(c < 16) S.append("0");
            S.append(QString::number( c, 16) + " ");
            j++;
        }

        if(j!=16) {
            S2.fill(' ', 3 * (16 - j)); S.append(S2);
        }

        S.append(": ");

        translatePrintableChars(d+i*16, j, S);

        s_ret.append(S + "\n");
    }

    return s_ret;
}

void rv_disasm_view::translatePrintableChars(unsigned char *dataptr, int length, QString &S)
{
    unsigned char c, d;
    unsigned char *ptr = dataptr;

    for(int i=0; i < length; i++) {
        c=*ptr;
        if( (c >= 32) && (c <=126) ) d = c;
        else d = '.';
        S.append(d);
        ptr++;
    }
}


QString rv_disasm_view::html_header()
{
    int padding_right_offset;
    int padding_right_bytes;
    int padding_right_insn;

    padding_right_offset = 10;
    padding_right_bytes  = 10;
    padding_right_insn   = 40;

    if(!show_hex_bytes_cnt) {
        padding_right_bytes = 20;
    }



    QString html_out = "";
    html_out.append("<html><head>");
    html_out.append("<title>re|view dump - " + t->file->file_name + "</title>");

    html_out.append((QString)
                "<style type=\"text/css\">\n\n" +
                "table.asm_dump {               \n" +
                "    border: 			solid 1px #c0c0c0; \n" +
                "    font-family: 		\"Liberation Mono\", \"Courier\"; \n" +
                "    font-size:			9pt; \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_normal)
                        + " \n" +
                "} \n\n" +

                "/* --- ROWS --- */ \n\n" +

                "tr.asm_odd_row { \n" +
                "    background-color: " +
                        html_processor::qcolor_to_html_color(bgcolor1)
                        + "\n" +
                "} \n\n" +

                "tr.asm_even_row  { \n" +
                "    background-color:" +
                        html_processor::qcolor_to_html_color(bgcolor2)
                        + "\n} \n\n" +

                "tr#dash_line_bottom { \n" +
                "    background-image: url('uline_dashed.png');\n"+
                "    background-position: 0% 100%;\n"+
                "    background-repeat: repeat-x;\n"+
                "}\n"+

                "tr#solid_line_top  { \n" +
                    "    background-image: url('uline_full.png');\n"+
                    "    background-position: 0% 0%;\n"+
                    "    background-repeat: repeat-x;\n"+
                "} \n\n" +

                "/* --- HEX BOX --- */ \n\n" +

                "tr.hex_box  { \n" +
                "    background-color: 	" +
                        html_processor::qcolor_to_html_color(bgcolor_hex)
                        + " \n" +
                "}\n\n" +

                    "tr.hex_box  td { \n" +
                    "    padding: 0; \n" +
                    "}\n\n" +


                    "td.center { \n" +
                    "    text-align: center; \n" +
                    "}\n\n" +

                "textarea { \n" +
                "    font-family: 		\"Liberation Mono\", \"Courier\"; \n" +
                "    font-size:			8pt; \n" +
                "    margin:			2px auto; \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_normal)
                        + " \n" +
                "} \n\n" +


                "/* --- COLUMNS --- */ \n\n" +

                "td.col_offset { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_offset)
                        + " \n" +
                "    padding-right:		" +
                    QString::number(padding_right_offset) +
                    "px; \n" +
                "} \n\n" +

                    "td.col_extras { \n" +

                    "} \n\n" +

                "td.col_va { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_va)
                        + "  \n" +
                "    font-weight:		bold; \n" +
                "    padding-right:		10px; \n" +
                "} \n\n" +

                "td.col_bytes { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_bytes)
                        + "  \n" +
                "    padding-right:		" +
                    QString::number(padding_right_bytes) +
                    "px; \n" +
                "} \n\n" +

                "td.col_insn { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn)
                        + "  \n" +
                "    font-weight:		bold; \n" +
                "    padding-right:		" +
                    QString::number(padding_right_insn) +
                    "px; \n" +
                "} \n\n" +

                "td.col_operands { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn)
                        + "   \n" +
                "    padding-left:		16px; \n" +

                "} \n\n" +

                    "td.col_operands a { \n" +
                    "    margin-left:		-18px; \n" +
                    "    padding-right:		8px; \n" +
                    "} \n\n" +


                    "a { \n" +
                    "    color:				" +
                            html_processor::qcolor_to_html_color(dis_color_insn_jmp_ret)
                            + "   \n" +
                    "    text-decoration: none;"
                    "    padding:		  2px; \n" +

                    "} \n\n" +

                    "a:hover { \n" +
                    "    background-color: #ffe0e0;    \n" +
                    "} \n\n" +

                    "a.dest_va { \n" +
                    "    font-weight: bold;\n" +
                    "} \n\n" +

                "/* --- COLORS SYMBOLS, ... --- */ \n\n" +

                "span.imports { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_imports)
                        + "   \n" +
                "font-weight: bold; \n" +
                "} \n\n" +

                "span.symbols { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_symbols)
                        + "  \n"
                    "font-weight: bold; \n" +
                "} \n\n" +

                "span.names { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_names)
                        + "  \n" +
                    "font-weight: bold; \n" +
                "} \n\n" +

                "span.functions { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(txt_color_functions)
                        + "  \n" +
                    "font-weight: bold; \n" +
                "} \n\n" +

                "span.strings { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_string)
                        + "\n" +
                "    background-color: " +  html_processor::qcolor_to_html_color(dis_bgcolor_string) +
                    "\n" +
                "} \n\n" +

                "/* --- INSN --- */ \n\n" +

                "span.insn_call, span.insn_call a { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_call)
                        + "  \n" +
                "} \n\n" +

                "span.insn_jmp_ret, span.insn_jmp_ret a { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_jmp_ret)
                        + " \n" +
                "} \n\n" +

                "span.insn_jcc, span.insn_jcc a { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_jcc)
                        + " \n" +
                "} \n\n" +

                "span.insn_cmp { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_cmp)
                        + " \n" +
                "} \n\n" +

                "span.insn_interrupt { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_interrupt)
                        + " \n" +
                "} \n\n" +

                "span.insn_stack { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_stack)
                        + " \n" +
                "} \n\n" +

                "span.insn_nop { \n" +
                "    color:				" +
                        html_processor::qcolor_to_html_color(dis_color_insn_nop) +
                    "\n" +
                "    font-weight: normal; \n" +
                "} \n\n" +

                "span.make_bold { \n" +
                "    font-weight: bold; \n" +
                "} \n\n" +

                "span.hdr { \n" +
                "    font-family: \"Arial\", \"Verdana\";"
                "    font-size: 10pt; \n" +
                "    color: #404040; \n" +
                "} \n\n" +

                    "table.asm_dump img { \n" +
                    "    vertical-align: middle;\n" +
                    "    margin-right: 5px;\n" +
                    "} \n\n" +

                    "table.asm_dump a { \n" +
                    "    padding: 0 5px;\n" +
                    "} \n\n" +

                    "table.asm_dump a img { \n" +
                    "    margin-right: 0px;\n" +
                    "    border: 0;\n" +
                    "} \n\n" +

                    ".hex_box img { \n" +
                    "    padding-left: 2px;\n" +
                    "    padding-right: 2px;\n" +
                    "} \n\n" +

                    ".hex_box img:hover { \n" +
                    "    background-color: #c8d0d8;\n" +
                    "} \n\n" +



                    "img.xrefs { \n" +
                    "    padding: 2px;\n" +
                    "} \n\n" +


                    "img.xrefs:hover { \n" +
                    "    background-color: #c8d0d8;\n" +
                    "} \n\n" +

                    "select.xrefs { \n" +
                    "    font-size: 8pt;\n" +
                    "    color:     #303030;\n" +
                    "    font-family: 		\"Liberation Mono\", \"Courier\";\n" +
                    "} \n\n" +

                    "option.odd { \n" +
                    "    background-color: #E0E4E8;\n" +
                    "} \n\n" +

                    "option.even { \n" +
                    "    background-color: #F0F4F8;\n" +
                    "} \n\n" +



                    "</style> \n\n\n" +

                "<script type=\"text/javascript\"> \n\n" +

                    "function show_hide_hex_box(h_id) \n" +
                    "{ \n" +
                    "        if(document.getElementById(h_id).style.display == \"none\") { \n" +
                    "            document.getElementById(h_id).style.display = \"block\"; \n" +
                    "            document.getElementById('img_' + h_id).src = 'data_small_new2.png';        \n" +
                    "			} else { \n" +
                    "            document.getElementById(h_id).style.display = \"none\"; \n" +
                    "            document.getElementById('img_' + h_id).src = 'data_small_new.png';        \n" +
                    "			} \n" +
                    "} \n" +
                    "\n" +
                    "function show_hide_xrefs(xr_id)\n" +
                    "{\n" +
                    "        if(document.getElementById(xr_id).style.visibility == \"hidden\") { \n" +
                    "            document.getElementById(xr_id).style.visibility = \"visible\"; \n" +
                    "            document.getElementById(xr_id).style.marginTop = \"17px\"; \n" +
                    "        } \n" +
                    "            \n" +
                    "        else { \n" +
                    "            document.getElementById(xr_id).style.visibility = \"hidden\"; \n" +
                    "        }\n" +
                    "}\n" +
                    "\n" +
                    "function goto_xref(s_xr_id)\n" +
                    "{\n" +
                    "        document.location.href=\"#\" + document.getElementById(s_xr_id).value;        \n" +
                    "		document.getElementById(s_xr_id).options[document.getElementById(s_xr_id).selectedIndex].style.backgroundColor = \"#a0a8b0\";\n" +
                    "		\n" +
                    "		if(clicked_line)\n" +
                    "			clicked_line.style.backgroundColor = clicked_line_color;\n" +
                    "				\n" +
                    "		if(document.getElementById(document.getElementById(s_xr_id).value)) {\n" +
                    "			clicked_line = document.getElementById(document.getElementById(s_xr_id).value).parentElement;\n" +
                    "			clicked_line_color = clicked_line.style.backgroundColor;\n" +
                    "			clicked_line.style.backgroundColor = \"#f0f0cc\";\n" +
                    "		}\n" +
                    "}\n" +
                    "\n" +
                    "var clicked_line = 0;\n" +
                    "var clicked_line_color = \"\";\n" +
                    "\n" +
                    "function hoverize(evt) \n" +
                    "{\n" +
                    "        evt = (evt) ? evt : ((window.event) ? window.event : null);\n" +
                    "        if (evt) {\n" +
                    "            var elem = (evt.target) ? evt.target : \n" +
                    "                       ((evt.srcElement) ? evt.srcElement : null);\n" +
                    "            if (elem && elem.parentElement && (elem.parentElement.className == \"asm_even_row\")) {\n" +
                    "                switch (evt.type) {\n" +
                    "                    case \"mouseover\":\n" +
                    "					if(elem.parentElement != clicked_line) {\n" +
                    "						elem.parentElement.style.backgroundColor = \"#e0e0e0\";\n" +
                    "						}\n" +
                    "                        break;\n" +
                    "                    case \"mouseout\":\n" +
                    "						if(elem.parentElement != clicked_line) {\n" +
                    "							elem.parentElement.style.backgroundColor = \"#F0F4F8\";\n" +
                    "						}\n" +
                    "                        break;\n" +
                    "					case \"click\":\n" +
                    "						if(clicked_line != 0) clicked_line.style.backgroundColor = \"#F0F4F8\";\n" +
                    "						elem.parentElement.style.backgroundColor = \"#f0f0cc\";\n" +
                    "						clicked_line = elem.parentElement;\n" +
                    "						clicked_line_color = \"F0F4F8\";\n" +
                    "						break;\n" +
                    "					\n" +
                    "                }\n" +
                    "            }\n" +
                    "			\n" +
                    "		if (elem && elem.parentElement && (elem.parentElement.className == \"asm_odd_row\")) {\n" +
                    "                switch (evt.type) {\n" +
                    "                    case \"mouseover\":\n" +
                    "					if(elem.parentElement != clicked_line) {\n" +
                    "						elem.parentElement.style.backgroundColor = \"#e0e0e0\";\n" +
                    "						}\n" +
                    "                        break;\n" +
                    "                    case \"mouseout\":\n" +
                    "						if(elem.parentElement != clicked_line) {\n" +
                    "							elem.parentElement.style.backgroundColor = \"#ffffff\";\n" +
                    "						}\n" +
                    "                        break;\n" +
                    "					case \"click\":\n" +
                    "						if(clicked_line != 0) clicked_line.style.backgroundColor = \"#ffffff\";\n" +
                    "						elem.parentElement.style.backgroundColor = \"#f0f0cc\";\n" +
                    "						clicked_line = elem.parentElement;\n" +
                    "						clicked_line_color = \"#ffffff\";\n" +
                    "\n" +
                    "						break;\n" +
                    "					\n" +
                    "                }\n" +
                    "            }						\n" +
                    "        }\n" +
                    "		\n" +
                    "		if (elem && evt && (evt.type == \"click\")) {\n" +
                    "			if(elem.className == \"dest_va\") {\n" +
                    "				if(elem.href) e=elem;\n" +
                    "				else e = elem.parentElement;\n" +
                    "				h = e.hash;\n" +
                    "				h = h.substr(1,8);\n" +
                    "				\n" +
                    "				if(clicked_line)\n" +
                    "				clicked_line.style.backgroundColor = clicked_line_color;\n" +
                    "				\n" +
                    "				if(document.getElementById(h)) {\n" +
                    "					clicked_line = document.getElementById(h).parentElement;\n" +
                    "					clicked_line_color = clicked_line.style.backgroundColor;\n" +
                    "					clicked_line.style.backgroundColor = \"#f0f0cc\";\n" +
                    "				}\n" +
                    "			}\n" +
                    "		}\n" +
                    "    \n" +
                    "}\n" +
                    "\n" +
                    "function key_press(evt)\n" +
                    "{\n" +
                    "	evt = (evt) ? evt : ((window.event) ? window.event : null);\n" +
                    "    if (evt) {\n" +
                    "		switch (evt.keyCode) {\n" +
                    "		// UP\n" +
                    "		case 38:\n" +
                    "		if(!clicked_line) break;\n" +
                    "		clicked_line.style.backgroundColor = clicked_line_color;\n" +
                    "		r = clicked_line.rowIndex;\n" +
                    "		if(!r) break;\n" +
                    "		r = r - 1;\n" +
                    "		clicked_line = clicked_line.parentElement.rows[r];\n" +
                    "		\n" +
                    "		clicked_line_color = clicked_line.style.backgroundColor;\n" +
                    "		clicked_line.style.backgroundColor = \"#f0f0cc\";\n" +
                    "		if(clicked_line.offsetTop < document.body.scrollTop) return true;\n" +
                    "\n" +
                    "		return false;\n" +
                    "\n" +
                    "		break;    \n" +
                    "			\n" +
                    "		// DOWN		\n" +
                    "		case 40:\n" +
                    "		if(!clicked_line) break;\n" +
                    "		clicked_line.style.backgroundColor = clicked_line_color;\n" +
                    "		\n" +
                    "		r = clicked_line.rowIndex;\n" +
                    "		if(!r) break;\n" +
                    "		r = r + 1;\n" +
                    "		clicked_line = clicked_line.parentElement.rows[r];\n" +
                    "		\n" +
                    "		clicked_line_color = clicked_line.style.backgroundColor;\n" +
                    "		clicked_line.style.backgroundColor = \"#f0f0cc\";\n" +
                    "		if(clicked_line.offsetTop > (document.body.scrollTop + document.body.clientHeight - 100)) return true;\n" +
                    "		return false;\n" +
                    "		break;    \n" +
                    "        }\n" +
                    "	}\n" +
                    "}\n" +
                    "\n" +
                    "document.onmouseover=hoverize;\n" +
                    "document.onmouseout=hoverize;\n" +
                    "document.onclick=hoverize;\n" +
                    "document.onkeydown=key_press;\n" +




                "</script> \n\n" +

                "</head>\n\n" +

                "<body>\n\n" +

                    "<p></p><span class='hdr'><center>");

            if(mode== DV_MODE_TARGET) html_out += "<img src='disasm_view_t_32_16.png'/>";
            else html_out += "<img src='disasm_view_sub_filter_t3.png'/>";

            html_out +=
            "<b>Disassembly:</b> " + t->file->file_name +
                    + "</center></span><p></p>" +

                "<table class=\"asm_dump\" cellspacing=0 align=\"center\">\n";


    return html_out;
}


