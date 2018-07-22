#include "logger.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>

Logger::Logger()
{
    m_file = new QFile("TCPPrintService.log");
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Log file open error";
        delete m_file;
        m_file = 0;
    }
}

Logger::~Logger()
{
    m_file->close();
    delete m_file;
}

void Logger::logMessage(const QString &msg, LogType lt)
{
    if (!m_file) {
        return;
    }

    QTextStream out(m_file);

    switch (lt) {
    case LogPrint: {
        out << "Print  ";
        break;
    }
    case LogError: {
        out << "Error  ";
        break;
    }
    case LogDebug: {
        out << "Debug  ";
        break;
    }
    case LogAccess: {
        out << "Access ";
        break;
    }
    case LogInfo: {
        out << "Info   ";
        break;
    }
    }

    QDateTime dt(QDateTime::currentDateTime());

    out << dt.toString(Qt::ISODate) << " " << msg << "\n";
}
