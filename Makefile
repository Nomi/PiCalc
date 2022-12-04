CC=g++#or clang++ #or g++
FLAGS=-lgmp
STANDARD=c++20

all : ${ALLTARGETS}
	${CC} *.h -std=${STANDARD} *.cpp -o picalc ${FLAGS}


.PHONY: 
	phony

clean: 
	rm -f picalc