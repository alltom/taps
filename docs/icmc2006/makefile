NAME=taps_icmc2006

default: $(NAME).ps

tryit: $(NAME).ps

$(NAME).pdf: $(NAME).ps
#	ps2pdf -dPDFSETTINGS=/prepress -dCompatibilityLevel=1.3 $(NAME).ps $(NAME).pdf
	ps2pdf -dCompatibilityLevel=1.3 $(NAME).ps $(NAME).pdf

final.dvi: final.tex
	latex final.tex

final.ps: final.dvi
	dvips -G0 -t letter final.dvi -o final.ps

final.pdf: final.ps
	ps2pdf -dPDFSETTINGS=/prepress -dCompatibilityLevel=1.3 final.ps final.pdf

$(NAME).ps: $(NAME).dvi
	dvips -G0 -t letter $(NAME).dvi -o $(NAME).ps

$(NAME).dvi: $(NAME).tex
	latex $(NAME)

bib:
	rm *.bbl
	latex $(NAME)
	bibtex $(NAME)
	latex $(NAME)
	latex $(NAME)

page: 
	dvips -p $(FPAGE) -l $(FPAGE) $(NAME).dvi -o $(NAME).$(FPAGE).ps

clean:
	rm -f core *.dvi *.BAK *.CKP */*.BAK */*.CKP *~




