# Usage

## Build or run
1. Optionally pull this image:
```
docker pull vagabondan/transaq
```
or get the Dockerfile to build it later by yourself:
```
wget https://raw.githubusercontent.com/vagabondan/txcproxy/master/Dockerfile
```
2. Download docker-compose:
```
wget https://raw.githubusercontent.com/vagabondan/txcproxy/master/docker-compose.yaml
```
3. Run (it will use pulled image or build new one, see point 1 above):
```
docker-compose up -d
```

## How to interact with transaq connector
The image contains [transaq connector library](https://www.finam.ru/files/616TXmlConnector.2.21.2.zip) inside. It is .Net DLL. It's authors supposed that one should include it as part of one's application. It is very inconvenient for some purposes especially when one uses Linux rather than Windows OS.
Very common method to avoid Windows OS lock restrictions is to use [Wine](https://www.winehq.org/) under Linux. But .NET library should be enclosed into some wrapper that can proxy requests to and return responses from library.
Such wrapper [txcproxy](https://github.com/vagabondan/TXCProxy) was originally written by [novikovag](https://github.com/novikovag) and a little bit modified and dockerized by [vagabondan](https://github.com/vagabondan). The image contains it too.
<br>
After starting container [txcproxy](https://github.com/vagabondan/TXCProxy) will listen on port 4444 by default. It uses successive numbers of ports to create and keep permanent connections to clients: one port by one new connection. So to work correctly one should expose range of ports from container, i.e. 4444-4449 or even wider in accordance with the planning numbers of simultaneous connections to [txcproxy](https://github.com/vagabondan/TXCProxy).
<br>
The details of communication with txcproxy are described in its [documentation in Russian](https://raw.githubusercontent.com/vagabondan/txcproxy/master/docs/txcproxy.pdf).

## To build, install and run TXCProxy outside container

### Сборка TXCProxy
TXCProxy разрабатывался с использованием компилятора и инструментов входящих в 
состав проекта [Open Watcom](http://www.openwatcom.org/). При настроенной среде, 
сборка осуществляется простым запуском команды wmake в каталоге с распакованными 
исходными текстами, в результате должен появиться исполняемый файл txcproxy.exe

#### Visual C++

    cl *.c ws2_32.lib /link /out:txcproxy.exe

#### MinGW   
    
    gcc -o txcproxy.exe *.c -lws2_32
    
### Установка
Установка заключается в создании произвольного каталога и копирование в него файла 
txcproxy.exe и библиотеки [txmlconnector.dll](http://www.finam.ru/howtotrade/tconnector/default.asp)

### Запуск
Строка запуск на платформе Windows будет иметь следующий вид:

    txcproxy.exe -tq_addr -tq_port [-px_addr -px_port [options]]

На платформах *NIX под [wine](http://www.winehq.org):

    WINEDEBUG=-all wine txcproxy.exe -tq_addr -tq_port [-px_addr -px_port [options]]

TXCProxy принимает следующие параметры (tq_addr и tq_port являются обязательными):
    
    | Параметр | Описание                             | Значение по умолчанию 
    -------------------------------------------------------------------------
    | addr     | IP адрес входящих подключений        | ANY 
    | port     | номер порта входящих подключений     | 4444 
    | tq_addr  | IP адрес сервера TRANSAQ             | - 
    | tq_port  | номер порта TRANSAQ                  | - 
    | px_type  | тип прокси                           | - 
    | px_addr  | IP адрес прокси сервера              | - 
    | px_port  | номер порта прокси сервера           | - 
    | px_user  | имя пользователя для прокси          | - 
    | px_pass  | пароль для прокси                    | - 
    | dll      | путь к библиотеке коннектора         |.\txmlconnector.dll 
    | ll2      | уровень детализации логов коннектора | 0 
