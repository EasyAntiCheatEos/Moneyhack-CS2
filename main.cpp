#include "Loader/loader.hpp"

#include <thread>

int __stdcall main(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{
	auth::load();
	return 1;
}
