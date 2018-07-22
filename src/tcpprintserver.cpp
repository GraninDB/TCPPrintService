                ////---------------------------------------------------------------------
                //#include <vcl.h>
                //#include <stdio.h>
                #include "tcpprintserver.h"

                #include "windows.h"
                #include "winspool.h"

                //#pragma hdrstop

                //#include "TCPPrintMain.h"
                //#include "TCPPrintAboutUnit.h"

                ////---------------------------------------------------------------------
                //#pragma resource "*.dfm"
                //TTCPPrintForm *TCPPrintForm;
                ////---------------------------------------------------------------------
                //__fastcall TCPPrintServer::TTCPPrintForm(TComponent *AOwner)
                //	: TForm(AOwner)
                //{
                //}
                ////---------------------------------------------------------------------

                //void __fastcall TCPPrintServer::FileNew1Execute(TObject *Sender)
                //{
                //  // Do nothing
                //}
                ////---------------------------------------------------------------------------

                void TCPPrintServer::WebPrintServerClientConnect(TObject *Sender, TCustomWinSocket *Socket)
                {
                    TMemoryStream* PrnStream = new TMemoryStream;
                    Socket->Data=PrnStream;
                }
                //---------------------------------------------------------------------------

                void TCPPrintServer::WebPrintServerClientRead(TObject *Sender, TCustomWinSocket *Socket)
                {
                    int ResLn = Socket->ReceiveLength();
                    char * Rb = new char[ResLn];
                    Socket->ReceiveBuf(Rb, ResLn);

                    if (((TMemoryStream*)Socket->Data)->Size==0 && ResLn>3) {
                        if (Rb[0]=='G' && Rb[1]=='E' && Rb[2]=='T' ) {
                            QString PrinterName = "";
                            int srv_port = 0;
                            TServerSocket *srv_SS = NULL;

                            for(int c=0; c<ServiceList->Count; c++) {
                                srv_SS=((Srv*)ServiceList->Items[c])->SS;

                                for(int a=0; a<srv_SS->Socket->ActiveConnections; a++) {
                                    if (srv_SS->Socket->Connections[a]==Socket) {
                                        srv_port = srv_SS->Port;
                                        PrinterName = ((Srv*)ServiceList->Items[c])->PN;
                                        break;
                                    }
                                }
                            }
                            QString SendMsg =  "<Html><header><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\"></header>" +
                                DateTimeToStr(Now()) + String("<hr><b>printer:</b> ") + PrinterName + String("<br><b>port:</b> ") +
                                IntToStr(srv_port) + String("<hr>") + String("</Html>");
                            QString MsgHeader = "Content-Length: " + IntToStr(SendMsg.Length()) + "\r\nContent-Type: text/HTML\r\n\r\n";
                            Socket->SendText(SendMsg);
                            ((TMemoryStream*)Socket->Data)->WriteBuffer(Rb, ResLn);
                            Socket->Close();
                            delete Rb;
                            return;
                        } else {
                            ((TMemoryStream*)Socket->Data)->WriteBuffer(Rb,ResLn);
                            delete Rb;
                            return;
                        }

                    } else {
                        ((TMemoryStream*)Socket->Data)->WriteBuffer(Rb, ResLn);
                        delete Rb;
                    }

                }

                void TCPPrintServer::WebPrintServerClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
                {
                    int ResLn=0;

                    do {
                        ResLn=Socket->ReceiveLength();
                        if (ResLn==0) {
                                break;
                        }
                        char * Rb = new char[ResLn];
                        Socket->ReceiveBuf(Rb,ResLn);
                        ((TMemoryStream*)Socket->Data)->WriteBuffer(Rb,ResLn);
                        delete Rb;
                    }while(1);


                    int JobSize=((TMemoryStream*)Socket->Data)->Size;
                    char * PrnBuf = new char[JobSize];
                    ((TMemoryStream*)Socket->Data)->Position = 0;
                    ((TMemoryStream*)Socket->Data)->Read(PrnBuf,JobSize);

                    if (JobSize>3) {
                        if (PrnBuf[0]=='G' && PrnBuf[1]=='E' && PrnBuf[2]=='T') {
                            Log("GET from " + Socket->RemoteAddress, false);
                            delete PrnBuf;
                            delete (TMemoryStream*)Socket->Data;
                            return;
                        }
                    }
                    if(CB_Prn->Checked)((TMemoryStream*)Socket->Data)->SaveToFile("TCPPrint.prn");
                    delete (TMemoryStream*)Socket->Data;

                    QString PrinterName="";
                    int srv_port=0;
                    TServerSocket * srv_SS=NULL;

                    for (int c=0; c<ServiceList->Count; c++) {
                        srv_SS=((Srv*)ServiceList->Items[c])->SS;

                        for (int a=0; a<srv_SS->Socket->ActiveConnections;a++) {
                            if (srv_SS->Socket->Connections[a]==Socket) {
                                srv_port = srv_SS->Port;
                                PrinterName =((Srv*)ServiceList->Items[c])->PN;
                                break;
                            }
                        }
                    }

                    Log("Client: " + Socket->RemoteAddress + ", Service: " + IntToStr(srv_port) +
                         ",  Printer: " + PrinterName + ", JobSize: " + IntToStr(JobSize), false);
                    int pp = 0;
                    bool print_rez = RawDataToPrinter(PrinterName.c_str(), PrnBuf, JobSize, Socket->RemoteAddress, pp);  //   PrinterName->Text.c_str()
                    delete PrnBuf;
                    Log("Client=" + Socket->RemoteAddress + ", PrintRezult=" + IntToStr(print_rez), false);
                }

                bool TCPPrintServer::RawDataToPrinter(LPSTR szPrinterName, LPBYTE lpData, DWORD dwCount, String IpAddr,int &PrintedPages)
                {
                    HANDLE hPrinter;
                    DOC_INFO_1 DocInfo;
                    DWORD dwJob;
                    DWORD dwBytesWritten;

                    // Need a handle to the printer.
                    if (!OpenPrinter(szPrinterName, &hPrinter, NULL)) {
                        Log(IpAddr + " - OpenPrinter=false " + szPrinterName, true);
                        return false;
                    }

                    // Fill in the structure with info about this "document."
                    DocInfo.pDocName = "TCPPrint";
                    DocInfo.pOutputFile = NULL;
                    DocInfo.pDatatype = "RAW";

                    // Inform the spooler the document is beginning.
                    if ((dwJob = StartDocPrinter(hPrinter, 1, (LPSTR)&DocInfo)) == 0){
                        ClosePrinter(hPrinter);
                        Log(IpAddr + " - StartDocPrinter=false",true);
                        return false;
                    }
                    // Start a page.
                    if (!StartPagePrinter(hPrinter)) {
                        EndDocPrinter(hPrinter);
                        ClosePrinter(hPrinter);
                        Log(IpAddr + " - StartPagePrinter=false", true);
                        return false;
                    }

                    // Send the data to the printer.
                    if (!WritePrinter(hPrinter, lpData, dwCount, &dwBytesWritten)) {
                        EndPagePrinter(hPrinter);
                        EndDocPrinter(hPrinter);
                        ClosePrinter(hPrinter);
                        Log(IpAddr + " - WritePrinter=false", true);
                        return false;
                    }

                    // End the page.
                    if (!EndPagePrinter(hPrinter)) {
                        EndDocPrinter( hPrinter );
                        ClosePrinter( hPrinter );
                        Log(IpAddr + " - EndPagePrinter=false", true);
                        return false;
                    }

                    // Inform the spooler that the document is ending.
                    if (!EndDocPrinter(hPrinter)) {
                        ClosePrinter( hPrinter );
                        Log(IpAddr + " - EndDocPrinter=false", true);
                        return false;
                    }
                    // Tidy up the printer handle.
                    ClosePrinter(hPrinter);
                    // Check to see if correct number of bytes were written.
                    if (dwBytesWritten != dwCount) {
                        Log(IpAddr + " - dwBytesWritten != dwCount", true);
                        return false;
                    }
                    return true;
                }

                void TCPPrintServer::WebPrintServerClientError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
                {
                    Socket->Close();
                    ErrorCode = 0;
                }

                //void __fastcall TCPPrintServer::Log(String LogMessage, bool err)
                //{
                //LogMessage=DateTimeToStr(Now())+"-"+LogMessage+"\r\n";
                //  FILE * F;
                //  String LogFileName="TCPPrintJob.log";
                //  if(err)LogFileName="TCPPrintERR.log";
                //         if((F=fopen(LogFileName.c_str(),"a+"))==NULL) return;
                //         fwrite(LogMessage.c_str(),LogMessage.Length(),1,F);

                //         fclose(F);
                //}

                void __fastcall TCPPrintServer::MuEx(TObject *Sender, Exception *E)
                {
                    String msgE=E->Message;

                    //if (SystemForm->ShowException->Checked)Application->ShowException(E);
                    Log(String("MuEx::")+msgE,true);
                    if (msgE.Pos("Access violation")>0) {
                        Close();
                    }
                }
                //---------------------------------------------------------------------------

                void __fastcall TCPPrintServer::Btn_AddClick(TObject *Sender)
                {
                //ValueListEditor1->Strings->Add("Printer name=port num.") ;
                    TPrinter * print=new TPrinter;
                    for (int i=0; i<print->Printers->Count; i++) {
                        if (ValueListEditor1->Strings->Text.Pos(print->Printers->Strings[i])==0) {
                            ValueListEditor1->Strings->Add(print->Printers->Strings[i]+"=");
                        }
                    }

                    delete print;
                 /* PRINTDLG pd;
                    ZeroMemory(&pd, sizeof(PRINTDLG));

                    // инициализация PRINTDLG
                    pd.lStructSize = sizeof(PRINTDLG);
                    pd.hwndOwner = Handle;
                    pd.nCopies = 1;
                    pd.nFromPage = 0xFFFF;
                    pd.nToPage = 0xFFFF;
                    pd.nMinPage = 1;
                    pd.nMaxPage = 0xFFFF;
                    pd.Flags = PD_ENABLEPRINTHOOK;
                   pd.lpfnPrintHook = PrintHookProc;

                    if (PrintDlg(&pd))
                    {
                        // что нибудб печатаем...

                        if (pd.hDevMode) GlobalFree(pd.hDevMode);
                        if (pd.hDevNames) GlobalFree(pd.hDevNames);
                    }

                 */
                /* char *def_string = "Отсутствует принтер";
                    char buffer[MAX_PATH];
                    GetProfileString("windows", "printer", def_string,
                                     buffer, MAX_PATH);

                    AnsiString Abuffer(buffer);
                    AnsiString Device, Driver, Port;

                    // Анализируя буфер,получим свойства принтера
                    int first_comma = Abuffer.Pos(",");
                    Device = Abuffer.SubString(1, first_comma - 1);
                    Abuffer = Abuffer.SubString(first_comma + 1,
                              Abuffer.Length() - first_comma);
                    int second_comma = Abuffer.Pos(",");
                    Driver = Abuffer.SubString(1, second_comma - 1);
                    Port = Abuffer.SubString(second_comma + 1,
                                             Abuffer.Length() - second_comma);
                  Log("Device="+Device,false) ;
                   Log("Abuffer="+Abuffer,false) ;
                    Log("Driver="+Driver,false) ;
                     Log("Port="+Port,false) ;

                 */
                }

                //---------------------------------------------------------------------------
                String TCPPrintServer::ListJobsForPrinter(QString szPrinterName, bool htm)
                {
                    String br="";
                    String h2s="<h2>";
                    String h2e="</h2>";
                    if (szPrinterName.Trim().Length()==0) {
                        if (!htm) {
                            h2s="";
                            h2e="";
                        }
                        return h2s + "Please, choose printer name..." + h2e;
                    }
                    HANDLE         hPrinter;
                    DWORD          dwNeeded, dwReturned, i;
                    JOB_INFO_1     *pJobInfo;

                    // Нам понадобится дескриптор принтера, поэтому откроем его
                    if (!OpenPrinter( szPrinterName.c_str(), &hPrinter, NULL)) {
                        return "Err Open Printer: " + szPrinterName;
                    }

                    // Сперва вызовем EnumJobs() чтобы определить, сколько нам потребуется памяти
                    if (!EnumJobs( hPrinter, 0, 0xFFFFFFFF, 1, NULL, 0, &dwNeeded, &dwReturned)) {
                        // В любом случае надо обработать ошибку нехватки памяти
                        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                            ClosePrinter( hPrinter );
                            return "Memory Err (EnumJobs)";
                        }
                    }

                    // Распределяем достаточное количество памяти для структуры JOB_INFO_1
                    // плюс дополнительные данные - dwNeeded из предыдущего вызова
                    // говорит нам, какой объём необходим
                    if ((pJobInfo = (JOB_INFO_1 *)malloc(dwNeeded)) == NULL) {
                        ClosePrinter( hPrinter );
                        return "Memory Err (JobInfo)";
                    }

                    // Вызываем EnumJobs() снова, заполняя тем самым структуры
                    if (!EnumJobs(hPrinter, 0, 0xFFFFFFFF, 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded, &dwReturned)) {
                        ClosePrinter( hPrinter );
                        free( pJobInfo );
                        return "Err EnumJobs";
                    }

                    // Больше дескриптор принтера нам не нужен, поэтому закрываем его
                    ClosePrinter(hPrinter);

                    // dwReturned говорит нам, сколько текущих заданий
                    // Просто отображаем количество найденных заданий
                    b r ="<br>\r\n";
                    if (!htm) {
                        br = "";
                    }
                    QString RetStr = szPrinterName + "\r\n" + br + "Total print jobs=";
                    RetStr = RetStr + String(dwReturned) + "\r\n " + br;

                    // Проще сделать это в цикле и пройтись по каждому заданию
                    for (int i=0; i<dwReturned; i++) {
                        // pJobInfo[i] это структура JOB_INFO_1 для данного задания
                        RetStr=RetStr+ String(pJobInfo[i].pMachineName)+" - " + String(pJobInfo[i].pUserName)+" - " + String(pJobInfo[i].pDocument)+" - " + String(pJobInfo[i].pStatus)+"\r\n "+br;
                    }

                    // Очищаем
                    free(pJobInfo);
                    return RetStr;
                }

                void TCPPrintServer::Show_queue(void)
                {
                    char CNS[100];
                    unsigned long CN = sizeof(CNS);
                    GetComputerName(CNS, &CN);
                    QString ComputerNameStr = String(CNS);
                    String RunStr = "rundll32 printui.dll, PrintUIEntry /o /n\"\\\\" + ComputerNameStr + "\\" +
                            ValueListEditor1->Cells[0][ValueListEditor1->Row] + "\"";

                    //ShowMessage(RunStr);
                    WinExec(RunStr.c_str(), SW_SHOW);
                }
                //---------------------------------------------------------------------------

                void TCPPrintServer::ValueListEditor1StringsChange(TObject *Sender)
                {
                    // ShowMessage(ValueListEditor1->RowCount);
                    if (ValueListEditor1->RowCount>1 && ValueListEditor1->Cells[0][ValueListEditor1->Row].Trim().Length()>0) {
                        Btn_Delete->Enabled = true;
                        Btn_Job->Enabled = true;
                    }

                    if (ValueListEditor1->Cells[0][ValueListEditor1->Row].Trim().Length()==0 && ValueListEditor1->RowCount<3) {
                        Btn_Delete->Enabled=false;
                        Btn_Job->Enabled=false;
                    }
                }
                //---------------------------------------------------lf &------------------------


                void __fastcall TCPPrintServer::ValueListEditor1DblClick(TObject *Sender)
                {
                    Show_queue();
                }
                //---------------------------------------------------------------------------


