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
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "patient.h"

#include <utils/global.h>
#include <utils/log.h>
#include <translationutils/constanttranslations.h>

#include <medicalutils/global.h>

#include <coreplugin/constants.h>

// include Qt headers
#include <QHash>
#include <QString>

using namespace Trans::ConstantTranslations;
using namespace Core;
using namespace Core::Internal;

static const char* const MAIN_PATIENT_TAG  = "PatientDatas";

namespace Core {
namespace Internal {

/**
  \brief Private Part of Core::Patient
  \internal
*/
class PatientPrivate
{
public:
    PatientPrivate()
    {
        m_XmlTags.insert(Patient::UID ,        "UID");
        m_XmlTags.insert(Patient::Name ,       "Name" );
        m_XmlTags.insert(Patient::Surname ,    "Surname" );
        m_XmlTags.insert(Patient::Gender ,     "Gender" );
        m_XmlTags.insert(Patient::DateOfBirth ,"DateOfbirth");
        m_XmlTags.insert(Patient::Weight ,     "Weight");
        m_XmlTags.insert(Patient::WeightUnit , "WeightUnit");
        m_XmlTags.insert(Patient::Height ,     "Height" );
        m_XmlTags.insert(Patient::HeightUnit , "HeightUnit" );
        m_XmlTags.insert(Patient::Creatinine,  "Creatinine" );
        m_XmlTags.insert(Patient::CreatinineUnit,  "CreatinineUnit" );
        m_XmlTags.insert(Patient::IMC ,        "IMC");
        m_XmlTags.insert(Patient::CreatinClearance , "CreatinClearance" );
        m_XmlTags.insert(Patient::ICD10Deceases ,    "ICD10Deceases");
        m_XmlTags.insert(Patient::DrugsAtcAllergies ,   "DrugsAtcAllergies" );
        m_XmlTags.insert(Patient::DrugsUidAllergies ,   "DrugsUidAllergies" );
        m_XmlTags.insert(Patient::DrugsInnAllergies ,   "DrugsInnAllergies" );
        m_XmlTags.insert(Patient::DrugsInnAtcAllergies ,   "DrugsInnAtcAllergies" );
//        m_XmlTags.insert(Patient::ATCAllergies ,   "ATCAllergies" );
//        m_XmlTags.insert(Patient::INNAllergies ,   "INNAllergies" );
    }

    QString getXmlTag(const int key) const
    {
        return m_XmlTags.value(key);
    }

    int getReferenceFromTag(const QString &tag) const
    {
        if (m_XmlTags.values().indexOf(tag)==-1)
            return -1;
        return m_XmlTags.key(tag);
    }

    QHash<int,QVariant> m_Values;
    QHash<int,QString>  m_XmlTags;
};

}  // End Internal
}  // End Core


/** Constructor */
Patient::Patient() : d(new PatientPrivate)
{
    Q_ASSERT(d);
}

/** \brief Destructor */
Patient::~Patient()
{
    if (d) delete d;
    d=0;
}

/** \brief Clears all the recorded datas */
void Patient::clear()
{
    d->m_Values.clear();
}

/** \brief Return trus if patient has got the data referenced by enumerator Patient::Reference */
bool Patient::has(const Reference ref) const
{
    return d->m_Values.keys().contains(ref);
}


/** \brief Get the value of the patient according to the enumerator Patient::Reference. */
QVariant Patient::value(Reference ref) const
{
    switch (ref)
    {
        case YearsOld :
        {
            if (has(DateOfBirth)) {
                return MedicalUtils::ageYears(d->m_Values.value(DateOfBirth).toDate());
            }
            break;
        }
        case Age :
        {
            if (d->m_Values.contains(ref))
                return d->m_Values.value(ref);
            if (has(DateOfBirth)) {
                QString tmp;
                tmp = MedicalUtils::readableAge(d->m_Values.value(DateOfBirth).toDateTime());
                d->m_Values.insert(Age,tmp);
                return tmp;
            }
            break;
        }
        case CreatinClearance :
        {
            if (has(ref) && (!d->m_Values.value(ref).isNull()))
                return d->m_Values.value(ref);
            // If we can not retreive it from command line --> calculate it
            if (value(YearsOld).toInt()>0) {
                if (has(Creatinine) && has(Gender) && has(Weight)) {
                    bool isMale = d->m_Values.value(Gender).toString().startsWith("M");
                    return MedicalUtils::clearanceCreatinin(value(YearsOld).toInt(),
                                                            d->m_Values.value(Weight).toDouble(),
                                                            d->m_Values.value(Creatinine).toDouble(),
                                                            isMale);
                }
            }
            break;
        }
        default :
        {
            if (d->m_Values.contains(ref))
                return d->m_Values.value(ref);
        }
    }
    return QVariant();
}

/** \brief Defines a value of the patient according to the enumerator Patient::Reference. */
void Patient::setValue(Reference ref, const QVariant &value)
{
    if (ref==DateOfBirth) {
        QDate date = QDate::fromString(value.toString(),"dd/MM/yyyy");
        d->m_Values.insert(ref, date);
        return;
    }
    d->m_Values.insert(ref, value);
}

/**
  \brief Creates the XML output of the whole patient datas (datas are coded).
  The XML looks like :
  \verbatim
  <PatientDatas>
     <FullName>"encoded"</FullName>
     [...]
  </PatientDatas>
  \endverbatim
  \sa fromXml()
*/
QString Patient::toXml() const
{
    QHash<QString, QString> datas;
    foreach(int k, d->m_Values.keys()) {
        datas.insert(d->getXmlTag(k), d->m_Values.value(k).toString());
    }
    return Utils::createXml(MAIN_PATIENT_TAG,datas,4,true);
}

/**
  \brief Retreive patient's datas from the XML input (datas are coded).
  \sa toXml()
*/
bool Patient::fromXml(const QString &xml)
{
    QHash<QString,QString> datas;
    if (!Utils::readXml(xml, MAIN_PATIENT_TAG, datas, true)) {
        Utils::Log::addError("Patient", "Error while reading XML patient's datas (Patient::fromXml())");
        return false;
    }
    foreach(const QString &k, datas.keys()) {
        int id = d->getReferenceFromTag(k);
        if (id!=-1) {
            d->m_Values.insert(id, datas.value(k));
        }
    }
//    qWarning() << d->m_Values;
    return true;
}

/**
  \brief Find and replace all tokens that refering to the patient datas.
  Tokens are :
  - [[NAME]]
  - [[FULLNAME]]
  - [[DATEOFBIRTH]]
  - [[AGE]]
  - [[WEIGHT]]
  - [[HEIGHT]]
  - [[CLCR]]
  \sa Utils::replaceTokens(), mfDrugsConstants
*/
void Patient::replaceTokens(QString &stringWillBeModified)
{
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_PATIENTNAME, value(Patient::Name).toString());
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_PATIENTSURNAME, value(Patient::Surname).toString());
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_PATIENTYEARSOLD, value(Patient::YearsOld).toString() );
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_PATIENTAGE,  value(Patient::Age).toString() );
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_WEIGHT,      value(Patient::Weight).toString() );
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_HEIGHT,        value(Patient::Height).toString() );
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_DATEOFBIRTH, value(Patient::DateOfBirth).toString() );
    Utils::replaceToken(stringWillBeModified, Constants::TOKEN_CLCR,        value(Patient::CreatinClearance).toString() );
}
