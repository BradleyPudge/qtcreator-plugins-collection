/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "diagnostic.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

using namespace ClangCodeModel;

Diagnostic::Diagnostic()
    : m_severity(Unknown)
    , m_length(0)
{}

Diagnostic::Diagnostic(Severity severity, const SourceLocation &location, unsigned length, const QString &spelling)
    : m_severity(severity)
    , m_loc(location)
    , m_length(length)
    , m_spelling(spelling)
{}

const QString Diagnostic::severityAsString() const
{
    if (m_severity == Unknown)
        return QString();

    static QStringList strs = QStringList()
            << QCoreApplication::translate("Diagnostic", "ignored")
            << QCoreApplication::translate("Diagnostic", "note")
            << QCoreApplication::translate("Diagnostic", "warning")
            << QCoreApplication::translate("Diagnostic", "error")
            << QCoreApplication::translate("Diagnostic", "fatal")
               ;

    return strs.at(m_severity);
}
