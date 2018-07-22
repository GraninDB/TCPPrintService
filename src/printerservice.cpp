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

#if defined(SERVICE_LOG_TO_FILE)
    m_logger = new Logger();
#endif
}

PrinterService::~PrinterService()
{
#if defined(SERVICE_LOG_TO_FILE)
    delete m_logger;
#endif
}

bool PrinterService::loadSettings()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);

    QString configName(paths.at(1) + "/settings.json");

    QFile settingsFile(configName);
    if (!settingsFile.open(QIODevice::ReadOnly)) {
        QString logString = "Couldn't open config file: " + configName;
#if defined(SERVICE_LOG_TO_FILE)
        m_logger->logMessage(logString, Logger::LogError);
#else
        logMessage(logString, MessageType::Error);
#endif
        return false;
    }

    QByteArray jsonData = settingsFile.readAll();
    settingsFile.close();

    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));
    QJsonObject jsonObject = jsonDoc.object();
    QJsonValue printersValue = jsonObject.value("printers");
    QJsonArray jsonArray = printersValue.toArray();

    foreach (const QJsonValue &v, jsonArray) {
        PrinterDaemon::DaemonSettings ds;
        ds.localPrinterName = v.toObject().value("localname").toString();
        ds.isActive = v.toObject().value("active").toBool();
        ds.storeJobs = v.toObject().value("storejobs").toBool();
        ds.port = v.toObject().value("port").toInt();

        ds.jobsPath = v.toObject().value("jobspath").toString();
        if (!ds.jobsPath.isEmpty()) {
            QDir dir;
            if (!dir.exists(ds.jobsPath)) {
                dir.mkpath(ds.jobsPath);
            }
        }

        QString log = v.toObject().value("log").toString();
        ds.log = PrinterDaemon::LogNone;
        if (log.toLower().indexOf("p")>=0) {
            ds.log |= PrinterDaemon::LogPrintJob;
        }
        if (log.toLower().indexOf("e")>=0) {
            ds.log |= PrinterDaemon::LogErrors;
        }
        if (log.toLower().indexOf("d")>=0) {
            ds.log |= PrinterDaemon::LogDebug;
        }
        if (log.toLower().indexOf("a")>=0) {
            ds.log |= PrinterDaemon::LogAccess;
        }

        QStringList subnets = v.toObject().value("subnets").toString().replace(" ", "").split(',');
        foreach (const QString &sn, subnets) {
            ds.subnets.append(QHostAddress::parseSubnet(sn));
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
    }

    foreach (const PrinterDaemon::DaemonSettings &ds, m_daemonSettings) {
        PrinterDaemon *daemon = new PrinterDaemon(ds, m_logger, app);
        if (ds.isActive) {
            if (!daemon->startListening()) {
                logString = QString("Failed to start. Error: %1").arg(daemon->errorString());
#if defined(SERVICE_LOG_TO_FILE)
                m_logger->logMessage(logString, Logger::LogError);
#else
                logMessage(logString, MessageType::Error);
#endif
                app->exit(-98);
                error = true;
            }
        }
        m_daemons.append(daemon);
    }

    if (!error) {
        logString = QString("Service started successful");
#if defined(SERVICE_LOG_TO_FILE)
        m_logger->logMessage(logString, Logger::LogInfo);
#else
        logMessage(logString, MessageType::Information);
#endif
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
