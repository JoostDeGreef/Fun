
.PHONY: all
all:
	@for M in `ls Makefile.*|grep -v Makefile.inc`; \
	do \
	  make -f $${M} all; \
	done
	
.PHONY: clean
clean:
	rm -f *~
	rm -rf tmp
	rm -rf bin
	

