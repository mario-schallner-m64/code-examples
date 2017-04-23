#include "rv_sideinfo_listbox.h"

rv_sideinfo_listbox::rv_sideinfo_listbox(QWidget *parent) :
    QListWidget(parent)
{
    setToolTip("Drag and drop to reorder the widgets on screen");
}

void rv_sideinfo_listbox::dropEvent(QDropEvent *event)
{
    QListWidget::dropEvent(event);
    emit drop_happened();
}
