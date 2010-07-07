
#include <iostream>
#include <fstream>
#include <unistd.h>

#include <gdhe/client.hpp>







using namespace jafar;



int main(int argc, char **argv)
{
	gdhe::Client gdheClient("localhost");
	gdheClient.launch_server();
	gdheClient.connect();

	if (argc != 3) { std::cerr << "Usage: demo_source <file-name> <delay-ms>" << std::endl; return 1; }

	std::fstream f(argv[1], std::ios_base::in);
	int delay = atoi(argv[2]);
	std::string line;
	if (f.is_open())
	{
		int i = 0;
		while (!f.eof())
		{
			++i;
			getline (f,line);
			std::cout << "evaluating line " << i << std::endl;
			gdheClient.eval(line);
			usleep(delay*1000);
		}
		f.close();
	}
	
}


