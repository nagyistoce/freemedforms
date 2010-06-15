/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                      *
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
#ifndef THEMEPRIVATE_H
#define THEMEPRIVATE_H

#include <coreplugin/itheme.h>

#include <coreplugin/core_exporter.h>

#include <QObject>
#include <QIcon>
#include <QPointer>
#include <QCache>

/**
 * \file theme.h
 * \author Eric MAEKER <eric.maeker@free.fr>
 * \version 0.0.10
 * \date 09 Aug 2009
*/

namespace Core {
namespace Internal {

class CORE_EXPORT ThemePrivate : public Core::ITheme
{
    Q_OBJECT
public:
    ThemePrivate( QObject *parent = 0, const int cacheSize = 100 );
    ~ThemePrivate();

    void setThemeRootPath( const QString & absPath );
    void setThemeRelativeRootPath( const QString & relativePathFromAppBinary );

    void refreshCache();
    void setCacheMaxCost( const int max );

    QIcon icon( const QString & fileName, IconSize size = SmallIcon );
    QString iconFullPath( const QString &fileName, IconSize size = SmallIcon );
    QPixmap splashScreen( const QString &fileName );

protected:
    void setSmallIconPath( const QString &absPath );
    void setMediumIconPath( const QString &absPath );
    void setBigIconPath( const QString &absPath );

private:
    QCache<QString, QIcon> m_IconCache;
    QString m_AbsolutePath;
    QString m_SmallIconPath, m_MediumIconPath, m_BigIconPath;
};

} // end Internal
} // end Core

#endif // THEMEPRIVATE_H
