#include "vehicle.h"
#include <stdlib.h>  
#include <stdio.h>  
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>  

using namespace std;

vehicle::vehicle(tunnel &T,int id)
{
	this->need_to_go = false;
	this->id = id;
	this->T = &T;
	my_r = T.r[id];
	my_w = T.w[id];
	content = (int *)load_mem(KEY_SHAREDMEM_CONTENT);
	reader_count = (int *)load_mem(KEY_SHAREDMEM_READER_COUNT);
	sem_mutex_reader_count = load_sem(KEY_SEM_MAILBOX_READER_COUNT);
	sem_mutex_read_or_write = load_sem(KEY_SEM_MAILBOX_READ_OR_WRITE);
	sem_mutex_writer = load_sem(KEY_SEM_MAILBOX_WRITER);
	sem_waiting = load_sem(KEY_SEM_WAITING);
	count = (int *)load_mem(KEY_SHAREDMEM_VEHICLE_COUNT);

	this->mailboxs_pointers = new int[T.total_number_of_mailboxes];
	for (int i = 0; i < T.total_number_of_mailboxes; i++)
	{
		this->mailboxs_pointers[i] = 0;
	}
	
	idx_r = 0;
	idx_w = 0;
}

void vehicle::waiting_and_in()
{
	int semid = sem_waiting;
	
	while (true)
	{
		P(semid, 0);
		if (*count > 0)
		{
			printf("[Info]:  Car %d is in the tunnel\n", this->id);
			*count = *count - 1;
			V(semid, 0);
			break;
		}
		else
		{
			V(semid, 0);
			if (idx_r >= my_r.size() && idx_w >= my_w.size())
			{
				continue;
			}
			if (idx_r < my_r.size() && idx_w >= my_w.size())
			{
				printf("[Info]:  Car %d is outside the tunnel reading for %d sec\n", this->id, my_r[idx_r].duration);
				sleep(my_r[idx_r].duration);
				idx_r++;
				continue;
			}

			if (idx_r >= my_r.size() && idx_w < my_w.size())
			{
				printf("[Info]:  Car %d is outside the tunnel writing for %d sec\n", this->id, my_w[idx_w].duration);
				sleep(my_w[idx_w].duration);
				idx_w++;
				continue;
			}

			if (my_r[idx_r].id < my_w[idx_w].id)
			{
				printf("[Info]:  Car %d is outside the tunnel reading for %d sec\n", this->id, my_r[idx_r].duration);
				sleep(my_r[idx_r].duration);
				idx_r++;
			}
			else
			{
				printf("[Info]:  Car %d is outside the tunnel writing for %d sec\n", this->id, my_w[idx_w].duration);
				sleep(my_w[idx_w].duration);
				idx_w++;
			}
		}


	}
}

void vehicle::leave()
{
	P(this->sem_waiting, 0);
	*count=*count+1;
	V(this->sem_waiting, 0);
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("[Info]:  Car %d is leaving\n",this->id);
	printf("[Detail]:        Runing time:%lf:\n", (((double)(now.tv_sec - this->start_time.tv_sec))));
	printf("[Detail]:        Memory Summary:\n");
	for (int i = 0; i < readed_msg.size(); i++)
	{
		printf("[Detail]:            Messge No. %d:",i);
		for (int j = 0; j < readed_msg[i].size(); j++)
		{
			if (readed_msg[i][j] == -1)
			{
				printf("***mailbox botton***");
				break;
			}
			else
			{
				printf("%c", readed_msg[i][j]);
			}
		}
		printf("\n");
	}
	/*P(this->sem_waiting, 0);
	if (*count == T->maximum_number_of_cars_in_tunnel)
	{
		printf("*************************************************\n");
		printf("[Info]:                 Tunnel Status\n");
		printf("[Detail]:                    Tunnel Memory:\n");
		for (int i = 0; i < T->total_number_of_mailboxes; i++)
		{
			printf("[Detail]:                    mailbox No.%d:",i);
			for (int j = 0; j < T->memory_segment_size; j++)
			{
				if (content[i*T->memory_segment_size + j] == 0)
				{
					break;
				}
				printf("%c", content[i*T->memory_segment_size + j]);
			}
			printf("\n");
		}
		printf("*************************************************\n");
	}
	V(this->sem_waiting, 0);*/
}


void vehicle::write_to_mailbox(w_message msg)
{
	P(sem_mutex_read_or_write, msg.mailbox_number);
	P(sem_mutex_writer, msg.mailbox_number);
	int j = 0;
	while (j < T->memory_segment_size && content[msg.mailbox_number*T->memory_segment_size +j] != 0)j++;
	for (int i=j; i < T->memory_segment_size && i-j<msg.message.length(); i++)
	{
		content[msg.mailbox_number*T->memory_segment_size +i] = msg.message[i - j];
	}
	sleep(msg.duration);
	V(sem_mutex_writer, msg.mailbox_number);
	V(sem_mutex_read_or_write, msg.mailbox_number);
}

