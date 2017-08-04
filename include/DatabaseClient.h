/************************************************************************
 * Copyright (C) 2017 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

/**
  Class that manages database connections with a provided GUI configuration.
  Richard Palmer
  November 2014
  */

#pragma once
#ifndef QTOOLS_DATABASE_CLIENT_H
#define QTOOLS_DATABASE_CLIENT_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <string>
#include "DatabaseOptionsDialog.h"
#include "QTools_Export.h"

namespace QTools
{

class QTools_EXPORT DatabaseClient
{
public:
    explicit DatabaseClient( QTools::DatabaseOptionsDialog* dboptions);
    ~DatabaseClient();

    bool reconnect();
    void disconnect();

    bool isConnected() const { return _db.isOpen();}

    // Returns true only if connected and if tables exist.
    bool hasTables() const;

    // Returns true iff given SQL successfully executed.
    bool execQuery( const std::string& sql);

    // Call to get the last error on false being returned from execQuery().
    std::string getLastError() const;

private:
    QTools::DatabaseOptionsDialog* _dboptions;
    QSqlDatabase _db;
};  // end class

}   // end namespace

#endif
