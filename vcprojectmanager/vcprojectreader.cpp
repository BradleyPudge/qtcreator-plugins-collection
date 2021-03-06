#include "vcprojectreader.h"

#include <coreplugin/icore.h>
#include <projectexplorer/projectnodes.h>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QDir>
#include <QDebug>

using namespace ProjectExplorer;

namespace VcProjectManager {
namespace Internal {

#define STRING(s) static const char str##s[] = #s

STRING(Configuration);
STRING(Configurations);
STRING(File);
STRING(Files);
STRING(Filter);
STRING(FileConfiguration);
STRING(Globals);
STRING(Name);
STRING(Platform);
STRING(Platforms);
STRING(PublishingData);
STRING(References);
STRING(RelativePath);
STRING(Tool);
STRING(ToolFiles);
STRING(VisualStudioProject);
STRING(Version);

using namespace VcProjectInfo;

VcProjectInfo::Filter::~Filter()
{
    qDeleteAll(filters);
    qDeleteAll(files);
}

Project::Project()
    : files(0)
{
}

Project::~Project()
{
    delete files;
}

VcProjectReader::VcProjectReader()
    : m_project(0)
    , m_currentFilter(0)
{
}

Project *VcProjectReader::parse(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly))
        return 0;

    setDevice(&file);

    m_project = new Project;
    m_project->projectFilePath = QFileInfo(filePath).canonicalPath();

    if (!atEnd()) {
        // read xml declaration
        readNextNonSpace();
        if (isStartDocument())
            readNextNonSpace();
        else
            customError(QObject::tr("XML Declaration missing"));

        if (elementStarts(strVisualStudioProject)) {
            readVisualStudioProject();
            readNextNonSpace();
        }
        else
            missingMandatoryTag(strVisualStudioProject);

        if (!isEndDocument())
            unexpectedElement();
    } else {
        customError(QObject::tr("Project file is empty"));
    }

    if (hasError()) {
        // TODO: error
        qDebug() << errorString() << lineNumber() << columnNumber();
        return 0;
    }

    qDebug() << "parsing successful";
    return m_project;
}

void VcProjectReader::readVisualStudioProject()
{
    m_project->displayName = attrStr(strName);

    // Platforms is mandatory element
    readNextNonSpace();
    if (elementStarts(strPlatforms)) {
        readPlatforms();
        readNextNonSpace();
    } else {
        missingMandatoryTag(strPlatforms);
    }

    // ToolFiles is optional element
    if (elementStarts(strToolFiles)) {
        readToolFiles();
        readNextNonSpace();
    }

    // PublishingData is optional element
    if (elementStarts(strPublishingData)) {
        readPublishingData();
        readNextNonSpace();
    }

    // Configurations is mandatory element
    if (elementStarts(strConfigurations)) {
        readConfigurations();
        readNextNonSpace();
    } else {
        missingMandatoryTag(strConfigurations);
    }

    // References is optional element
    if (elementStarts(strReferences)) {
        readReferences();
        readNextNonSpace();
    }

    // Files is optional element
    if (elementStarts(strFiles)) {
        readFiles();
        readNextNonSpace();
    }

    // Globals is optional element
    if (elementStarts(strGlobals)) {
        readGlobals();
        readNextNonSpace();
    }

    if (!elementEnds(strVisualStudioProject))
        unexpectedElement();
}

void VcProjectReader::readPlatforms()
{
    while (!elementEnds(strPlatforms) && !hasError()) {
        readNextNonSpace();
    }
}

void VcProjectReader::readToolFiles()
{
    while (!elementEnds(strToolFiles) && !hasError()) {
        readNextNonSpace();
    }
}

void VcProjectReader::readPublishingData()
{
    while (!elementEnds(strPublishingData) && !hasError()) {
        readNextNonSpace();
    }
}

void VcProjectReader::readConfigurations()
{
    while (!elementEnds(strConfigurations) && !hasError()) {
        readNextNonSpace();
    }
}

void VcProjectReader::readReferences()
{
    while (!elementEnds(strReferences) && !hasError()) {
        readNextNonSpace();
    }
}

void VcProjectReader::readFiles()
{
    m_project->files = new Filter;
    m_currentFilter = m_project->files;
    readNextNonSpace();
    while (!elementEnds(strFiles) && !hasError()) {
        if (elementStarts(strFile)) {
            readFile();
            readNextNonSpace();
        } else if (elementStarts(strFilter)) {
            readFilter();
            readNextNonSpace();
        } else {
            unexpectedElement();
        }
    }
}

void VcProjectReader::readFile()
{
    File *file = new File;
    file->relativePath = attrStr(strRelativePath);
#if !defined(Q_OS_WIN)
    file->relativePath.replace(QLatin1Char('\\'), QDir::separator());
#endif
    m_currentFilter->files.append(file);

    readNextNonSpace();
    while (!elementEnds(strFile) && !hasError()) {
        if (elementStarts(strFileConfiguration)) {
            readFileConfiguration();
            readNextNonSpace();
        } else {
            unexpectedElement();
        }
    }
}

void VcProjectReader::readFileConfiguration()
{
    while (!elementEnds(strFileConfiguration) && !hasError()) {
        readNextNonSpace();
    }
}

void VcProjectReader::readFilter()
{
    Filter *filter = new Filter;
    filter->name = attrStr(strName);
    m_currentFilter->filters.append(filter);

    Filter *prevFilter = m_currentFilter;
    m_currentFilter = filter;

    readNextNonSpace();
    while (!elementEnds(strFilter) && !hasError()) {
        if (elementStarts(strFile)) {
            readFile();
            readNextNonSpace();
        } else if (elementStarts(strFilter)) {
            readFilter();
            readNextNonSpace();
        } else {
            unexpectedElement();
        }
    }

    m_currentFilter = prevFilter;
}

void VcProjectReader::readGlobals()
{
    while (!elementEnds(strGlobals) && !hasError()) {
        readNextNonSpace();
    }
}

QString VcProjectReader::attrStr(const char *attrName)
{
    // TODO: errors?
    return attributes().value(QLatin1String(attrName)).toString();
}

bool VcProjectReader::elementStarts(const char *str) const
{
    return isStartElement() && name() == QLatin1String(str);
}

bool VcProjectReader::elementEnds(const char *str) const
{
    return isEndElement() && name() == QLatin1String(str);
}

void VcProjectReader::readNextNonSpace()
{
    if (!hasError()) {
        do {
            readNext();
        } while (isCharacters() && text().toString().trimmed().isEmpty());
    }
    m_currentElement = name().toString();
//    qDebug() << "current element:" << (isEndElement() ? "\\": "") << m_currentElement;
}

void VcProjectReader::customError(const QString &message)
{
    if (!hasError())
        raiseError(message);
}

void VcProjectReader::missingMandatoryTag(const char *tagName)
{
    customError(QLatin1String("<") + QLatin1String(tagName) + QLatin1String("> expected"));
}

void VcProjectReader::unexpectedElement()
{
    customError(QLatin1String("Unexpected ") + name());
}

} // namespace Internal
} // namespace VcProjectManager
