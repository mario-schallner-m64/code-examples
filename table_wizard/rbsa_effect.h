#ifndef RBSA_EFFECT_H
#define RBSA_EFFECT_H

#include <QObject>

typedef struct rbsa_effect_parameter_s {
    QString name;
    int i_value;
    float f_value;
    QString type;
    QString desc;
} rbsa_effect_parameter;

class rbsa_effect : public QObject
{
    Q_OBJECT
public:
    explicit rbsa_effect(QObject *parent = 0);

    static void init_effects();
    static void execute(rbsa_effect *e);

signals:

public slots:

public:
    QString name;
    QString desc;
    QList<rbsa_effect_parameter> parameters;

    static QList<rbsa_effect *> effects;
};

#endif // RBSA_EFFECT_H
