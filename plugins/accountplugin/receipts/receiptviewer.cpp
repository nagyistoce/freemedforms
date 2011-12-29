/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developpers : Pierre-Marie DESOMBRE <pm.desombre@medsyn.fr>,      *
 *                     Eric MAEKER, <eric.maeker@gmail.com>                *
 *  Contributors :                                                         *
 *      NAME <MAIL@ADRESS>                                                 *
 ***************************************************************************/
#include "receiptviewer.h"
#include "receiptsmanager.h"	
#include "receiptsIO.h"
#include "findReceiptsValues.h"
#include "choiceDialog.h"
#include "distance.h"

#include "constants.h"

#include "ui_receiptviewer.h"

#include <accountbaseplugin/constants.h>
#include <accountbaseplugin/workingplacesmodel.h>

#include <accountplugin/ledger/ledgerviewer.h>

#include <coreplugin/icore.h>
#include <coreplugin/iuser.h>
#include <coreplugin/ipatient.h>
#include <coreplugin/isettings.h>

#include <utils/widgets/spinboxdelegate.h>

#include <QAbstractItemModel>
#include <QDebug>
#include <QMessageBox>
#include <QFrame>
#include <QPushButton>
#include <QKeySequence>
#include <QString>
#include <QBrush>
#include <QColor>

enum { WarnDebugMessage = false };

static inline Core::IUser *user() { return Core::ICore::instance()->user(); }
static inline Core::IPatient *patient() { return Core::ICore::instance()->patient(); }
static inline Core::ISettings *settings() { return Core::ICore::instance()->settings(); }

using namespace ReceiptsConstants;
using namespace Constants;

namespace InternalAmount {

    class AmountModel : public QAbstractTableModel
    {
    public:
        enum ColumnRepresentation {
            Col_Cash = 0,
            Col_Cheque,  // Devise monétaire
            Col_Visa,
            Col_Banking,
            Col_Other,
            Col_Du,
            Col_Debtor,
            Col_Site,
            Col_DistRule,
            Col_Count
        };

        AmountModel(QObject *parent = 0) : QAbstractTableModel(parent)
        {
             m_listsOfValuesbyRows = new QVector<QList<QVariant> >;
        }
        
        int rowCount(const QModelIndex &parent ) const {
            Q_UNUSED(parent);
            return m_listsOfValuesbyRows->size();
            }
            
        int columnCount(const QModelIndex &parent = QModelIndex()) const {
            Q_UNUSED(parent);
            return int(Col_Count);
            }
        
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const
        {
            return QAbstractTableModel::index(row,column,parent);
        }
        
        bool insertRows( int position, int count, const QModelIndex & parent = QModelIndex() ){
            beginInsertRows(parent, position, position+count-1);
            for (int row=0; row < count; row++) {
                
                QList<QVariant> list;
                for (int j = 0; j < Col_Count; j += 1)
                {
            	    list << QVariant(0);
            	    }
                m_listsOfValuesbyRows -> append(list);
            }
            QList<QVariant> listDouble;
            listDouble = m_listsOfValuesbyRows->at(position);
            m_headersRows.append(listDouble[Col_Debtor].toString());
            endInsertRows();
            return true;
            
        }
        
        bool removeRows(int position, int count, const QModelIndex & parent = QModelIndex()){
            Q_UNUSED(parent);
            beginRemoveRows(parent, position, position+count-1);
            int rows = m_listsOfValuesbyRows->size();
            for (int row=0; row < count; row++) {
                if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " row =" << QString::number(row) ;
                m_listsOfValuesbyRows -> remove(rows - row -1);
            }
            endRemoveRows();
            return true;
        }
        
        bool submit(){
            return QAbstractTableModel::submit();
        }


        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
        {
            
            QVariant data;
            if (!index.isValid()) {
                if (WarnDebugMessage)
                    qWarning() << __FILE__ << QString::number(__LINE__) << "index not valid" ;
                return QVariant();
                }
                
            
            if (role==Qt::EditRole || role==Qt::DisplayRole) {
               int row = index.row();
               const QList<QVariant> & valuesListByRow = m_listsOfValuesbyRows->at(row);
               data = valuesListByRow[index.column()];
            }
            return data;
        }

        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
        {
            bool ret = true;
            if (!index.isValid()) {
                if (WarnDebugMessage)
                    qWarning() << __FILE__ << QString::number(__LINE__) << "index not valid" ;
                return false;
            }

            if (role==Qt::EditRole) {
                QList<QVariant> list;
                int row = index.row();
                if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " row =" << QString::number(row) ;
                if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " value =" << value.toString() ;
                list = m_listsOfValuesbyRows->at(row);
                switch(index.column()){
                case Col_Cash :
                    list.replace(Col_Cash,value);
                    break;
                case Col_Cheque :
                    list.replace(Col_Cheque,value);
                    break;
                case Col_Visa :
                    list.replace(Col_Visa,value);
                    break;
                case Col_Banking :
                    list.replace(Col_Banking,value);
                    break;
                case Col_Other :
                    list.replace(Col_Other,value);
                    break;
                case Col_Du :
                    list.replace(Col_Du,value);
                    break;
                case Col_Debtor :
                    list.replace(Col_Debtor,value);
                    break;
                case Col_Site :
                    list.replace(Col_Site,value);
                    break;
                case Col_DistRule :
                    list.replace(Col_DistRule,value);
                    break;
                default :
                    break;
                }
                m_listsOfValuesbyRows->replace(row,list);
                Q_EMIT dataChanged(index, index);
                ret = true;
            }
            return ret;
        }

        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
        {
            if (role==Qt::DisplayRole) {
                if (orientation==Qt::Horizontal) {
                    switch (section) {
                    case Col_Cash: return m_headersColumns.value(Col_Cash);//"Cash";
                    case Col_Cheque: return m_headersColumns.value(Col_Cheque);//"Cheque";
                    case Col_Visa: return m_headersColumns.value(Col_Visa);//return "Visa";
                    case Col_Banking: return m_headersColumns.value(Col_Banking);//"Banking";
                    case Col_Other: return m_headersColumns.value(Col_Other);//"Other";
                    case Col_Du: return m_headersColumns.value(Col_Du);//"Du";
                    case Col_Debtor : return m_headersColumns.value(Col_Debtor);//"debtor";
                    case Col_Site : return m_headersColumns.value(Col_Site);//"site";
                    case Col_DistRule : return m_headersColumns.value(Col_DistRule);//"distRule";
                     //return QVariant(m_headersColumns[section]);
                    }
                    }
                else if (orientation==Qt::Vertical) {
                    return QVariant(m_headersRows[section]);
                   }
            }
         
                return QVariant();
                
            
        }
        
