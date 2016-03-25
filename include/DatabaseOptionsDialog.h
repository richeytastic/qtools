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
