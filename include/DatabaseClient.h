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
