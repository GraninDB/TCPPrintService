TCPPrintService


## Возможности

	Сервис предназначен для эмуляции сетевого принтера.


## Как пользоваться

	1. Скомпилировать проект.
	2. Скопировать TCPPrintService.exe в любой каталог.
	3. Скопировать нужные библиотеки Qt.
	4. Создать файл настроек. 
		Для Windows 10 C:/ProgramData/TCPPrintService/settings.json
	5. Установить сервис TCPPrintService.exe -install. Установку производить от имени администратора.
		5.1 Для проверки запуска сервиса запустить его с параметром -exec. В консоль будет выведена информация по запуску.
	6. Если необходимо, настроить сервис для автоматического запуска.
	7. Открыть входящие порты (9100 ...).
	8. Запустить сервис из консоли "Службы", либо командой TCPPrintService.exe без параметров.


	Возможен запуск как приложения командой TCPPrintService.exe -exec.
	Если приложение скомпилировано с опцией DEFINES += LOG_TO_CONSOLE, то при запуске с параметром -exec 
	все сообщения и отладочная информация будут выводится в консоль.
	
	Пример запуска
		$ TCPPrintService.exe -exec
		configName "C:/ProgramData/TCPPrintService/settings.json"
		logType "textfile"
		Log file name is "C:/ProgramData/TCPPrintService/TCPPrintService.log"
		Folder created - "C:/ProgramData/TCPPrintService/jobs/HP Universal Printing PCL 5 (v6.1.0)"
		listen QHostAddress("192.168.28.64")
		listen QHostAddress(QHostAddress::Any)
		"Service started successful" 5

	По умолчанию (если параметр не указан) все сообщения сервис записывает в event log - "logtype": "system".
	Для того, чтобы сообщения записывались в файле настроек необходимо указать параметр "logtype": "textfile"  

## Пример файла настроек

	{
	  "logtype: "system",
	  "printers": [
		{
		  "listen": "192.168.0.1",
		  "localname": "HP Universal Printing PCL 5 (v6.1.0)",
		  "active": true,
		  "port": 9100,
		  "subnets": "192.168.1.0/24, ::/32",
		  "log": "ape",
		  "storejobs": true,
		  "jobspath": "jobs/HP Universal Printing PCL 5 (v6.1.0)"
		},
		{
		  "localname": "HP3524",
		  "active": true,
		  "port": 9101,
		  "subnets": "",
		  "log": "p",
		  "storejobs": false,
		  "jobspath": ""
		}
	  ]
	}
	listen		- регистрируется только на указанном адресе. Если не указано, то сервис прослушивает на всех интерфейсах 
	logtype		- тип журнала "system" - event log, "textfile" - Для Windows 10 это файл C:/ProgramData/TCPPrintService/TCPPrintService.log
	
	localname 	- имя локального принтера в системе
	active		- (true/false) активность сервиса для указанного принтера
	port		- номер порта для указанного принтера
	subnets		- подсети с которых разрешен досут к принтеру (подсети указываются через запятую)
					Если указана пустая строка, то проверка не производится.
	log			- логирование (a - access, p - print, e - error, d - debug)
	storejobs	- (true/false) сохранять файлы заданий в файл
	jobspath	- путь, по которому будут сохраняться задания. Если указан относительный путь, то папка будет создана относительно
					папки "C:/ProgramData/TCPPrintService/" для Windows 10.
	
## Параметры командной строки
TCPPrintService.exe -help

TCPPrintService.exe -[i|u|e|t|p|r|c|v|h]                                                                      
        -i(nstall) [account] [password] : Install the service, optionally using given account and password
        -u(ninstall)    : Uninstall the service.
        -e(xec)         : Run as a regular application. Useful for debugging.
        -t(erminate)    : Stop the service.
        -p(ause)        : Pause the service.
        -r(esume)       : Resume a paused service.
        -c(ommand) num  : Send command code num to the service.
        -v(ersion)      : Print version and status information.
        -h(elp)         : Show this help
        No arguments    : Start the service.

## Для компиляции необходимо скачать 
Qt Solutions Component: Service

The QtService component is useful for developing Windows services
and Unix daemons.

https://github.com/qtproject/qt-solutions/tree/master/qtservice

Проект был скомпилирован с использованием Qt 5.11.2