        bool setHeaderData( int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole )
        {
            if (role == Qt::EditRole||role == Qt::DisplayRole) {
                if (orientation == Qt::Vertical) {
                    m_headersRows.insert(section,value.toString());
                    }
                else if (orientation == Qt::Horizontal){
                    m_headersColumns.insert(section,value.toString());
                        }
                     } 
              
            else {
                return false;
                }

            Q_EMIT QAbstractTableModel::headerDataChanged(orientation, section, section) ;
            return true;
        }

        QSqlError lastError(){
            return lastError();
            }

        Qt::ItemFlags flags(const QModelIndex &index) const
        {
            if (   index.column()==Col_Cash 
                || index.column()==Col_Cheque
                || index.column()==Col_Visa
                || index.column()==Col_Banking
                || index.column()==Col_Other 
                || index.column()==Col_Du
                || index.column()==Col_Debtor
                || index.column()==Col_Site
                || index.column()==Col_DistRule ) {
                return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
        }


    private:
        QVector<QList<QVariant> > *m_listsOfValuesbyRows;
        QStringList m_headersRows;
        QStringList m_headersColumns;
        int m_rows ;
        
    };
}  // End namespace Internal


//////////////////////////////////////////////////////////////////////////
/////////////treeview/////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

treeViewsActions::treeViewsActions(QWidget *parent):QTreeView(parent){
    m_deleteThesaurusValue = new QAction(trUtf8("Delete this value."),this);
    m_choosePreferedValue = new QAction(trUtf8("Choose this value like the preferred."),this);
    m_userUuid = user()->uuid();
    connect(m_choosePreferedValue,SIGNAL(triggered(bool)),this,SLOT(choosePreferedValue(bool)));
    connect(m_deleteThesaurusValue,SIGNAL(triggered(bool)),this,SLOT(deleteBox(bool)));
    connect(user(), SIGNAL(userChanged()), this, SLOT(userIsChanged()));
    }
    
treeViewsActions::~treeViewsActions(){}

void treeViewsActions::userIsChanged(){
    m_userUuid = user()->uuid();
    if (!fillActionTreeView())
        {
        	qWarning()  << __FILE__ << QString::number(__LINE__) << "index is not valid";
        }
}

/*void treeViewsActions::mousePressEvent(QMouseEvent *event){
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " in  tree clicked" ;
    if(event->button() == Qt::RightButton){
        if(isChildOfThesaurus()){
            blockSignals(true);
            if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " in treeview right button " ;
            m_menuRightClic = new QMenu(this); 
            m_menuRightClic -> addAction(m_choosePreferedValue);
            m_menuRightClic-> addAction(m_deleteThesaurusValue);
            m_menuRightClic->exec(event->globalPos());
            blockSignals(false);
        }

    }
    if(event->button() == Qt::LeftButton){
            if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " in left press button " ;
            blockSignals(false);
            QTreeView::mousePressEvent(event);
    }
}*/

void treeViewsActions::mouseReleaseEvent(QMouseEvent *event){
    if (WarnDebugMessage)
        qDebug() << __FILE__ << QString::number(__LINE__) << " in  tree clicked" ;
    if(event->button() == Qt::RightButton){
        if(isChildOfThesaurus()){
            blockSignals(true);
            if (WarnDebugMessage)
                qDebug() << __FILE__ << QString::number(__LINE__) << " in treeview release right button " ;
            m_menuRightClic = new QMenu(this);
            m_menuRightClic -> addAction(m_choosePreferedValue);
            m_menuRightClic-> addAction(m_deleteThesaurusValue);
            m_menuRightClic->exec(event->globalPos());
            blockSignals(false);
        }

    }
    if(event->button() == Qt::LeftButton) {
        if (WarnDebugMessage)
            qDebug() << __FILE__ << QString::number(__LINE__) << " in left button " ;
        blockSignals(false);
        QTreeView::mouseReleaseEvent(event);
        
    }
}

void treeViewsActions::deleteBox(bool b){
    Q_UNUSED(b);
    QMessageBox msgBox;
    msgBox.setText("Do you want to delete chosen item ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    QModelIndex index;
    switch(ret){
    case QMessageBox::Yes :
        index = currentIndex();
        deleteItemFromThesaurus(index);
        break;
    case QMessageBox::No :
        break;
    }
}

void treeViewsActions::choosePreferedValue(bool b){
    Q_UNUSED(b);
    qDebug() << __FILE__ << QString::number(__LINE__) << "in choosePreferedValue";
    QMessageBox msgBox;
    msgBox.setText("Do you want to choose this item as preferred value ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    QModelIndex index;
    switch(ret){
    case QMessageBox::Yes :
        index = currentIndex();
        if (!addPreferedItem(index))
        {
            QMessageBox::warning(0,trUtf8("Warning"),trUtf8("Unable to choose this item."),
                                 QMessageBox::Ok);
        }
        break;
    case QMessageBox::No :
        break;
    }
} 

bool treeViewsActions::addPreferedItem(QModelIndex &index){
    bool ret = true;
    QString data = index.data().toString();
    receiptsEngine r;
    if (!r.addBoolTrue(data))
    {
        QMessageBox::warning(0,trUtf8("Warning"),trUtf8("Cannot change value bool in thesaurus :")+data,
                             QMessageBox::Ok);
        ret = false;
    }
    fillActionTreeView();
    return ret;
}

bool treeViewsActions::isChildOfThesaurus() {
    bool ret = false;
    QModelIndex current = currentIndex();
    QModelIndex indexParent = treeModel()->parent(current);
    QString dataParent = treeModel()->data(indexParent).toString();
    QStringList valuesOfThesaurus = m_mapSubItems.values("Thesaurus");
    if (WarnDebugMessage)
        qDebug() << __FILE__ << QString::number(__LINE__) << " dataParent =" << dataParent ;
    if (dataParent .contains(trUtf8("Thesaurus"))|| valuesOfThesaurus.contains(dataParent)) {
        ret = true;
    }
    return ret;
}

bool treeViewsActions::fillActionTreeView()
{
    bool b = true;
    m_actionsTreeModel = new QStandardItemModel;
    QStringList listOfMainActions;
    QMap<QString,QString> parametersMap;
    parametersMap.insert(tr("Thesaurus"),"thesaurus");
    parametersMap.insert(tr("Values"),"values");
    parametersMap.insert(tr("Preferred Value"),"Preferred Value");
    parametersMap.insert(tr("Round trip"),"Round trip");

    listOfMainActions = parametersMap.keys();
    //insert items from tables if available
    //QMap<QString,QString> m_mapSubItems;
    m_mapSubItems.clear();
    receiptsManager manager;
    QString strKeysParameters;
    foreach(strKeysParameters,listOfMainActions){
        QString table = parametersMap.value(strKeysParameters);
        if (WarnDebugMessage)
        qDebug() << __FILE__ << QString::number(__LINE__) << "table" << table;
        QStringList listOfItemsOfTable;
        listOfItemsOfTable = manager.getParametersDatas(m_userUuid,table).keys();//QHash<QString,QVariant> name,uid
        QString strItemsOfTable;
        foreach(strItemsOfTable,listOfItemsOfTable){
            m_mapSubItems.insertMulti(strKeysParameters,strItemsOfTable);
        }
        //default values if unavailables :
        if (listOfItemsOfTable.size()<1) {
            if (WarnDebugMessage) {
                qDebug() << __FILE__ << QString::number(__LINE__) << "listOfItemsOfTable.size()<1"  ;
                qDebug() << __FILE__ << QString::number(__LINE__) << "strKeysParameters  =" << strKeysParameters ;
            }
            if (strKeysParameters == tr("Thesaurus")) {
                if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " in thesaurus " ;
                m_mapSubItems.insertMulti(tr("Thesaurus"),"CS");
                m_mapSubItems.insertMulti(tr("Thesaurus"),"V");
            }
            else {
                if (WarnDebugMessage)
                    qWarning() << __FILE__ << QString::number(__LINE__)
                               << " No default value for "<< strKeysParameters ;
            }
        }
    }


    QStandardItem *parentItem = treeModel()->invisibleRootItem();
    if (!parentItem->index().isValid()){
        if (WarnDebugMessage)
            qWarning() << __FILE__ << QString::number(__LINE__) << "parentItem is not valid";
    }
    QString strMainActions;
    /*QMap<int,QStandardItem*>*/ m_mapOfMainItems.clear();
    foreach(strMainActions,listOfMainActions) {
        if (WarnDebugMessage)
            qDebug() << __FILE__ << QString::number(__LINE__) << " strMainActions =" << strMainActions ;
        QStandardItem *actionItem = new QStandardItem(strMainActions);
        actionItem->setEditable(false);
        actionItem->setEnabled(true);
        int row = 0;
        //treeViewsActions colors
        if (strMainActions == tr("Preferred Value")) {
            QBrush red(Qt::red);
            actionItem->setForeground(red);
            m_mapOfMainItems.insert(PREFERENTIAL_VALUE,actionItem);
            row = 0;
        } else if (strMainActions == tr("Thesaurus")) {
            QBrush red(Qt::red);
            actionItem->setForeground(red);
            m_mapOfMainItems.insert(THESAURUS,actionItem);
            row = 1;
        } else if (strMainActions == tr("Values")) {
            QBrush blue(Qt::blue);
            actionItem->setForeground(blue);
            m_mapOfMainItems.insert(VALUES,actionItem);
            row = 2;
        } else if (strMainActions == tr("Round trip")) {
            QBrush blue(Qt::blue);
            actionItem->setForeground(blue);
            m_mapOfMainItems.insert(ROUND_TRIP,actionItem);
            row = 3;
        } else {
            if (WarnDebugMessage)
                qWarning() << __FILE__ << QString::number(__LINE__) << "Error color treeViewsActions." ;
        }
        
        if (WarnDebugMessage)
            qDebug() << __FILE__ << QString::number(__LINE__) << QString::number(row);
        
        }//
        
        for (int i = 0; i < rows_MaxParam; i += 1)
        {
        	QStandardItem *actionItem = m_mapOfMainItems.value(i);
        	treeModel()->insertRow(i,actionItem);
                QStringList listSubActions;
                listSubActions = m_mapSubItems.values(actionItem->text());
                if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << "listSubActions" << QString::number(listSubActions.size())
                << " " << actionItem->text();
                QString strSubActions;
                foreach(strSubActions,listSubActions){
                    if (WarnDebugMessage)
                        qDebug() << __FILE__ << QString::number(__LINE__) << " strSubActions =" << 
                         strSubActions ;
                    QStandardItem *subActionItem = new QStandardItem(strSubActions);
                    actionItem->appendRow(subActionItem);
                    if (!subActionItem->index().isValid()) {
                        if (WarnDebugMessage)
                            qWarning() << __FILE__ << QString::number(__LINE__) << "subActionItem is not valid";
                        b = false;
                    }
                }
        }//end of  actionItem
    if (WarnDebugMessage)
            qDebug() << __FILE__ << QString::number(__LINE__)  ;
    setHeaderHidden(true);
    setSortingEnabled ( false );
    setStyleSheet("background-color: rgb(201, 201, 201)");
    // actionsTreeView->setStyleSheet("foreground-color: red");
    setModel(treeModel());
    if (WarnDebugMessage)
            qDebug() << __FILE__ << QString::number(__LINE__) << "ACTION TREEVIEW FILLED UP"  ;
    return b;
}

bool treeViewsActions::deleteItemFromThesaurus(QModelIndex &index){
    bool ret = true;
    QString data = index.data().toString();
    const QString userUid = user()->uuid();
    receiptsEngine r;
    if (!r.deleteFromThesaurus(data,userUid)) {
        QMessageBox::warning(0,trUtf8("Warning"),trUtf8("Cannot delete in thesaurus :")+data,QMessageBox::Ok);
        ret = false;
    }
    fillActionTreeView();
    return ret;
}

void treeViewsActions::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    if (e->type()==QEvent::LanguageChange) {
        delete m_actionsTreeModel;
            if (WarnDebugMessage)
                qDebug() << __FILE__ << QString::number(__LINE__) << " langage changed " ;
        if (!fillActionTreeView()) {
            if (WarnDebugMessage)
                qWarning() << __FILE__ << QString::number(__LINE__) << "index is not valid";
            }
        reset();
        }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////RECEIPTS VIEWER/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ReceiptViewer::ReceiptViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceiptViewer)
{
    if (user())
        m_userUuid = user()->uuid();
    receiptsManager rManager;
    m_kilometers = 0.00 ;
    m_distanceRuleValue = 0.00;
    m_insuranceUid = 0;
    m_model = new InternalAmount::AmountModel(this);
    ui->setupUi(this);
    ui->saveAndQuitButton->hide();
    ui->quitButton->hide();
    ui->amountsView->setShowGrid(false);
    /*ui->amountsView->verticalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->amountsView->verticalHeader()->setDefaultSectionSize(10);
    ui->amountsView->verticalHeader()->setDefaultAlignment(Qt::AlignTop);*/
    ui->amountsView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->amountsView->verticalHeader()->setStretchLastSection ( false );
    ui->amountsView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->amountsView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->amountsView->horizontalHeader()->setCascadingSectionResizes (true);
    //ui->amountsView->horizontalHeader()->setStretchLastSection ( true );
    m_model->setHeaderData(int(Cash),Qt::Horizontal,tr("Cash"));
    m_model->setHeaderData(Check,Qt::Horizontal,tr("Check"));
    m_model->setHeaderData(Visa,Qt::Horizontal,tr("Visa"));
    m_model->setHeaderData(Banking,Qt::Horizontal,tr("Banking"));
    m_model->setHeaderData(Other,Qt::Horizontal,tr("Other"));
    m_model->setHeaderData(Due,Qt::Horizontal,tr("Du"));

    ui->amountsView->setModel(m_model);
    ui->amountsView->setItemDelegateForColumn(Cash, new Utils::SpinBoxDelegate(this,0.00,100.00,true));
    ui->amountsView->setItemDelegateForColumn(Check, new Utils::SpinBoxDelegate(this,0.00,100.00,true));
    ui->amountsView->setItemDelegateForColumn(Visa, new Utils::SpinBoxDelegate(this,0.00,100.00,true));
    ui->amountsView->setItemDelegateForColumn(Banking, new Utils::SpinBoxDelegate(this,0.00,100.00,true));
    ui->amountsView->setItemDelegateForColumn(Other, new Utils::SpinBoxDelegate(this,0.00,100.00,true));
    ui->amountsView->setItemDelegateForColumn(Due, new Utils::SpinBoxDelegate(this,0.00,100.00,true));
    ui->amountsView->setColumnHidden(InternalAmount::AmountModel::Col_Debtor,true);
    ui->amountsView->setColumnHidden(InternalAmount::AmountModel::Col_Site,true);
    ui->amountsView->setColumnHidden(InternalAmount::AmountModel::Col_DistRule,true);
       
    ui->amountsView->resizeRowsToContents();
    
    ui->dateExecution->setDisplayFormat("yyyy-MM-dd");
    ui->dateExecution->setDate(QDate::currentDate());
    ui->datePayment->setDisplayFormat("yyyy-MM-dd");
    ui->datePayment->setDate(QDate::currentDate());
    /*ui->dateBanked->setDisplayFormat("yyyy-MM-dd");
    ui->dateBanked->setDate(QDate::currentDate());
    ui->dateBook->setDisplayFormat("yyyy-MM-dd");
    ui->dateBook->setDate(QDate::currentDate());*/
    ui->inputRadioButton->setChecked(true);
    ui->saveButton->setShortcut(QKeySequence::InsertParagraphSeparator);
    ui->quitButton->setShortcut(QKeySequence("Ctrl+q"));
    ui->thesaurusButton->setShortcut(QKeySequence("Ctrl+t"));
    ui->returnedListView->setStyleSheet("background-color: rgb(201, 201, 201)");
    
    m_modelReturnedList = new QStringListModel;
    ui->returnedListView->setModel(m_modelReturnedList);
    ui->returnedListView->setEnabled(true);
    ui->returnedListView->show();
    m_actionTreeView = new treeViewsActions(this);
    m_vbox = new QVBoxLayout;
    m_vbox->addWidget(m_actionTreeView);
    ui->actionsBox->setLayout(m_vbox);
    if (!m_actionTreeView->fillActionTreeView()) {
        if (WarnDebugMessage)
                qWarning() << __FILE__ << QString::number(__LINE__) << "index is not valid";
        }
    
    //preferential choices in the tree view.
    QString site = QString("Sites");
    QString distRule = QString("Distance rules");
    QString debtor = QString("Debtor");
    m_siteUid = firstItemChoosenAsPreferential(site);
    m_distanceRuleValue = firstItemChoosenAsPreferential(distRule).toDouble();
    m_distanceRuleType = rManager.getPreferedDistanceRule().toString();
    m_insuranceUid = firstItemChoosenAsPreferential(debtor);
    if (WarnDebugMessage)
        qDebug() << __FILE__ << QString::number(__LINE__)
                 << " site,dist,ins preferred =" << m_siteUid.toString()
                 << QString::number(m_distanceRuleValue)
                 << m_insuranceUid.toString() ;
    
    //right click
    m_clear = new QAction(trUtf8("Clear all."),this);
    //ui_controlreceipts
    m_control = new ControlReceipts(this);
    m_control->hide();
    ui->othersBox->hide();
    ui->othersLabel->hide();
    ui->displayRadioButton->setCheckable(true);
    connect(ui->quitButton,SIGNAL(pressed()),this,SLOT(close()));
    connect(ui->saveButton,SIGNAL(pressed()),this,SLOT(save()));
    //connect(ui->othersBox,SIGNAL(activated(const QString&)),this,SLOT(othersWidgets(const QString&)));
    connect(ui->saveAndQuitButton,SIGNAL(pressed()),this,SLOT(saveAndQuit()));
    connect(ui->thesaurusButton,SIGNAL(pressed()),this,SLOT(saveInThesaurus()));
    connect(ui->displayRadioButton,SIGNAL(clicked(bool)),this,SLOT(showControlReceipts(bool)));
    if (!connect(m_actionTreeView,SIGNAL(clicked(const QModelIndex&)),this,SLOT(actionsOfTreeView(const QModelIndex&))))
    {
        if (WarnDebugMessage)
            qWarning() << __FILE__ << QString::number(__LINE__) << "unable to connect m_actionTreeView";
    }
    
    connect(m_clear,SIGNAL(triggered(bool)),this,SLOT(clearAll(bool)));
    connect(m_control,SIGNAL(isClosing()),this,SLOT(controlReceiptsDestroyed()));
    connect(user(), SIGNAL(userChanged()), this, SLOT(userUid()));
    
}

ReceiptViewer::~ReceiptViewer()
{
    delete ui;
}

void ReceiptViewer::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        delete m_actionTreeView;
        delete m_vbox;
        if (WarnDebugMessage)
            qDebug() << __FILE__ << QString::number(__LINE__) << " in  ReceiptViewer::changeEvent(QEvent *e)"  ;
        //treeViewsActions
        m_actionTreeView = new treeViewsActions(this);
        m_vbox = new QVBoxLayout;
        m_vbox->addWidget(m_actionTreeView);
        ui->actionsBox->setLayout(m_vbox);
        if (!m_actionTreeView->fillActionTreeView()) {
            if (WarnDebugMessage)
                qWarning()  << __FILE__ << QString::number(__LINE__) << "index is not valid";
        }
        if (!connect(m_actionTreeView,SIGNAL(clicked(const QModelIndex&)),this,SLOT(actionsOfTreeView(const QModelIndex&)))) {
            if (WarnDebugMessage)
                qWarning() << __FILE__ << QString::number(__LINE__) << "unable to connect m_actionTreeView";
        }
        //amountsView
        m_model->setHeaderData(int(Cash),Qt::Horizontal,tr("Cash"));
        m_model->setHeaderData(Check,Qt::Horizontal,tr("Check"));
        m_model->setHeaderData(Visa,Qt::Horizontal,tr("Visa"));
        m_model->setHeaderData(Banking,Qt::Horizontal,tr("Banking"));
        m_model->setHeaderData(Other,Qt::Horizontal,tr("Other"));
        m_model->setHeaderData(Due,Qt::Horizontal,tr("Du"));
        break;
    default:
        break;
    }
}

void ReceiptViewer::setPosition(QWidget *parent){
    QRect rect = parent->rect();
    setGeometry(parent->x(),parent->y(),rect.width()-10,rect.height()-10);
}

void ReceiptViewer::deleteLine()
{
    QMessageBox::information(0,"try","delete line",QMessageBox::Ok);
}



void ReceiptViewer::actionsOfTreeView(const QModelIndex & index) {
    if (WarnDebugMessage)
        qDebug() << __FILE__ << QString::number(__LINE__) << "in actionsOfTreeView(const QModelIndex& index) ";
    QString data = index.data(Qt::DisplayRole).toString();
    if (WarnDebugMessage)
        qDebug() << __FILE__ << QString::number(__LINE__) << " DATA =" << data;
    receiptsManager manager;
    QHash<QString,QString> hashOfValues;
    int typeOfPayment = ReceiptsConstants::Cash;
    double percentage = 100.00;
    QVariant  debtor;
    QVariant site;
    QVariant distrules;
    if(index.row() == VALUES && index.parent() == QModelIndex() ){ //values
        findReceiptsValues *rv = new findReceiptsValues(this);
        if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " in findReceiptsValues "  ;
        if(rv->exec() == QDialog::Accepted) {
            hashOfValues = rv -> getChoosenValues();
            choiceDialog choice(rv,false);
            if(hashOfValues.keys().size() > 0){
                if(choice.exec() == QDialog::Accepted){
                    QStandardItemModel * model = choice.getChoicePercentageDebtorSiteDistruleModel();
                    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
                        typeOfPayment = model->data(model->index(i,choice.TYPE_OF_CHOICE),Qt::DisplayRole).toInt();
                        percentage = model->data(model->index(i,choice.PERCENTAGE),Qt::DisplayRole).toDouble();
                        debtor = model->data(model->index(i,choice.DEBTOR),Qt::DisplayRole);
                        site = model->data(model->index(i,choice.SITE),Qt::DisplayRole);
                        distrules = model->data(model->index(i,choice.DISTRULES),Qt::DisplayRole);
                       /*QStringList*/ m_listOfValues << hashOfValues.keys();
                       m_listOfValues.removeDuplicates();
                       m_modelReturnedList->setStringList(m_listOfValues);
                       fillModel(hashOfValues,typeOfPayment,percentage,debtor,site,distrules,i);
                     }
                     delete model;
                 }
               }
             }
         }
    if(index.row() == PREFERENTIAL_VALUE && index.parent() == QModelIndex()){// preferential act of payment
        if (WarnDebugMessage)
            	qDebug() << __FILE__ << QString::number(__LINE__) << " PREFERENTIAL_VALUE";
        choiceDialog choice(this,false);
        if(choice.exec() == QDialog::Accepted){
            QStandardItemModel * model = choice.getChoicePercentageDebtorSiteDistruleModel();
            if (WarnDebugMessage)
            	qDebug() << __FILE__ << QString::number(__LINE__) << " model->rowCount() =" << QString::number(model->rowCount()) ;

            for (int i = 0; i < model->rowCount(); ++i) {
                QHash<QString,QString> hashOfValues;
                const QString userUuid = m_userUuid;
                typeOfPayment = model->data(model->index(i,choice.TYPE_OF_CHOICE),Qt::DisplayRole).toInt();
                if (WarnDebugMessage)
                      qDebug() << __FILE__ << QString::number(__LINE__) << " typeOfPayment =" << QString::number(typeOfPayment) ;
                
                percentage = model->data(model->index(i,choice.PERCENTAGE),Qt::DisplayRole).toDouble();
                debtor = model->data(model->index(i,choice.DEBTOR),Qt::DisplayRole);
                if (WarnDebugMessage)
    	              qDebug() << __FILE__ << QString::number(__LINE__) << " debtor =" << debtor.toString() ;
                site = model->data(model->index(i,choice.SITE),Qt::DisplayRole);
                distrules = model->data(model->index(i,choice.DISTRULES),Qt::DisplayRole);
                if (WarnDebugMessage)
    	               qDebug() << __FILE__ << QString::number(__LINE__) << " preferred value =" << data ;
                hashOfValues = manager.getPreferentialActFromThesaurus(userUuid);
                QString preferredAct = hashOfValues.keys()[0] ;
                if (WarnDebugMessage)
    	               qDebug() << __FILE__ << QString::number(__LINE__) << " preferential acts =" << preferredAct;
                if (preferredAct == "NULL")
                {
                	  qWarning() << __FILE__ << QString::number(__LINE__) << "preferredAct == NULL" ;
                	  return;
                    }
                if (hashOfValues.size() < 1)
                {
            	    hashOfValues.insertMulti("NULL","0");//preferential act
            	    QMessageBox::warning(0,trUtf8("Warning"),trUtf8("You have to insert your preferred "
            	                 	  "value\nin thesaurus\nand choose it as preferred."),QMessageBox::Ok);
                    }           
                m_listOfValues << hashOfValues.keys();
                m_listOfValues.removeDuplicates();
                m_modelReturnedList->setStringList(m_listOfValues);
                fillModel(hashOfValues,typeOfPayment,percentage,debtor,site,distrules,i);
                }
            delete model;
            }
        }
        
