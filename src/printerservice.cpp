#include "printerservice.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>
#include <QString>

PrinterService::PrinterService(int argc, char **argv) :
    QtService<QCoreApplication>(argc, argv, "TCP Print service")
{
    setServiceDescription("TCP Print service");
    setServiceFlags(QtServiceBase::CanBeSuspended);

    m_logger = new Logger();
}

PrinterService::~PrinterService()
{
    delete m_logger;
}

bool PrinterService::loadSettings()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    m_appDataFolder = paths.at(1) + "/";
    QString configName(m_appDataFolder + "settings.json");

    qInfo() << "Info   Config file name is" << configName;

    QFile settingsFile(configName);
    QString logString;
    if (!settingsFile.open(QIODevice::ReadOnly)) {
        logString = "Couldn't open config file: " + configName;
        m_logger->setJournalType(Logger::System);
        m_logger->logMessage(logString, Logger::Error);

        qInfo() << "Error  Couldn't open config file: " + configName;

        return false;
    }

    QByteArray jsonData = settingsFile.readAll();
    settingsFile.close();

    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));
    QJsonObject jsonObject = jsonDoc.object();

    QJsonValue logType = jsonObject.value("logtype");

    qInfo() << "Info   Logging type" << logType.toString().toLower();

    if (logType.toString().toLower() == "textfile") {
        m_logger->setJournalType(Logger::TextFile);
    } else {
        m_logger->setJournalType(Logger::System);
    }

    QJsonValue printersValue = jsonObject.value("printers");
    QJsonArray jsonArray = printersValue.toArray();

    foreach (const QJsonValue &v, jsonArray) {
        PrinterDaemon::DaemonSettings ds;
        ds.localPrinterName = v.toObject().value("localname").toString();
        ds.isActive = v.toObject().value("active").toBool();
        ds.storeJobs = v.toObject().value("storejobs").toBool();
        ds.port = v.toObject().value("port").toInt();

        ds.jobsPath = v.toObject().value("jobspath").toString();
        if (!QDir::isAbsolutePath(ds.jobsPath)) {
            ds.jobsPath = m_appDataFolder + ds.jobsPath;
        }
        if (!ds.jobsPath.isEmpty()) {
            QDir dir;
            if (!dir.exists(ds.jobsPath)) {
                if (dir.mkpath(ds.jobsPath)) {
                    m_logger->logMessage("Folder created - " + ds.jobsPath, Logger::Info);
                } else {
                    m_logger->logMessage("Couldn't create folder -" + ds.jobsPath, Logger::Error);
                }
            }
        }

        QString address = v.toObject().value("listen").toString();
        if (address.isNull() || address.isEmpty()) {
            ds.listen = QHostAddress::AnyIPv4;
        } else {
            ds.listen = QHostAddress(address);
        }

        m_logger->logMessage("Listen address for printer \"" + ds.localPrinterName + "\" is " +
            ds.listen.toString() + " port " + QString::number(ds.port), Logger::Info);

        QString log = v.toObject().value("log").toString();
        ds.log = Logger::None;
        if (log.toLower().indexOf("p") >=0 ) {
            ds.log |= Logger::Print;
        }
        if (log.toLower().indexOf("e") >=0 ) {
            ds.log |= Logger::Error;
        }
        if (log.toLower().indexOf("d") >=0 ) {
            ds.log |= Logger::Debug;
        }
        if (log.toLower().indexOf("a") >=0 ) {
            ds.log |= Logger::Access;
        }

        QStringList subnets = v.toObject().value("subnets").toString().replace(" ", "").split(',');
        foreach (const QString &sn, subnets) {
            if (!sn.isEmpty()) {
                ds.subnets.append(QHostAddress::parseSubnet(sn));
            }
        }

        m_daemonSettings.append(ds);
    }

    return true;
}

void PrinterService::start()
{
    QCoreApplication *app = application();
    bool error = false;
    QString logString;

    if (!loadSettings()) {
        app->exit(-99);
        error = true;
    } else {
        foreach (const PrinterDaemon::DaemonSettings &ds, m_daemonSettings) {
            PrinterDaemon *daemon = new PrinterDaemon(ds, m_logger, app);
            if (ds.isActive) {
                if (!daemon->startListening()) {
                    logString = QString("Failed to start. Error: %1").arg(daemon->errorString());
                    m_logger->logMessage(logString, Logger::Error);
                    app->exit(-98);
                    error = true;
                } else {
                    m_daemons.append(daemon);
                }
            }
        }
    }

    if (!error) {
        logString = QString("Service started successful");
        m_logger->logMessage(logString, Logger::Info);
   }
}

void PrinterService::pause()
{
    foreach (PrinterDaemon *d, m_daemons) {
        d->pause();
    }
}

void PrinterService::resume()
{
    foreach (PrinterDaemon *d, m_daemons) {
        d->resume();
    }
}
