SRC += csb.c pca9685.c
# pca9685.c
OBJ = ${SRC:.c=.o}
CFLAGS += --std=c99 -o2 -g -Wall
LIBS += -lwiringPi -lwiringPiDev

# %.o:%.c

main.o: $(SRC)
	gcc $^ ${LIBS} ${CFLAGS} -o $@

clean:
	# gcc $< -g -Wall -O2 -o $@ -lwiringPi -lwiringPiDev
	# gcc csb.c –o csb pc9685.c –lwiringPi -lwiringPiDev
	# gcc -o csb csb.c pca9685.c -lwiringPi -lwiringPiDev --std=c99
	rm -rf *.o
