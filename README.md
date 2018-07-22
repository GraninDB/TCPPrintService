TCPPrintService


## Возможности

	Сервис предназначен для эмуляции сетевого принтера.


## Как пользоваться

	1. Скомпилировать проект.
	2. Скопировать TCPPrintService.exe в любой каталог.
	3. Скопировать нужные библиотеки Qt.
	4. Создать файл настроек.
	5. Установить сервис TCPPrintService.exe -install.
	6. Если необходимо, настроить сервис для автоматического запуска.
	7. Открыть входящие порты (9100 ...).
	8. Запустить сервис из консоли "Службы", либо командой TCPPrintService.exe без параметров.


	Возможен запуск как приложения командой TCPPrintService.exe -exec.
	

## Пример файла настроек

	{
	  "printers": [
		{
		  "localname": "HP Universal Printing PCL 5 (v6.1.0)",
		  "active": true,
		  "port": 9100,
		  "subnets": "192.168.1.0/24, ::/32",
		  "log": "ape",
		  "storejobs": true,
		  "jobspath": "jobs/HP1"
		},
		{
		  "localname": "HP3524",
		  "active": true,
		  "port": 9101,
		  "subnets": "192.168.22.0/24",
		  "log": "p",
		  "storejobs": true,
		  "jobspath": ""
		}
	  ]
	}
	
	localname 	- имя локального принтера в системе
	active		- (true/false) активность сервиса для указанного принтера
	port		- номер порта для указанного принтера
	subnets		- подсети с которых разрешен досут к принтеру (подсети указываются через запятую)
	log			- логирование (a - access, p - print, e - error, d - debug)
	storejobs	- (true/false) сохранять файлы заданий в файл
	jobspath	- путь, по которому будут сохраняться задания

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

Проект был скомпилирован с использованием 