#ifndef PRINTERSERVICE_H
#define PRINTERSERVICE_H
#include <QList>
#include <QString>

#include "printerdaemon.h"
#include "qtservice.h"
#include "logger.h"

//class PrinterDaemon;

class PrinterService : public QtService<QCoreApplication>
{
public:
    PrinterService(int argc, char **argv);
    ~PrinterService();
    bool loadSettings();

protected:
    void start();
    void pause();
    void resume();

private:
    QList<PrinterDaemon::DaemonSettings> m_daemonSettings;
    QList<PrinterDaemon*> m_daemons;
    Logger* m_logger = 0;
};

#endif // PRINTERSERVICE_H
