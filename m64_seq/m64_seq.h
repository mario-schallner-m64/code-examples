#ifndef M64_SEQ_H
#define M64_SEQ_H

#include <QObject>
#include <m64_seq_step.h>
#include <QStringList>

class m64_seq : public QObject
{
    Q_OBJECT
public:
    explicit m64_seq(QObject *parent = 0);
    QList<m64_seq_step *> steps;

    QString sequence;
    int position = 0;
    int play_position = 0;
    int len = 0;
    int nr;

signals:

public slots:
    void next_step();
    bool parse(QString seq);
    bool parse_lengths(QString seq);
    void set_position(int p);
    void create_midi_data(int gt);

    QString serialize(); // overwrites QString sequence
};

#endif // M64_SEQ_H
