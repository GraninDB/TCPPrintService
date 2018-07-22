#ifndef PRINTERDAEMON_H
#define PRINTERDAEMON_H

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <windows.h>

#include "qtservice.h"
#include "logger.h"

class PrinterDaemon : public QTcpServer
{
    Q_OBJECT
public:
    enum LogType {
        LogNone = 0,
        LogPrintJob = 1,
        LogErrors = 2,
        LogDebug = 4,
        LogAccess = 8
    };

    class DaemonSettings
    {
    public:
        QString localPrinterName;
        bool isActive;
        quint16 port;
        QList<QPair<QHostAddress, int>> subnets;
        int log;
        bool storeJobs;
        QString jobsPath;
    };

    PrinterDaemon(DaemonSettings sett, Logger* logger, QObject* parent = 0);

    bool startListening();
    void incomingConnection(int socket);
    void pause();
    void resume();

    WINBOOL sendRawDataToPrinter(LPTSTR szPrinterName, LPBYTE lpData, DWORD dwCount);
private slots:
    void onClientRead();
    void onClientClose();

private:
    bool m_disabled;
    DaemonSettings m_settings;
    Logger* m_logger = 0;
    QByteArray clientData;
    void storePrintJobToFile();
};

#endif // PRINTERDAEMON_H
