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

#pragma once
#include <texteditor/semantichighlighter.h>
#include <utils/runextensions.h>
#include <QFutureInterface>
#include <QSharedPointer>

namespace GoEditor {

class GoSemanticInfo;
class GoEditorDocument;

namespace Internal {

class SingleShotHighlightTask :
        public QRunnable
{
public:
    struct Result
    {
        QFuture<TextEditor::HighlightingResult> highlightFuture;
        QFuture<QSharedPointer<GoSemanticInfo>> semanticFuture;
    };

    SingleShotHighlightTask();
    ~SingleShotHighlightTask();
    void setFilename(const QString &filename);
    void setText(const QByteArray &text);

    Result start();
    void run() override;

private:
    QFutureInterface<TextEditor::HighlightingResult> m_highlightFuture;
    QFutureInterface<QSharedPointer<GoSemanticInfo>> m_semanticFuture;
    QString m_filename;
    QByteArray m_text;
};

} // namespace Internal
} // namespace GoEditor
