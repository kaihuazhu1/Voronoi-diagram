voronoi2: voronoi2.o stage1.o malloc.o intersect.o dcel.o stage2.o read.o stage3.o stage4.o
	gcc -Wall -o voronoi2 voronoi2.o stage1.o malloc.o intersect.o dcel.o stage2.o read.o stage3.o stage4.o -g -lm

voronoi2.o: voronoi2.c stage1.h malloc.h intersect.h dcel.h stage2.h read.h stage3.h stage4.h
	gcc -Wall -o voronoi2.o voronoi2.c -c -g -lm

stage1.o: stage1.c stage1.h malloc.h
	gcc -Wall -o stage1.o stage1.c -c -g

malloc.o: malloc.c malloc.h
	gcc -Wall -o malloc.o malloc.c -c -g

dcel.o: dcel.c dcel.h malloc.h
	gcc -Wall -o dcel.o dcel.c -c -g

intersect.o: intersect.c intersect.h 
	gcc -Wall -o intersect.o intersect.c -c -g
	
stage2.o: stage2.c stage2.h malloc.h intersect.h dcel.h
	gcc -Wall -o stage2.o stage2.c -c -g -lm

read.o: read.c read.h
	gcc -Wall -o read.o read.c -c -g
	
stage3.o: stage3.c stage3.h malloc.h intersect.h dcel.h read.h
	gcc -Wall -o stage3.o stage3.c -c -g -lm

stage4.o: stage4.c stage4.h malloc.h intersect.h dcel.h read.h
	gcc -Wall -o stage4.o stage4.c -c -g -lm

clean: voronoi2
	rm *.o voronoi2