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

#pragma once
#ifndef QTOOLS_DATABASEOPTIONSDIALOG_H
#define QTOOLS_DATABASEOPTIONSDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <boost/unordered_map.hpp>
#include <vector>
#include "QTools_Export.h"

namespace Ui {
class DatabaseOptionsDialog;
}

namespace QTools
{

class QTools_EXPORT DatabaseOptionsDialog : public QDialog
{ Q_OBJECT
public:
    explicit DatabaseOptionsDialog( QWidget *parent = 0);
    ~DatabaseOptionsDialog();

    // Populate entries in form from file returning true on success.
    bool loadConfig( const QString& fname);

    // Save out the current configuration to the provided file.
    bool saveConfig( const QString& fname);

    bool hasValidConfig() const;

    void enableTypeSelection( bool);

    QString getHost() const;
    void setHost( const QString&);

    int getPort() const;    // Returns -1 if didn't get a valid integer
    void setPort( int);

    QString getName() const;
    void setName( const QString&);

    QString getType() const;
    void setType( const QString&);

    QString getUsername() const;
    void setUsername( const QString&);

    QString getPassword() const;
    void setPassword( const QString&);

    QString getStatus() const;  // From label
    void setStatus( const QString&);

signals:
    void acceptedNewConfig();

private:
    Ui::DatabaseOptionsDialog *ui;

    boost::unordered_map<std::string, int> _dbTypeMap;
    //boost::unordered_map<std::string, int> _dbPortMap;
    std::vector<std::string> _dbTypes;

    void addDatabaseTypes();

private slots:
    void clickedButtonBox( QAbstractButton*);
};  // end class

}   // end namespace

#endif
