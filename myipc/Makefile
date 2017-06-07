##==========================================================================
##
## @file Makefile
## Betriebsysteme myipc Makefile
## Beispiel 3
##
## @author Maria Kanikova  <ic16b002@technikum-wien.at>
## @author Christian Fuhry <ic16b055@technikum-wien.at>
## @author Sebastian Boehm <ic16b032@technikum-wien.at>
##
## @date 2017/06
##
## ------------------------------------------------------------- variables --
##

CC=gcc52
CFLAGS=-DDEBUG -Wall -pedantic -Werror -Wextra -g -O3
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECT1=sender.o
OBJECT2=empfaenger.o
OBJECT3=prog.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

##
## --------------------------------------------------------------- targets --
##

all: myipc

sender: $(OBJECT3) $(OBJECT1)
	$(CC) $(CFLAGS) -o $@ $^ -lsem182

empfaenger: $(OBJECT3) $(OBJECT2)
	$(CC) $(CFLAGS) -o $@ $^ -lsem182

clean:
	$(RM) *.o *~ sender
	$(RM) *.o *~ empfaenger
	$(RM) *.o *~ prog

distclean: clean
	$(RM) -r doc

doc: html pdf

html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf

##
## =================================================================== eof ==
##
