#include <iostream>

#include "../src/global/ddfs_status.hpp"
#include "../src/global/ddfs_global.hpp"
#include "../src/cluster/ddfs_clusterPaxos.hpp"

using namespace std;

int main(int argc, const char *argv[])
{

	DDFS_STATUS k = DDFS_OK;
	ddfsStatus status(k);

	red r1;

	cout << "Red value : " << r1.getV() << "\n";

	cout << "Red initalize : " << r1.initialize() << "\n";


	ddfsGlobal::initialize();

	ddfsClusterPaxos *cluster = new ddfsClusterPaxos("192.168.2.16");

	cout << "Sleeping for three seconds.\n";
	sleep(3);
	cout << "Trying to add dd1 \n";
	cluster->addMember("192.168.2.15");

    cout << "Trying to add dd2 \n";
    cluster->addMember("192.168.2.14");

    cout << "Sleeping for 4 seconds.\n";
	sleep(3);

	cout << "Starting the leader election : \n";
	status = cluster->leaderElection();

	if(status.compareStatus(ddfsStatus(DDFS_OK)) == true)
		cout << "Leader Election Successfully Completed.\n";
	else
		cout << "Leader Election failed.\n";

#if 0
	cout << "Trying to add testNode-2 \n";
	cluster->addMember("192.168.2.14");
#endif
	int inputValue;
	cin >> inputValue;

	cout << "Exiting";
	sleep(3);

	return 0;
}
