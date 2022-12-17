#include "Scripting/Scripting.hpp"

const char* sourceCode = R"src(
	#-((-2 + 4) * 3 - 12 * 12)
	!true
)src";

int main()
{
	tako::Scripting::Run(sourceCode);
	return 0;
}
