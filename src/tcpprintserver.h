//                #ifndef TCPPRINTSESRVER_H
//                #define TCPPRINTSESRVER_H
//                #include <QObject>
//                #include <QString>
//                #include <QtNetwork/QTcpServer>
//
//                class TCPPrintServer : public QObject
//                {
//                    Q_OBJECT;
//
//                //        void __fastcall WebPrintServerClientConnect(TObject *Sender,
//                //          TCustomWinSocket *Socket);
//                //        void __fastcall WebPrintServerClientRead(TObject *Sender,
//                //          TCustomWinSocket *Socket);
//                //        void __fastcall WebPrintServerClientDisconnect(TObject *Sender,
//                //          TCustomWinSocket *Socket);
//                //        void __fastcall WebPrintServerClientError(TObject *Sender,
//                //          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
//                //          int &ErrorCode);
//                public:
//                    bool RawDataToPrinter(LPSTR szPrinterName, LPBYTE lpData, DWORD dwCount, String IpAddr,int &PrintedPages);
//                    void  Log(String LogMessage, bool err);
//                    void  MuEx(TObject *Sender, Exception *E) ;
//                    QString  ListJobsForPrinter( String szPrinterName,bool htm );
//                    void  Show_queue(void);
//                    void  StartService();
//
//                private:
//                    QList<Srv> *ServerList;
//                    struct Srv
//                      {
//                       QTcpServer *ServerObject; //SS;
//                       QString m_printerName; //PN ;
//                      };
//                };
//                #endif TCPPRINTSESRVER_H
