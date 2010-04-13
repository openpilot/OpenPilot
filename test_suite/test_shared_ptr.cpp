/**
 * \file test_shared_ptr.cpp
 *
 * \date 19/03/2010
 * \author jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <map>
#include <list>
#include <vector>
#include "boost/numeric/ublas/vector.hpp"

using namespace boost;
using namespace std;

class Linked;
typedef shared_ptr<Linked> link_t;
typedef boost::numeric::ublas::vector<link_t> linksVec_t;
typedef weak_ptr<Linked> wklink_t;

class Linked {
	public:
		linksVec_t linksVec;
		size_t id;
		string type;
		string name;
		Linked(string _type, string _name):type(_type), name(_name){}
		virtual ~Linked(){}
		void resize(size_t size){linksVec.resize(size);}
		void addObj(size_t i, link_t obj_p){
			linksVec(i) = obj_p;
		}
		void printIds(){
			cout << "IDs : ";
			for (size_t i = 0; i<linksVec.size(); i++)
				cout << linksVec(i)->id << "; ";
			cout << endl;
		}
		void printTypes(){
			cout << "Types : ";
			for (size_t i = 0; i<linksVec.size(); i++)
				cout << linksVec(i)->type << "; ";
			cout << endl;
		}
		void printNames(){
			cout << "Names : ";
			for (size_t i = 0; i<linksVec.size(); i++)
				cout << linksVec(i)->name << "; ";
			cout << endl;
		}
		void printFriends(){
			cout << type << " " << name << "'s Friends ";
			printTypes();
			cout << type << " " << name << "'s Friends ";
			printNames();}
};

class Cat : public Linked {
	public:
		Cat(string name) : Linked((string)"Cat", name) {}
};

class Dog : public Linked {
	public:
		Dog(string name) : Linked(string("Dog"), name) {}
};

void test_shared_ptr01(void) {

	shared_ptr<Cat> cp1(new Cat("meow"));
	shared_ptr<Cat> cp2(new Cat("miau"));
	shared_ptr<Dog> dp1(new Dog("bub"));

	cp1->resize(2);
	cp1->addObj(0, cp2);
	cp1->addObj(1, dp1);

	cp2->resize(2);
	cp2->addObj(0, cp1);
	cp2->addObj(1, dp1);

	dp1->resize(3);
	dp1->addObj(0, cp1);
	dp1->addObj(1, cp2);
	dp1->addObj(2, dp1);

	cp1->printFriends();
	cp2->printFriends();
	dp1->printFriends();

	dp1->linksVec(2)->linksVec(1)->linksVec(1)->printFriends();
}

BOOST_AUTO_TEST_CASE( test_shared_ptr )
{
	test_shared_ptr01();
}

