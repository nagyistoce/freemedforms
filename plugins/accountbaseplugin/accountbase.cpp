/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                     **
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
#include "accountbase.h"
#include "constants.h"
#include "accountdata.h"

#include <utils/global.h>
#include <utils/log.h>

#include <coreplugin/isettings.h>
#include <coreplugin/icore.h>
#include <coreplugin/constants.h>

#ifdef FREEACCOUNT
#    include <coreplugin/commandlineparser.h>
#endif

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QFile>
#include <QDir>
#include <QMultiHash>
#include <QMap>
#include <QMultiMap>
#include <QList>
#include <QSet>

using namespace AccountDB;
using namespace AccountDB::Internal;
using namespace Trans::ConstantTranslations;


static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }
static inline Core::CommandLine *commandLine()  { return Core::ICore::instance()->commandLine(); }

static inline bool connectDatabase(QSqlDatabase &DB, const QString &file, const int line)
{
    if (!DB.isOpen()) {
        if (!DB.open()) {
            Utils::Log::addError("DrugsBase", tkTr(Trans::Constants::UNABLE_TO_OPEN_DATABASE_1_ERROR_2)
                                 .arg(DB.connectionName()).arg(DB.lastError().text()),
                                 file, line);
            return false;
        }
    }
    return true;
}


namespace AccountDB {
namespace Internal {
/**
  \brief Private part of DrugsBase
  \internal
*/
class AccountBasePrivate
{
public:
    AccountBasePrivate(AccountBase *base) : q(base) {}
    ~AccountBasePrivate()
    {
    }

public:
    AccountBase *q;
    bool m_LogChrono;
};
}  // End Internal
}  // End AccountDB


//--------------------------------------------------------------------------------------------------------
//--------------------------------- Initialization of static members -------------------------------------
//--------------------------------------------------------------------------------------------------------
AccountBase *AccountBase::m_Instance = 0;
bool AccountBase::m_initialized = false;

//--------------------------------------------------------------------------------------------------------
//-------------------------------------- Initializing Database -------------------------------------------
//--------------------------------------------------------------------------------------------------------
/** \brief Returns the unique instance of DrugsBase. If it does not exists, it is created */
AccountBase *AccountBase::instance()
{
    if (!m_Instance) {
        m_Instance = new AccountBase(qApp);
        m_Instance->init();
    }
    return m_Instance;
}

