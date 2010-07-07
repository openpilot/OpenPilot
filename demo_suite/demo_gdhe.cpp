
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
	gdhe::Sphere sphere(0.5);
	sphere.setPose(1.0, 1, 1, 0, 0, 0);
	sphere.setColor(0, 0, 255);
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
	std::cout << "Moving target..." << std::endl;
	gdheClient.setCameraTarget(0,-1,-1);
	sleep(1);
	std::cout << "... refresh" << std::endl;
	sphere.refresh();
	sleep(3);
	std::cout << "Dumping to test.ppm..." << std::endl;
	gdheClient.dump("test.ppm");
	sleep(3);
	std::cout << "Removing objects" << std::endl;
	gdheClient.removeObject(&dala);
	sphere.remove();
	sleep(3);
	
	std::cout << "Testing ellipsoid" << std::endl;
	gdhe::Grid grid(10, 1);
	gdheClient.addObject(&grid);
	gdhe::Robot rob("atrv", 0, 0, 0, 0, 0, 0);
	gdheClient.addObject(&rob);
	
	gdhe::Ellipsoid ellipsoid(12);
	jblas::vec3 x; x(0) = 2; x(1) = 2; x(2) = 0;
	jblas::sym_mat33 P; 
	P(0,0) = 1;
	P(1,0) = 0.0; P(1,1) = 0.5;
	P(2,0) = 0.0; P(2,1) = 0.0; P(2,2) = 0.5;
	ellipsoid.set(x, P);
	ellipsoid.setColor(255,0,0);
	gdheClient.addObject(&ellipsoid);
	sleep(60);
	
	
	std::cout << "Terminating program" << std::endl;
	
}


