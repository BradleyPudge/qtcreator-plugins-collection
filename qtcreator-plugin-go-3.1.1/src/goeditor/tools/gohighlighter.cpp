/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Sergey Shambir

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/

#include "gohighlighter.h"
#include "lexical/goscanner.h"

#include <texteditor/texteditorconstants.h>

namespace GoEditor {

using namespace GoEditor::Internal;

/// @return List that maps enum Format values to TextEditor plugin formats
QVector<TextEditor::TextStyle> initFormatCategories()
{
    QVector<TextEditor::TextStyle> categories(Format_FormatsAmount);
    categories[Format_Number] = TextEditor::C_NUMBER;
    categories[Format_Rune] = TextEditor::C_NUMBER;
    categories[Format_String] = TextEditor::C_STRING;
    categories[Format_Keyword] = TextEditor::C_KEYWORD;
    categories[Format_Comment] = TextEditor::C_COMMENT;
    categories[Format_Identifier] = TextEditor::C_TEXT;
    categories[Format_Whitespace] = TextEditor::C_VISUAL_WHITESPACE;
    categories[Format_PredeclaratedType] = TextEditor::C_TYPE;
    categories[Format_PredeclaratedConst] = TextEditor::C_ENUMERATION;
    categories[Format_PredeclaratedFunction] = TextEditor::C_FUNCTION;
    categories[Format_Operator] = TextEditor::C_OPERATOR;

    return categories;
}

GoHighlighter::GoHighlighter(QTextDocument *document) :
    TextEditor::SyntaxHighlighter(document)
{
    setTextFormatCategories(initFormatCategories());
}

GoHighlighter::~GoHighlighter()
{
}

void GoHighlighter::highlightBlock(const QString &text)
{
    int initialState = previousBlockState();
    if (initialState == -1)
        initialState = 0;
    setCurrentBlockState(highlightLine(text, initialState));
}

int GoHighlighter::highlightLine(const QString &text, int initialState)
{
    GoScanner scanner(text.constData(), text.size());
    scanner.setState(initialState);

    FormatToken tk;
    while ((tk = scanner.read()).format() != Format_EndOfBlock)
        setFormat(tk.begin(), tk.length(), formatForCategory(tk.format()));
    return scanner.state();
}

} // namespace GoEditor