/**
   \brief Constructor.
   \private
*/
AccountBase::AccountBase(QObject *parent)
    : QObject(parent), Utils::Database(), d(0)
{
    d = new AccountBasePrivate(this);
    setObjectName("AccountBase");

    using namespace AccountDB::Constants;
    addTable(Table_MedicalProcedure,  "medical_procedure");
    addTable(Table_BankDetails,       "bank_details");
    addTable(Table_Deposit,           "deposit");
    addTable(Table_Account,           "account");
    addTable(Table_Acts,              "acts");
    addTable(Table_Assets,            "assets");
    addTable(Table_AssetsRates,       "assets_rates");
    addTable(Table_Movement,          "movement");
    addTable(Table_AvailableMovement, "available_movement");
    addTable(Table_Insurance,         "insurance");
    addTable(Table_Sites,             "sites");
    addTable(Table_Users,             "users");
    addTable(Table_Percent,           "percentages");
    addTable(Table_Rules,             "rules");
    addTable(Table_DistanceRules,     "distance_rules");
    addTable(Table_Thesaurus,         "thesaurus");
//    addTable(Table_Session,           "session");
//    addTable(Table_Banking,           "banking");
//    addTable(Table_NameIndex,         "nameindex");

    addField(Table_MedicalProcedure, MP_ID,             "MP_ID",          FieldIsUniquePrimaryKey);
    addField(Table_MedicalProcedure, MP_UID,            "MP_UUID",        FieldIsUUID);
    addField(Table_MedicalProcedure, MP_USER_UID,       "MP_USER_UID",    FieldIsUUID);
    addField(Table_MedicalProcedure, MP_NAME,           "NAME",           FieldIsShortText);
    addField(Table_MedicalProcedure, MP_ABSTRACT,       "ABSTRACT",       FieldIsLongText);
    addField(Table_MedicalProcedure, MP_TYPE,           "TYPE",           FieldIsShortText);
    addField(Table_MedicalProcedure, MP_AMOUNT,         "AMOUNT",         FieldIsReal);
    addField(Table_MedicalProcedure, MP_REIMBOURSEMENT, "REIMBOURSEMENT", FieldIsReal);
    addField(Table_MedicalProcedure, MP_DATE,           "DATE",           FieldIsDate);
    /** Add a link to INSURANCE table ? One MP is reimburse by a specific insurance... */

//    "CREATE TABLE 	actes_disponibles ("  --> medical_procedure
//            "id_acte_dispo  int(10)  	UNSIGNED  	       		NOT NULL  	 auto_increment ,"
//            "nom_acte 	varchar(50) 	COLLATE utf8_unicode_ci 	NOT NULL  			,"
//            "desc_acte 	varchar(200) 	COLLATE utf8_unicode_ci 	NOT NULL  			,"
//            "type 		varchar(100) 	COLLATE utf8_unicode_ci 	NOT NULL 			,"
//            "montant_total 	double 						NOT NULL 			,"
//            "montant_tiers 	double 						NOT NULL 			,"
//            "date_effet	date						NOT NULL			,"
//            "PRIMARY KEY(id_acte_dispo));";

    addField(Table_BankDetails,  BANKDETAILS_ID,             "BD_ID",             FieldIsUniquePrimaryKey);
    /** \todo this field must be not null */
    addField(Table_BankDetails,  BANKDETAILS_USER_UID,       "BD_USER_UID",       FieldIsUUID);
    addField(Table_BankDetails,  BANKDETAILS_LABEL,          "BD_LABEL",          FieldIsShortText);
    addField(Table_BankDetails,  BANKDETAILS_OWNER,          "BD_OWNER",          FieldIsShortText);
    addField(Table_BankDetails,  BANKDETAILS_OWNERADRESS,    "BD_OWNERADRESS",    FieldIsShortText);
    addField(Table_BankDetails,  BANKDETAILS_ACCOUNTNUMBER,  "BD_ACCNUMB",        FieldIsShortText);
    addField(Table_BankDetails,  BANKDETAILS_IBAN,           "BD_IBAN",           FieldIsShortText);
    addField(Table_BankDetails,  BANKDETAILS_BALANCE,        "BD_BALANCE",        FieldIsReal);
    addField(Table_BankDetails,  BANKDETAILS_BALANCEDATE,    "BD_BALANCEDATE",    FieldIsDate, "CURRENT_DATE");
    addField(Table_BankDetails,  BANKDETAILS_COMMENT,        "BD_COMMENT",        FieldIsLongText);
    addField(Table_BankDetails,  BANKDETAILS_DEFAULT,        "BD_ISDEFAULT",      FieldIsBoolean);
    /** Link owner to user db ? */

//    "CREATE TABLE 	comptes_bancaires ("  -->  bank_details
//                    "id_compte	      int(10)		UNSIGNED 			NOT NULL	auto_increment 	,"
//                    "id_usr 	      int(10) 		UNSIGNED 			NOT NULL 			,"
//                    "libelle 	      varchar(150) 	COLLATE utf8_unicode_ci  	NULL 				,"
//                    "titulaire 	      varchar(100) 	COLLATE utf8_unicode_ci 	NULL 				,"
//                    "nom_banque 	      varchar(50) 	COLLATE utf8_unicode_ci 	NULL 				,"
//                    "rib_code_banque      varchar(5) 	COLLATE utf8_unicode_ci 	NULL 				,"
//                    "rib_code_guichet     varchar(5) 	COLLATE utf8_unicode_ci  	NULL 				,"
//                    "rib_numcompte 	      varchar(11) 	COLLATE utf8_unicode_ci 	NULL 				,"
//                    "rib_cle 	      varchar(2) 	COLLATE utf8_unicode_ci 	NULL 				,"
//                    "solde_initial 	      double 						NULL				,"
//                    "remarque 	      blob 		 	 			NULL 				,"
//                    "PRIMARY KEY(id_compte));";

    addField(Table_Deposit,  DEPOSIT_ID,           "DEP_ID",          FieldIsUniquePrimaryKey);
    addField(Table_Deposit,  DEPOSIT_USER_UID,     "USER_UID",        FieldIsUUID);
    addField(Table_Deposit,  DEPOSIT_ACCOUNT_ID,   "DEP_ACCDOUNT_ID", FieldIsLongInteger);
    addField(Table_Deposit,  DEPOSIT_TYPE,         "TYPE",            FieldIsShortText);
    addField(Table_Deposit,  DEPOSIT_DATE,         "DATE",            FieldIsDate, "CURRENT_DATE");
    addField(Table_Deposit,  DEPOSIT_PERTIODSTART, "PERIOD_START",    FieldIsDate, "CURRENT_DATE");
    addField(Table_Deposit,  DEPOSIT_PERIODEND,    "PERIOD_END",      FieldIsDate, "CURRENT_DATE");
    addField(Table_Deposit,  DEPOSIT_CONTENT,      "CONTENT",         FieldIsBlob);
    addField(Table_Deposit,  DEPOSIT_COMMENT,      "COMMENT",         FieldIsLongText);
    addField(Table_Deposit,  DEPOSIT_ISVALID,      "ISVALID",         FieldIsBoolean);

//    "CREATE TABLE 	depots 	  ("  ---> deposit
//                  "id_depot  	int(11)  	 	  				NOT NULL  	 auto_increment  ,"
//                  "id_usr 	int(11) 						NOT NULL 			 ,"
//                  "id_compte 	int(10) 		UNSIGNED 			NOT NULL 			,"
//                  "type_depot 	varchar(10) 		COLLATE utf8_unicode_ci		NULL 				,"
//                  "date 		date 							NOT NULL  			,"
//                  "periode_deb 	date 							NULL 	         		,"
//                  "periode_fin 	date 							NULL  			        ,"
//                  "blob_depot 	blob 			 				NULL 			        ,"
//                  "remarque 	text                    COLLATE utf8_unicode_ci         NULL 				,"
//                  "valide 	tinyint(4) 						NULL 			        ,"
//                  "PRIMARY KEY(id_depot));";
/*        ACCOUNT_ID = 0,
        ACCOUNT_UID,
        ACCOUNT_USER_UID,
        ACCOUNT_PATIENT_UID,
        ACCOUNT_PATIENT_NAME,
        ACCOUNT_SITE_ID,
        ACCOUNT_INSURANCE_ID,
        ACCOUNT_DATE,
        ACCOUNT_MEDICALPROCEDURE_XML,
        ACCOUNT_MEDICALPROCEDURE_TEXT,
        ACCOUNT_COMMENT,
        ACCOUNT_CASHAMOUNT,
        ACCOUNT_CHEQUEAMOUNT,
        ACCOUNT_VISAAMOUNT,
        ACCOUNT_INSURANCEAMOUNT,
        ACCOUNT_OTHERAMOUNT,
        ACCOUNT_DUEAMOUNT,
        ACCOUNT_DUEBY,
        ACCOUNT_ISVALID,
        ACCOUNT_TRACE,
        ACCOUNT_MaxParam*/
    addField(Table_Account,  ACCOUNT_ID,              "ACCOUNT_ID",     FieldIsUniquePrimaryKey);
    addField(Table_Account,  ACCOUNT_UID,             "ACCOUNT_UID",    FieldIsUUID);
    addField(Table_Account,  ACCOUNT_USER_UID,        "USER_UID",       FieldIsUUID);
    addField(Table_Account,  ACCOUNT_PATIENT_UID,     "PATIENT_UID",    FieldIsLongInteger);
    addField(Table_Account,  ACCOUNT_PATIENT_NAME,    "PATIENT_NAME",   FieldIsLongInteger);
    addField(Table_Account,  ACCOUNT_SITE_ID,         "SITE_ID",        FieldIsLongInteger);
    addField(Table_Account,  ACCOUNT_INSURANCE_ID,    "INSURANCE_ID",   FieldIsLongInteger);
    addField(Table_Account,  ACCOUNT_DATE,            "DATE",           FieldIsDate, "CURRENT_DATE");
    addField(Table_Account,  ACCOUNT_MEDICALPROCEDURE_XML,   "MP_XML", FieldIsLongText);
    addField(Table_Account,  ACCOUNT_MEDICALPROCEDURE_TEXT,   "MP_TXT", FieldIsLongText);
    addField(Table_Account,  ACCOUNT_COMMENT,         "COMMENT",        FieldIsLongText);
    addField(Table_Account,  ACCOUNT_CASHAMOUNT,      "CASH",         FieldIsReal);
    addField(Table_Account,  ACCOUNT_CHEQUEAMOUNT,    "CHEQUE",         FieldIsReal);
    addField(Table_Account,  ACCOUNT_VISAAMOUNT,      "VISA",         FieldIsReal);
    addField(Table_Account,  ACCOUNT_INSURANCEAMOUNT, "BANKING",         FieldIsReal);
    addField(Table_Account,  ACCOUNT_OTHERAMOUNT,     "OTHER",         FieldIsReal);
    addField(Table_Account,  ACCOUNT_DUEAMOUNT,       "DUE",         FieldIsReal);
    addField(Table_Account,  ACCOUNT_DUEBY,           "DUE_BY", FieldIsLongText);
    addField(Table_Account,  ACCOUNT_ISVALID,         "VALID",         FieldIsBoolean);
    addField(Table_Account,  ACCOUNT_TRACE,           "TRACE",          FieldIsBlob);
    
    
    addField(Table_Acts,  ACTS_ID,              "ACTS_ID",     FieldIsUniquePrimaryKey);
    addField(Table_Acts,  ACTS_UID,             "ACTS_UID",    FieldIsUUID);
    addField(Table_Acts,  ACTS_USER_UID,        "USER_UID",       FieldIsUUID);
    addField(Table_Acts,  ACTS_PATIENT_UID,     "PATIENT_UID",    FieldIsLongInteger);
    addField(Table_Acts,  ACTS_PATIENT_NAME,    "PATIENT_NAME",   FieldIsLongInteger);
    addField(Table_Acts,  ACTS_SITE_ID,         "SITE_ID",        FieldIsLongInteger);
    addField(Table_Acts,  ACTS_INSURANCE_ID,    "INSURANCE_ID",   FieldIsLongInteger);
    addField(Table_Acts,  ACTS_DATE,            "DATE",           FieldIsDate, "CURRENT_DATE");
    addField(Table_Acts,  ACTS_MEDICALPROCEDURE_TEXT,   "MP_TXT", FieldIsLongText);
    addField(Table_Acts,  ACTS_COMMENT,         "COMMENT",        FieldIsLongText);
    addField(Table_Acts,  ACTS_TRACE,           "TRACE",          FieldIsBlob);

//    "CREATE TABLE 	honoraires	("   --> amount
//                    "id_hono  	int(11)  	 	UNSIGNED  			NOT NULL	 auto_increment  ,"
//                    "id_usr 	int(11) 						NOT NULL			 ,"
//                    "id_drtux_usr 	int(11) 						NULL 				 ,"
//                    "patient 	varchar(75) 		COLLATE utf8_unicode_ci		NOT NULL 			,"
//                    "id_site        varchar(10)    	        COLLATE utf8_unicode_ci         NULL                            ,"
//                    "id_payeurs     varchar(36)             COLLATE utf8_unicode_ci         NULL                            ,"
//                    "GUID 		varchar(36) 		COLLATE utf8_unicode_ci		NULL 				,"
//                    "praticien 	varchar(75) 		COLLATE utf8_unicode_ci 	NOT NULL			,"
//                    "date 		date 							NOT NULL 			,"
//                    "acte 		blob 		                        		NULL 				,"
//                    "acteclair      varchar(12)             COLLATE utf8_unicode_ci         NULL                            ,"
//                    "remarque 	blob 			 	 			NULL 				,"
//                    "esp 		double 							NOT NULL 			,"
//                    "chq 		double 							NOT NULL 			,"
//                    "cb 		double 							NOT NULL			,"
//                    "daf 		double 							NOT NULL 			,"
//                    "autre 		double 							NOT NULL 			,"
//                    "du 		double 							NOT NULL 			,"
//                    "du_par		varchar(100) 		COLLATE utf8_unicode_ci		NULL 			        ,"
//                    "valide 	tinyint(1) 						NULL				,"
//                    "tracabilite 	blob 			 				NULL				,"
//                    "PRIMARY KEY(id_hono));";

    // Fr == Immobilisations
    addField(Table_Assets,  ASSETS_ID,             "ASSETS_ID",        FieldIsUniquePrimaryKey);
    addField(Table_Assets,  ASSETS_USER_UID,       "USER_UID",      FieldIsUUID);
    addField(Table_Assets,  ASSETS_ACCOUNT_ID,     "ACCOUNT_ID",    FieldIsLongInteger);
    addField(Table_Assets,  ASSETS_LABEL,          "LABEL",         FieldIsShortText);
    addField(Table_Assets,  ASSETS_DATE,           "DATE",          FieldIsDate, "CURRENT_DATE");
    addField(Table_Assets,  ASSETS_DURATION,       "DURATION",      FieldIsReal);
    addField(Table_Assets,  ASSETS_MODE,           "MODE",          FieldIsInteger);
    addField(Table_Assets,  ASSETS_VALUE,          "VALUE",         FieldIsReal);
    addField(Table_Assets,  ASSETS_TAXEACTS,       "TAX ACTS",      FieldIsReal);
    addField(Table_Assets,  ASSETS_YEARLY_RESULT,  "YEARLY VALUE",  FieldIsReal);
    addField(Table_Assets,  ASSETS_RESIDUAL_VALUE, "RESIDUAL VALUE",FieldIsReal);
    addField(Table_Assets,  ASSETS_YEARS,          "YEARS TO RUN",  FieldIsInteger);
    addField(Table_Assets,  ASSETS_RATE,           "RATE",          FieldIsReal);
    addField(Table_Assets,  ASSETS_MOVEMENT,       "MOVEMENT",      FieldIsBlob);
    addField(Table_Assets,  ASSETS_COMMENT,        "COMMENT",       FieldIsLongText);
    addField(Table_Assets,  ASSETS_TRACE,          "TRACE",         FieldIsBlob);

//    "CREATE TABLE 	immobilisations ("  --> assets
//                    "id_immob  	int(10)  	 	UNSIGNED  			NOT NULL  	 auto_increment  ,"
//                    "id_usr 	int(10) 		UNSIGNED 			NOT NULL 			,"
//                    "id_compte 	int(10) 		UNSIGNED 			NOT NULL 			,"
//                    "libelle 	varchar(150) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "date_service 	date 							NOT NULL			,"
//                    "duree 		int(11) 						NOT NULL 			,"
//                    "mode 		tinyint(1) 						NOT NULL			,"
//                    "valeur 	bigint(20) 		UNSIGNED 			NOT NULL			,"
//                    "montant_tva 	double 							NULL 				,"
//                    "valeur_residuelle bigint(20) 						NOT NULL			,"
//                    "resultat 	blob 			 				NOT NULL			,"
//                    "mouvements 	blob 			 	 			NULL 				,"
//                    "remarque 	text 			COLLATE utf8_unicode_ci	        NULL 				,"
//                    "tracabilite 	blob 			 	 			NULL				,"
//                    "PRIMARY KEY(id_immob));";
    addField(Table_AssetsRates,  ASSETSRATES_ID,       "ASSETSRATES_ID", FieldIsUniquePrimaryKey);
    addField(Table_AssetsRates,  ASSETSRATES_USER_UID, "USER_UID",       FieldIsUUID);
    addField(Table_AssetsRates,  ASSETSRATES_NAME,     "NAME",           FieldIsShortText);
    addField(Table_AssetsRates,  ASSETSRATES_YEARS,    "YEARS_RANGE",    FieldIsBlob);
    addField(Table_AssetsRates,  ASSETSRATES_RATES,    "RATE",           FieldIsReal);
    addField(Table_AssetsRates,  ASSETSRATES_DATE,     "DATE",           FieldIsDate,"CURRENT_DATE");
    
//    addTable(Table_Movement, "movement");
    addField(Table_Movement,  MOV_ID,             "MOV_ID",     FieldIsUniquePrimaryKey);
    addField(Table_Movement,  MOV_AV_MOVEMENT_ID, "AV_MOV_ID",  FieldIsLongInteger);
    addField(Table_Movement,  MOV_USER_UID,       "USER_UID",   FieldIsUUID);
    addField(Table_Movement,  MOV_ACCOUNT_ID,     "ACCOUNT_ID", FieldIsLongInteger);
    addField(Table_Movement,  MOV_TYPE,           "TYPE",       FieldIsInteger);
    addField(Table_Movement,  MOV_LABEL,          "LABEL",      FieldIsShortText);
    addField(Table_Movement,  MOV_DATE,           "DATE",       FieldIsDate, "CURRENT_DATE");
    addField(Table_Movement,  MOV_DATEOFVALUE,    "DATEVALUE",  FieldIsDate, "CURRENT_DATE");
    addField(Table_Movement,  MOV_AMOUNT,         "ACTS",       FieldIsReal);
    addField(Table_Movement,  MOV_COMMENT,        "COMMENT",    FieldIsLongText);
    addField(Table_Movement,  MOV_VALIDITY,       "VALIDITY",   FieldIsInteger);
    addField(Table_Movement,  MOV_TRACE,          "TRACE",      FieldIsBlob);
    addField(Table_Movement,  MOV_ISVALID,        "ISVALID",    FieldIsBoolean);
    addField(Table_Movement,  MOV_DETAILS,        "DETAILS",    FieldIsShortText);

    //    "CREATE TABLE 	mouvements	("  --> movement
//                    "id_mouvement  int(10) 			UNSIGNED   			NOT NULL    	 auto_increment ,"
//                    "id_mvt_dispo  int(11)             					NOT NULL    			,"
//                    "id_usr        int(10) 			UNSIGNED      			NOT NULL                    	,"
//                    " id_compte    int(10) 			UNSIGNED    			NOT NULL 			,"
//                    " type         tinyint(3) 		UNSIGNED  			NOT NULL			,"
//                    " libelle      varchar(100)        	COLLATE utf8_unicode_ci 	NOT NULL			,"
//                    " date         date                					NOT NULL			,"
//                    " date_valeur  date                					NOT NULL 			,"
//                    " montant      double UNSIGNED     					NOT NULL			,"
//                    "remarque      blob                 					NULL  				,"
//                    " valide       tinyint(4)          					NOT NULL			,"
//                    " tracabilite  blob               					NULL   				,"
//                    " validation   tinyint(1)         					NULL				,"
//                    " detail       varchar(100)        	COLLATE utf8_unicode_ci 	NULL  				,"
//                    "PRIMARY KEY(id_mouvement));";

    addField(Table_AvailableMovement,  AVAILMOV_ID,           "AVAILMOV_ID",   FieldIsUniquePrimaryKey);
    addField(Table_AvailableMovement,  AVAILMOV_PARENT,       "PARENT",        FieldIsShortText);
    addField(Table_AvailableMovement,  AVAILMOV_TYPE,         "TYPE",          FieldIsInteger);
    addField(Table_AvailableMovement,  AVAILMOV_LABEL,        "LABEL",         FieldIsShortText);
    addField(Table_AvailableMovement,  AVAILMOV_CODE,         "CODE",         FieldIsShortText);
    addField(Table_AvailableMovement,  AVAILMOV_COMMENT,      "COMMENT",       FieldIsLongText);
    addField(Table_AvailableMovement,  AVAILMOV_DEDUCTIBILITY,"TAX_DEDUCTIBLE",FieldIsBoolean);
    /** Add user_uid, and uid */

//    "CREATE TABLE 	mouvements_disponibles ("  --> available_movement
//                    "id_mvt_dispo  	int(10)  	 	UNSIGNED  			NOT NULL  	 auto_increment  ,"
//                    "id_mvt_parent 	int(11) 					 	NULL 				,"
//                    "type 		tinyint(3) 		UNSIGNED 			NOT NULL  			,"
//                    "libelle 	varchar(100) 		COLLATE utf8_unicode_ci 	NOT NULL   			,"
//                    "remarque 	blob 							NULL 				,"
//                    "PRIMARY KEY(id_mvt_dispo));";

//    addTable(Table_Insurance, "insurance");
    addField(Table_Insurance,  INSURANCE_ID,          "INSURANCE_ID", FieldIsUniquePrimaryKey);
    addField(Table_Insurance,  INSURANCE_UID,         "INSURANCE_UID",   FieldIsLongInteger);
    addField(Table_Insurance,  INSURANCE_NAME,        "NAME",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_ADRESS,      "ADRESS",   FieldIsLongText);
    addField(Table_Insurance,  INSURANCE_CITY,        "CITY",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_ZIPCODE,     "ZIPCODE",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_COUNTRY,     "COUNTRY",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_TEL,         "TEL",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_FAX,         "FAX",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_MAIL,        "MAIL",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_CONTACT,     "CONTACT",   FieldIsShortText);
    addField(Table_Insurance,  INSURANCE_PREF,        "PREFERED",  FieldIsBoolean);
    
    /** Create an IDENTITY table */

//    "CREATE TABLE 	payeurs	("  --> insurance
//                    "ID_Primkey  	bigint(20)  	 	  				NOT NULL  	 auto_increment  ,"
//                    "id_payeurs 	bigint(20) 						NOT NULL 			,"
//                    "nom_payeur 	varchar(100) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "adresse_payeur varchar(100) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "ville_payeur 	varchar(100) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "code_postal 	varchar(5) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "telephone_payeur varchar(100) 		COLLATE utf8_unicode_ci 	NULL				,"
//                    "PRIMARY KEY(ID_Primkey));";

//    addTable(Table_Sites, "sites");
    addField(Table_Sites,  SITES_ID,      "SITE_ID", FieldIsUniquePrimaryKey);
    addField(Table_Sites,  SITES_UID,     "SITE_UID",   FieldIsLongInteger);
    addField(Table_Sites,  SITES_NAME,    "NAME",   FieldIsShortText);
    addField(Table_Sites,  SITES_ADRESS,  "ADRESS",   FieldIsLongText);
    addField(Table_Sites,  SITES_CITY,    "CITY",   FieldIsShortText);
    addField(Table_Sites,  SITES_ZIPCODE, "ZIPCODE",   FieldIsShortText);
    addField(Table_Sites,  SITES_COUNTRY, "COUNTRY",   FieldIsShortText);
    addField(Table_Sites,  SITES_TEL,     "TEL",   FieldIsShortText);
    addField(Table_Sites,  SITES_FAX,     "FAX",   FieldIsShortText);
    addField(Table_Sites,  SITES_MAIL,    "MAIL",   FieldIsShortText);
    addField(Table_Sites,  SITES_CONTACT, "CONTACT",   FieldIsShortText);
    addField(Table_Sites,  SITES_PREF,    "PREFERED",  FieldIsBoolean);

//    "CREATE TABLE	sites ("  --> sites
//                    "ID_Primkey  	bigint(10)  	 	  				NOT NULL 	 auto_increment  ,"
//                    "id_site 	varchar(10) 		COLLATE utf8_unicode_ci 	NOT NULL			,"
//                    "site 		varchar(100) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "adresse_site 	varchar(100) 		COLLATE utf8_unicode_ci  	NULL 				,"
//                    "cp_site 	varchar(5) 		COLLATE utf8_unicode_ci		NULL 				,"
//                    "ville_site 	varchar(100) 		COLLATE utf8_unicode_ci  	NULL 				,"
//                    "telsite 	varchar(50) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "faxsite 	varchar(50) 		COLLATE utf8_unicode_ci 	NULL 				,"
//                    "mailsite 	varchar(100) 		COLLATE utf8_unicode_ci 	NULL				,"
//                    "PRIMARY KEY(ID_Primkey));";

//    addTable(Table_Users, "users");
    addField(Table_Users,  USER_ID,      "USER_ID",      FieldIsUniquePrimaryKey);
    addField(Table_Users,  USER_UID,     "USER_UID",     FieldIsUUID);
    addField(Table_Users,  USER_LOG,     "LOG",          FieldIsShortText);
    addField(Table_Users,  USER_PASS,    "PASS",         FieldIsShortText);
    addField(Table_Users,  USER_EMR_UID, "EMR_USER_UID", FieldIsShortText);
//    "CREATE TABLE	utilisateurs ("  --> users
//                    "id_usr  	int(10)  	 	UNSIGNED  			NOT NULL  	 auto_increment ,"
//                    "nom_usr 	varchar(150) 		COLLATE utf8_unicode_ci		NULL 				,"
//                    "login 	varchar(15) 			COLLATE utf8_unicode_ci		NOT NULL			,"
//                    "password 	varchar(50) 		COLLATE utf8_unicode_ci		NOT NULL			,"
//                    "id_drtux_usr 	int(11) 			 			NULL 				,"
//                    "PRIMARY KEY(id_usr));";

    addField(Table_Percent,  PERCENT_ID,      "PCT_ID",      FieldIsUniquePrimaryKey);
    addField(Table_Percent,  PERCENT_UID,     "PCT_UID",     FieldIsUUID);
    addField(Table_Percent,  PERCENT_USER_UID,"PCT_USER_UID",FieldIsUUID);
    addField(Table_Percent,  PERCENT_TYPE,    "TYPE",        FieldIsShortText);
    addField(Table_Percent,  PERCENT_VALUES,  "VALUE",       FieldIsReal);
//    "CREATE TABLE	pourcentages ("  --> pourcentages
//                            "id_pourcent	int(10)		UNSIGNED			NOT NULL	auto_increment ,"
//                            "type		varchar(100)	COLLATE utf8_unicode_ci		NULL				,"
//                            "pourcentage	varchar(6)	COLLATE utf8_unicode_ci		NOT NULL			,"
//                            "PRIMARY KEY(id_pourcent));";

    addField(Table_Rules,  RULES_ID,      "RULES_ID",      FieldIsUniquePrimaryKey);
    addField(Table_Rules,  RULES_UID,     "RULES_UID",     FieldIsUUID);
    addField(Table_Rules,  RULES_TYPE,    "TYPE",          FieldIsShortText);
    addField(Table_Rules,  RULES_VALUES,  "VALUES",        FieldIsShortText);
    addField(Table_Rules,  RULES_PREF,    "PREFERED",      FieldIsBoolean);
    
    
    addField(Table_DistanceRules,  DISTRULES_ID,      "DISTRULES_ID",      FieldIsUniquePrimaryKey);
    addField(Table_DistanceRules,  DISTRULES_UID,     "DISTRULES_UID",     FieldIsUUID);
    addField(Table_DistanceRules,  DISTRULES_TYPE,    "TYPE",              FieldIsShortText);
    addField(Table_DistanceRules,  DISTRULES_VALUES,  "VALUES",            FieldIsShortText);
    addField(Table_DistanceRules,  DISTRULES_PREF,    "PREFERED",          FieldIsBoolean);

    addField(Table_Thesaurus,  THESAURUS_ID,      "THESAURUS_ID",      FieldIsUniquePrimaryKey);
    addField(Table_Thesaurus,  THESAURUS_UID,     "THESAURUS_UID",     FieldIsUUID);
    addField(Table_Thesaurus,  THESAURUS_USERUID, "THESAURUS_USERUID", FieldIsUUID);
    addField(Table_Thesaurus,  THESAURUS_VALUES,  "VALUES",            FieldIsShortText);
    addField(Table_Thesaurus,  THESAURUS_PREF,    "PREFERED",          FieldIsBoolean);

//    addTable(Table_Session, "session");
//    addField(Table_Session,  PAIE_ID,          "BANKING_ID",      FieldIsUniquePrimaryKey);
//    addField(Table_Session,  PAIE_ACCOUNT_UID, "_UID",     FieldIsUUID);
//    addField(Table_Session,  PAIE_DATE,        "",     FieldIs);
//    addField(Table_Session,  PAIE_INSURANCE,   "",     FieldIs);
//    addField(Table_Session,  PAIE_USER_UID,    "",     FieldIs);
//    addField(Table_Session,  PAIE_SITE_UID,    "",     FieldIs);
//    addField(Table_Session,  PAIE_MEDICPROC,   "",     FieldIs);
//    addField(Table_Session,  PAIE_CASHACTS,      "CASH",           FieldIsReal);
//    addField(Table_Session,  PAIE_CHEQUEACTS,    "CHEQUE",         FieldIsReal);
//    addField(Table_Session,  PAIE_VISAACTS,      "VISA",           FieldIsReal);
//    addField(Table_Session,  PAIE_INSURANCEACTS, "INSURANCE",      FieldIsReal);
//    addField(Table_Session,  PAIE_OTHERACTS,     "OTHER",          FieldIsReal);
//    addField(Table_Session,  PAIE_DUEACTS,       "DUE",            FieldIsReal);
//    addField(Table_Session,  PAIE_DUEBY,           "DUE_BY",         FieldIsShortText);
//    addField(Table_Session,  PAIE_ISVALID,         "ISVALID",        FieldIsBoolean);
//    addField(Table_Session,  PAIE_TRACE,           "TRACE",          FieldIsBlob);

//    "CREATE TABLE	seances  ("  --> paiement ?
//                          "id_paiement    int(11)			unsigned			NOT NULL     	auto_increment ,"
//                          "id_hono  	int(11)  	 	UNSIGNED  			NOT NULL	 		,"
//                          "date 		date 							NOT NULL 			,"
//                          "id_payeurs 	bigint(20) 						NOT NULL 			,"
//                          "id_usr         varchar(10)                                             NULL                            ,"
//                          "id_site        varchar(10)                                             NULL                            ,"
//                          "acte 		text                    COLLATE  utf8_unicode_ci	NULL 				,"
//                          "esp 		double 							NOT NULL 			,"
//                          "chq 		double 							NOT NULL 			,"
//                          "cb 		double 							NOT NULL			,"
//                          "daf 		double 							NOT NULL 			,"
//                          "autre 		double 							NOT NULL 			,"
//                          "du 		double 							NOT NULL 			,"
//                          "valide 	tinyint(1) 						NULL				,"
//                          "PRIMARY KEY(id_paiement));";

//    addTable(Table_Banking, "banking");
//    addField(Table_Banking,  BANKING_ID,      "BANKING_ID",      FieldIsUniquePrimaryKey);
//    addField(Table_Banking,  BANKING_UID,     "_UID",     FieldIsUUID);
//    "CREATE TABLE IF NOT EXISTS rapprochement_bancaire ("  --> banking_cycle ??
//                  "id_rap_bank   int(11) UNSIGNED  NOT NULL  auto_increment ,"
//                  "id_compte     int(11) UNSIGNED  null,"
//                  "solde         double NULL,"
//                  "PRIMARY KEY(id_rap_bank));";

//    addTable(Table_NameIndex, "nameindex");
//   "CREATE TABLE IF NOT EXISTS  nameindex ("  --> nameindex
//          "id_index         INTEGER PRIMARY KEY,"
//          "name             varchar(50)                NULL,"
//          "surname          varchar(50)               NULL,"
//          "guid             varchar(6)                NOT NULL);";

    init();
}

