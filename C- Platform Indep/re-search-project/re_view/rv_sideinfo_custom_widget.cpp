#include "rv_sideinfo_custom_widget.h"

rv_sideinfo_custom_widget::rv_sideinfo_custom_widget(QWidget *parent) :
    QGroupBox(parent)
{   
    mode = RV_CW_NONE;
    last_hv = 0;
    setStyleSheet("color: rgb(64, 64, 64);");

    lil_up_down.load(":/icons/lil_up_down.png");
    draw_up_down = false;
    have_height = false;

    slider = new rv_slider(Qt::Vertical, parent);
    connect(slider, SIGNAL(mouse_release()),
            this, SLOT(slider_released()));

    slider->hide();
    slider->setFixedSize(20, 120);


    setMouseTracking(true);

    QFont myfont = font();

    myfont.setBold(true);
    myfont.setUnderline(true);
    setFont(myfont);
    myfont.setUnderline(false);

    text_box_output = new QPlainTextEdit(this);
    text_box_script = new rv_script_code_editor(this);

    QFont inner_font = QFont("Liberation Mono", 8, QFont::Normal);
    inner_font.setUnderline(false);

    text_box_output->setFont(inner_font);

    //setStyleSheet("color: rgb(112, 87, 130);");

    text_box_output->setPlainText("no output yet.");

    text_box_output->setHidden(true);

    QVBoxLayout *vl = new QVBoxLayout();

    QHBoxLayout *hl_label = new QHBoxLayout();
    label_offset_d.setText("Offset:");
    label_offset_d.setStyleSheet("Color: rgb(64, 64, 64);");
    label_offset_d.setFont(inner_font);
    label_offset_v.setFont(inner_font);
    label_offset_v.setStyleSheet("color: rgb(112, 87, 130);");

    hl_label->addWidget(&label_pic1);
    hl_label->addWidget(&label_pic2);
    hl_label->addWidget(&label_pic3);
    label_pic1.hide();
    label_pic2.hide();
    label_pic3.hide();

    hl_label->addWidget(&label_offset_d);
    hl_label->addWidget(&label_offset_v);

    vl->addLayout(hl_label);

    vl->addWidget(text_box_script);
    vl->addWidget(text_box_output);

    pushbutton_abort = new QPushButton(QIcon(":/icons/package-purge.png"), "", this);
    pushbutton_abort->setToolTip("Abort running script");
    pushbutton_abort->setFont(QFont("Liberation Mono", 8, QFont::Normal));

    pushbutton_run = new QPushButton(QIcon(":/icons/re_script_run3.png"), "", this);
    pushbutton_run->setToolTip("Execute script");
    connect(pushbutton_run, SIGNAL(clicked()),
            this, SLOT(try_execute_with_last_hv()));

    pushbutton_editor_enlarge = new QPushButton(QIcon(":/icons/editor_enlarge2.png"),
                                                      "", this);
    pushbutton_editor_enlarge->setToolTip("Enlarge editor (into new window)");

    myfont.setBold(false);
    checkbox_details = new QCheckBox("Edit", this);
    checkbox_details->setFont(myfont);
    checkbox_details->setStyleSheet("color: rgb(112, 87, 130);");

    checkbox_details->setToolTip("Toggle edit-/output-mode");
    checkbox_details->setCheckable(true);
    checkbox_details->setChecked(true);
    checkbox_details->setIcon(QIcon(":/icons/notes_1.png"));
    connect(checkbox_details, SIGNAL(toggled(bool)),
            this, SLOT(toggle_edit_mode(bool)));
    connect(pushbutton_abort, SIGNAL(clicked()),
            this, SLOT(abort_execution()));
    connect(pushbutton_editor_enlarge, SIGNAL(clicked()),
            this, SLOT(editor_enlarge()));

    QHBoxLayout *hl2 = new QHBoxLayout();
    hl2->addWidget(checkbox_details);
    hl2->addWidget(pushbutton_editor_enlarge);
    hl2->addStretch();
    hl2->addWidget(pushbutton_abort);
    hl2->addWidget(pushbutton_run);

    vl->addLayout(hl2);

    setLayout(vl);

    sp = new rv_script_processor(this);
    sp->set_output_textbox(text_box_output);
    setMinimumWidth(200);

    text_box_large = new rv_script_code_editor();
    text_box_large->hide();
    connect(text_box_large, SIGNAL(textChanged()), this,
            SLOT(editor_sync()));

    text_box_large->setPlainText("// enter your script here\n\n");

    setMinimumHeight(180);
    setMaximumHeight(1000);
}

rv_sideinfo_custom_widget::~rv_sideinfo_custom_widget()
{
      delete text_box_large;
}


void rv_sideinfo_custom_widget::cursor_offset_changed_to(rv_hex_edit *hv, re_addr_t offset)
{
    if(!(mode & RV_CW_CURSOR_CONNECTED)) return;
    label_offset_v.setText(re_core::dword_to_hex_string(offset));
    execute(hv);
}

void rv_sideinfo_custom_widget::selection_changed(rv_hex_edit *hv, re_addr_t from, re_addr_t to)
{
    if(!(mode & RV_CW_SELECTION_CONNECTED)) return;
    label_offset_v.setText(
                re_core::dword_to_hex_string(from) +
                "-" +
                re_core::dword_to_hex_string(to)
                );
    execute(hv);
}

