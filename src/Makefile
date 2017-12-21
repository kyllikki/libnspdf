#

CFLAGS+=-g -Wall -Wextra

OBJS=xref.o byte_class.o cos_decode.o cos_object.o pdf_doc.o

.PHONY:all clean

all:xref

xref:$(OBJS)

clean:
	${RM} xref $(OBJS)
