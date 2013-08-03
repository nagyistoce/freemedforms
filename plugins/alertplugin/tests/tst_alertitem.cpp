/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developers:                                                       *
 *       Eric MAEKER, <eric.maeker@gmail.com>,                             *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "../alertplugin.h"
#include "../alertitem.h"
#include "../alertcore.h"
#include "../alertbase.h"

#include <utils/log.h>
#include <utils/global.h>

#include "alertitemeditordialog.h"
#include "blockingalertdialog.h"
#include "alertplaceholderwidget.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QPointer>

using namespace Alert;
using namespace Internal;

//static inline Core::ITheme *theme() {return Core::ICore::instance()->theme();}
//static inline Core::IUser *user() {return Core::ICore::instance()->user();}
static inline Alert::AlertCore *alertCore() {return Alert::AlertCore::instance();}
static inline Alert::Internal::AlertBase &alertBase() {return Alert::AlertCore::instance()->alertBase();}

void AlertPlugin::test_alertitem_object()
{
    // Test the AlertItem interface
    QDateTime start = QDateTime::currentDateTime().addSecs(-60*60*24);
    QDateTime expiration = QDateTime::currentDateTime().addSecs(60*60*24);

    AlertItem item = alertBase().createVirtualItem();
    item.setLabel(item.label() + " (item)");
    item.setThemedIcon("identity.png");
    item.setViewType(AlertItem::NonBlockingAlert);
    item.setPriority(AlertItem::High);
    item.setRemindLaterAllowed(true);
    item.clearRelations();
    item.clearTimings();
    item.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item.addTiming(AlertTiming(start, expiration));

    AlertItem item2 = alertBase().createVirtualItem();
    item2.setThemedIcon("next.png");
    item2.setViewType(AlertItem::NonBlockingAlert);
    item2.clearRelations();
    item2.clearTimings();
    item2.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient2"));
    item2.addTiming(AlertTiming(start, expiration));

    AlertItem item3;
    item3.setUuid(Utils::Database::createUid());
    item3.setThemedIcon("ok.png");
    item3.setLabel("Just a simple alert (item3)");
    item3.setCategory("Test");
    item3.setDescription("Simple basic static alert that needs a user comment on overriding");
    item3.setViewType(AlertItem::NonBlockingAlert);
    item3.setPriority(AlertItem::Low);
    item3.setOverrideRequiresUserComment(true);
    item3.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item3.addTiming(AlertTiming(start, expiration));

    AlertItem item4;
    item4.setUuid(Utils::Database::createUid());
    item4.setThemedIcon("elderly.png");
    item4.setLabel("Related to all patient (item4)");
    item4.setCategory("Test");
    item4.setDescription("Related to all patients and was validated for patient2 by user1.<br /> Static alert");
    item4.setViewType(AlertItem::NonBlockingAlert);
    item4.setPriority(AlertItem::Medium);
    item4.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item4.addValidation(AlertValidation(QDateTime::currentDateTime(), "user1", "patient2"));
    item4.addTiming(AlertTiming(start, expiration));

    AlertItem item5;
    item5.setUuid(Utils::Database::createUid());
    item5.setLabel("Simple basic Blocking alert test (item5)");
    item5.setCategory("Test");
    item5.setDescription("Aoutch this is a Blocking alert !");
    item5.setViewType(AlertItem::BlockingAlert);
    item5.setRemindLaterAllowed(true);
    item5.setOverrideRequiresUserComment(true);
    item5.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item5.addTiming(AlertTiming(start, expiration));

    AlertItem item6;
    item6.setUuid(Utils::Database::createUid());
    item6.setLabel("Simple basic Blocking user alert (item6)");
    item6.setCategory("Test user alert");
    item6.setDescription("Aoutch this is a Blocking alert !<br />For you, <b>user1</b>!");
    item6.setViewType(AlertItem::BlockingAlert);
    item6.setRemindLaterAllowed(true);
    item6.addRelation(AlertRelation(AlertRelation::RelatedToUser, "user1"));
    item6.addTiming(AlertTiming(start, expiration));
    item6.addScript(AlertScript("check_item6", AlertScript::CheckValidityOfAlert, "(1+1)==2;"));
    item6.addScript(AlertScript("onoverride_item6", AlertScript::OnOverridden, "(1+1)==2;"));

    AlertItem item7;
    item7.setUuid(Utils::Database::createUid());
    item7.setLabel("Simple basic alert (item7)");
    item7.setCategory("Test validated alert");
    item7.setDescription("Aoutch this is an error you should not see this !<br /><br />Validated for patient1.");
    item7.setViewType(AlertItem::NonBlockingAlert);
    item7.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item7.addValidation(AlertValidation(QDateTime::currentDateTime(), "user1", "patient1"));
    item7.addTiming(AlertTiming(start, expiration));

    AlertItem item8;
    item8.setUuid(Utils::Database::createUid());
    item8.setLabel("Scripted alert (item8)");
    item8.setCategory("Test scripted alert");
    item8.setDescription("A valid alert with multiple scripts.");
    item8.setViewType(AlertItem::NonBlockingAlert);
    item8.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item8.addTiming(AlertTiming(start, expiration));
    item8.addScript(AlertScript("check_item8", AlertScript::CheckValidityOfAlert, "(1+1)==2;"));

    AlertItem item9;
    item9.setUuid(Utils::Database::createUid());
    item9.setLabel("INVALID Scripted alert (item9)");
    item9.setCategory("Test scripted alert");
    item9.setDescription("A invalid alert with multiple scripts. YOU SHOULD NOT SEE IT !!!!");
    item9.setViewType(AlertItem::NonBlockingAlert);
    item9.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item9.addTiming(AlertTiming(start, expiration));
    item9.addScript(AlertScript("check_item9", AlertScript::CheckValidityOfAlert, "(1+1)==3;"));

    AlertItem item10;
    item10.setUuid(Utils::Database::createUid());
    item10.setLabel("Cycling alert for all patients (item10)");
    item10.setCategory("Test cycling alert");
    item10.setDescription("Testing a cycling alert with:<br />- scripts<br />- relation to all patients.<br />Static alert.");
    item10.setViewType(AlertItem::NonBlockingAlert);
    item10.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    AlertTiming cycling(start, expiration);
    cycling.setCyclingDelayInYears(5);
    cycling.setNumberOfCycles(32565);
    item10.addTiming(cycling);
    item10.addScript(AlertScript("check_item10", AlertScript::CheckValidityOfAlert, "(1+1)==2;"));
    item10.addScript(AlertScript("startdate_item10", AlertScript::CyclingStartDate, "var currentDate = new Date(); currentDate.setDate(currentDate.getDate()-2); currentDate;"));

    AlertItem item11;
    item11.setUuid(Utils::Database::createUid());
    item11.setLabel("Test script interactions (item11)");
    item11.setCategory("Test script");
    item11.setDescription("Test script interaction with alertitem:<br />- redefine priority to HIGH<br />- change the label adding \"WAAAAAAHHHHHHOOUUUUHHH\"<br/>- add a comment");
    item11.setViewType(AlertItem::BlockingAlert);
    item11.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item11.addTiming(AlertTiming(start, expiration));
    item11.addScript(AlertScript("check_item11", AlertScript::CheckValidityOfAlert,
                                 "print(\"CURRENT ALERT PROPERTY:\"+alert.priority());"
                                 "alert.setPriority(0);"
                                 "print(\"CURRENT ALERT LABEL:\"+alert.label());"
                                 "alert.setLabel(alert.label()+\"<br />WAAAAAAHHHHHHOOUUUUHHH\");"
                                 "alert.setComment(\"Niah niah niah comment added by the script...\");"
                                 "true;"));

    // Db save/get
    if (false) {
        if (!alertBase().saveAlertItem(item))
            qWarning() << "ITEM WRONG";
        if (!alertBase().saveAlertItem(item2))
            qWarning() << "ITEM2 WRONG";
        if (!alertBase().saveAlertItem(item3))
            qWarning() << "ITEM3 WRONG";
        if (!alertBase().saveAlertItem(item4))
            qWarning() << "ITEM4 WRONG";
        if (!alertBase().saveAlertItem(item5))
            qWarning() << "ITEM5 WRONG";
        if (!alertBase().saveAlertItem(item6))
            qWarning() << "ITEM6 WRONG";
        if (!alertBase().saveAlertItem(item7))
            qWarning() << "ITEM7 WRONG";
        if (!alertBase().saveAlertItem(item8))
            qWarning() << "ITEM8 WRONG";
        if (!alertBase().saveAlertItem(item9))
            qWarning() << "ITEM9 WRONG";
        if (!alertBase().saveAlertItem(item10))
            qWarning() << "ITEM10 WRONG";
        if (!alertBase().saveAlertItem(item11))
            qWarning() << "ITEM11 WRONG";

        Internal::AlertBaseQuery query;
        query.setAlertValidity(Internal::AlertBaseQuery::ValidAlerts);
//        query.setAlertValidity(Internal::AlertBaseQuery::InvalidAlerts);
        query.addUserAlerts("user1");
        query.addUserAlerts("user2");
        query.addPatientAlerts("patient1");
        query.addPatientAlerts("patient2");
        query.addPatientAlerts("patient3");
//        query.addUserAlerts();
        QVector<AlertItem> test = alertBase().getAlertItems(query);
        qWarning() << test.count();
//        for(int i=0; i < test.count(); ++i) {
//            qWarning() << "\n\n" << test.at(i).timingAt(0).start() << test.at(i).timingAt(0).end() << test.at(i).relationAt(1).relatedToUid();
//        }
//        qWarning() << "\n\n";
        //    AlertItem t = AlertItem::fromXml(item.toXml());
        //    qWarning() << (t.toXml() == item.toXml());
    }

    // To XML
    if (false) {
        qWarning() << item.toXml();
        qWarning() << item11.toXml();
    }

    // Blocking alerts
    if (false) {
        item.setViewType(AlertItem::BlockingAlert);
        item.setOverrideRequiresUserComment(true);
        QToolButton *test = new QToolButton;
        test->setText("Houlala");
        test->setToolTip("kokokokokok");
        QList<QAbstractButton*> buttons;
        buttons << test;

        BlockingAlertDialog::executeBlockingAlert(QList<AlertItem>() <<  item << item2 << item3 << item4 << item5, buttons);
        //    BlockingAlertDialog::executeBlockingAlert(item4);
    }

    // Alert editor
    if (false) {
        AlertItemEditorDialog dlg;
        dlg.setEditableParams(AlertItemEditorDialog::FullDescription | AlertItemEditorDialog::Timing);
        dlg.setEditableParams(AlertItemEditorDialog::Label | AlertItemEditorDialog::Timing);
        dlg.setEditableParams(AlertItemEditorDialog::Label | AlertItemEditorDialog::Timing | AlertItemEditorDialog::Types);

        AlertTiming &time = item.timingAt(0);
        time.setCycling(true);
        time.setCyclingDelayInDays(10);
        dlg.setAlertItem(item);
        if (dlg.exec()==QDialog::Accepted) {
            dlg.submit(item);
        }
        qWarning() << item.toXml();
    }

    // Alert packs
//    if (true) {
//        q->registerAlertPack(settings()->path(Core::ISettings::BundledAlertPacks) + "/test");
//    }

//    // PlaceHolders
//    if (true) {
//        // Put placeholder in the plugin manager object pool
//        _placeholdertest = new AlertPlaceHolderWidget; // object should not be deleted
//        pluginManager()->addObject(_placeholdertest);

//        // Create the dialog && the placeholder
//        QDialog dlg;
//        QVBoxLayout lay(&dlg);
//        dlg.setLayout(&lay);
//        lay.addWidget(_placeholdertest->createWidget(&dlg));

//        // Check alerts
//        q->checkAlerts(AlertCore::CurrentPatientAlerts | AlertCore::CurrentUserAlerts | AlertCore::CurrentApplicationAlerts);

//        // Exec the dialog
//        dlg.exec();
//    }

}

