/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                     **
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
/**
  \class Core::FileManager
  Manages recent file context.\n
  Settings management :
  - You can save/get from settings using getRecentFilesFromSettings() and saveRecentFiles()
  - You can set a specific settings key (just after instanciation) using setSettingsKey()
  - The Core::ICore::instance()->settings() is used when accessing to settings
  Recent managers are limited to 10 entries.
*/
#include "filemanager.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/constants_tokensandsettings.h>

#include <QDir>
#include <QDebug>

using namespace Core;

static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }


/*!
    Adds the \a fileName to the list of recent files.
*/
void FileManager::addToRecentFiles(const QString &fileName)
{
    if (fileName.isEmpty())
        return;
    QString prettyFileName(QDir::toNativeSeparators(fileName));
    m_recentFiles.removeAll(prettyFileName);
    if (m_recentFiles.count() > m_maxRecentFiles)
        m_recentFiles.removeLast();
    m_recentFiles.prepend(prettyFileName);
}

/*!
    Returns the list of recent files.
*/
QStringList FileManager::recentFiles() const
{
    return m_recentFiles;
}

void FileManager::saveRecentFiles() const
{
    Core::ISettings *s = settings();
    s->beginGroup(Constants::S_RECENTFILES_GROUP);
    if (m_Key.isEmpty())
        s->setValue(Constants::S_RECENTFILES_KEY, m_recentFiles);
    else
        s->setValue(m_Key, m_recentFiles);
    s->endGroup();
}

void FileManager::getRecentFilesFromSettings()
{
    Core::ISettings *s = settings();
    m_recentFiles.clear();
    s->beginGroup(Constants::S_RECENTFILES_GROUP);
    if (m_Key.isEmpty())
        m_recentFiles = s->value(Constants::S_RECENTFILES_KEY).toStringList();
    else
        m_recentFiles = s->value(m_Key).toStringList();
    s->endGroup();
}

/*!
  The current file is e.g. the file currently opened when an editor is active,
  or the selected file in case a Project Explorer is active ...
  \sa currentFile
  */
void FileManager::setCurrentFile(const QString &filePath)
{
    if (m_currentFile == filePath)
        return;
    m_currentFile = filePath;
//    emit currentFileChanged(m_currentFile);
}

/*!
  Returns the absolute path of the current file
  The current file is e.g. the file currently opened when an editor is active,
  or the selected file in case a Project Explorer is active ...
  \sa setCurrentFile
  */
QString FileManager::currentFile() const
{
    return m_currentFile;
}
