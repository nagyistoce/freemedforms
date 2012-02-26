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
 *   Main Developper : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef DRUGINFOS_PREGNANCYCLASSIFICATION_H
#define DRUGINFOS_PREGNANCYCLASSIFICATION_H

#include <coreplugin/itoolpage.h>

namespace DrugInfos {

class PregnancyClassificationPage : public Core::IToolPage
{
public:
    PregnancyClassificationPage(QObject *parent) : IToolPage(parent) {}

    virtual QString id() const;
    virtual QString name() const;
    virtual QString category() const;
    virtual QIcon icon() const;

    // widget will be deleted after the show
    virtual QWidget *createPage(QWidget *parent = 0);
};

namespace Ui {
class PregnancyClassificationWidget;
}

class PregnancyClassificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PregnancyClassificationWidget(QWidget *parent = 0);
    ~PregnancyClassificationWidget();

protected Q_SLOTS:
    void computeJavascriptFile();

    void on_download_clicked();
    void on_editClassification_clicked();

//    void indexPageDownloaded(QNetworkReply *reply);
//    void substancePageDownloaded(QNetworkReply *reply);
//    void on_process_clicked();

//protected:
//    void changeEvent(QEvent *e);

//Q_SIGNALS:
//    void downloadFinished();
private:
    Ui::PregnancyClassificationWidget *ui;
};

} // namespace DrugInfos

#endif // DRUGINFOS_PREGNANCYCLASSIFICATION_H
