#
# Makefile
#

LATEXMK=latexmk -bibtex

MAIN=HDF5_RFC

TEXFILES=$(wildcard *.tex)

all: $(TEXFILES)
	@$(LATEXMK) -e '$$pdflatex=q/pdflatex %O -shell-escape %S/' -pdf $(MAIN)

force:
	@$(LATEXMK) -f -pdf $(MAIN)

clean:
	@$(LATEXMK) -c

distclean: clean
	@$(LATEXMK) -C

help:
	@echo -e "Usage : make [target]\n\
	all		produce the PDF (default)\n\
	force		force compilation if possilbe\n\
	clean		clean  unnecessary files\n\
	distclean	clean deeper\n\
	help		display this help"

