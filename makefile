all:
	gcc raycast.c -o raycast -lm

clean:
	rm -rf raycast *~
