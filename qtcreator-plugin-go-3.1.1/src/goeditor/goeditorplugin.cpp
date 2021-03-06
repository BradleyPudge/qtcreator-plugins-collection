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

#include "goeditorplugin.h"
#include "goeditorconstants.h"
#include "goeditorfactory.h"
#include "goeditorwidget.h"
#include "tools/gocompletionassist.h"
#include "tools/gohoverhandler.h"
#include "tools/gooutlinewidget.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/messagemanager.h>
#include <texteditor/texteditorsettings.h>

#include <QAction>
#include <QMenu>

#include <QtPlugin>

using namespace GoEditor::Internal;
using namespace GoEditor::Constants;

static const QLatin1String ERROR_TOOL_NOT_INSTALLED("GoEditor: Program '%1' is not installed. Please install it to enable all editor features.");

/*******************************************************************************
 * List of Go keywords
 ******************************************************************************/
static const char *const LIST_OF_GO_KEYWORDS[] = {
    "break",
    "case",
    "chan",
    "const",
    "continue",
    "default",
    "defer",
    "else",
    "fallthrough",
    "for",
    "func",
    "go",
    "goto",
    "if",
    "import",
    "interface",
    "map",
    "package",
    "range",
    "return",
    "select",
    "struct",
    "switch",
    "type",
    "var"
};


/*******************************************************************************
 * List of Go predeclarated entities.
 * See http://golang.org/ref/spec#Iota
 ******************************************************************************/

static const char *const LIST_OF_GO_TYPES[] = {
    "bool",
    "byte",
    "complex64",
    "complex128",
    "error",
    "float32",
    "float64",
    "int",
    "int8",
    "int16",
    "int32",
    "int64",
    "rune",
    "string",
    "uint",
    "uint8",
    "uint16",
    "uint32",
    "uint64",
    "uintptr"
};

static const char *const LIST_OF_GO_CONSTANTS[] = {
    "true",
    "false",
    "iota",
    "nil"
};

static const char *const LIST_OF_GO_FUNCS[] = {
    "append",
    "cap",
    "close",
    "complex",
    "copy",
    "delete",
    "imag",
    "len",
    "make",
    "new",
    "panic",
    "print",
    "println",
    "real",
    "recover"
};

/// Copies identifiers from array to QSet
static void copyIdentifiers(const char * const words[], size_t bytesCount, QSet<QString> &result)
{
    const size_t count = bytesCount / sizeof(const char * const);
    for (size_t i = 0; i < count; ++i)
        result.insert(QLatin1String(words[i]));
}

GoEditorPlugin *GoEditorPlugin::m_instance = NULL;

GoEditorPlugin::GoEditorPlugin()
{
    m_instance = this;
    copyIdentifiers(LIST_OF_GO_KEYWORDS, sizeof(LIST_OF_GO_KEYWORDS), m_goKeywords);
    copyIdentifiers(LIST_OF_GO_TYPES, sizeof(LIST_OF_GO_TYPES), m_goPredeclaratedTypes);
    copyIdentifiers(LIST_OF_GO_CONSTANTS, sizeof(LIST_OF_GO_CONSTANTS), m_goPredeclaratedConsts);
    copyIdentifiers(LIST_OF_GO_FUNCS, sizeof(LIST_OF_GO_FUNCS), m_goPredeclaratedFuncs);
    connect(this, SIGNAL(reportedError(QString)), this, SLOT(reportErrorOnce(QString)));
}

GoEditorPlugin::~GoEditorPlugin()
{
    removeObject(m_factory);
    m_instance = 0;
}

bool GoEditorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    if (!Core::MimeDatabase::addMimeTypes(QLatin1String(RC_GO_MIMETYPE_XML), errorString))
        return false;

    m_factory = new GoEditorFactory(this);
    addObject(m_factory);

    addAutoReleasedObject(new GoOutlineWidgetFactory);
    addAutoReleasedObject(new GoCompletionAssistProvider);
    addAutoReleasedObject(new GoHoverHandler(this));

    // Add MIME overlay icons (these icons displayed at Project dock panel)
    const QIcon icon = QIcon::fromTheme(QLatin1String(C_GO_MIME_ICON));
    if (!icon.isNull())
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, C_GO_MIMETYPE);

    return true;
}

void GoEditorPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag GoEditorPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}

QSet<QString> GoEditorPlugin::goKeywords()
{
    return m_instance->m_goKeywords;
}

QSet<QString> GoEditorPlugin::goPredeclaratedTypes()
{
    return m_instance->m_goPredeclaratedTypes;
}

QSet<QString> GoEditorPlugin::goPredeclaratedConsts()
{
    return m_instance->m_goPredeclaratedConsts;
}

QSet<QString> GoEditorPlugin::goPredeclaratedFuncs()
{
    return m_instance->m_goPredeclaratedFuncs;
}

void GoEditorPlugin::reportToolNotInstalled(const QString &toolCommand)
{
    QString errorMessage(ERROR_TOOL_NOT_INSTALLED);
    emit m_instance->reportedError(errorMessage.arg(toolCommand));
}

void GoEditorPlugin::reportErrorOnce(const QString &errorMessage)
{
    if (!m_reportedErrors.contains(errorMessage)) {
        m_reportedErrors.insert(errorMessage);
        Core::MessageManager::write(errorMessage);
    }
}

Q_EXPORT_PLUGIN2(GoEditor, GoEditorPlugin)

