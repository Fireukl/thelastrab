CC = gcc
CFLAGS = -Wall -g -I../libmysyslog_(Fireukl) -I.
LDFLAGS = -L../libmysyslog_(Fireukl) -Wl,-rpath=../libmysyslog_(Fireukl) -lmysyslog
TARGET = bin/myrpc-server
OBJ = obj/server.o obj/config_parser.o
DEB_DIR = deb_pkg
DEB_NAME = myrpc-server_(Fireukl)
VERSION = 1.0-1
ARCH = amd64
DEB_PATH = ../myrpc_server_(Fireukl)/deb

.PHONY: all clean deb

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS)

obj/server.o: server.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/config_parser.o: config_parser.c config_parser.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

deb: $(TARGET)
	@echo "Building Debian package..."
	@rm -rf $(DEB_DIR)
	@mkdir -p $(DEB_DIR)/usr/bin
	@mkdir -p $(DEB_DIR)/DEBIAN
	@mkdir -p $(DEB_PATH)

	@# Install binary
	install -m 755 $(TARGET) $(DEB_DIR)/usr/bin/myrpc-server

	@# Create control file
	@echo "Package: $(DEB_NAME)" > $(DEB_DIR)/DEBIAN/control
	@echo "Version: $(VERSION)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Architecture: $(ARCH)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Maintainer: Fireukl <fireukl@gmail.com>" >> $(DEB_DIR)/DEBIAN/control
	@echo "Description: myRPC Server Application" >> $(DEB_DIR)/DEBIAN/control

	@# Build package
	fakeroot dpkg-deb --build $(DEB_DIR) $(DEB_PATH)/$(DEB_NAME).deb
	@echo "Package built: $(DEB_PATH)/$(DEB_NAME)_$(VERSION)_$(ARCH).deb"

clean:
	rm -rf obj bin $(DEB_DIR) deb
