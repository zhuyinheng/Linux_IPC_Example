#include <string>
#include <sys/shm.h>  
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "utils.h"
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
							 (Linux-specific) */
};
void create_sem(int key, int number)
{
	int semid = semget((key_t)key, number, IPC_CREAT|0666);
	if (semid < 0)
	{
		perror("Semget error");
		exit(EXIT_FAILURE);
	}
	union semun sem_un;
	for(int i=0;i<number;i++)
	{
		sem_un.val = 1;
		int rt=semctl(semid, i, SETVAL, sem_un);
		if(rt<0)
		{
			perror("Semctl Set val error");
			exit(EXIT_FAILURE);
		}
	}
}
void create_sharedmem(int key, int number)
{
	int shmid = shmget((key_t)key, number, 0666 | IPC_CREAT);
	if (shmid == -1)
	{
		printf("shmget failed\n");
		exit(EXIT_FAILURE);
	}
}
void destroy_sharedmem(int key)
{
	int shmid = shmget((key_t)key, 0, IPC_CREAT);
	if (shmid == -1)
	{
			printf("Destroy null");
			exit(EXIT_FAILURE);
	}
	else
	{
		int ret = shmctl(shmid, IPC_RMID, 0);
		if (ret < 0)
		{
			printf("Destroy failed");
			exit(EXIT_FAILURE);
		}

	}
}
int load_sem(int key)
{
	int semid = semget((key_t)key, 0, IPC_CREAT);
	if (semid == -1)
	{
		perror("load error");
	}
	return semid;
}
int P(int semid, int which)    // 表示通过 信号量值-1
{
	sembuf mysembuf;
	mysembuf.sem_num = which;
	mysembuf.sem_op = -1;
	mysembuf.sem_flg = SEM_UNDO;
	return semop(semid, &mysembuf, 1);
}
int V(int semid, int which)    // 表示释放 信号量值+1
{
	sembuf mysembuf;
	mysembuf.sem_num = which;
	mysembuf.sem_op = 1;
	mysembuf.sem_flg = SEM_UNDO;
	return semop(semid, &mysembuf, 1);
}
void *load_mem(int key)
{
	int shmid = shmget((key_t)key, 0, IPC_CREAT);
	if (shmid == -1)
	{
		perror("shmget failed\n");
		exit(EXIT_FAILURE);
	}
	void *shm = shmat(shmid, (void*)0, 0);
	if (shm == (void*)-1)
	{
		perror("shmat failed\n");
		exit(EXIT_FAILURE);
	}
	return shm;
}
string getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
	return tmp;
}