void vehicle::read_from_mailbox(r_message msg, vector<int> &rt)
{
	P(sem_mutex_reader_count,msg.mailbox_number);
	reader_count[msg.mailbox_number]++;
	if (reader_count[msg.mailbox_number] == 1) P(sem_mutex_read_or_write,msg.mailbox_number);
	V(sem_mutex_reader_count, msg.mailbox_number);
	sleep(msg.duration);
	int j;
	for (j = this->mailboxs_pointers[msg.mailbox_number]; j < T->memory_segment_size && content[msg.mailbox_number* T->memory_segment_size +j] != 0; j++)
	{
		rt.push_back(content[msg.mailbox_number*T->memory_segment_size +j]);
	}
	if (j - this->mailboxs_pointers[msg.mailbox_number] < msg.length)
	{
		rt.push_back(-1);
	}
	else
	{
		rt.push_back(0);
	}
	this->mailboxs_pointers[msg.mailbox_number] = j;
	
	P(sem_mutex_reader_count, msg.mailbox_number);
	reader_count[msg.mailbox_number]--;
	if (reader_count[msg.mailbox_number] == 0) V(sem_mutex_read_or_write, msg.mailbox_number);
	V(sem_mutex_reader_count, msg.mailbox_number);
	
	vector<int>* tmp = new  vector<int>(rt);
	this->readed_msg.push_back(*tmp);
}

void vehicle::run()
{
	
	waiting_and_in();
	gettimeofday(&(this->start_time), NULL);
	while (idx_r < my_r.size() && idx_w < my_w.size())
	{
		struct timeval now;
		gettimeofday(&(now), NULL);
		if (now.tv_sec-this->start_time.tv_sec > this->T->tunnel_travel_time)
		{
			break;
		}
		if (my_r[idx_r].id < my_w[idx_w].id)
		{
			vector<int>	rt;
			read_from_mailbox(my_r[idx_r], rt);
			printf("[Info]:  Car %d is Reading\n", this->id);
			printf("[Detail]:        Read %d sec in mailbox No. %d\n", my_r[idx_r].duration, my_r[idx_r].mailbox_number);
			printf("[Detail]:        The content:");
			for (int i = 0; i < rt.size(); i++)
			{
				if (rt[i] == -1)
				{
					printf("***mailbox botton***");
				}
				else
				{
					printf("%c", rt[i]);
				}
			}
			printf("\n");
			idx_r++;
		}
		else
		{
			write_to_mailbox(my_w[idx_w]);
			printf("[Info]:  Car %d is Writing\n", this->id);
			printf("[Detail]:        Write %d sec in mailbox No. %d\n", my_w[idx_w].duration, my_w[idx_w].mailbox_number);
			printf("[Detail]:        The content:");
			cout << my_w[idx_w].message << endl;
			idx_w++;
		}
	}
	while (idx_r < my_r.size())
	{
		struct timeval now;
		gettimeofday(&(now), NULL);
		if (now.tv_sec - this->start_time.tv_sec > this->T->tunnel_travel_time)
		{
			break;
		}
		vector<int>	rt;
		read_from_mailbox(my_r[idx_r], rt);
		printf("[Info]:  Car %d is Reading\n", this->id);
		printf("[Detail]:        Read %d sec in mailbox No. %d\n", my_r[idx_r].duration, my_r[idx_r].mailbox_number);
		printf("[Detail]:        The content:");
		for (int i = 0; i < rt.size(); i++)
		{
			if (rt[i] == -1)
			{
				printf("***mailbox botton***");
			}
			else
			{
				printf("%c", rt[i]);
			}
		}
		printf("\n");
		idx_r++;
	}
	while (idx_w < my_w.size()) 
	{
		struct timeval now;
		gettimeofday(&(now), NULL);
		if (now.tv_sec - this->start_time.tv_sec > this->T->tunnel_travel_time)
		{
			break;
		}
		write_to_mailbox(my_w[idx_w]);
		printf("[Info]:  Car %d is Writing\n", this->id);
		printf("[Detail]:        Write %d sec in mailbox No. %d\n", my_w[idx_w].duration, my_w[idx_w].mailbox_number);
		printf("[Detail]:        The content:");
		cout << my_w[idx_w].message << endl;
		idx_w++;
	}
	
	struct timeval now;
	gettimeofday(&(now), NULL);
	if (now.tv_sec - this->start_time.tv_sec < this->T->tunnel_travel_time)
	{
		printf("[Info]:  Car %d is Runing\n", this->id);
		sleep(this->T->tunnel_travel_time - ((double)(now.tv_sec - this->start_time.tv_sec)));
	}
	leave();

}

vehicle::~vehicle()
{
}