    if (manager.getDistanceRules().keys().contains(data))
    {
    	  if (WarnDebugMessage)
            	qDebug() << __FILE__ << QString::number(__LINE__) << " in getDistanceRules";
    	  m_distanceRuleValue = manager.getDistanceRules().value(data).toDouble();
    	  m_distanceRuleType = data;
    	  if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " m_distanceRuleValue =" << QString::number(m_distanceRuleValue) ;
    	  if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " m_distanceRuleType =" << m_distanceRuleType ;
        }
    if (index.row() == ROUND_TRIP && index.parent() == QModelIndex())
    {
    	  if (WarnDebugMessage)
            	qDebug() << __FILE__ << QString::number(__LINE__) << " in ROUND_TRIP";
    	  choiceDialog dist(this,true);
    	  if (dist.exec()== QDialog::Accepted)
    	  {
              QStandardItemModel * model = dist.getChoicePercentageDebtorSiteDistruleModel();
              for (int i = 0; i < model->rowCount(); i += 1)
              {
    	          QHash<QString,QString> hashOfValues;
    	          m_kilometers = dist.getDistanceNumber(m_distanceRuleType);
                  double value = m_kilometers *m_distanceRuleValue;
                  if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " m_distanceRuleValue =" << QString::number(m_distanceRuleValue) ;
                  if (m_distanceRuleValue == 0.00)
                  {
                  	  qWarning() << __FILE__ << QString::number(__LINE__) << "no m_distanceRuleValue available" ;
                  	  const QString information = trUtf8("No distance rule value available !\n"
                  	                                     "You should create one in Preferences.");
                  	  QMessageBox::warning(0,trUtf8("Warning"),information,QMessageBox::Ok);
                  	  return;
                      }
                  typeOfPayment = model->data(model->index(i,dist.TYPE_OF_CHOICE),Qt::DisplayRole).toInt();
                  percentage = model->data(model->index(i,dist.PERCENTAGE),Qt::DisplayRole).toDouble();
                  debtor = model->data(model->index(i,dist.DEBTOR),Qt::DisplayRole);
                  site = model->data(model->index(i,dist.SITE),Qt::DisplayRole);
                  distrules = model->data(model->index(i,dist.DISTRULES),Qt::DisplayRole);;
    	  	  hashOfValues.insertMulti("DistPrice",QString::number(value));
    	          if (WarnDebugMessage)
    	          qDebug() << __FILE__ << QString::number(__LINE__) << " distance =" << QString::number(m_kilometers) ;
    	          if (WarnDebugMessage)
    	          qDebug() << __FILE__ << QString::number(__LINE__) << " value =" << QString::number(value) ;
    	          m_listOfValues << trUtf8("Kilometers");
    	          m_listOfValues.removeDuplicates();
                  m_modelReturnedList->setStringList(m_listOfValues);
    	          fillModel(hashOfValues,typeOfPayment,percentage,debtor,site,distrules,i);
    	          }
    	          delete model;
    	      }
    	  
        }
   /* if (manager.getHashOfSites().keys().contains(data))
    {
    	  m_siteUid = manager.getHashOfSites().value(data);
    	  if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " m_siteUid =" << m_siteUid.toString() ;
        }*/
    /*if (manager.getHashOfInsurance().keys().contains(data))
    {
    	  m_insuranceUid = manager.getHashOfInsurance().value(data);
    	  if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " m_insuranceUid =" << m_insuranceUid.toString() ;
        }*/
    if (manager.getHashOfThesaurus().keys().contains(data))
    {
        if (WarnDebugMessage)
                qDebug() << __FILE__ << QString::number(__LINE__) << " IN THESAURUS " ;
        choiceDialog choice(this,false);
        if(choice.exec() == QDialog::Accepted){
            QStandardItemModel * model = choice.getChoicePercentageDebtorSiteDistruleModel();
            for (int i = 0; i < model->rowCount(); i += 1)
            {
                QHash<QString,QString> hashOfValues;
                if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " IN THESAURUS FOR "  ;
                typeOfPayment = model->data(model->index(i,choice.TYPE_OF_CHOICE),Qt::DisplayRole).toInt();
                percentage = model->data(model->index(i,choice.PERCENTAGE),Qt::DisplayRole).toDouble();
                debtor = model->data(model->index(i,choice.DEBTOR),Qt::DisplayRole);
                site = model->data(model->index(i,choice.SITE),Qt::DisplayRole);
                distrules = model->data(model->index(i,choice.DISTRULES),Qt::DisplayRole);
                QStringList list;
                list = data.split("+");
                receiptsEngine r;
                QHash<QString,double> hashFromMp ;
                const QString field = "NAME";
                QString str;
                foreach(str,list){
                    if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " str =" << str ;
                    if (str .contains( trUtf8("thesaurus")))
                    {
                    	  qWarning() << __FILE__ << QString::number(__LINE__) << "no thesaurus value available" ;
                    	  return;
                        }
                    hashFromMp = r.getFilteredValueFromMedicalProcedure(str,field);
                    if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " hashFromMp.size() " << QString::number(hashFromMp.size());
                    QString value = QString::number(hashFromMp.value(str));
                    if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << value;
                    if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << QString::number(percentage);
                    hashOfValues.insertMulti(str,value);
                    if (WarnDebugMessage)
                    qDebug() << __FILE__ << QString::number(__LINE__) << " hashOfValues.size() in getHashOfThesaurus " << QString::number(hashOfValues.size());
                    m_listOfValues << str;
                    }
                m_listOfValues.removeDuplicates();
                m_modelReturnedList->setStringList(m_listOfValues);
                fillModel(hashOfValues,typeOfPayment,percentage,debtor,site,distrules,i);
                }
                delete model;
            }
        }
        //actionTreeView->reset();
}

