/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
#include "uitools.h"

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QListView>

#include <QDebug>

using namespace Script;

UiTools::UiTools(QObject *parent) :
    QObject(parent)
{
}

bool UiTools::addItem(QWidget *widget, const QString &item)
{
    // QListWidget
    QListWidget *listWidget = qobject_cast<QListWidget*>(widget);
    if (listWidget) {
        listWidget->addItem(item);
        return true;
    }
    // QComboBox
    QComboBox *combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        combo->addItem(item);
        return true;
    }
    return false;
}

bool UiTools::addItems(QWidget *widget, const QStringList &items)
{
    // QListWidget
    QListWidget *listWidget = qobject_cast<QListWidget*>(widget);
    if (listWidget) {
        listWidget->addItems(items);
        return true;
    }
    // QComboBox
    QComboBox *combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        combo->addItems(items);
        return true;
    }
    return false;
}

bool UiTools::addJoinedItem(QWidget *widget, const QString &item, const QString &separator)
{
    QStringList items = item.split(separator);
    return addItems(widget, items);
}

bool UiTools::clear(QWidget *widget)
{
    // QListWidget
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(widget);
    if (view) {
        if (view->model()) {
            view->model()->removeRows(0, view->model()->rowCount());
        }
        return true;
    }
    // QComboBox
    QComboBox *combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        combo->clear();
        return true;
    }
    return false;
}

QStringList UiTools::selectedItems(QWidget *widget)
{
    QStringList toReturn;
//    // QListWidget
//    QListWidget *listWidget = qobject_cast<QListWidget*>(widget);
//    if (listWidget) {
//        if (listWidget->selectionModel()->hasSelection()) {
//            QModelIndexList sel = listWidget->selectionModel()->selectedIndexes();
//            foreach(const QModelIndex &index, sel) {
//                toReturn << index.data();
//            }
//        }
//        return toReturn;
//    }
    // QListView
    QListView *listView = qobject_cast<QListView*>(widget);
    if (listView) {
        if (listView->selectionModel()->hasSelection()) {
            QModelIndexList sel = listView->selectionModel()->selectedIndexes();
            foreach(const QModelIndex &index, sel) {
                toReturn << index.data().toString();
            }
        }
        return toReturn;
    }
    // QComboBox
    QComboBox *combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        toReturn << combo->currentText();
        return toReturn;
    }
    return toReturn;
}
