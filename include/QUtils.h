#pragma once
#ifndef QTOOLS_QUTILS_H
#define QTOOLS_QUTILS_H

#include "QTools_Export.h"
#include <QString>

namespace QTools
{

// Get the directory part of fname and place the remainder without file
// extension (if any) in rem (if not NULL).
QTools_EXPORT QString getDirectory( const QString& fname, QString* rem=NULL);

}   // end namespace

#endif




