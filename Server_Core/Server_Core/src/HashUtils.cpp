#include "HashUtils.hpp"
#include <Windows.h>
#include <bcrypt.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#pragma comment(lib, "bcrypt.lib")

namespace HashUtils {
    std::string SHA256(const std::string& input)
    {
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("BCryptOpenAlgorithmProvider failed");

        DWORD hashObjectSize = 0, dataSize = 0;
        status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjectSize, sizeof(DWORD), &dataSize, 0);
        if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("BCryptGetProperty failed");

        std::vector<BYTE> hashObject(hashObjectSize);
        BCRYPT_HASH_HANDLE hHash = nullptr;

        status = BCryptCreateHash(hAlg, &hHash, hashObject.data(), hashObjectSize, nullptr, 0, 0);
        if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("BCryptCreateHash failed");

        status = BCryptHashData(hHash, (PUCHAR)input.data(), static_cast<ULONG>(input.size()), 0);
        if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("BCryptHashData failed");

        std::vector<BYTE> hash(32); // SHA-256 output = 32 bytes
        status = BCryptFinishHash(hHash, hash.data(), static_cast<ULONG>(hash.size()), 0);
        if (!BCRYPT_SUCCESS(status)) throw std::runtime_error("BCryptFinishHash failed");

        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        std::ostringstream oss;
        for (BYTE b : hash)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;

        return oss.str();
    }
}
