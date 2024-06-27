#include "../include/SharedObject.hpp"

#if defined __WIN32__ || defined _WIN64 || defined _WIN32
#include <windows.h>
#elif defined __unix__ || defined __linux__ || defined __FreeBSD__
#include <dlfcn.h>
#else
#error "Unknown or undefined operating system (not windows nor *nix)"
#endif

SharedObject::SharedObject()
{
	handle = nullptr;
	errorMessage = "";
}

SharedObject::~SharedObject() { Close(); }

#if defined __WIN32__ || defined _WIN64

bool SharedObject::Open(const std::string &file)
{
	Close();
	handle = LoadLibrary(file.c_str());
	const int msgSize = 256 * 1024;
	thread_local char msg[msgSize];
	DWORD err = GetLastError();
	if (err == 0) {
		errorMessage = "";
	} else {
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg,
			msgSize, NULL);
		errorMessage = msg;
	}
	filePath = file;
	return handle != nullptr;
}

void SharedObject::Close()
{
	if (handle) {
		FreeLibrary((HINSTANCE)handle);
		handle = nullptr;
	}
	errorMessage = "";
}

void *SharedObject::GetVoidSymbol(const std::string &symbol)
{
	if (handle)
		return (void *)GetProcAddress((HINSTANCE)handle, symbol.c_str());
	return nullptr;
}

#elif defined __unix__

bool SharedObject::Open(const std::string &file)
{
	handle = dlopen(file.c_str(), RTLD_NOW);
	errorMessage = dlerror();
	filePath = file;
	return handle != nullptr;
}

void SharedObject::Close()
{
	if (handle) {
		dlclose(handle);
		handle = nullptr;
	}
	errorMessage = "";
}

void *SharedObject::GetVoidSymbol(const std::string &symbol)
{
	if (handle) {
		return dlsym(handle, symbol.c_str());
	}
	return nullptr;
}

#endif
