#include <sys/types.h>  
#include <sys/ipc.h>  
#include <sys/shm.h>  
#include <sys/sem.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include "sem_com.h"  
 
#define BUFFER_SIZE 2048  
  
pid_t pid;  
int shmid;//要映射的共享内存区标识符  
int semid;//定义信号量，用于实现共享内存的进程之间的互斥  
char *shm_addr;//共享内存映射到的地址  
char buff[BUFFER_SIZE];  
      
void function_init()  
{  
      
    //创建一个信号量  
    semid = semget(ftok("/home",2), 1, 0666|IPC_CREAT);  
    init_sem(semid, 1);//初始值为1  
      
    //创建共享内存
    shmid = shmget(ftok("/home", 3), BUFFER_SIZE, 0666|IPC_CREAT);
    if (shmid < 0) {  
        perror("shmget");  
        exit(1);  
    } else {  
        printf("create shard-memory shmid: %d\n", shmid);  
    }  
      
}  
  
void function_end()  
{  
      
    //删除信号量  
    del_sem(semid);   
    //解除共享内存映射  
    if ((shmdt(shm_addr)) < 0) {  
        perror("shmdt");  
        exit(1);  
    }   
          
    //删除共享内存IPC_RMID    删除共享内存段  
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {  
        perror("shmctl");  
        exit(1);  
    } else {  
        printf("Delete shared-memory\n");  
    }  
          
} 

int main()  
{  
      
    function_init();  
      
    pid = fork();  
    if (pid == 0) {//子进程  
        //映射共享内存
        key_t key = ftok("/home", 3);
        if (key == -1) {
        	perror("ftok error");
        	exit(1);
        }
        int shmid1 = shmget(key, 0, 0);
        if (shmid1 == -1) {
        	perror("shmget error");
        	exit(1);
        }
        shm_addr = shmat(shmid1, NULL, 0);  
        if (shm_addr == (void *)-1) {  
            perror("Child:　shmat");  
            exit(1);  
        } else  
        {  
            do{  
                sem_p(semid);  
                printf("\nChild:Attach shared-memory %p :%s \n", shm_addr, shm_addr);  
                if (strncmp(shm_addr, "quit", 4) == 0) {  
                    break;  
                }  
                memset(shm_addr, 0, BUFFER_SIZE);  
                sem_v(semid);  
            } while(1);  
              
        }  
        //显示共享内存情况  
        //system("ipcs -m");  
          
    } else if(pid > 0){//父进程  
        //映射共享内存  
        shm_addr = shmat(shmid, NULL, 0);  
          
        //父进程向共享内存写入数据  
        do {  
              
            sem_p(semid);//信号值减一  
            printf("Parent:Enter some text to the shared memory(enter 'quit' to exit):\n");  
            if(fgets(buff, BUFFER_SIZE, stdin) == NULL) {  
                perror("fgets");  
                sem_v(semid);  
                break;  
            }  
            strncpy(shm_addr, buff, strlen(buff));  
            sem_v(semid);  
        } while(strncmp(buff, "quit", 4) != 0);  
                  
        function_end();   
    }   
          
    exit(0);  
}
