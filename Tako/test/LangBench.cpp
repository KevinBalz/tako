#include "Scripting/Scripting.hpp"

const char* sourceCode = R"src(
	# This is a comment
	# var a = 2 + 2;
	# if (2 <= 4 or true)
	# {
	#	return 4 / 2;
	# }
	print 2 + 2 <= -((4 + 2) / 2 + 3 * 4);
	print "Hello World!";
	var myvar = (2 + 3) * -(3 - 4);
	print myvar;
	var heyo = myvar = myvar + 1;
	print myvar;
	print heyo;
	print "Hello " + "Carla!";
)src";

int main()
{
	tako::Scripting::Run(sourceCode);
	return 0;
}
