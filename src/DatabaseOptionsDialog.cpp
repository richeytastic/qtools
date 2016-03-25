#include "DatabaseOptionsDialog.h"
using QTools::DatabaseOptionsDialog;
#include "ui_DatabaseOptionsDialog.h"
#include <QMessageBox>
#include <fstream>
#include <sstream>
#include <string>
#include <QDir>


// private
void DatabaseOptionsDialog::addDatabaseTypes()
{
    ui->dbTypeComboBox->addItem( "ODBC (MS SQL Server etc.)");
    ui->dbTypeComboBox->addItem( "Oracle");
    ui->dbTypeComboBox->addItem( "MySQL");
    ui->dbTypeComboBox->addItem( "PostgreSQL (v7.3 and above)");
    ui->dbTypeComboBox->addItem( "SQLite (v2)");
    ui->dbTypeComboBox->addItem( "SQLite (v3)");
    ui->dbTypeComboBox->addItem( "Sybase Adaptive Server");
    ui->dbTypeComboBox->addItem( "IBM DB2 (v7.1 and above)");
    ui->dbTypeComboBox->addItem( "Borland Interbase");
    ui->dbTypeComboBox->setCurrentIndex(0);

    _dbTypes.resize(9);
    _dbTypes[0] = "QODBC";
    _dbTypes[1] = "QOCI";
    _dbTypes[2] = "QMYSQL";
    _dbTypes[3] = "QPSQL";
    _dbTypes[4] = "QSQLITE2";
    _dbTypes[5] = "QSQLITE";
    _dbTypes[6] = "QTDS";
    _dbTypes[7] = "QDB2";
    _dbTypes[8] = "QIBASE";

    for ( int i = 0; i < _dbTypes.size(); ++i)
        _dbTypeMap[ _dbTypes[i]] = i;

    /*
    // Set the default ports for each of the supported DBMSs
    _dbPortMap[ _dbTypes[0]] = 2399;    // ODBC/JDBC FileMaker (1433 for MS SQL Server?)
    _dbPortMap[ _dbTypes[1]] = 2483;    // Oracle. Also 2484 for SSL secure, 1521, 1528 for earlier releases (unofficial)
    _dbPortMap[ _dbTypes[2]] = 3306;    // MySQL
    _dbPortMap[ _dbTypes[3]] = 5432;    // Postgre
    _dbPortMap[ _dbTypes[4]] = ;        // 118, 156
    _dbPortMap[ _dbTypes[5]] = ;
    _dbPortMap[ _dbTypes[6]] = 6262;    // Sybase Advantage Database Server (unofficial)
    _dbPortMap[ _dbTypes[7]] = -1;      // IBM DB2
    _dbPortMap[ _dbTypes[8]] = 3050;    // Interbase
    */
}   // end addDatabaseTypes



// private slot
void DatabaseOptionsDialog::clickedButtonBox(QAbstractButton *button)
{
    const QDialogButtonBox::ButtonRole brole = ui->buttonBox->buttonRole(button);
    if ( brole == QDialogButtonBox::ApplyRole || brole == QDialogButtonBox::AcceptRole)
        emit acceptedNewConfig();
}   // end clickedButtonBox



DatabaseOptionsDialog::DatabaseOptionsDialog( QWidget *parent) : QDialog(parent), ui(new Ui::DatabaseOptionsDialog)
{
    ui->setupUi(this);
    connect( ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT( clickedButtonBox(QAbstractButton*)));
    addDatabaseTypes();
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}   // end ctor



DatabaseOptionsDialog::~DatabaseOptionsDialog()
{
    delete ui;
}   // end dtor



// public
bool DatabaseOptionsDialog::hasValidConfig() const
{
    return !ui->serverNameLineEdit->text().isEmpty() && !ui->portNumberLineEdit->text().isEmpty() &&
           !ui->usernameLineEdit->text().isEmpty() && !ui->passwordLineEdit->text().isEmpty();
}   // end hasValidConfig



// public
void DatabaseOptionsDialog::enableTypeSelection( bool enable)
{
    ui->dbTypeComboBox->setEnabled(enable);
}   // end enableTypeSelection



