#include "QUtils.h"
#include <QStringRef>
#include <QDir>


QString QTools::getDirectory( const QString& fname, QString* rem)
{
    const int slashIdx = fname.lastIndexOf( QDir::separator());
    if ( rem != NULL)
    {
        const int dotIdx = fname.lastIndexOf('.');
        *rem = QStringRef(&fname, slashIdx+1, dotIdx-slashIdx-1).toString();
    }   // end if
    return QStringRef(&fname, 0, slashIdx).toString();
}   // end getDirectory
