#include <Windows.h>
#include <iostream>
void blog_info(const char* format, ...)
{
	char str[256];
	char out[256];

	va_list args;
	va_start(args, format);
	vsprintf_s(str, format, args);
	va_end(args);
	sprintf_s(out, "======[INFO] %s\n", str);

	// to DbgView
	OutputDebugStringA(out);

	// to stdout
	//fprintf(stdout, out);
	std::cout << out;
}