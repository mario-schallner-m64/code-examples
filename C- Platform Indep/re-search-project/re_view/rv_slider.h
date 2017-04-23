#ifndef RV_SLIDER_H
#define RV_SLIDER_H

#include <QSlider>
#include <QPainter>
#include <QPaintEvent>

class rv_slider : public QSlider
{
    Q_OBJECT
public:
    explicit rv_slider(Qt::Orientation orientation, QWidget * parent = 0 );

protected:
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void paintEvent(QPaintEvent *ev);

signals:
    void mouse_release();

public slots:

};


#endif // RV_SLIDER_H
