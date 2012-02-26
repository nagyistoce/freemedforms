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
#include "pregnancyclassification.h"

#include <coreplugin/icore.h>
#include <coreplugin/imainwindow.h>
#include <coreplugin/globaltools.h>
#include <coreplugin/isettings.h>
#include <coreplugin/ftb_constants.h>

#include <utils/global.h>
#include <utils/log.h>
#include <extensionsystem/pluginmanager.h>
#include <translationutils/constanttranslations.h>

#include <QScriptEngine>
#include <QDir>

#include "ui_pregnancyclassification.h"

using namespace DrugInfos;
static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }
static inline ExtensionSystem::PluginManager *pluginManager() {return ExtensionSystem::PluginManager::instance();}

static inline QString workingPath()     {return QDir::cleanPath(settings()->value(Core::Constants::S_TMP_PATH).toString() + "/TgaPregDb/") + QDir::separator();}
static inline QString databaseAbsPath()  {return Core::Tools::drugsDatabaseAbsFileName();}

static inline QString databaseDescriptionFile() {return QDir::cleanPath(settings()->value(Core::Constants::S_SVNFILES_PATH).toString() + "/global_resources/sql/drugdb/tga_preg/description.xml");}

QString PregnancyClassificationPage::id() const {return "PregnancyClassificationPage";}
QString PregnancyClassificationPage::name() const {return "Pregnancy classification";}
QString PregnancyClassificationPage::category() const {return Core::Constants::CATEGORY_DRUGINFOSDATABASE;}
QIcon PregnancyClassificationPage::icon() const {return QIcon();}

namespace {

const char* const  PREGNANCY_TGA_URL            = "http://www.tga.gov.au/webfiles/medicinesInPregnancyData.js";
const char* const  TGA_PREGNANCY_DATABASE_NAME  = "TGA_PREG";

enum FieldType {
    Field_Name,
    Field_Category,
    Field_Safety,
    Field_Class1,
    Field_Class2,
    Field_Class3
};

typedef QHash<FieldType, QString> PregnancyRecord;

/**
 * Load a pregnancy db file.
 * \param {QString} fileName The .js pregnancy filename
 * \param {QList<PregnancyRecord>} records a reference to a list of records in which all read records will be loaded into
 * \param {QString} [errorMsg] An optional QString used to get the error message if loading failed
 * \return true if success, false if failed
 */
bool load(const QString &fileName, QList<PregnancyRecord> &records, QString *errorMsg = 0)
{
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        if (errorMsg)
            *errorMsg = QString("Error while opening %1").arg(fileName);
        return false;
    }
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();
    QScriptEngine myEngine;
    QScriptValue fun = myEngine.evaluate("(" + contents + ")", fileName);

    if (!fun.isValid() || !fun.isFunction()) {
        if (errorMsg)
            *errorMsg = QString("Error while evaluating the function inside %1").arg(fileName);
        return false;
    }

    QScriptValue obj = fun.call();

    if (!obj.isValid() || !obj.isObject()) {
        if (errorMsg)
            *errorMsg = QString("Error: the function does not return a valid object");
        return false;
    }

    records.clear();
    QVariantMap drugsMap = obj.toVariant().toMap();

    foreach (const QString &key, drugsMap.keys()) {
        QVariant val = drugsMap[key];
        if (val.toList().empty())
            continue;
        QVariantMap rMap = val.toList()[0].toMap();
        PregnancyRecord rec;
        rec.insert(Field_Name, key);
        rec.insert(Field_Category, rMap["Category"].toString());
        rec.insert(Field_Safety, rMap["Safety"].toString());
        rec.insert(Field_Class1, rMap["Class1"].toString());
        rec.insert(Field_Class2, rMap["Class2"].toString());
        rec.insert(Field_Class3, rMap["Class3"].toString());
        records << rec;
    }

    return true;
}

}

// widget will be deleted after the show
QWidget *PregnancyClassificationPage::createPage(QWidget *parent)
{
    return new PregnancyClassificationWidget(parent);
}


PregnancyClassificationWidget::PregnancyClassificationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PregnancyClassificationWidget)
{
    ui->setupUi(this);
    computeJavascriptFile();
}

PregnancyClassificationWidget::~PregnancyClassificationWidget()
{
}

void PregnancyClassificationWidget::computeJavascriptFile()
{
    QList<QHash<FieldType, QString> > pregnancyList;
    QString errorMsg;
    // NOTE POUR ERIC: change le chemin du fichier par le tien
    QString jsFile = "/Users/eric/Desktop/Programmation/freemedforms/__nonfree__/resources/drugs/pregnancy/medicinesInPregnancyData.js";
    if (load(jsFile, pregnancyList, &errorMsg)) {
        qDebug("SUCCESS");
        foreach (const PregnancyRecord &rec, pregnancyList) {
            qDebug("%s, %s, %s, %s, %s, %s", qPrintable(rec[Field_Name]),
                   qPrintable(rec[Field_Category]),
                   qPrintable(rec[Field_Safety]),
                   qPrintable(rec[Field_Class1]),
                   qPrintable(rec[Field_Class2]),
                   qPrintable(rec[Field_Class3]));
        }
    } else
        qDebug("FAILURE: %s", qPrintable(errorMsg));
}

void PregnancyClassificationWidget::on_download_clicked()
{
}

void PregnancyClassificationWidget::on_editClassification_clicked()
{
}