void ReceiptViewer::fillModel(QHash<QString,QString> &hashOfValues, 
                              int typeOfPayment, 
                              double percentage,
                              const QVariant & debtor, 
                              const QVariant & site, 
                              const QVariant & distrules,
                              const int row){
    Q_UNUSED(row);
    int rowOfAmountModel = 0;
    rowOfAmountModel = m_model->rowCount(QModelIndex());
    if (WarnDebugMessage)
    qDebug() << __FILE__ << QString::number(__LINE__) << "m_model->rowCount()  =" << QString::number(rowOfAmountModel) ;
    double value = 0.00;
    QHashIterator<QString,QString> it(hashOfValues);
    while(it.hasNext()){
        it.next();
        value += it.value().toDouble();
        if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << QString::number(value);
    }
    value = value*percentage/100.00;
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " values =" << QString::number(value);
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " row =" << QString::number(row)  ;
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " column =" << QString::number(typeOfPayment) ;
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " debtor =" << debtor.toString() ;
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " value =" << QString::number(value) ;
    if (!m_model->insertRows(row,1,QModelIndex()))
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << "unable to insert row = "+QString::number(row) ;
        }
    QModelIndex indexValue = m_model->index(rowOfAmountModel, typeOfPayment);
    QModelIndex indexDebtor = m_model->index(rowOfAmountModel, InternalAmount::AmountModel::Col_Debtor);
    QModelIndex indexSite = m_model->index(rowOfAmountModel, InternalAmount::AmountModel::Col_Site);
    QModelIndex indexDistrules = m_model->index(rowOfAmountModel, InternalAmount::AmountModel::Col_DistRule);
    //header vertical is debtor
    m_model->setHeaderData(rowOfAmountModel,Qt::Vertical,debtor,Qt::EditRole);
    if (!m_model->setData(indexValue, value, Qt::EditRole))
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << "unable to setData" ;
        }
    if (!m_model->setData(indexDebtor, debtor, Qt::EditRole))
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << "unable to setData" ;
        }
    if (!m_model->setData(indexSite, site, Qt::EditRole))
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << "unable to setData" ;
        }
    if (!m_model->setData(indexDistrules, distrules , Qt::EditRole))
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << "unable to setData" ;
        }
    m_model->submit();
}

