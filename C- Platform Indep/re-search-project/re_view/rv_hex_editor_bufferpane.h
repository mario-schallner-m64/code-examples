#ifndef RV_HEX_EDITOR_BUFFERPANE_H
#define RV_HEX_EDITOR_BUFFERPANE_H

#include <QWidget>
#include <QTabWidget>

namespace Ui {
    class rv_hex_editor_bufferpane;
}

class rv_hex_editor_bufferpane : public QWidget
{
    Q_OBJECT

public:
    explicit rv_hex_editor_bufferpane(QWidget *parent = 0);
    ~rv_hex_editor_bufferpane();

    void    insert_buffer_tab_widget(QTabWidget *w);
    void    enable_buffers(bool checked);

signals:
    void paste_buffer_selection_to_main();
    void paste_whole_buffer_to_main();


private:
    Ui::rv_hex_editor_bufferpane *ui;
};

#endif // RV_HEX_EDITOR_BUFFERPANE_H
