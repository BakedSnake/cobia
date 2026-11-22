PREFIX=/usr/local
INSTALL_DIR=$(PREFIX)/bin
PERL_PREFIX=/usr/lib64/perl5/5.44
PERL_INSTALL_DIR=$(PERL_PREFIX)/Cobia
CC = clang
CFLAGS = -std=gnu99 -Wall -Wextra -Wpedantic -Wunused-value -I/usr/include/libxml2 -I/usr/lib64/perl5/5.44/x86_64-linux-thread-multi/CORE
LDFLAGS = -lvirt -lxml2 -lyaml -lpthread -lperl
SOURCES = src/cobia.c src/xml.c src/virt.c src/yaml.c src/console.c src/cfg.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = cobia
TUI_SOURCES = src/tui.c src/xml.c src/virt.c src/yaml.c src/console.c src/cfg.c
TUI_OBJECTS = $(TUI_SOURCES:.c=.o)
TUI = tcob

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TUI): $(TUI_OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -lncurses -ltinfo

clean:
	rm -v cobia tcob src/*.o

install:
	mkdir -p $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/*.pm $(PERL_INSTALL_DIR)
	install -m 0755 src/cobia-cfg.pl $(INSTALL_DIR)/cobia-cfg
	install -m 0755 $(TARGET) $(INSTALL_DIR)
	install -m 0755 $(TUI) $(INSTALL_DIR)

uninstall:
	rm -vf $(INSTALL_DIR)/cobia-cfg
	rm -vf $(INSTALL_DIR)/cobia
	rm -vf $(INSTALL_DIR)/tcob
	rm -vrf $(PERL_INSTALL_DIR)

.PHONY: all
all : $(TARGET) $(TUI)

.PHONY: tui
tui: $(TUI)
