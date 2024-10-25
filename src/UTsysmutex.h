#ifndef UTsysmutex_H
#define UTsysmutex_H

#include <boost/interprocess/sync/named_mutex.hpp>

namespace YSolowork::util {

// 异常类: CrossPlatformSysMutex 异常
class CrossPlatformSysMutexError : public std::exception {
public:
    inline explicit CrossPlatformSysMutexError(const std::string& message) : msg_(message) {}

    // 覆盖 what() 方法，返回错误信息
    inline const char* what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

class CrossPlatformSysMutex {
public:
    inline explicit CrossPlatformSysMutex(const std::string &name) : mutexName(name)
    {
        // 延迟初始化 named_mutex
        mutex = std::make_unique<boost::interprocess::named_mutex>(boost::interprocess::open_or_create, mutexName.c_str());
    }

    inline ~CrossPlatformSysMutex()
    {
        mutex->remove(mutexName.c_str());
    }

    inline bool try_lock() 
    {
        try {
            return mutex->try_lock();
        } catch (const boost::interprocess::interprocess_exception &e) {
            throw CrossPlatformSysMutexError("CrossPlatformSysMutex::lock() failed - " + std::string(e.what()));
        }
    }

    inline void unlock() 
    {
        try {
            mutex->unlock();
        } catch (const boost::interprocess::interprocess_exception &e) {
            throw CrossPlatformSysMutexError("CrossPlatformSysMutex::unlock() failed - " + std::string(e.what()));
        }
        
    }

private:
    std::string mutexName;
    std::unique_ptr<boost::interprocess::named_mutex> mutex;
};

} // namespace YSolowork::util

#endif // UTsysmutex_H