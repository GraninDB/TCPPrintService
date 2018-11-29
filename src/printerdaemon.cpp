#include "printerdaemon.h"

#include <QDebug>
#include <QUuid>
#include <QMessageLogger>

PrinterDaemon::PrinterDaemon(DaemonSettings sett, Logger *logger, QObject *parent)
    : QTcpServer(parent), m_disabled(false)
{
    m_settings = sett;
#if defined(SERVICE_LOG_TO_FILE)
    m_logger = logger;
#endif
}

bool PrinterDaemon::startListening()
{
    return listen(QHostAddress::Any, m_settings.port);
}

void PrinterDaemon::incomingConnection(int socket)
{
    if (m_disabled) {
        return;
    }

    QTcpSocket* s = new QTcpSocket(this);
    s->setSocketDescriptor(socket);

    QString logString;

    logString = "Local address: " + s->localAddress().toString() +
            "protocol: " + QString::number(s->localAddress().protocol());
#if defined(SERVICE_LOG_TO_FILE)
    m_logger->logMessage(logString, Logger::LogAccess);
#else
    QtServiceBase::instance()->logMessage(logString, QtServiceBase::MessageType::Information);
#endif

    if (m_settings.subnets.count() == 0) {
        connect(s, SIGNAL(readyRead()), this, SLOT(onClientRead()));
        connect(s, SIGNAL(disconnected()), this, SLOT(onClientClose()));
        return;
    }

    for (int i=0; i<m_settings.subnets.count(); i++){
        if (s->peerAddress().isInSubnet(m_settings.subnets[i])) {
            connect(s, SIGNAL(readyRead()), this, SLOT(onClientRead()));
            connect(s, SIGNAL(disconnected()), this, SLOT(onClientClose()));

            if (m_settings.log | PrinterDaemon::LogAccess) {
                logString = "Access from " + s->peerAddress().toString() +
                    " to printer \"" + m_settings.localPrinterName + "\" is allowed";
#if defined(SERVICE_LOG_TO_FILE)
                m_logger->logMessage(logString, Logger::LogAccess);
#else
                QtServiceBase::instance()->logMessage(logString, QtServiceBase::MessageType::Information);
#endif
            }
            return;
        }
    }

    s->disconnect();

    if (m_settings.log | PrinterDaemon::LogAccess) {
        logString = "Access from " + s->peerAddress().toString() +
            " to printer \"" + m_settings.localPrinterName + "\" is denied";
#if defined(SERVICE_LOG_TO_FILE)
        m_logger->logMessage(logString, Logger::LogAccess);
#else
        QtServiceBase::instance()->logMessage(logString, QtServiceBase::MessageType::Warning);
#endif
    }

}

void PrinterDaemon::pause()
{
    m_disabled = true;
}

void PrinterDaemon::resume()
{
    m_disabled = false;
}

void PrinterDaemon::onClientRead()
{
    if (m_disabled)
        return;

    QTcpSocket* socket = (QTcpSocket*)sender();

    if (socket->bytesAvailable() > 0) {
        clientData += socket->readAll();
    }
}

void PrinterDaemon::onClientClose()
{
    QTcpSocket* socket = (QTcpSocket*)sender();

    clientData += socket->readAll();
    long dataLength = clientData.length();

    LPBYTE lpData;
    lpData = (unsigned char*)(clientData.data());

    LPTSTR pn = new wchar_t[m_settings.localPrinterName.length() + 1];
    m_settings.localPrinterName.toWCharArray(pn);
    pn[m_settings.localPrinterName.length()] = '\0';

    if (sendRawDataToPrinter(pn, lpData, dataLength)) {
        if (m_settings.log | PrinterDaemon::LogPrintJob) {
            QString logString("Print job from " + socket->peerAddress().toString() +
                " completed successful");
#if defined(SERVICE_LOG_TO_FILE)
            m_logger->logMessage(logString, Logger::LogPrint);
#else
            QtServiceBase::instance()->logMessage(logString, QtServiceBase::MessageType::Information);
#endif

        }
    }

    storePrintJobToFile();

    socket->deleteLater();

    clientData.clear();

}

BOOL PrinterDaemon::sendRawDataToPrinter(LPTSTR szPrinterName, LPBYTE lpData, DWORD dwCount)
{
    BOOL bStatus = FALSE;
    HANDLE hPrinter = NULL;
    DOC_INFO_1 DocInfo;
    DWORD dwJob = 0L;
    DWORD dwBytesWritten = 0L;

    bStatus = OpenPrinter(szPrinterName, &hPrinter, NULL);
    if (bStatus) {
        wchar_t docName[100], dataType[20];
        wcscpy_s(docName, 100, L"Print print service");
        wcscpy_s(dataType, 20, L"RAW");
        DocInfo.pDocName = docName;
        DocInfo.pOutputFile = NULL;
        DocInfo.pDatatype = dataType;
        dwJob = StartDocPrinter(hPrinter, 1, (LPBYTE)&DocInfo);
        if (dwJob > 0) {
            bStatus = StartPagePrinter(hPrinter);
            if (bStatus) {
                bStatus = WritePrinter(hPrinter, lpData, dwCount, &dwBytesWritten);
                bStatus = bStatus && EndPagePrinter(hPrinter);
            }
            bStatus = bStatus && EndDocPrinter(hPrinter);
        }
        bStatus = bStatus && ClosePrinter(hPrinter);
    }

    if (!bStatus || (dwBytesWritten != dwCount)) {
        bStatus = FALSE;
    } else {
        bStatus = TRUE;
    }
    return bStatus;
}

void PrinterDaemon::storePrintJobToFile()
{
    QUuid uuid;
    uuid = QUuid::createUuid();
    QString uuidString = uuid.toString();

    if (m_settings.storeJobs) {
        QString fileName;

        fileName = m_settings.jobsPath + "/" + uuidString + ".prn";

        QFile prnFile(fileName);

        QTcpSocket* socket = (QTcpSocket*)sender();

        if (!prnFile.open(QIODevice::WriteOnly)) {
            if (m_settings.log | PrinterDaemon::LogErrors) {
                QString logString("Can not store print job from " + socket->peerAddress().toString());
#if defined(SERVICE_LOG_TO_FILE)
                m_logger->logMessage(logString, Logger::LogError);
#else
                QtServiceBase::instance()->logMessage(logString, QtServiceBase::MessageType::Error);
#endif
            }
        } else {
            prnFile.write(clientData);
            prnFile.close();

            if (m_settings.log | PrinterDaemon::LogPrintJob) {
                QString logString("Print job from " + socket->peerAddress().toString() +
                    " stored successful with filename \"" + uuidString + "\"");
#if defined(SERVICE_LOG_TO_FILE)
                m_logger->logMessage(logString, Logger::LogPrint);
#else
                QtServiceBase::instance()->logMessage(logString, QtServiceBase::MessageType::Information);
#endif
            }
        }
    }
}
