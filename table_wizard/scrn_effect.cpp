#include "scrn_effect.h"

QList<scrn_effect *> scrn_effect::effects;

scrn_effect::scrn_effect(QObject *parent) :
    QObject(parent)
{
}

void scrn_effect::init_effects()
{
    scrn_effect *eff;
    scrn_effect_parameter ep;

    // --- inc Y ---------------------------------------------------------------
    eff = new scrn_effect();
    eff->name = "INC/DEC Y";
    eff->desc  = "Increase Y by amount P1\n\n";
    eff->desc += "Parameters:\n";
    eff->desc += "int P1: amount to increase\n";

    ep.name = "amount";
    ep.type = "integer";
    ep.desc = "Description: amount to increase Y\n";
    ep.desc += "Range: -255 ... +255";
    ep.i_value = 1;
    eff->parameters.append(ep);

    effects.append(eff);

    // --- SIN Y ---------------------------------------------------------------
    eff = new scrn_effect();
    eff->name = "SIN Y";
    eff->desc  = "sin move rb from current Y\n\n";
    eff->desc += "Parameters:\n";
    eff->desc += "int P1: amplitude\n";

    ep.name = "amplitude";
    ep.type = "integer";
    ep.desc = "Description: amplitude of sinus\n";
    ep.desc += "Range: 1 ... +255";
    ep.i_value = 32;
    eff->parameters.append(ep);

    effects.append(eff);

    // --- Set Z ---------------------------------------------------------------
    eff = new scrn_effect();
    eff->name = "SET Z";
    eff->desc  = "set z value for rb\n\n";
    eff->desc += "Parameters:\n";
    eff->desc += "int P1: Z-value\n";

    ep.name = "Z";
    ep.type = "integer";
    ep.desc = "Description: Z-Value\n";
    ep.desc += "Range: 0 ... +255";
    ep.i_value = 100;
    eff->parameters.append(ep);

    effects.append(eff);
}

void scrn_effect::execute(scrn_effect *e, rb *rbar, int scrn_frame_nr)
{
    unsigned char work_buf[256];

    if(scrn_frame_nr < e->start) return;
    if(scrn_frame_nr > e->end) return;
}
