bakerproblem: bakerproblem.c
	gcc -o bakerproblem bakerproblem.c
.PHONY: clean
clean:
	rm -f bakerproblem
	make clean

