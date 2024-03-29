/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2014 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main developers: Christian A Reiter                                   *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef ADDREMOVECOMBOBOX_H
#define ADDREMOVECOMBOBOX_H

#include <listviewplugin/listview_exporter.h>
#include <listviewplugin/extendedview.h>

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
QT_END_NAMESPACE

/**
 * \file addremovecombobox.h
 * \author Christian A. Reiter
 * \version 0.8.0
 * \date 18 Oct 2012
*/

namespace Views {
class LISTVIEW_EXPORT AddRemoveComboBox : public QWidget
{
    Q_OBJECT

public:
    explicit AddRemoveComboBox(QWidget *parent = 0);
    explicit AddRemoveComboBox(const QString &labelText, QWidget *parent = 0);
    void initialize();
    ~AddRemoveComboBox();

    void setLabel(const QString &text);

    inline void setModel(QAbstractItemModel *model) { Q_ASSERT(mCombo); mCombo->setModel(model); }
    inline void setModelColumn(int column) { Q_ASSERT(mCombo); mCombo->setModelColumn(column); }

    inline int currentIndex() const { Q_ASSERT(mCombo); return  mCombo->currentIndex(); }
    inline void setCurrentIndex(int index) { Q_ASSERT(mCombo); mCombo->setCurrentIndex(index); }
    inline void setMinimumRowsAllowed(int rows) { minimumRowsAllowed = rows < 0? 0 : rows; }

    void changeEvent(QEvent *e);

Q_SIGNALS:
    void aboutToAddItem();                              //!< emitted before Item is added
    void itemAdded(const QModelIndex &index);           //!< item was successfully added
    void aboutToRemoveItem(const QModelIndex &index);   //!< emitted before Item removal
    void itemRemoved();                                 //!< the Item was removed
    void currentIndexChanged(int index);                //!< current index was changed (int version)
    void currentIndexChanged(const QModelIndex &index); //!< current index was changed (QModelIndex version)

public Q_SLOTS:
    void setEditText(const QString &text);

    void addItem();
    void removeItem();

private Q_SLOTS:
    void updateUi();
    void translateIntIndexChanged(int index);

private:
    QModelIndex intIndexToQModelIndex(int intIndex) const;
    QLabel *mLabel;
    QComboBox *mCombo;
    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    int minimumRowsAllowed;
};

} // end namespace Views
#endif // ADDREMOVECOMBOBOX_H
