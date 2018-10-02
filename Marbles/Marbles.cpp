// marbles.cpp : Defines the entry point for the console application.
//

#if !defined NDEBUG
// Implemented to resolve: "error LNK2019: unresolved external symbol __iob_func referenced in function loader_log
FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
}
#endif // #if !defined NDEBUG

#if defined CONFIG_DEBUG
namespace marbles
{
	int allocs = 0;
}
#endif // defined CONFIG_DEBUG

//#include "behaviour\Behaviour.h"

//int UserTask(void* user)
//{
//	(void)user;
//	return 0;
//}

//int main(int argc, char* argv[])
//{
//	//task task = application.start(UserTask, NULL);
//	(void)argc; (void) argv;
//	return 0;
//}
