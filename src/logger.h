#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QTextStream>

#include "qtservice.h"

class Logger
{
public:
    enum LogType {
        None = 0x00,
        Print,
        Error,
        Debug,
        Access,
        Info
    };

    enum JournalType {
        Unknown,
        System,
        TextFile
    };

    Logger();
    ~Logger();

    void setJournalType(JournalType type);
    void logMessage(const QString &msg, LogType lt);
private:
    QFile* m_file = nullptr;
    JournalType m_journalType = Unknown;
};

#endif // LOGGER_H
