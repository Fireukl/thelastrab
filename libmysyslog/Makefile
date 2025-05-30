CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I./include
TARGET = libmysyslog.so

all: $(TARGET)

$(TARGET): libmysyslog.o
	$(CC) -shared -o $(TARGET) libmysyslog.o

libmysyslog.o: libmysyslog.c
	$(CC) $(CFLAGS) -c libmysyslog.c

clean:
	rm -f $(TARGET) *.o

deb: clean all
	# Create temporary directory structure
	mkdir -p deb_pkg/usr/lib
	mkdir -p deb_pkg/usr/include
	mkdir -p deb_pkg/DEBIAN
	# Copy the shared library to usr/lib
	install -m 644 $(TARGET) deb_pkg/usr/lib/
	# Copy the header file to usr/include
	install -m 644 libmysyslog.h deb_pkg/usr/include/
	# Create control file
	echo "Package: libmysyslog" > deb_pkg/DEBIAN/control
	echo "Version: 1.0-1" >> deb_pkg/DEBIAN/control
	echo "Architecture: amd64" >> deb_pkg/DEBIAN/control
	echo "Maintainer: fireukl <fireukl@outlook.com>" >> deb_pkg/DEBIAN/control
	echo "Description: mysyslog library" >> deb_pkg/DEBIAN/control
	# Build the package
	fakeroot dpkg-deb --build deb_pkg ../deb/libmysyslog.deb
	# Clean up temporary files
	rm -rf deb_pkg

.PHONY: all clean deb