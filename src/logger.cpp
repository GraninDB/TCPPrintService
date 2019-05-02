#include "logger.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>

Logger::Logger()
{
}

Logger::~Logger()
{
    if (m_file) {
        m_file->close();
        delete m_file;
    }
}

void Logger::setJournalType(Logger::JournalType type)
{
    if (type == Logger::Unknown) {
        return;
    } else {
        m_journalType = type;
    }

    if (m_journalType == TextFile) {
        QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);

        QString logName(paths.at(1) + "/TCPPrintService.log");
        m_file = new QFile(logName);
        if (!m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QtServiceBase::instance()->logMessage("Log file open error", QtServiceBase::MessageType::Information);
            delete m_file;
            m_file = nullptr;

            qInfo() << "Couldn't open log file" << logName;
        } else {
            qInfo() << "Log file name is" << logName;
        }
    }
}

void Logger::logMessage(const QString &msg, LogType logType)
{
    if (logType == None) {
        return;
    }

    QString strLogType;

    switch (logType) {
    case Print: {
        strLogType = "Print";
        break;
    }
    case Error: {
        strLogType = "Error";
        break;
    }
    case Debug: {
        strLogType = "Debug";
        break;
    }
    case Access: {
        strLogType = "Access";
        break;
    }
    case Info: {
        strLogType = "Info";
        break;
    }
    case None: {
        break;
    }
    }

    qInfo() << strLogType << msg;

    switch (m_journalType) {
    case Unknown:{
        return;
    }
    case TextFile :{
        if (!m_file) {
            return;
        }

        QTextStream out(m_file);
        QDateTime dt(QDateTime::currentDateTime());

        out << dt.toString(Qt::ISODate) << " " ;
        out << strLogType.leftJustified(7);
        out << msg << "\n";
        return;
    }
    case System :{
        switch (logType) {
        case Print: {
            QtServiceBase::instance()->logMessage(msg, QtServiceBase::MessageType::Information);
            break;
        }
        case Error: {
            QtServiceBase::instance()->logMessage(msg, QtServiceBase::MessageType::Error);
            break;
        }
        case Debug: {
            QtServiceBase::instance()->logMessage(msg, QtServiceBase::MessageType::Warning);
            break;
        }
        case Access: {
            QtServiceBase::instance()->logMessage(msg, QtServiceBase::MessageType::Information);
            break;
        }
        case Info: {
            QtServiceBase::instance()->logMessage(msg, QtServiceBase::MessageType::Information);
            break;
        }
        case None: {
            break;
        }
        }

        return;
    }
    }
}
