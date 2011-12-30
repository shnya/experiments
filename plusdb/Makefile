# Makefile

PROGRAM = plusdb
OBJS = plusdb.o page.o file.o

CXX = g++
CXXFLAGS = -g -Wall
DESTDIR = /usr/local

.PHONY: all clean install uninstall upload
all : $(PROGRAM)

$(PROGRAM) : $(OBJS)
	$(CXX) -o $(PROGRAM) $^

scheme :  $(OBJS)
	$(CXX) -o $(PROGRAM) $^

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

debug :
	$(RM) $(PROGRAM) $(OBJS)
	$(CXX) $(CXXFLAGS) -DDEBUG scheme.cc -o $(PROGRAM)

clean:
	$(RM) $(PROGRAM) $(OBJS)

.PHONY: check-syntax
check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)

install: $(PROGRAM)
	[ ! -e $(DESTDIR)/bin ] && mkdir -p $(DESTDIR)/bin; \
	cp $(PROGRAM) $(DESTDIR)/bin/$(PROGRAM)

uninstall:
	$(RM) $(DESTDIR)/bin/$(PROGRAM)

upload:
	tar zcvf ../$(PROGRAM).tar.gz -C ../ $(PROGRAM) 
	scp ../$(PROGRAM).tar.gz shnya@shnya.jp:www/public/
	rm ../$(PROGRAM).tar.gz
