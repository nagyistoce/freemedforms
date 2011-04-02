/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "formeditordialog.h"
#include "episodemodel.h"
#include "constants_db.h"
#include "episodebase.h"

#include <utils/global.h>

#include <QSortFilterProxyModel>

#include "ui_formeditordialog.h"

using namespace Form;
static inline Form::Internal::EpisodeBase *episodeBase() { return Form::Internal::EpisodeBase::instance(); }

FormEditorDialog::FormEditorDialog(EpisodeModel *model, EditionModes mode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormEditorDialog)
{
    ui->setupUi(this);
//    ui->formSelector->setFormType(Form::FormFilesSelectorWidget::A);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(EpisodeModel::IsEpisode);
    proxyModel->setFilterFixedString("false");
    ui->treeView->setModel(proxyModel);
    ui->treeView->header()->hide();
    for(int i = 0; i< EpisodeModel::MaxData; ++i)
        ui->treeView->hideColumn(i);
    ui->treeView->showColumn(EpisodeModel::Label);
    ui->stackedWidget->setCurrentWidget(ui->formAdder);
}

FormEditorDialog::~FormEditorDialog()
{
    delete ui;
}

void FormEditorDialog::on_addForm_clicked()
{
    QString insertTo;
    if (!ui->treeView->selectionModel()->hasSelection()) {
        bool yes = Utils::yesNoMessageBox(tr("Insert as root form ?"), tr("You did not selected a form, do you want to add the sub-form as root form ?"));
        if (yes)
            insertTo = Constants::ROOT_FORM_TAG;
        else
            return;
    } else {
        QModelIndex idx = ui->treeView->selectionModel()->currentIndex();
        idx = proxyModel->mapToSource(idx);
        insertTo = proxyModel->sourceModel()->index(idx.row(), EpisodeModel::FormUuid, idx.parent()).data().toString();
    }

    // Save to database
    QList<Form::FormIODescription *> selected = ui->formSelector->selectedForms();
    if (selected.isEmpty() || insertTo.isEmpty())
        return;
    episodeBase()->addSubForms(insertTo, selected);

    // Re-create the patient form
    // --->>>>>
    //
}

void FormEditorDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
