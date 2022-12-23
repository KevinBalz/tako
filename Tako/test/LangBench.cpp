#include "Scripting/Scripting.hpp"

const char* sourceCode = R"src(
	#42; # meaning of life
	#print "Start!";
	#print -((-2 + 4) * 3 - 12 * 12);
	#print !true;
	#print !(5 - 4 > 3 * 2 == !nil);
	#print "Hello World" == "Hello World";
	var meaning = 42;
	print meaning;
	meaning = "there is no meaning";
	print meaning;
	{
		var meaning = 12;
		print meaning;
		{
			var meaning = meaning + 1;
			print meaning;
			meaning = 14;
			print meaning;
		}
		print meaning;
	}
	print meaning;
)src";

int main()
{
	tako::Scripting::Run(sourceCode);
	return 0;
}
