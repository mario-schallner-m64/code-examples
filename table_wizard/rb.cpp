#include "rb.h"

rb::rb(QObject *parent) :
    QObject(parent)
{
    size=0;
    y=0;
    z=10;

    for(int i=0; i<256; i++) values[i] = 0;

    size = 7;
    values[0] = 6;
    values[1] = 14;
    values[2] = 3;
    values[3] = 1;
    values[4] = 3;
    values[5] = 14;
    values[6] = 6;
}

bool rb::sort_rb_Z(rb *l, rb *r)
{
    return (l->z > r->z);
}


QString rb::dump(int mode, QString prefix, int max)
{
    QString tmpstr, tmpval;
    int maxctr=0;

    tmpstr = "; --- " + name + " ---\n";

    for(int i=0; i<size; i++) {

        if(!maxctr) tmpstr += prefix + " ";

        tmpval = QString::number(values[i],16);

        if(mode == 0)
           tmpval = (QString)"$" + tmpval;

        else if(mode == 1)
           tmpval = (QString)"0x" + tmpval;

        else if(mode == 2)
            tmpval = (QString)"$0" + tmpval;

        else if(mode == 3)
            tmpval = (QString)"0x0" + tmpval;

        else tmpval = QString::number(values[i],10);

        if(i!=(size-1)) {
            if(maxctr < (max-1)) {
                tmpstr.append(tmpval + ", ");
                maxctr++;
            }
            else {
                tmpstr.append(tmpval + "\n");
                maxctr = 0;
            };
        }
        else tmpstr.append(tmpval + "\n; --- end ---");

    }

    return tmpstr;
}



















