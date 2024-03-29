/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2014 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  The FreeAccount plugins are free, open source FreeMedForms' plugins.   *
 *  (C) 2010-2011 by Pierre-Marie Desombre, MD <pm.desombre@medsyn.fr>     *
 *  and Eric Maeker, MD <eric.maeker@gmail.com>                            *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *  Main Developers:  Pierre-Marie DESOMBRE <pm.desombre@medsyn.fr>,      *
 *                     Eric MAEKER, <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *      NAME <MAIL@ADDRESS.COM>                                            *
 ***************************************************************************/
#ifndef AVAILABLEMOVEMENTPAGE_H
#define AVAILABLEMOVEMENTPAGE_H

#include <coreplugin/ioptionspage.h>

#include <QPointer>
#include <QString>
#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QDir>
#include <QStringList>


#include "ui_availablemovementpage.h"

/**
 * \file availablemovementpage.h
 * \author Eric MAEKER <eric.maeker@gmail.com>
 * \version 0.8.0
 * \date 16 Aug 2012
*/

namespace Core {
class ISettings;
}

namespace AccountDB {
class AvailableMovementModel;
}

namespace Account {
namespace Internal {

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////  AccountUserWidget  //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AvailableMovementWidget : public QWidget, private Ui::AvailableMovementWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AvailableMovementWidget)

public:
    explicit AvailableMovementWidget(QWidget *parent = 0);
    ~AvailableMovementWidget();
    void setDataToUi();

    static void writeDefaultSettings( Core::ISettings *s );

public Q_SLOTS:
    void saveToSettings(Core::ISettings *s = 0);
    void on_movComboBox_currentIndexChanged(int index);
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_parentEdit_textChanged(const QString & text);

private:
    void changeEvent(QEvent *e);
    void showEvent(QShowEvent *event);
    void saveModel();
//    bool fillEmptyAvailableModel();
    void fillParentCombo();
    void fillHugeWidgets();
//    QStandardItemModel * availableMovementModelByLocale();
    void setCompletionList(const QString & text);

private:
    AccountDB::AvailableMovementModel *m_Model;
    QDataWidgetMapper *m_Mapper;
    QStringList m_completionList;
};

}  // End Internal


class AvailableMovementPage : public Core::IOptionsPage
{
public:
    AvailableMovementPage(QObject *parent = 0);
    ~AvailableMovementPage();

    QString id() const;
    QString displayName() const;
    QString category() const;
    QString title() const {return displayName();}
    int sortIndex() const;

    void resetToDefaults();
    void checkSettingsValidity();
    void apply();
    void finish();

    QString helpPage() {return QString();}

    static void writeDefaultSettings(Core::ISettings *s) {Internal::AvailableMovementWidget::writeDefaultSettings(s);}

    QWidget *createPage(QWidget *parent = 0);
private:
    QPointer<Internal::AvailableMovementWidget> m_Widget;
};

}  // End namespace Account

#endif // AVAILABLEMOVEMENTPAGE_H
