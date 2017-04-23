#include "rv_script_highlighter.h"

rv_script_highlighter::rv_script_highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(QColor(0x40,0x80,0x40));
    rule.pattern = QRegExp("\\b(INSN_|OP_)[A-Z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(QColor(0x80, 0x80, 0x80));

    //functionFormat.setFontItalic(true);
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(QColor(0x30, 0x30, 0x80));
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+\\s*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(QColor(0x80, 0x20, 0x20));
    //rule.pattern = QRegExp("\".*\"(?!\")");
    rule.pattern = QRegExp("\".*\"");
    rule.pattern.setMinimal(true);

    rule.format = quotationFormat;
    highlightingRules.append(rule);



    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

    keywordFormat.setForeground(Qt::black);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\bconst\\b" << "\\bfunction\\b" << "\\bif\\s*(?=\\()\\b"
                    << "\\bfor\\s*(?=\\()\\b"
                    << "\\bwhile\\s*(?=\\()\\b" << "\\bArray\\s*(?=\\()\\b" ;
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(QColor(0x80, 0x80, 0x80));
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void rv_script_highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
