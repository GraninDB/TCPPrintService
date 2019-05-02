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
	Все сообщения и отладочная информация выводятся в консоль.
	
	Пример запуска
		$ TCPPrintService.exe -exec
		Info   Config file name is "C:/ProgramData/TCPPrintService/settings.json"
		Info   Logging type "textfile"
		Info   Log file name is C:/ProgramData/TCPPrintService/TCPPrintService.log
		Info   Folder created - C:/ProgramData/TCPPrintService/jobs/HP Universal Printing PCL 5 (v6.1.0)
		Info   Listen address for printer "HP Universal Printing PCL 5 (v6.1.0)" is 192.168.0.1 port 9100
		Info   Listen address for printer "HP3524" is 0.0.0.0 port 9101
		Info   Service started successful

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
	listen		- регистрируется только на указанном адресе. Если не указано, или указана пустая строка, 
					то сервис прослушивает на всех интерфейсах, но только с протоколом IPv4.
					Для работы по протоколу IPv6 необходимо указать адрес IPv6 для прослушивания.
					Работа по протоколу IPv6 не проверялась.
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