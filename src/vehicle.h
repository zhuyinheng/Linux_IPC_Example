#pragma once
#include "tunnel.h"
#include "utils.h"
#include <vector>
#include <sys/types.h>
using namespace std;
class vehicle
{
private:
	int idx_r, idx_w;
	vector<r_message> my_r;
	vector<w_message> my_w;
	int id;
	tunnel *T;
	struct timeval start_time_excluding_waiting;
	struct timespec start_time_including_waiting;
	int *count,
		*content,
		*reader_count,
		*mailboxs_pointers;
	int sem_mutex_reader_count,
		sem_mutex_read_or_write,
		sem_mutex_writer,
		sem_waiting;
	vector<vector<int> > readed_msg;
public:
	vehicle(tunnel &T, int id);
	void waiting_and_in();
	void leave();
	void write_to_mailbox(w_message);
	void read_from_mailbox(r_message, vector<int> &rt);
	void run();
	~vehicle();
};

