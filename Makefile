# Общие настройки
VERSION = 1.0-1
ARCH = amd64

# Пути к проектам
LIB_DIR = libmysyslog1
SERVER_DIR = server
CLIENT_DIR = client

# Цели по умолчанию
.PHONY: all lib server client clean deb

all: lib server client

lib:
	$(MAKE) -C $(LIB_DIR)

server: lib
	$(MAKE) -C $(SERVER_DIR)

client: lib
	$(MAKE) -C $(CLIENT_DIR)

clean:
	$(MAKE) -C $(LIB_DIR) clean
	$(MAKE) -C $(SERVER_DIR) clean
	$(MAKE) -C $(CLIENT_DIR) clean

deb: lib server client
	$(MAKE) -C $(LIB_DIR) deb
	$(MAKE) -C $(SERVER_DIR) deb
	$(MAKE) -C $(CLIENT_DIR) deb
