CC = gcc
CFLAGS = -Wall -g -I../libmysyslog1
LDFLAGS = -L../libmysyslog1 -Wl,-rpath=../libmysyslog1 -lmysyslog
TARGET = bin/client
OBJ = obj/client.o
DEB_DIR = deb_pkg
DEB_NAME = myrpc-client
VERSION = 1.0-1
ARCH = amd64
DEB_PATH = ../client/deb

.PHONY: all clean deb

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS)

obj/client.o: client.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

deb: $(TARGET)
	@echo "Building Debian package..."
	@rm -rf $(DEB_DIR)
	@mkdir -p $(DEB_DIR)/usr/bin
	@mkdir -p $(DEB_DIR)/DEBIAN
	@mkdir -p $(DEB_PATH)

	@# Install binary
	install -m 755 $(TARGET) $(DEB_DIR)/usr/bin/myrpc-client

	@# Create control file
	@echo "Package: $(DEB_NAME)" > $(DEB_DIR)/DEBIAN/control
	@echo "Version: $(VERSION)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Section: net" >> $(DEB_DIR)/DEBIAN/control
	@echo "Priority: optional" >> $(DEB_DIR)/DEBIAN/control
	@echo "Architecture: $(ARCH)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Maintainer: Your Name <you@example.com>" >> $(DEB_DIR)/DEBIAN/control
	@echo "Description: myRPC Client Application" >> $(DEB_DIR)/DEBIAN/control
	@echo " Remote procedure call client for myRPC system" >> $(DEB_DIR)/DEBIAN/control

	@# Build package
	fakeroot dpkg-deb --build $(DEB_DIR) $(DEB_PATH)/$(DEB_NAME).deb
	@echo "Package built: $(DEB_PATH)/$(DEB_NAME)_$(VERSION)_$(ARCH).deb"

clean:
	rm -rf obj bin $(DEB_DIR) deb
