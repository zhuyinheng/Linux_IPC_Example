#pragma once
#ifndef UTILS
#define UTILS


#define KEY_SHAREDMEM_VEHICLE_COUNT 11100
#define KEY_SHAREDMEM_READER_COUNT 11200
#define KEY_SHAREDMEM_CONTENT 11300
#define KEY_SEM_WAITING 10100
#define KEY_SEM_MAILBOX_READ_OR_WRITE 10200
#define KEY_SEM_MAILBOX_WRITER 10308
#define KEY_SEM_MAILBOX_READER_COUNT 1048

#include <string>
#include <stdlib.h>  
#include <stdio.h>
#include <sys/shm.h>  
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
using namespace std;
extern void create_sem(int key, int number);
extern void create_sharedmem(int key, int number);
extern int load_sem(int key);
extern int P(int semid, int which);
extern int V(int semid, int which);
extern void *load_mem(int key);
extern string getTime();
class r_message
{
public:
	int length, mailbox_number, duration,id;
	bool used;
	r_message(int length, int mailbox_number, int duration,int id)
	{
		this->length = length;
		this->mailbox_number = mailbox_number;
		this->duration = duration;
		this->id = id;
		used = false;
	}
};
class w_message
{
public:
	string message;
	int mailbox_number, duration,id;
	w_message(string message, int mailbox_number, int duration,int id)
	{
		this->message = message;
		this->mailbox_number = mailbox_number;
		this->duration = duration;
		this->id = id;
	}
};

#endif // Utils