void ReceiptViewer::save()
{
    using namespace ::Internal;
    receiptsEngine rIO;
    QString userUuid = user()->uuid();
    QString textOfListOfActs = m_listOfValues.join("+");
    for (int row = 0; row < m_model->rowCount(QModelIndex()); row += 1)
    {
        double cash = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Cash)).toDouble();
        double cheque = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Cheque)).toDouble();
        double visa = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Visa)).toDouble();
        double banking = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Banking)).toDouble();
        double other = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Other)).toDouble();
        double due = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Du)).toDouble();
        QVariant insurance = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Debtor));
        QVariant insuranceUid = rIO.getInsuranceUidFromInsurance(insurance.toString());
        QVariant site = m_model->data(m_model->index(row,InternalAmount::AmountModel::Col_Site));
        QVariant siteUid = rIO.getSiteUidFromSite(site.toString());
    
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " values =" << QString::number(cash)+ " "
                                                                     << QString::number(cheque)+ " "
                                                                     << QString::number(visa)+ " "
                                                                     << QString::number(banking)+ " "
                                                                     << QString::number(other)+ " "
                                                                     << QString::number(due) 
                                                                     << "site uid = "+siteUid.toString()
                                                                     << "insurance uid = "+insuranceUid.toString();
    QString patientUid = patient()->uuid();
    if (patientUid.isEmpty())
    {
    	  patientUid = "no-patient-uid";
        }
    QString patientName = patient()->data(Core::IPatient::FullName).toString();
    if (patientName.isEmpty())
    {
    	  patientName = "Patient Name";
        }

    QHash<int,QVariant> hash;
    hash.insert(ACCOUNT_UID,"UID");
    hash.insert(ACCOUNT_USER_UID,userUuid);
    hash.insert(ACCOUNT_PATIENT_UID,patientUid);
    hash.insert(ACCOUNT_PATIENT_NAME,patientName);
    hash.insert(ACCOUNT_SITE_ID,siteUid);//AccountDB::Constants::SITES_UID
    hash.insert(ACCOUNT_INSURANCE_ID,insuranceUid);
    hash.insert(ACCOUNT_DATE,ui->dateExecution->date()/*.toString("yyyy-MM-dd")*/);
    hash.insert(ACCOUNT_MEDICALPROCEDURE_XML, QVariant());
    hash.insert(ACCOUNT_MEDICALPROCEDURE_TEXT,textOfListOfActs);
    hash.insert(ACCOUNT_COMMENT, QVariant());
    hash.insert(ACCOUNT_CASHAMOUNT,cash);
    hash.insert(ACCOUNT_CHEQUEAMOUNT,cheque);
    hash.insert(ACCOUNT_VISAAMOUNT,visa);
    hash.insert(ACCOUNT_INSURANCEAMOUNT,banking);
    hash.insert(ACCOUNT_OTHERAMOUNT,other);
    hash.insert(ACCOUNT_DUEAMOUNT,due);
    hash.insert(ACCOUNT_DUEBY,insurance);
    hash.insert(ACCOUNT_ISVALID,0);
    hash.insert(ACCOUNT_TRACE, QVariant());
    receiptsEngine r;
    if (!r.insertIntoAccount(hash,userUuid)) {
        QMessageBox::warning(0,trUtf8("Warning"),trUtf8("Error inserting into AccountModel!"),QMessageBox::Ok);
    }
    }
    clearAll(true);
}

