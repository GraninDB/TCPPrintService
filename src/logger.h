#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QTextStream>

class Logger
{
public:
    enum LogType {
        LogPrint = 0x00,
        LogError,
        LogDebug,
        LogAccess,
        LogInfo
    };

    Logger();
    ~Logger();

    void logMessage(const QString &msg, LogType lt);
private:
    QFile* m_file;
};

#endif // LOGGER_H
