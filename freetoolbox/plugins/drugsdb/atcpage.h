/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main Developper : Eric MAEKER, MD <eric.maeker@gmail.com>             *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef ATCPAGE_H
#define ATCPAGE_H

#include <coreplugin/itoolpage.h>
#include <coreplugin/ftb_constants.h>

#include <QWidget>


namespace DrugsDbCreator {

class AtcPage : public Core::IToolPage
{
    Q_OBJECT
public:
    explicit AtcPage(QObject *parent = 0);

    virtual QString id() const {return "AtcPage";}
    virtual QString name() const {return "ATC";}
    virtual QString category() const {return Core::Constants::CATEGORY_ATC_HELPERS;}
    virtual QIcon icon() const {return QIcon();}

    // widget will be deleted after the show
    virtual QWidget *createPage(QWidget *parent = 0);
};

namespace Ui {
    class AtcPage;
}

class AtcWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AtcWidget(QWidget *parent = 0);
    ~AtcWidget();

private:
    Ui::AtcPage *ui;
};


}  //  End namespace DrugsDbCreator

#endif // ATCPAGE_H
