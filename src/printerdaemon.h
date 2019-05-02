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
    class DaemonSettings
    {
    public:
        QString localPrinterName;
        bool isActive;
        QHostAddress listen;
        int interfaceWaitTime;
        quint16 port;
        QList<QPair<QHostAddress, int>> subnets;
        int log;
        bool storeJobs;
        QString jobsPath;
    };

    PrinterDaemon(DaemonSettings sett, Logger* logger, QObject* parent = nullptr);

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
    Logger* m_logger = nullptr;
    QByteArray clientData;
    void storePrintJobToFile();
};

#endif // PRINTERDAEMON_H