void ReceiptViewer::saveAndQuit()
{
    save();
    close();
}

void ReceiptViewer::mousePressEvent(QMouseEvent *event){
  if(event->button() == Qt::RightButton){
    if (WarnDebugMessage)
    	      qDebug() << "in right clic" << __FILE__ << QString::number(__LINE__) ;
    m_menu         = new QMenu(this);
    m_menu        -> addAction(m_clear);
    //m_menu        -> exec(QCursor::pos());
    //m_menu->exec(ui->returnedListView->mapToGlobal(QPoint(0, 0)));
    m_menu->exec(event->globalPos());
  }
}
  
void ReceiptViewer::saveInThesaurus(){
    QString listOfValuesStr = m_listOfValues.join("+");
    receiptsEngine r;
    if(r.insertInThesaurus(listOfValuesStr,m_userUuid)){
        QMessageBox::information(0,trUtf8("Information"),listOfValuesStr+"\n"+trUtf8(" has been saved in thesaurus !"),QMessageBox::Ok);
        }
    m_actionTreeView->fillActionTreeView();
}

void ReceiptViewer::clearAll(bool b)
{
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " in clearAll ";
    if (b==false) {
        qWarning() << __FILE__ << QString::number(__LINE__) << "Clear all is uncheckable." ;
    }
    m_listOfValues.clear();
    m_modelReturnedList->removeRows(0,m_modelReturnedList->rowCount(),QModelIndex());
    //clear accountmodel
    m_model->removeRows(0,m_model->rowCount(QModelIndex()),QModelIndex());
}

