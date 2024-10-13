#include "UTdynlib.h"

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

} // namespace YSolowork::util