void rv_sideinfo_custom_widget::execute(rv_hex_edit *hv)
{
    if(!sp || !hv) return;
    last_hv = hv;
    if(sp->engine->isEvaluating()) return;

    struct rv_hv_selection sel;
    bool sel_ok = hv->get_selection(sel);

    QScriptValue sel_obj = sp->engine->newObject();
    if(sel_ok) {
        sel_obj.setProperty("start", (quint32)sel.start);
        sel_obj.setProperty("end", (quint32)sel.end);
        sel_obj.setProperty("len", (quint32)(sel.end - sel.start + 1));
        sel_obj.setProperty("is_valid", true);
        QScriptValue d = sp->engine->newArray(sel.len);
        for(int i=0;i<sel.len;i++) {
            d.setProperty(i, (hv->get_data_ptr() + sel.start)[i]);
        }
        sel_obj.setProperty("data", d);
    } else {
        sel_obj.setProperty("start", 0);
        sel_obj.setProperty("end", 0);
        sel_obj.setProperty("len", 0);
        sel_obj.setProperty("is_valid", false);
        sel_obj.setProperty("data", 0);
    }

    sp->engine->globalObject().setProperty("selection", sel_obj);

    sp->engine->globalObject().setProperty("offset",
                                           (quint32)hv->get_cursor_offset());

    sp->engine->globalObject().setProperty("buffer_name",
                                           hv->buffer_name);

    execute();
}

void rv_sideinfo_custom_widget::execute()
{
    QScriptValue result =
            sp->engine->evaluate(text_box_script->toPlainText());

    if (result.isError()) {
        text_box_output->appendPlainText(
                    (QString)"rv|script error"
                    +
                    QString::fromLatin1("%0: %1")
                    .arg(result.property("lineNumber").toInt32())
                    .arg(result.toString()));
    }
}

void rv_sideinfo_custom_widget::try_execute_with_last_hv()
{
    if(last_hv) execute(last_hv);
    else execute();
}

QPlainTextEdit *rv_sideinfo_custom_widget::get_text_edit_outbox()
{
    return text_box_output;
}

void rv_sideinfo_custom_widget::toggle_edit_mode(bool show)
{
    text_box_script->setShown(show);
    text_box_output->setShown(!show);
    pushbutton_editor_enlarge->setShown(show);
}

void rv_sideinfo_custom_widget::abort_execution()
{
    if(!sp) return;
    sp->engine->abortEvaluation();
}

void rv_sideinfo_custom_widget::editor_enlarge()
{
    text_box_large->setPlainText(text_box_script->toPlainText());
    text_box_large->show();
}

void rv_sideinfo_custom_widget::editor_sync()
{
    text_box_script->setPlainText(
                text_box_large->toPlainText());
}

void rv_sideinfo_custom_widget::set_mode(int m)
{
    mode = m;
    if(mode & RV_CW_CURSOR_CONNECTED) {
        label_pic1.setPixmap(QPixmap(":/icons/connected_to_cursor2.png"));
        label_pic1.show();
        label_pic1.setMaximumWidth(16);
        label_offset_d.hide();
        pushbutton_run->hide();
    }
    if(mode & RV_CW_SELECTION_CONNECTED) {
        label_pic2.setPixmap(QPixmap(":/icons/connected_to_selection2.png"));
        label_pic2.show();
        label_pic2.setMaximumWidth(16);
        label_offset_d.setText("Sel.:");
        label_offset_d.hide();
        pushbutton_run->hide();
    }
}

void rv_sideinfo_custom_widget::paintEvent(QPaintEvent *event)
{
    QGroupBox::paintEvent(event);

    QPainter p(this);
    //p.fillRect(rect(), Qt::black);
    //p.fillRect(contentsRect(), Qt::darkGray);
    if(draw_up_down) {
        p.drawPixmap(rect().width() - lil_up_down.width(), 0,
                     lil_up_down);
    }
}

void rv_sideinfo_custom_widget::mousePressEvent(QMouseEvent *event)
{
    if(draw_up_down && slider->isHidden()) {
        slider->setMinimum(180);
        //slider->setMaximum(parentWidget()->height() - y());
        slider->setMaximum(parentWidget()->height() - 5);
        slider->move(x() + width() - slider->width(),
                     y() + lil_up_down.height() + 5
                     );
        my_height = height();
        //emit my_height_changed(this, my_height);
        slider->setValue(height());
        slider->show();
        connect(slider, SIGNAL(valueChanged(int)),
                this, SLOT(change_height(int)));
    }
}

void rv_sideinfo_custom_widget::mouseMoveEvent(QMouseEvent *event)
{
    p = event->pos();

    if( (p.x() >= (rect().width() - lil_up_down.width() - 10))
            && (p.y() < (lil_up_down.height() + 10)  )) {
        draw_up_down = true;
        return;
    }

    if( (p.y() < text_box_script->y())
            && (p.x() > (rect().width() / 2)) ) {
        draw_up_down = true;
        t1.singleShot(500, this, SLOT(unset_draw_up_down()));
        repaint();
    }
}

void rv_sideinfo_custom_widget::unset_draw_up_down()
{
    if( (p.x() < (rect().width() - lil_up_down.width() - 10))
            || (p.y() > (lil_up_down.height() + 10) )) {
        draw_up_down = false;
        slider->hide();
        repaint();
    }
}

void rv_sideinfo_custom_widget::slider_released()
{
    slider->hide();
    disconnect(slider, SIGNAL(valueChanged(int)),
            this, SLOT(change_height(int)));
    emit my_height_changed(this, my_height);
    draw_up_down = false;
    repaint();
}

void rv_sideinfo_custom_widget::change_height(int h)
{
    my_height = h;
    have_height = true;

    emit my_height_changed(this, my_height);
}

int rv_sideinfo_custom_widget::get_height()
{
    if(have_height)     return my_height;
    else return 0;
}
