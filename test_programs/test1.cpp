#include <iostream>
#include <string>

#include "../global/ddfs_global.h"

using namespace std;

int main(int argc, const char *argv[])
{

	ddfsGlobal::initialize("/tmp/red.log");

	return 0;
}
