
PNG2FONT_OBJS=png2font.o pngread.o
PNG2BITMAP_OBJS=png2bitmap.o pngread.o

LDFLAGS=-L/opt/local/lib
CFLAGS=-Wall -02 -g -I/opt/local/include

all: png2font png2bitmap

png2font: ${PNG2FONT_OBJS}
	gcc -o $@ ${LDFLAGS} ${CFLAGS} $^ -lpng

png2bitmap: ${PNG2BITMAP_OBJS}
	gcc -o $@ ${LDFLAGS} ${CFLAGS} $^ -lpng