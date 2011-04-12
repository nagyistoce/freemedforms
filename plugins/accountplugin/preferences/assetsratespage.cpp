/***************************************************************************
 *   Copyright (C) 2009 by Docteur Pierre-Marie Desombre, GP               *
 *   pm.desombre@medsyn.fr                                                 *
 *   original from Eric Maeker, MD.                                        *
 *   eric.maeker@free.fr                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       Pierre-Marie DESOMBRE <pm.desombre@medsyn.fr>                     *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "assetsratespage.h"
#include <accountplugin/constants.h>

#include <accountbaseplugin/constants.h>

#include <utils/log.h>
#include <utils/global.h>
#include <translationutils/constanttranslations.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/itheme.h>
#include <coreplugin/iuser.h>

#include <coreplugin/constants_icons.h>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QRegExp>
#include <QLocale>
#include <QUuid>

using namespace Account;
using namespace Account::Internal;
using namespace Trans::ConstantTranslations;

static inline Core::ISettings *settings() { return Core::ICore::instance()->settings(); }
static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline Core::IUser *user() { return Core::ICore::instance()->user(); }


/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////  AccountUserPage  //////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
AssetsRatesPage::AssetsRatesPage(QObject *parent) :
        IOptionsPage(parent), m_Widget(0) { setObjectName("AssetsRatesPage"); }

AssetsRatesPage::~AssetsRatesPage()
{
    if (m_Widget) delete m_Widget;
    m_Widget = 0;
}

QString AssetsRatesPage::id() const { return objectName(); }
QString AssetsRatesPage::name() const { return tkTr(Trans::Constants::ASSETSRATES); }
QString AssetsRatesPage::category() const { return tkTr(Trans::Constants::ACCOUNTANCY); }

void AssetsRatesPage::resetToDefaults()
{
    m_Widget->writeDefaultSettings(settings());
    m_Widget->setDatasToUi();
}

void AssetsRatesPage::applyChanges()
{qDebug() << __FILE__ << QString::number(__LINE__) << " applyChanges ";
    if (!m_Widget) {
        return;
    }
    m_Widget->saveToSettings(settings());
}

void AssetsRatesPage::finish() { delete m_Widget; }

void AssetsRatesPage::checkSettingsValidity()
{
    QHash<QString, QVariant> defaultvalues;
//    defaultvalues.insert(DrugsDB::Constants::S_AVAILABLEDOSAGESBACKGROUNGCOLOR, DrugsDB::Constants::S_DEF_AVAILABLEDOSAGESBACKGROUNGCOLOR);

    foreach(const QString &k, defaultvalues.keys()) {
        if (settings()->value(k) == QVariant())
            settings()->setValue(k, defaultvalues.value(k));
    }
    settings()->sync();
}

QWidget *AssetsRatesPage::createPage(QWidget *parent)
{
    if (m_Widget)
        delete m_Widget;
    m_Widget = new AssetsRatesWidget(parent);
    return m_Widget;
}

AssetsRatesWidget::AssetsRatesWidget(QWidget *parent) :
        QWidget(parent), m_Model(0), m_Mapper(0)
{
    setObjectName("AssetsRatesWidget");
    setupUi(this);
    m_user_uid = user()->value(Core::IUser::Uuid).toString();
    addButton->setIcon(theme()->icon(Core::Constants::ICONADD));
    addButton->setText("New");
    deleteButton->setIcon(theme()->icon(Core::Constants::ICONREMOVE));
    deleteButton->setText("Delete");
    beginSpinBox->setRange(0,1000);
    endSpinBox->setRange(0,1000);
    rateDoubleSpinBox->setRange(0.00,1000.00);
    rateDoubleSpinBox->setSingleStep(0.01);
    m_Model = new AccountDB::AssetsRatesModel(this);
    if (m_Model->rowCount() < 1)
    {
    	  if (!writeDefaultsWithLocale())
    	  {
    	  	  QMessageBox::warning(0,trUtf8("Warning"),trUtf8("Unable to fill assetsratesmodel whith local .csv"),
    	  	                       QMessageBox::Ok);
    	      }
        }
    assetsRatesUidLabel->setText("");
    m_Mapper = new QDataWidgetMapper(this);
    m_Mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    m_Mapper->setModel(m_Model);
    m_Mapper->setCurrentModelIndex(QModelIndex());
    m_Mapper->addMapping(assetsRatesUidLabel,AccountDB::Constants::ASSETSRATES_USER_UID);
    m_Mapper->addMapping(nameEdit, AccountDB::Constants::ASSETSRATES_NAME);
    m_Mapper->addMapping(dateEdit, AccountDB::Constants::ASSETSRATES_DATE);
    m_Mapper->addMapping(rateDoubleSpinBox, AccountDB::Constants::ASSETSRATES_RATES);
    //m_Mapper->toFirst();
    assetsNameComboBox->setModel(m_Model);
    assetsNameComboBox->setModelColumn(AccountDB::Constants::ASSETSRATES_NAME);
    setDatasToUi();
    connect(m_Mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(changeSpinBoxes(int)));
}

AssetsRatesWidget::~AssetsRatesWidget()
{
    //saveModel();
}

void AssetsRatesWidget::setDatasToUi()
{
    qDebug() << __FILE__ << QString::number(__LINE__) << "index row  =" << QString::number(assetsNameComboBox->currentIndex());
    m_Mapper->setCurrentIndex(assetsNameComboBox->currentIndex());
}

void AssetsRatesWidget::saveModel()
{
    qDebug() << __FILE__ << QString::number(__LINE__) << " currentIndex =" << QString::number(m_Mapper->currentIndex());
    if (m_Model->isDirty()) {
        bool yes = Utils::yesNoMessageBox(tr("Save changes ?"),
                                          tr("You make changes into the assetsrates table.\n"
                                             "Do you want to save them ?"));
        if (yes) {
           if (!m_Model->submit()) {qDebug() << __FILE__ << QString::number(__LINE__) << " assetsrates no submit ";
                Utils::Log::addError(this, tkTr(Trans::Constants::UNABLE_TO_SAVE_DATA_IN_DATABASE_1).
                                                   arg(tr("assetsrates")), __FILE__, __LINE__);
            }
        } 
        else {
            m_Model->revert();
        }
    }
    qDebug() << __FILE__ << QString::number(__LINE__) << " assets rates error =" << m_Model->lastError().text();
}

void AssetsRatesWidget::on_assetsNameComboBox_currentIndexChanged(int index)
{
    m_Mapper->setCurrentIndex(assetsNameComboBox->currentIndex());
}

void AssetsRatesWidget::on_addButton_clicked()
{
    qDebug() << __FILE__ << QString::number(__LINE__) << " rowCount1 =" << QString::number(m_Model->rowCount());
    if (!m_Model->insertRow(m_Model->rowCount()))
        Utils::Log::addError(this, "Unable to add row", __FILE__, __LINE__);
    qDebug() << __FILE__ << QString::number(__LINE__) << " rowCount2 =" << QString::number(m_Model->rowCount());
    assetsNameComboBox->setCurrentIndex(m_Model->rowCount()-1);
    assetsRatesUidLabel->setText(m_user_uid);
    assetsRatesUidLabel->setFocus();
    QDate currentDate = QDate::currentDate();
    dateEdit->setDate(currentDate);
    dateEdit->setFocus();
    //qDebug() << __FILE__ << QString::number(__LINE__) << " percentUidLabel =" << percentUidLabel->text();
    //qDebug() << __FILE__ << QString::number(__LINE__) << " currentIndex =" << QString::number(m_Mapper->currentIndex());

}

void AssetsRatesWidget::on_deleteButton_clicked()
{
    if (!m_Model->removeRow(assetsNameComboBox->currentIndex()))
    {
    	  Utils::Log::addError(this, "Unable to remove row", __FILE__, __LINE__);
        }
    assetsNameComboBox->setCurrentIndex(m_Model->rowCount() - 1);
}

void AssetsRatesWidget::saveToSettings(Core::ISettings *sets)
{
    if (!m_Model->submit()) {
        Utils::Log::addError(this, tkTr(Trans::Constants::UNABLE_TO_SAVE_DATA_IN_DATABASE_1).arg(tr("assetsrates")));
        Utils::warningMessageBox(tr("Can not submit assetsrates to your personnal database."),
                                 tr("An error occured during assetsrates saving. Datas are corrupted."));
    }
    if (!insertYearsRange())
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << " Unable to insert years range !" ;
        }
        connect(nameEdit,SIGNAL(textEdited(const QString &)),assetsNameComboBox,SLOT(setEditText(const QString &)));
        update();
}

void AssetsRatesWidget::writeDefaultSettings(Core::ISettings *s)
{
//    Utils::Log::addMessage("AssetsRatesWidget", tkTr(Trans::Constants::CREATING_DEFAULT_SETTINGS_FOR_1).arg("AssetsRatesWidget"));
//    s->sync();
}

void AssetsRatesWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
//        int s = defaultCombo->currentIndex();
//        defaultCombo->clear();
//        defaultCombo->addItem(tkTr(Trans::Constants::NO));
//        defaultCombo->addItem(tkTr(Trans::Constants::YES));
//        defaultCombo->setCurrentIndex(s);
        break;
    default:
        break;
    }
}

QString AssetsRatesWidget::calcAssetsRatesUid(){
    QString uuidStr;
    uuidStr = QUuid::createUuid().toString();
    return uuidStr;
}

bool AssetsRatesWidget::insertYearsRange(){
    bool ret = true;
    QString beginYear = QString::number(beginSpinBox->value());
    QString endYear = QString::number(endSpinBox->value());
    QString yearRange = QString("%1_%2").arg(beginYear,endYear);
    m_Model->setFilter("");
    qDebug() << __FILE__ << QString::number(__LINE__) << " m_Model->rowCount =" << QString::number((m_Model->rowCount())) ;
    
    if (!m_Model->setData(m_Model->index(m_Model->rowCount()-1,AccountDB::Constants::ASSETSRATES_YEARS),
          yearRange,Qt::EditRole))
    {
        qWarning() << __FILE__ << QString::number(__LINE__) 
                   << "unable to insert years range "+m_Model->lastError().text();
        }
    return ret;
}

static QString getCsvDefaultFile()
{
    QString sqlDirPath = settings()->path(Core::ISettings::BundleResourcesPath) + "/sql/account";
    QDir dir(sqlDirPath);
    if (!dir.exists())
        return QString();
    QString fileName = QString("assets_rates_%1.csv").arg(QLocale().name());
    QFile file(dir.absolutePath() + QDir::separator() + fileName);
    if (!file.exists())
        return QString();
    return file.fileName();
}

QStandardItemModel *AssetsRatesWidget::assetsRatesModelByLocale()
{
    QStandardItemModel *model = new QStandardItemModel;
    QString csvFileName = getCsvDefaultFile();
    qDebug() << __FILE__ << QString::number(__LINE__) << " csvFileName =" << csvFileName ;
    QFile file(getCsvDefaultFile());
    // some validity checking
    if (!file.exists()) {
        LOG_ERROR(tkTr(Trans::Constants::FILE_1_DOESNOT_EXISTS).arg(QLocale().name() + " " + tr("Assets rates")));
        return model;
    }
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        LOG_ERROR(tkTr(Trans::Constants::FILE_1_ISNOT_READABLE).arg(file.fileName()));
        return model;
    }

    // read the content with UTF8 coding system
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    // skip first line
    //stream.readLine();
    //int row = 0;
    while (!stream.atEnd())
    {
        int row = 0;
        QString line = stream.readLine();
        QStringList listOfSeparators;
        listOfSeparators << ",\"" << ";\"" << QString("\t\"")
                         << ",''" << ";''" << QString("\t''");
        QString separator;
        QString separatorStr;
        foreach(separatorStr,listOfSeparators){
            if (line.contains(separatorStr)){
                separator = separatorStr;
                }
            }
        if (!line.contains("ASSETSRATES_ID")){
            QList<QStandardItem*> listOfItemsData;
            QStringList listOfItems;
            listOfItems = line.split(separator);
            for (int i = 0; i < AccountDB::Constants::ASSETSRATES_MaxParam ; i += 1){
        	QStandardItem * item = new QStandardItem;
        	//qDebug() << __FILE__ << QString::number(__LINE__) << " listOfItems[i] =" << listOfItems[i] ;
        	QString itemOfList = listOfItems[i];
        	itemOfList.remove("\"");
        	itemOfList.remove("'");
        	item->setData(itemOfList);
        	listOfItemsData << item;
        	}
            model->appendRow(listOfItemsData);
            ++row;  
            }
    }
    return model;
}


bool AssetsRatesWidget::writeDefaultsWithLocale(){
    bool test = false;
    QStandardItemModel * model = assetsRatesModelByLocale();
    int assetsratesModelRows = model->rowCount();
    QString strList;
    for (int i = 0; i < assetsratesModelRows; i += 1){
        if (!m_Model->insertRows(m_Model->rowCount(),1,QModelIndex()))
    	    {
    	    qWarning() << __FILE__ << QString::number(__LINE__) << QString::number(m_Model->rowCount()) ;
    	    }
    	  		    QString strValues;
    	  	for (int j = 0; j < AccountDB::Constants::ASSETSRATES_MaxParam ; j += 1){
    	  		QStandardItem * item = model->itemFromIndex(model->index(i,j));
    	  		QVariant value = item->data();
    	  		if (value.canConvert(QVariant::String))
    	  		{
    	  			  QString strValue = value.toString().replace("'","''");
    	  			  value = QVariant::fromValue(strValue);
    	  		    }
    	  		    strValues += value.toString()+" ";
    	  		if (!m_Model->setData(m_Model->index(m_Model->rowCount()-1,j),value,Qt::EditRole))
    	  		{
    	  			qWarning() << __FILE__ << QString::number(__LINE__) << "data not inserted !" ;  
    	  		    }
    	  	    }
    	  	    strList += strValues+"\n";
    	      test = m_Model->submit();
    	      }
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " values = \n" << strList;

    return test;
}

void AssetsRatesWidget::changeSpinBoxes(int index){
    int beginYear = 0;
    int endYear = 0;
    AccountDB::AssetsRatesModel model(this);
    QString range = model.data(model.index(index,AccountDB::Constants::ASSETSRATES_NAME),Qt::DisplayRole).toString();
    QString filter = QString("%1='%2' AND %3='%4'").arg("USER_UID",m_user_uid,"NAME",range);
    model.setFilter(filter);
    QString years = model.data(m_Model->index(0,AccountDB::Constants::ASSETSRATES_YEARS),Qt::DisplayRole).toString();
    QStringList listYears = years.split("_");
    if (listYears.size()>1)
    {
    	beginYear = listYears[0].toInt();
        endYear= listYears[1].toInt(); 
        }
    beginSpinBox->setValue(beginYear);
    endSpinBox->setValue(endYear);
}