// public
bool DatabaseOptionsDialog::loadConfig( const QString &fname)
{
    bool readFileOkay = false;
    std::ifstream ifile;
    try
    {
        ifile.open( fname.toStdString().c_str());
        if ( !ifile.is_open())
        {
            QMessageBox::warning( this, "Database Config", "Unable to open database configuration from file "
                                  + QApplication::applicationDirPath() + QDir::separator() + fname + "\nPlease set manually!");
            return false;
        }   // end if

        bool flagUnknownTokens = false;

        std::string ln, tok, val;
        while ( std::getline( ifile, ln))
        {
            if ( ln.empty() || ln[0] == '#')    // Ignore empty lines and lines with comments
                continue;

            std::istringstream iss(ln);
            iss >> tok >> val;
            const QString qval( val.c_str());
            if ( tok == "DB_HOST:")
                ui->serverNameLineEdit->setText( qval);
            else if ( tok == "DB_PORT:")
                ui->portNumberLineEdit->setText( qval);
            else if ( tok == "DB_NAME:")
                ui->dbNameLineEdit->setText( qval);
            else if ( tok == "DB_TYPE:")
            {
                if ( _dbTypeMap.count(qval.toStdString()))
                    ui->dbTypeComboBox->setCurrentIndex( _dbTypeMap.at(qval.toStdString()));
                else
                {
                    ui->dbTypeComboBox->setCurrentIndex( 0);
                    QMessageBox::warning(this, "Database Config", "Unknown DB type passed in configuration file! Setting to default.");
                }   // end else
            }   // end else if
            else if ( tok == "DB_USERNAME:")
                ui->usernameLineEdit->setText( qval);
            else if ( tok == "DB_PASSWORD:")
                ui->passwordLineEdit->setText( qval);
            else
                flagUnknownTokens = true;
        }   // end while

        ifile.close();

        if ( flagUnknownTokens)
            QMessageBox::warning(this, "Database Config", "There are unknown tokens in the database configuration file!");

        readFileOkay = hasValidConfig();    // Check that all entries have a valid value
    }   // end try
    catch ( const std::exception& e)
    {
        std::string errMsg = "Unable to read default database configuration; please set manually!\n[" + std::string(e.what()) + "]";
        QMessageBox::warning( this, "Database Config", QString::fromStdString( errMsg));
    }   // end catch

    return readFileOkay;
}   // end loadConfig



// public
bool DatabaseOptionsDialog::saveConfig( const QString& fname)
{
    bool savedOkay = false;
    std::ofstream ofile;
    try
    {
        ofile.open(fname.toStdString().c_str());
        if ( !ofile.is_open())
        {
            QMessageBox::critical( this, "Database Config", "Unable to open file for saving database configuration using file "
                                  + QApplication::applicationDirPath() + QDir::separator() + fname);
            return false;
        }   // end if

        ofile << "DB_HOST: " << getHost().toStdString() << std::endl;
        ofile << "DB_PORT: " << getPort() << std::endl;
        ofile << "DB_NAME: " << getName().toStdString() << std::endl;
        ofile << "DB_TYPE: " << getType().toStdString() << std::endl;
        ofile << "DB_USERNAME: " << getUsername().toStdString() << std::endl;
        ofile << "DB_PASSWORD: " << getPassword().toStdString() << std::endl;

        ofile.close();
        savedOkay = true;
    }   // end try
    catch ( const std::exception& e)
    {
        std::string errMsg = "Unable to save database configuration!\n[" + std::string(e.what()) + "]";
        QMessageBox::critical(this, "Database Config", QString::fromStdString( errMsg));
    }   // end catch

    return savedOkay;
}   // end saveConfig




QString DatabaseOptionsDialog::getHost() const
{
    return ui->serverNameLineEdit->text();
}   // end getHost


void DatabaseOptionsDialog::setHost( const QString& host)
{
    ui->serverNameLineEdit->setText(host);
}   // end setHost


int DatabaseOptionsDialog::getPort() const
{
    bool ok = false;
    int port = ui->portNumberLineEdit->text().toInt(&ok);
    if ( !ok)
        port = -1;
    return port;
}   // end getPort


void DatabaseOptionsDialog::setPort( int port)
{
    ui->serverNameLineEdit->setText( QString("%1").arg(port));
}   // end setPort



QString DatabaseOptionsDialog::getName() const
{
    return ui->dbNameLineEdit->text();
}   // end getName



void DatabaseOptionsDialog::setName(const QString &dbname)
{
    ui->dbNameLineEdit->setText(dbname);
}   // end setName



QString DatabaseOptionsDialog::getType() const
{
    return _dbTypes[ ui->dbTypeComboBox->currentIndex()].c_str();
}   // end getType



void DatabaseOptionsDialog::setType(const QString &dbType)
{
    assert( _dbTypeMap.count(dbType.toStdString()));
    ui->dbTypeComboBox->setCurrentIndex( _dbTypeMap.at(dbType.toStdString()));
}   // end setType



QString DatabaseOptionsDialog::getUsername() const
{
    return ui->usernameLineEdit->text();
}   // end getUsername


void DatabaseOptionsDialog::setUsername( const QString& uname)
{
    ui->usernameLineEdit->setText( uname);
}   // end setUsername


QString DatabaseOptionsDialog::getPassword() const
{
    return ui->passwordLineEdit->text();
}   // end getPassword


void DatabaseOptionsDialog::setPassword( const QString& pword)
{
    ui->passwordLineEdit->setText( pword);
}   // end setPassword



QString DatabaseOptionsDialog::getStatus() const
{
    return ui->dbStatusLabel->text();
}   // end getStatus



void DatabaseOptionsDialog::setStatus(const QString &status)
{
    ui->dbStatusLabel->setText(status);
}   // end setStatus
