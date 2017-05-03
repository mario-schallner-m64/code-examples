#include "rbsa_effect.h"

QList<rbsa_effect *> rbsa_effect::effects;

rbsa_effect::rbsa_effect(QObject *parent) :
    QObject(parent)
{
}

void rbsa_effect::init_effects()
{
    rbsa_effect *eff;
    rbsa_effect_parameter ep;

    // --- dup n ---------------------------------------------------------------
    eff = new rbsa_effect();
    eff->name = "DUP n";
    eff->desc  = "Duplicate current frame n times after current frame\n";
    eff->desc += "Parameters: \n";
    eff->desc += "int n";

    ep.name = "n";
    ep.type = "integer";
    ep.desc = "repeat count";
    ep.i_value = 4;

    eff->parameters.append(ep);

    effects.append(eff);

    // --- fade in outside -----------------------------------------------------
    eff = new rbsa_effect();
    eff->name = "fade in outside";
    eff->desc  = "fade rb in by growing from the center, outer colors fist\n";
    eff->desc += "note that this only works good, when the rb has an odd number of colors\n";
    eff->desc += "Parameters: \n";
    eff->desc += "none\n";
    eff->desc += "(the effect will use rb.size / 2 frames)\n";

    effects.append(eff);

    // --- fade out outside ----------------------------------------------------
    eff = new rbsa_effect();
    eff->name = "fade out outside";
    eff->desc  = "fade rb out by shrinking towards the center, removind inner colors fist\n";
    eff->desc += "note that this only works good, when the rb has an odd number of colors\n";
    eff->desc += "Parameters: \n";
    eff->desc += "none\n";
    eff->desc += "(the effect will use rb.size / 2 frames)\n";

    effects.append(eff);
}

void rbsa_effect::execute(rbsa_effect *e)
{
}

