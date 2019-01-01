barberproblem: barberproblem.c
	gcc barberproblem.c -o barberproblem -lpthread
.PHONY: clean
clean:
	rm -f barberproblem
	make clean
