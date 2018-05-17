#include <unistd.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include "vehicle.h"
#include "tunnel.h"
using namespace std;
int main()
{
	freopen("/home/ubuntu/linuxproj/sample.in", "r", stdin);
	//freopen("/home/ubuntu/linuxproj/sample.out", "a", stdout);
	tunnel T;
	pid_t mypid;
	int vehicle_number=0;

	T.reset_env_ipc();
	T.read();
	T.setup_ipc();
	for (int i = 0; i < T.total_number_of_cars; i++)
	{
		mypid = fork();
		if (mypid == 0)
		{
			vehicle_number = i;
			printf("[Info, Timestamp:%s]:  Car %d is coming\n", getTime().c_str(),vehicle_number);
			break;
		}
	}

	if (mypid == 0)//child
	{
		vehicle my_car(T,vehicle_number);
		my_car.run();
	}
	else
	{
		for (int i = 0; i < T.total_number_of_cars; i++)
		{
			pid_t t= wait(NULL);
		}
		T.print_memory();
	}
	return 0;
}