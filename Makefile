BINDIR =	/usr/local/bin
MANDIR =	/usr/local/man/man1
CC =		gcc
CFLAGS =	-O
#CFLAGS =	-g
#SYSVLIBS =	-lnsl -lsocket
LDFLAGS =	-s ${SYSVLIBS}
#LDFLAGS =	-g ${SYSVLIBS}
LIBS = -lpthread
EXEFILE = hippo

all:		hippo
	@echo "\nplease execute: make install\n"

hippo:	buildobject
#${CC} ${CFLAGS} util.o event.o anet.o hash.o command.o hippo.o  ${LDFLAGS} ${LIBS} -o hippo
	${CC} ${CFLAGS} *.o  ${LDFLAGS} ${LIBS} -o ${EXEFILE}

buildobject:	*.c
	${CC} ${CFLAGS} -c *.c

installall:	all	install

install:		
	@if [ ! -f "$(EXEFILE)" ]; then \
		echo "Please first execute: make\n"; \
		exit 1;\
	fi
	@if [ ! -d "$(MANDIR)" ]; then \
		mkdir "$(MANDIR)"; \
	fi
	rm -f ${BINDIR}/${EXEFILE}
	rm -f ${MANDIR}/${EXEFILE}.1
	cp ${EXEFILE} ${BINDIR}/${EXEFILE}
	cp ${EXEFILE}.1 ${MANDIR}/${EXEFILE}.1
	@echo "\nInstall ${EXEFILE} success."
	@echo "Thank you for the use of ${EXEFILE}"
	@echo "\n--------------------------------"
	@echo "[ Help ]"
	@echo " * man ${EXEFILE}"
	@echo " * ${BINDIR}/${EXEFILE} -h"
	@echo " * ${BINDIR}/${EXEFILE} --help"
	@echo "--------------------------------\n"

uninstall: 
	rm -f ${BINDIR}/${EXEFILE}
	rm -f ${MANDIR}/${EXEFILE}.1
	@echo "\n--------------------------------"
	@echo "Uninstall ${EXEFILE} success."
	@echo "\nThank you for the use of ${EXEFILE}"
	@echo "--------------------------------\n"

clean:
	rm -f ${EXEFILE} *.o core core.* *.core

