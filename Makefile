CC	= g++
C       = cpp
H	= h
CFLAGS = -g -I/group/dpa/include

LDFLAGS     = -L/usr/lib64/ -L/group/dpa/lib -lglut -lGL -lGLU -lOpenImageIO -lm

HFILES 	= Matrix.${H} Vector.${H} Utility.${H}
OFILES 	= Matrix.o Vector.o Utility.o 
PROJECT		= warper
PROJECT2        = interactive

all: ${PROJECT} ${PROJECT2}

${PROJECT}:	${PROJECT}.o $(OFILES)
	${CC} $(CFLAGS) -o ${PROJECT} ${PROJECT}.o $(OFILES) $(LDFLAGS)

${PROJECT}.o: ${PROJECT}.${C} $(HFILES)
	${CC} $(CFLAGS) -c ${PROJECT}.${C}

${PROJECT2}:	${PROJECT2}.o $(OFILES)
	${CC} $(CFLAGS) -o ${PROJECT2} ${PROJECT2}.o $(OFILES) $(LDFLAGS)

${PROJECT2}.o: ${PROJECT2}.${C} $(HFILES)
	${CC} $(CFLAGS) -c ${PROJECT2}.${C}

Matrix.o: Matrix.${C} Matrix.${H} Vector.${H} Utility.${H} 
	${CC} $(CFLAGS) -c Matrix.${C}

Vector.o: Vector.${C} Vector.${H} Utility.${H} 
	${CC} $(CFLAGS) -c Vector.${C}

Utility.o: Utility.${C} Utility.${H}
	${CC} $(CFLAGS) -c Utility.${C}

clean:
	rm core.*; rm *.o; rm *~; rm ${PROJECT} ${PROJECT2}