QVariant ReceiptViewer::firstItemChoosenAsPreferential(QString & item)
{
    QVariant variantValue = QVariant("No item");
    receiptsManager manager;
    if (item == "Distance rules")
    {
    	  variantValue = manager.m_preferedDistanceValue;
        }
    if (manager.getHashOfSites().keys().contains(item))
    {
    	  variantValue = manager.m_preferedSite;
        }
    if (manager.getHashOfInsurance().keys().contains(item))
    {
    	  variantValue = manager.m_preferedInsurance;
        }
    return variantValue;
}

void ReceiptViewer::showControlReceipts(bool b){
    if (b)
    {
    	  m_control->show();
        }
}

void ReceiptViewer::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event);
    m_control->resizeControlReceipts(this);
}

void ReceiptViewer::controlReceiptsDestroyed(){
    if (WarnDebugMessage)
    	      qDebug() << __FILE__ << QString::number(__LINE__) << " in controlReceiptsDestroyed " ;
    ui->inputRadioButton->setChecked(true);
}

void ReceiptViewer::userUid(){
    m_userUuid = user()->uuid();
}

/*void ReceiptViewer::createFirstTimeTxt(){
    qWarning() << __FILE__ << QString::number(__LINE__) << "create first time file." ;
    if (!m_fileFirstTime->open(QIODevice::WriteOnly))
    {
    	  qWarning() << __FILE__ << QString::number(__LINE__) << "m_fileFirstTime cannot be created." ;
        }
    m_fileFirstTime->close();
}*/

