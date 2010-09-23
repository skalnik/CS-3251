COMPILER = gcc
.PHONY: tcp udp clean tar

tcp: RPNcl-tcp RPNsrv-tcp

udp: RPNcl-udp RPNsrv-udp

RPNcl-tcp: RPNcl-tcp.c
		${COMPILER} -o $@ RPNcl-tcp.c

RPNsrv-tcp: RPNsrv-tcp.c
		${COMPILER} -o $@ RPNsrv-tcp.c

RPNcl-udp: RPNcl-udp.c
		${COMPILER} -o $@ RPNcl-udp.c

RPNsrv-udp: RPNsrv-udp.c
		${COMPILER} -o $@ RPNsrv-udp.c

tar: clean
		tar czvf skalnik.tar.gz RPN{cl,srv}-{udp,tcp}.c Makefile README

clean:
		rm -rf RPN{cl,srv}-{tcp,udp} skalnik.tar.gz