/** \brief Destructor. */
AccountBase::~AccountBase()
{
    if (d) delete d;
    d=0;
}

bool AccountBase::init()
{
    // only one base can be initialized
    if (m_initialized)
        return true;

    // test driver
     if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
         LOG_ERROR(tkTr(Trans::Constants::SQLITE_DRIVER_NOT_AVAILABLE));
         Utils::warningMessageBox(tkTr(Trans::Constants::APPLICATION_FAILURE),
                                  tkTr(Trans::Constants::SQLITE_DRIVER_NOT_AVAILABLE_DETAIL),
                                  "", qApp->applicationName());
          return false;
      }

     // log the path of the database
     QString pathToDb = settings()->path(Core::ISettings::ReadWriteDatabasesPath);
     LOG(tr("Searching databases into dir %1").arg(pathToDb));

#ifdef FREEACCOUNT
     if (commandLine()->value(Core::CommandLine::CL_Test).toBool()) {
         // check if database exists
         bool feed = !QFile(pathToDb + QDir::separator() + QString(Constants::DB_ACCOUNTANCY) + "-test.db").exists();

         // Connect normal Account Database
         createConnection(Constants::DB_ACCOUNTANCY, QString(Constants::DB_ACCOUNTANCY) + "-test.db", pathToDb,
                          Utils::Database::ReadWrite, Utils::Database::SQLite, "", "", 0, CreateDatabase);
         if (feed) {
             // send defaults datas to DB
             LOG("feeding test database with datas");
             QString content = Utils::readTextFile(settings()->path(Core::ISettings::BundleResourcesPath) + "/textfiles/freeaccount-testing-database.sql");
             if (!executeSQL(content, database()))
                 LOG_ERROR("can feed test database");
         }
     } else {
         // Connect normal Account Database
         createConnection(Constants::DB_ACCOUNTANCY, QString(Constants::DB_ACCOUNTANCY) + ".db", pathToDb,
                          Utils::Database::ReadWrite, Utils::Database::SQLite, "", "", 0, CreateDatabase);
     }
