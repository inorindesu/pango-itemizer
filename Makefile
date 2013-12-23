
pango-itemizer: main.c
	gcc main.c `pkg-config --libs --cflags pango pangoft2` -o pango-itemizer -Wall

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf pango-itemizer
