#ifndef SCRN_EFFECT_H
#define SCRN_EFFECT_H

#include <QObject>
#include <rb.h>

typedef struct scrn_effect_parameter_s {
    QString name;
    int i_value;
    float f_value;
    QString type;
    QString desc;
} scrn_effect_parameter;

class scrn_effect : public QObject
{
    Q_OBJECT
public:
    explicit scrn_effect(QObject *parent = 0);

    static void init_effects();
    static void execute(scrn_effect *e, rb *rbar, int scrn_frame_nr);


signals:

public slots:

public:
    QString name;
    QString desc;

    int start;
    int end;

    int step;
    int speed;

    QList<scrn_effect_parameter> parameters;

    static QList<scrn_effect *> effects;
};

#endif // SCRN_EFFECT_H
