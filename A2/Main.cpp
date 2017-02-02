#include "A2.hpp"

int main( int argc, char **argv ) 
{
    int dim = 650; //768;
	CS488Window::launch( argc, argv, new A2(), dim, dim, "Assignment 2" );
	return 0;
}
