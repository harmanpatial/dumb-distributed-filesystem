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

	ddfsClusterPaxos *cluster = new ddfsClusterPaxos("192.168.2.14");

	cout << "Sleeping for three seconds.\n";
	sleep(3);
	cout << "Trying to add dd1 \n";
	cluster->addMember("192.168.2.15");

    cout << "Trying to add dd3 \n";
    cluster->addMember("192.168.2.16");

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
