#include <cstdio>
#include <fstream>
#include "java_class_parse.h"

int main()
{
	std::string filename = "test.class";
	std::ifstream f(filename,std::fstream::in|std::fstream::binary);
	if(!f.is_open())
	{
		std::cout<<"f openfile wrong"<<std::endl;
	}
	ClassFile cf(f);
	method_info* info = lookup_method(cf,"main");
	//cf.print_method(info);
}
