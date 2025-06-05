CC := gcc
CFLAGS := -Wall -Wextra -fPIC -I./include
LDFLAGS := -shared
TARGET := libmysyslog.so
PACKAGE := libmysyslog
BUILD_DIR := deb_pkg
DEB_DIR := ../libmysyslog1/deb
VERSION := 1.0-1
ARCH := $(shell dpkg --print-architecture)

.PHONY: all clean deb install

all: $(TARGET)

$(TARGET): libmysyslog.o
	$(CC) $(LDFLAGS) -o $@ $^

libmysyslog.o: libmysyslog.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(TARGET)
	install -d $(DESTDIR)/usr/lib
	install -m 644 $(TARGET) $(DESTDIR)/usr/lib/
	install -d $(DESTDIR)/usr/include
	install -m 644 libmysyslog.h $(DESTDIR)/usr/include/

deb: clean all
	# Create directory structure
	install -d $(BUILD_DIR)/usr/lib
	install -d $(BUILD_DIR)/usr/include
	install -d $(BUILD_DIR)/DEBIAN
	install -d $(DEB_DIR)

	# Install files
	install -m 644 $(TARGET) $(BUILD_DIR)/usr/lib/
	install -m 644 libmysyslog.h $(BUILD_DIR)/usr/include/

	# Create control file
	@echo "Package: $(PACKAGE)" > $(BUILD_DIR)/DEBIAN/control
	@echo "Version: $(VERSION)" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Section: libs" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Priority: optional" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Architecture: $(ARCH)" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Maintainer: Your Name <your.email@example.com>" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Description: My custom syslog library" >> $(BUILD_DIR)/DEBIAN/control

	# Build package
	fakeroot dpkg-deb --build $(BUILD_DIR) $(DEB_DIR)/$(PACKAGE).deb

clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.o deb
