#include <iostream>
#include <string>

#include "../src/global/ddfs_global.h"

using namespace std;

int main(int argc, const char *argv[])
{
	string log_file = "/tmp/red.log";

	ddfsGlobal::initialize(log_file);

	return 0;
}
