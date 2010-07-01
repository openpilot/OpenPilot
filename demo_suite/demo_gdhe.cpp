
#include <iostream>

#include <gdhe/client.hpp>








using namespace jafar;



int main(int argc, char **argv)
{
	gdhe::Client gdheClient;
	std::cout << "Launching server" << std::endl;
	gdheClient.launch_server();
	std::cout << "Connecting to server" << std::endl;
	gdheClient.connect();

	std::cout << "Building objects" << std::endl;
	gdhe::Robot dala("atrv", -1, -1, -1, 0, 0, 0);
	gdhe::Sphere sphere(1.0, 1, 1, 0.5, 0, 0, 255);
	sleep(1);
	std::cout << "Adding objects" << std::endl;
	gdheClient.addObject(&dala);
	gdheClient.addObject(&sphere);
	sleep(2);
	std::cout << "Hiding objects" << std::endl;
	dala.hide();
	sphere.hide();
	sleep(2);
	std::cout << "Reshowing objects" << std::endl;
	dala.show();
	sphere.show();
	sleep(2);
	std::cout << "Moving objects..." << std::endl;
	dala.setPose(0,-1,-1,0,0,0);
	sleep(1);
	std::cout << "... refresh" << std::endl;
	dala.refresh();
	sleep(2);
	std::cout << "Changing attributes..." << std::endl;
	sphere.setColor(255,0,0);
	sphere.setRadius(0.3);
	sleep(1);
	std::cout << "... refresh" << std::endl;
	sphere.refresh();
	sleep(3);
	std::cout << "Removing objects" << std::endl;
	gdheClient.removeObject(&dala);
	sphere.remove();
	sleep(4);
	std::cout << "Terminating program" << std::endl;
	
}


