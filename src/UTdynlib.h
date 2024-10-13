#ifndef UTdynlib_H
#define UTdynlib_H

#include <memory>
#include <string>
#include <exception>
#include <functional>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace YSolowork::util {

// 异常类: 动态库异常
class DynamicLibraryException : public std::exception {
public:
    inline explicit DynamicLibraryException(const std::string& message) : msg_(message) {}

    // 覆盖 what() 方法，返回错误信息
    inline const char* what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

// 结构体: 动态库卸载器
struct LibraryDeleter {
    void operator()(void* handle) const;
};

// 类: 动态库加载器
class DynamicLibrary {
public:
    // 构造函数，只初始化库名，不立即加载库
    inline explicit DynamicLibrary(const std::string& libraryName) : libraryName(libraryName) {}

    // 加载库并使用 shared_ptr 管理
    void load();

    // 获取类型安全的函数指针，返回 std::function
    template<typename FuncType>
    std::function<FuncType> getFunction(const std::string& functionName);

    inline std::string getLibraryName() const { return libraryName; }

private:
    std::string libraryName;  // 存储库名
    std::shared_ptr<void> libraryHandle;  // 使用 shared_ptr 管理库句柄
    std::mutex loadMutex; // 保护加载操作的互斥量
};


// getFunction 模板函数实现
template<typename FuncType>
std::function<FuncType> DynamicLibrary::getFunction(const std::string& functionName) {
    if (!libraryHandle) {
        throw DynamicLibraryException("Library not loaded.");
    }

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
    // 将函数指针与库生命周期绑定
    std::weak_ptr<void> weakHandle = libraryHandle;
    return [func, weakHandle](auto&&... args) -> decltype(auto) // 万能引用
    {
        // 检查库是否已卸载
        if (auto lib = weakHandle.lock()) {
            // 通过 reinterpret_cast 将获取到的 void* 转换为模板类型 FuncType*
            auto typedFunc = reinterpret_cast<FuncType*>(func);
            // 调用函数指针并转发参数
            return typedFunc(std::forward<decltype(args)>(args)...);
        } else {
            throw DynamicLibraryException("Library has been unloaded. Function is no longer valid.");
        }
    };
}


} // namespace YSolowork::util

#endif // UTdynlib_H