#ifndef RV_SCRIPT_HIGHLIGHTER_H
#define RV_SCRIPT_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class rv_script_highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    rv_script_highlighter(QTextDocument *parent = 0);


protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // RV_SCRIPT_HIGHLIGHTER_H
