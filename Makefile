shm: sem_com.o shmem.o
	gcc sem_com.o shmem.o -o shm

sem_com.o: sem_com.h sem_com.c
	gcc -c sem_com.c
	
shmem.o: sem_com.h shmem.c
	gcc -c shmem.c
	
clean:
	rm -rf *.o shm
