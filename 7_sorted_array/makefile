
test: clean test.o lib_ar.a
	gcc -fprofile-arcs -ftest-coverage  -o test test.o lib_ar.a
	./test
	gcov sarr_lib

gcov: test
	valgrind --leak-check=full ./test
	gcov sarr_lib

test.o: test.c
	gcc -c -o test.o test.c

sarr_lib.o: sarr_lib.c
	gcc -fprofile-arcs -ftest-coverage -c -o sarr_lib.o sarr_lib.c

lib_ar.a: sarr_lib.o
	ar rcs lib_ar.a sarr_lib.o

clean:
	rm -rf *.d *.o *.gcno *.gcov *.gcda *.info *.a html test

sarr_lib.o test.o : sarr_lib.h
