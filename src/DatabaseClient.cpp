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

#include "DatabaseClient.h"
using QTools::DatabaseClient;
using QTools::DatabaseOptionsDialog;
#include <cassert>
#include <iostream>
#include <QtSql/QSqlError>



DatabaseClient::DatabaseClient( DatabaseOptionsDialog* dboptions) : _dboptions(dboptions)
{
    dboptions->enableTypeSelection(false);
    dboptions->setStatus("OFFLINE");
}   // end ctor



DatabaseClient::~DatabaseClient()
{
    disconnect();
}   // end dtor



bool DatabaseClient::reconnect()
{
    if ( !_dboptions->hasValidConfig())
    {
        std::cerr << "[WARNING] DatabaseClient::reconnect: Invalid database configuration params!" << std::endl;
        return false;
    }   // end if

    // Get DB connection options
    const QString dbtype = _dboptions->getType();
    const QString dbname = _dboptions->getName();
    const QString host = _dboptions->getHost();
    const int port = _dboptions->getPort();
    const QString uname = _dboptions->getUsername();
    const QString pword = _dboptions->getPassword();

    QString oldType;
    QString oldDBname;
    QString oldhost;
    int oldport;
    QString olduname;
    QString oldpword;

    // If DB connection options are the same as in the already connected DB, we return (no need to reconnect)
    if ( _db.isOpen())
    {
        oldType = _db.driverName();
        oldDBname = _db.databaseName();
        oldhost = _db.hostName();
        oldport = _db.port();
        olduname = _db.userName();
        oldpword = _db.password();

        if ( dbtype == oldType && dbname == oldDBname && oldhost == host && oldport == port && olduname == uname && oldpword == pword)
            return true;    // Already connected with the current configuration

        std::cout << "Attempting reconnection to database " << dbname.toStdString() << " with new configuration... ";
    }   // end if
    else
        std::cout << "Attempting initial connection to database " << dbname.toStdString() << " ... ";

    std::cout.flush();
    disconnect();

    // Attempt to connect with given parameters
    _db = QSqlDatabase::addDatabase( dbtype);
    _db.setDatabaseName( dbname);
    _db.setHostName( host);
    _db.setPort( port);
    _db.setUserName( uname);
    _db.setPassword( pword);
    bool openedDB = _db.open();

    if ( !openedDB)
    {
        std::cout << "FAILED!\n\t" << getLastError() << std::endl;
        if ( !oldhost.isEmpty())    // Reconnect to the old configuration
        {
            _db = QSqlDatabase::addDatabase( oldType);
            _db.setDatabaseName( oldDBname);
            _db.setHostName( oldhost);
            _db.setPort( oldport);
            _db.setUserName( olduname);
            _db.setPassword( oldpword);
            openedDB = _db.open();
            if ( !openedDB)
                std::cout << "FAILED to reconnect using old configuration!\n\t" << getLastError() << std::endl;
            else
            {
                // Reset the configuration options with the old data
                _dboptions->setType( oldType);
                _dboptions->setName( oldDBname);
                _dboptions->setHost( oldhost);
                _dboptions->setPort( oldport);
                _dboptions->setUsername( olduname);
                _dboptions->setPassword( oldpword);
                std::cout << "Reconnected using old configuration." << std::endl;
            }   // end else
        }   // end if
    }   // end if
    else
        std::cout << "connected" << std::endl;

    _dboptions->setStatus( openedDB ? "ONLINE" : "OFFLINE");
    return openedDB;
}   // end reconnect



void DatabaseClient::disconnect()
{
    // Disconnect if already connected
    if ( _db.isOpen())
    {
        std::cout << "Disconnecting from database " << _db.databaseName().toStdString() << " ... ";
        std::cout.flush();
        _db.close();
        std::cout << "disconnected" << std::endl;
    }   // end if
}   // end disconnect



bool DatabaseClient::hasTables() const
{
    return isConnected() && !_db.tables().empty();
}   // end hasTables



bool DatabaseClient::execQuery( const std::string& sql)
{
    assert( isConnected());
    QSqlQuery qry( _db);
    return qry.exec( sql.c_str());
}   // end execQuery



std::string DatabaseClient::getLastError() const
{
    return _db.lastError().text().toStdString();
}   // end getLastError