#else
     // Connect normal Account Database
     createConnection(Constants::DB_ACCOUNTANCY, QString(Constants::DB_ACCOUNTANCY) + ".db", pathToDb,
                      Utils::Database::ReadWrite, Utils::Database::SQLite, "", "", 0, CreateDatabase);
#endif

     if (checkDatabaseScheme())
         qWarning() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx OK";
     else
         qWarning() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx WRONG";

     m_initialized = true;
     return true;
}

void AccountBase::logChronos(bool state)
{}

bool AccountBase::createDatabase(const QString &connectionName , const QString &dbName,
                    const QString &pathOrHostName,
                    TypeOfAccess access, AvailableDrivers driver,
                    const QString & /*login*/, const QString & /*pass*/,
                    const int port,
                    CreationOption /*createOption*/
                   )
{
    qWarning() << "CREATE";
    if (connectionName != Constants::DB_ACCOUNTANCY)
        return false;
    LOG(tkTr(Trans::Constants::TRYING_TO_CREATE_1_PLACE_2)
        .arg(dbName).arg(pathOrHostName));
    // create an empty database and connect
    QSqlDatabase DB;
    if (driver == SQLite) {
        DB = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        if (!QDir(pathOrHostName).exists())
            if (!QDir().mkpath(pathOrHostName))
                tkTr(Trans::Constants::_1_ISNOT_AVAILABLE_CANNOTBE_CREATED).arg(pathOrHostName);
        DB.setDatabaseName(QDir::cleanPath(pathOrHostName + QDir::separator() + dbName));
        DB.open();
    }
    else if (driver == MySQL) {
        /** \todo param MySQL database */
    }

    // create db structure
    // before we need to inform Utils::Database of the connectionName to use
    setConnectionName(connectionName);

    qWarning() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

    if (createTables()) {
        LOG(tr("Database %1 %2 correctly created").arg(connectionName, dbName));
        return true;
    } else {
        qDebug() << __FILE__ << QString::number(__LINE__) << " database can not be created " ;
        LOG_ERROR(tr("ERROR : database can not be created %1 %2 %3")
                  .arg(connectionName, dbName, DB.lastError().text()));
    }
    qWarning() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    return false;
}

AccountData *AccountBase::getAccountByUid(const QString &uid)
{
    QSqlDatabase DB = QSqlDatabase::database(Constants::DB_ACCOUNTANCY);
    if (!connectDatabase(DB, __FILE__, __LINE__))
        return 0;
    if (uid.isEmpty())
        return 0;

    // construct the where clause
    QHash<int, QString> where;
    where.insert(Constants::ACCOUNT_UID, QString("=%1").arg(uid));
    QString req = select(Constants::Table_Account, where);
    QSqlQuery q(req,DB);
    if (q.isActive()) {
        if (q.next()) {
            AccountData *data = new AccountData();
            for(int i = 0 ; i < Constants::ACCOUNT_MaxParam; ++i) {
                data->setDatasFromDb(i, q.value(i));
            }
            return data;
        } else {
            LOG_ERROR("No account with an UID like " + uid);
            return 0;
        }
    } else {
        LOG_ERROR("No account with an UID like " + uid);
        LOG_QUERY_ERROR(q);
        return 0;
    }
    return 0;
}

