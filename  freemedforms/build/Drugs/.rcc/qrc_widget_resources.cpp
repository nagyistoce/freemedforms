/****************************************************************************
** Resource object code
**
** Created: Sun Sep 13 10:33:13 2009
**      by: The Resource Compiler for Qt version 4.5.2
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qglobal.h>

static const unsigned char qt_resource_data[] = {
  // /home/pmddeb/freemedform-project/plugins/drugsplugin/done.csv
  0x0,0x0,0x0,0x0,
  
    // /home/pmddeb/freemedform-project/plugins/drugsplugin/link-class-substances.csv
  0x0,0x0,0x0,0x0,
  
  
};

static const unsigned char qt_resource_name[] = {
  // done.csv
  0x0,0x8,
  0x6,0x48,0x47,0xc6,
  0x0,0x64,
  0x0,0x6f,0x0,0x6e,0x0,0x65,0x0,0x2e,0x0,0x63,0x0,0x73,0x0,0x76,
    // link-class-substances.csv
  0x0,0x19,
  0xd,0x1,0x8c,0xa6,
  0x0,0x6c,
  0x0,0x69,0x0,0x6e,0x0,0x6b,0x0,0x2d,0x0,0x63,0x0,0x6c,0x0,0x61,0x0,0x73,0x0,0x73,0x0,0x2d,0x0,0x73,0x0,0x75,0x0,0x62,0x0,0x73,0x0,0x74,0x0,0x61,
  0x0,0x6e,0x0,0x63,0x0,0x65,0x0,0x73,0x0,0x2e,0x0,0x63,0x0,0x73,0x0,0x76,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,
  // :/done.csv
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,
  // :/link-class-substances.csv
  0x0,0x0,0x0,0x16,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x4,

};

QT_BEGIN_NAMESPACE

extern bool qRegisterResourceData
    (int, const unsigned char *, const unsigned char *, const unsigned char *);

extern bool qUnregisterResourceData
    (int, const unsigned char *, const unsigned char *, const unsigned char *);

QT_END_NAMESPACE


int QT_MANGLE_NAMESPACE(qInitResources_widget_resources)()
{
    QT_PREPEND_NAMESPACE(qRegisterResourceData)
        (0x01, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qInitResources_widget_resources))

int QT_MANGLE_NAMESPACE(qCleanupResources_widget_resources)()
{
    QT_PREPEND_NAMESPACE(qUnregisterResourceData)
       (0x01, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

Q_DESTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qCleanupResources_widget_resources))

