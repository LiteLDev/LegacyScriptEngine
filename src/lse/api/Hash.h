#include <openssl/evp.h>
#include <string>

namespace lse::api::hash {

enum HashType { MD5, SHA1 };

std::string caculateHash(HashType type, const std::string& data) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  digest_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return {};

    if (EVP_DigestInit_ex(ctx, type == MD5 ? EVP_md5() : EVP_sha1(), nullptr)
        && EVP_DigestUpdate(ctx, data.data(), data.size()) && EVP_DigestFinal_ex(ctx, digest, &digest_len)) {
        char hex[EVP_MAX_MD_SIZE * 2 + 1];
        for (unsigned int i = 0; i < digest_len; ++i) {
            std::sprintf(hex + i * 2, "%02x", digest[i]);
        }
        hex[digest_len * 2] = '\0';
        EVP_MD_CTX_free(ctx);
        return std::string(hex);
    }

    EVP_MD_CTX_free(ctx);
    return {};
}
} // namespace lse::api