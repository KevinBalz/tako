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
	{
		var heyo = myvar * 2;
		print heyo;
	}
	print heyo;
	print "Hello " + "Carla!";
	if (2 + 2 == 4)
	{
		print "of course";
	}
	else
	{
		print "wait what";
	}
	print "hi" or 2;
	print nil or "yes";
	print nil and "yes";
	var i = 0;
	while (i < 10)
	{
		print i;
		i = i + 1;
	}
)src";

int main()
{
	tako::Scripting::Run(sourceCode);
	return 0;
}
