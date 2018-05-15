#include "tunnel.h"
#include <assert.h>
using namespace std;


tunnel::tunnel()
{
}

void tunnel::print_memory()
{
	int *content = (int *)load_mem(KEY_SHAREDMEM_CONTENT);
	printf("*************************************************\n");
	printf("[Info]:                 Tunnel Status\n");
	printf("[Detail]:                    Tunnel Memory:\n");
	for (int i = 0; i < total_number_of_mailboxes; i++)
	{
		printf("[Detail]:                    mailbox No.%d:", i);
		for (int j = 0; j < memory_segment_size; j++)
		{
			if (content[i*memory_segment_size + j] == 0)
			{
				break;
			}
			printf("%c", content[i*memory_segment_size + j]);
		}
		printf("\n");
	}
	printf("*************************************************\n");
}

void tunnel::setup_ipc()
{
	create_sharedmem(KEY_SHAREDMEM_CONTENT, sizeof(int)*total_number_of_mailboxes*memory_segment_size);
	create_sharedmem(KEY_SHAREDMEM_READER_COUNT, sizeof(int)*total_number_of_mailboxes);
	create_sharedmem(KEY_SHAREDMEM_VEHICLE_COUNT, sizeof(int));
	create_sem(KEY_SEM_WAITING, 1);
	create_sem(KEY_SEM_MAILBOX_READER_COUNT, total_number_of_mailboxes);
	create_sem(KEY_SEM_MAILBOX_READ_OR_WRITE, total_number_of_mailboxes);
	create_sem(KEY_SEM_MAILBOX_WRITER, total_number_of_mailboxes);
	int *count = (int *)load_mem(KEY_SHAREDMEM_VEHICLE_COUNT);
	*count = maximum_number_of_cars_in_tunnel;
}

void tunnel::clear_ipc()
{
	
}


void tunnel::reset_env_ipc()
{
	system("sudo ipcrm -a");
}

void tunnel::read()
{
	cin >> total_number_of_cars >>
		maximum_number_of_cars_in_tunnel >>
		tunnel_travel_time >>
		total_number_of_mailboxes >>
		memory_segment_size;
	string x;
	for (int i = 0; i < total_number_of_cars; i++)
	{
		cin >> x;
		assert(x[7] - 1 - '0' == i);
		string op, msg;
		int duration, mailboxnumber, id = 0, length;
		vector<r_message>* tmp_r = new vector<r_message>;
		vector<w_message>* tmp_w = new vector<w_message>;
		while (true)
		{
			id++;
			cin >> op;
			if (op == "end.")
			{
				break;
			}

			if (op == "w")
			{
				cin >> msg >> duration >> mailboxnumber;
				tmp_w->push_back(w_message(msg, mailboxnumber, duration, id));
			}
			else if (op == "r")
			{
				cin >> length >> duration >> mailboxnumber;
				tmp_r->push_back(r_message(length, mailboxnumber, duration, id));
			}
		}
		r.push_back(*tmp_r);
		w.push_back(*tmp_w);
	}

}

tunnel::~tunnel()
{
}
