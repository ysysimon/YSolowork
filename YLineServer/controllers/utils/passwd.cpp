#include "passwd.h"

#include <botan/hash.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>

namespace YLineServer::Passwd {

std::string generateSalt(size_t length) {
    Botan::AutoSeeded_RNG rng;
    std::vector<uint8_t> salt(length);
    rng.randomize(salt.data(), salt.size());
    return Botan::hex_encode(salt);
}

std::string hashPassword(const std::string& password, const std::string& salt, const std::string& ALGORITHM) {
    std::unique_ptr<Botan::HashFunction> hashFunction(Botan::HashFunction::create(ALGORITHM));
    hashFunction->update(password);
    hashFunction->update(salt);
    return Botan::hex_encode(hashFunction->final());
}

bool compareHash(const std::string& storedHash, const std::string& inputPasswordHash) {
    // 将两个哈希值转换为字节数组进行比较
    const std::vector<uint8_t>& storedHashBytes = Botan::hex_decode(storedHash);
    const std::vector<uint8_t>& inputHashBytes = Botan::hex_decode(inputPasswordHash);

    // 使用 Botan 的恒定时间比较函数进行安全比较
    return Botan::constant_time_compare(storedHashBytes.data(), inputHashBytes.data(), storedHashBytes.size());
}

} // namespace YLineServer::PASSWD