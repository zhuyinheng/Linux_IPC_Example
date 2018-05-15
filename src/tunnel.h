#pragma once
#include"utils.h"
#include<vector>
#include<iostream>
using namespace std;
class tunnel
{
public:
	vector<vector<r_message> > r;
	vector<vector<w_message> > w;
	int total_number_of_mailboxes, memory_segment_size;
	int total_number_of_cars, maximum_number_of_cars_in_tunnel, tunnel_travel_time;



	tunnel();
	void print_memory();
	void setup_ipc();
	void clear_ipc();
	void reset_env_ipc();
	void read();
	~tunnel();
};

