#include "UTdynlib.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace YSolowork::util {

void LibraryDeleter::operator()(void* handle) const 
{
    if (handle) 
    {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
    }
}

void DynamicLibrary::load() 
{
    if (libraryHandle) 
    {
        throw DynamicLibraryException("Library already loaded.");
    }

#ifdef _WIN32
    void* handle = LoadLibraryA(libraryName.c_str());
    if (!handle) {
        throw DynamicLibraryException("Failed to load library: " + libraryName);
    }
#else
    void* handle = dlopen(libraryName.c_str(), RTLD_NOW);
    if (!handle) {
        throw DynamicLibraryException("Failed to load library: " + libraryName + ", error: " + dlerror());
    }
#endif
    libraryHandle = std::shared_ptr<void>(handle, LibraryDeleter());
}

void* DynamicLibrary::getFunctionPointer(const std::string& functionName)
{
#ifdef _WIN32
    void* func = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(libraryHandle.get()), functionName.c_str()));
    if (!func) {
        throw DynamicLibraryException("Failed to load function: " + functionName);
    }
#else
    void* func = dlsym(libraryHandle.get(), functionName.c_str());
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        throw DynamicLibraryException("Failed to load function: " + functionName + ", error: " + std::string(dlsym_error));
    }
#endif
    return func;
}

} // namespace YSolowork::util