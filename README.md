# Руководство по сборке и использованию myRPC

## Сборка проекта

Для сборки `myRPC-client` и `myRPC-server` нужно перейти в главную директорию, в которой содержатся все файлы.
 1. Выполнить команду:
   ```sh
   make
   ```
 3. Для создания deb-пакета выполните команду:
   ```sh
   make deb
   ```
 3. Чтобы очистить директорию от скомпилированных файлов, используйте команду:
   ```sh
   make clean
   ```
## Установка и настройка

перейдите в соответствующие директории и выполните команды

```sh
sudo dpkg -i deb/myrpc-client.deb
sudo dpkg -i deb/myrpc-server.deb
sudo dpkg -i deb/libmysyslog.deb
```

### Настройка сервера

1. Создайте конфигурационный файл `/etc/myRPC/myRPC.conf`:
```conf
sudo mkdir -p /etc/myRPC
echo -e "port=5555\nsocket_type=stream" | sudo tee /etc/myRPC/myRPC.conf
```

2. Создайте файл пользователей `/etc/myRPC/users.conf`:
```conf
echo "your_username" | sudo tee /etc/myRPC/users.conf
```

## Использование

Сначала надо запустить приложение `myrpc-derver` это сервер, который будет ждать подключение клиента. Для запуска нужно выполнить команду:

```sh
sudo myrpc-server
```

Клиентское приложение `myRPC-client` позволяет отправлять команды на сервер для выполнения. Примеры использования:

1. Отправка команды с использованием потокового сокета (TCP):
```sh
# TCP-соединение
myrpc-client -h 127.0.0.1 -p 5555 -s -c "date"
```




