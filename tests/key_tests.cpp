// Copyright (c) NetFoundry Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <catch2/catch_all.hpp>
#include "p11.h"
#include "util.h"

#include <cstring>
#include <iostream>
#include <tlsuv/tls_engine.h>

#define xstr(s) str__(s)
#define str__(s) #s


TEST_CASE("key gen", "[key]") {
    tls_context *ctx = default_tls_context(nullptr, 0);

    tlsuv_private_key_t key;
    REQUIRE(ctx->generate_key(&key) == 0);

    char *pem;
    size_t pemlen;
    REQUIRE(key->to_pem(key, &pem, &pemlen) == 0);
    printf("priv key:\n%.*s\n", (int) pemlen, pem);

    tlsuv_private_key_t k1;
    char *pem2;
    REQUIRE(ctx->load_key(&k1, pem, pemlen) == 0);
    REQUIRE(k1 != nullptr);
    REQUIRE(k1->to_pem(k1, &pem2, &pemlen) == 0);

    REQUIRE_THAT(pem2, Catch::Matchers::Equals(pem));
    free(pem);
    free(pem2);
    key->free(key);
    k1->free(k1);
    ctx->free_ctx(ctx);
}

static void check_key(tlsuv_private_key_t key) {
    auto pub = key->pubkey(key);
    REQUIRE(pub != nullptr);

    char *pem = nullptr;
    size_t pemlen;
    CHECK(key->to_pem(key, &pem, &pemlen) == 0);
    CHECK(pem != nullptr);
    CHECK(pemlen > 0);
    free(pem);

    pem = nullptr;
    CHECK(pub->to_pem(pub, &pem, &pemlen) == 0);
    CHECK(pem != nullptr);
    CHECK(pemlen > 0);
    free(pem);

    const char *data = "this is an important message";
    size_t datalen = strlen(data);

    char sig[256];
    memset(sig, 0, sizeof(sig));
    size_t siglen = sizeof(sig);

    CHECK(-1 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));
    CHECK(0 == key->sign(key, hash_SHA256, data, datalen, sig, &siglen));
    CHECK(0 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));
    sig[0] = (char) (sig[0] ^ 0xff);
    CHECK(-1 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));

    pub->free(pub);
}


TEST_CASE("key-tests", "[key]") {
    tls_context *ctx = default_tls_context(nullptr, 0);

    tlsuv_private_key_t key = nullptr;
    WHEN("generated key") {
        REQUIRE(ctx->generate_key(&key) == 0);
        check_key(key);
    }

    WHEN("load RSA key") {
        const char *pem = R"(-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCcmbdVp2gH+YHC
VIcl+pLwXNTKMZZGZRcmhLu+JtTdc4wwV3xiL57REHGr6rY4Vfo8Yr5eXbzHEmjA
4Cp7qJs5/1CN2Zqg0TZ8ayCoz223M+FghwQmz5cuHke98YLcQSV6DrLiqPlZ2vMx
Z5hXWzDpZAPrgLA3GrivCuPSbkW9Tr7wzUZgy2gZounU+Hm237fXz0SDyyMIsDKs
NHKoIEjhYuCnxhuWhgipteogVu+LOX45LsBK9TslN5AxOQEvGD8zj9PcN0DBfMTR
qL6vWBlkSohhmPPhYOJAegOqgTMHMm83o9DjN3/3sG+RLU6dzMp7s7REJlp2zVbN
kF/XqU6FAgMBAAECggEACGAMNw+B99M3Rl5g7/4Lt8EvPDUUtWUYrN2ycMQA5GsW
l0tGgrXLR6EagzhFUJQAkxQw3DklLHxmj9ItU3m7+4kVFNELfQhTYqoUEBMv6flj
V1dBOJYnnrbN3XG6Lu9pArMgjBC/bTfRg9XnhtyArCrGLuUuB3RtZict8gYlwq8K
/IE7T2c9nLW+WZmZh0/2ouZJw1nFPSeuynasPpKSFHzvlKTBLMnGZWjiKifmNciy
JtU0296BxPcj5fTtoAa5YPkp7qGe5blWRYPqjCAi0TE3edxcVaAYtaLwhXXzmpQt
yDcHcY9lpfSkkGM4Hz27YUegIcpjPGHrD1lBJieYwQKBgQDXDTzBvbFtyReiDLW3
v5+oRtgriqDqed/sZR/sdaFb5tiUffENFXA3kFyIVzP/Aem3yRtprHgX9viIDzIE
bOSAOgYZv1Ie++Dyk2GnCUIbpV03Y8lMxX9/RUFpLefgyvq2T6OXyjeWe9OgnnIf
MGg1acmfmAXdlatBhoaa8E/gcQKBgQC6a0EQ0x7eyF/Rb020PbJL+qhQSjLelHF5
d4s7rhcp0MLTBfIjpVDkOLw78ujrl8e1hvvE+YqHpau5GSEXtpBBlvgrC54ZkQD6
3Qy64wXtScVQxETK02UM9iJQsEaQWi3opQnQ5tV2IVi3gCZLqNte3+Hxgq0VxA10
NURPSCTZVQKBgQCAbTVVdlVZfPgSHIkA/Pz536UFC7rhjHr/j7yq1+zPF2NL+pJT
//OOGzZHbdxtc9UBnqYyS39EwIbXqktyfR1QvlYaVjlSq5VBCGcO++Zw4CZ1B7CV
mnRzqwZPK80IX++tpI3L/kWIJtbRWw5INf5lt5FjL8SA+frWHOKR8OWi4QKBgDrA
uOYDk/Qk9MX+LWBEHaCCpG+Boxyxbj4ZJiGuEZDVQcHeWt1PKfpzwyelvDEcSg31
N/5xo25zEXcp61sc58Q0P4zZgX+PSt7FslBoYqLRoEV/RisiivOV02TY2bR/J37u
HPTg+5/ajKpw0iEAW/s/1mcWh1SX0KGydBAErdBtAoGBAIdjLkZBr4vybsUh8lof
8SsycNXmqcoQEgRcQNp8HYJlv46K6bvtiNNtuo0/ZTYkDSFZi6Wn/ridpIWO8Zpj
/kqmvc7l6e9XlPvhj2/VAM+mGAf3vf7VGZD6sOCbnNtIRCP1PGv64IhdpHJBTp7n
iemZJfIkLzyuwra/o7WkK+hK
-----END PRIVATE KEY-----
)";
        REQUIRE(0 == ctx->load_key(&key, pem, strlen(pem)));
        check_key(key);
    }

    WHEN("load EC key") {
        const char *pem = R"(-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIDkQFm34yO6vjVPw71v+O8l4r7TelfL8jLnakR8IbARvoAoGCCqGSM49
AwEHoUQDQgAEozzayHZuK1VKSJdnSlQtMWF0iLIkqGxbxWCL6/QlGAATbNSkcW8b
lAkOwU8XOpspVUfYbwPSVSoS2NXn1rE7iA==
-----END EC PRIVATE KEY-----
)";
        REQUIRE(0 == ctx->load_key(&key, pem, strlen(pem)));
        check_key(key);
    }
    key->free(key);
    ctx->free_ctx(ctx);
}


TEST_CASE("gen csr", "[engine]") {
    tls_context *ctx = default_tls_context(nullptr, 0);

    tlsuv_private_key_t key;
    REQUIRE(ctx->generate_key(&key) == 0);

    char *pem;
    size_t pemlen = 0;
    CHECK(ctx->generate_csr_to_pem(key, &pem, &pemlen,
                                   "C", "US",
                                   "O", "OpenZiti",
                                   "OU", "Developers",
                                   "CN", "CSR test",
                                   NULL) == 0);
    CHECK(pemlen == strlen(pem));
    printf("CSR:\n%.*s\n", (int) pemlen, pem);
    free(pem);
    pem = nullptr;

    CHECK(ctx->generate_csr_to_pem(key, &pem, nullptr,
                                   "C", "US",
                                   "O", "OpenZiti",
                                   "OU", "Developers",
                                   "CN", "CSR test",
                                   NULL) == 0);
    printf("CSR:\n%s\n", pem);

    key->free(key);
    ctx->free_ctx(ctx);
    free(pem);
}

#if defined(HSM_CONFIG)
#define HSM_DRIVER xstr(HSM_LIB)

TEST_CASE("pkcs11 valid pkcs#11 key", "[key]") {
    tls_context *ctx = default_tls_context(nullptr, 0);
    REQUIRE(ctx->load_pkcs11_key != nullptr);

    std::string keyType = GENERATE("ec", "rsa");
    std::string keyLabel = "test-" + keyType;
    tlsuv_private_key_t key = nullptr;

    int rc = 0;
    rc = ctx->load_pkcs11_key(&key, HSM_DRIVER, nullptr, "2222", nullptr, keyLabel.c_str());
    CHECK(rc == 0);
    REQUIRE(key != nullptr);

    WHEN(keyType << ": private key PEM") {
        char *pem;
        size_t pemlen;
        rc = key->to_pem(key, &pem, &pemlen);
        THEN("should fail") {
            CHECK(rc == -1);
            CHECK(pem == nullptr);
            CHECK(pemlen == 0);
        }
    }

    WHEN(keyType << ": public key PEM") {
        char *pem = nullptr;
        size_t pemlen;
        auto pub = key->pubkey(key);
        REQUIRE(pub != nullptr);
        THEN("should work") {
            CHECK(pub->to_pem(pub, &pem, &pemlen) == 0);
            CHECK(pem != nullptr);
            CHECK(pemlen > 0);
            Catch::cout() << std::string(pem, pemlen);
        }
        pub->free(pub);
        free(pem);
    }

    WHEN(keyType << ": get key cert") {
        tlsuv_certificate_t cert;
        char *pem = nullptr;
        size_t pemlen;
        CHECK(key->get_certificate(key, &cert) == 0);

        THEN("should be able to write cert to PEM") {
            CHECK(cert->to_pem(cert, 1, &pem, &pemlen) == 0);
            CHECK(pemlen > 0);
            CHECK(pem != nullptr);
            Catch::cout() << std::string(pem, pemlen) << std::endl;
            free(pem);
        }
        AND_THEN("should be able to store cert back to key") {
            CHECK(key->store_certificate(key, cert) == 0);
        }
        AND_THEN("verify using cert") {
            char sig[512];
            const char *data = "I want to sign and verify this";
            size_t datalen = strlen(data);

            memset(sig, 0, sizeof(sig));
            size_t siglen = sizeof(sig);

            CHECK(0 == key->sign(key, hash_SHA256, data, datalen, sig, &siglen));
            CHECK(0 == cert->verify(cert, hash_SHA256, data, datalen, sig, siglen));
        }
        cert->free(cert);
    }

    WHEN(keyType << ": sign and verify") {
        auto pub = key->pubkey(key);
        REQUIRE(pub != nullptr);
        THEN("should work") {

            const char *data = "this is an important message";
            size_t datalen = strlen(data);

            char sig[512];
            memset(sig, 0, sizeof(sig));
            size_t siglen = sizeof(sig);

            CHECK(-1 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));
            CHECK(0 == key->sign(key, hash_SHA256, data, datalen, sig, &siglen));
            CHECK(0 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));
        }
        pub->free(pub);
    }

    if (key) {
        key->free(key);
    }
    ctx->free_ctx(ctx);
}

TEST_CASE("gen-pkcs11-key-internals", "[key]") {
    p11_context p11;
    p11_key_ctx key;
    REQUIRE(p11_init(&p11, HSM_DRIVER, nullptr, "2222") == 0);
    REQUIRE(p11_gen_key(&p11, &key, "test-key") == 0);
}

TEST_CASE("gen-pkcs11-key", "[key]") {
    auto tls = default_tls_context(nullptr, 0);
    tlsuv_private_key_t key = nullptr;
    REQUIRE(tls->generate_pkcs11_key(&key, HSM_DRIVER, nullptr, "2222", "gen-key-test") == 0);

    WHEN("public key PEM") {
        char *pem = nullptr;
        size_t pemlen;
        auto pub = key->pubkey(key);
        REQUIRE(pub != nullptr);
        THEN("should work") {
            CHECK(pub->to_pem(pub, &pem, &pemlen) == 0);
            CHECK(pem != nullptr);
            CHECK(pemlen > 0);
            Catch::cout() << std::string(pem, pemlen);
        }
        pub->free(pub);
        free(pem);
    }
    key->free(key);
    tls->free_ctx(tls);
}
#endif

TEST_CASE("keychain", "[key]") {
    auto tls = default_tls_context(nullptr, 0);
    if (tls->load_keychain_key == nullptr) {
        tls->free_ctx(tls);
        SKIP("keychain not supported");
    }

    uv_timeval64_t now;
    uv_gettimeofday(&now);
    auto name = "testkey-" + std::to_string(now.tv_usec);

    GIVEN("generated private key") {
        fprintf(stderr, "using name: %s\n", name.c_str());
        tlsuv_private_key_t pk{};
        REQUIRE(tls->load_keychain_key(&pk, name.c_str()) != 0);

        REQUIRE(tls->generate_keychain_key(&pk, name.c_str()) == 0);

        char data[1024];
        uv_random(nullptr, nullptr, data, sizeof(data), 0, nullptr);
        size_t datalen = sizeof(data);

        char sig[512];
        memset(sig, 0, sizeof(sig));
        size_t siglen = sizeof(sig);

        WHEN("it can sign data") {
            REQUIRE(0 == pk->sign(pk, hash_SHA256, data, datalen, sig, &siglen));

            THEN("verify with its public key") {
                auto pub = pk->pubkey(pk);
                REQUIRE(pub != nullptr);

                char *pem = nullptr;
                size_t pem_len = 0;
                CHECK(pub->to_pem(pub, &pem, &pem_len) == 0);
                CHECK(pem != nullptr);
                if (pem) {
                    CHECK_THAT(pem, Catch::Matchers::StartsWith("-----BEGIN PUBLIC KEY-----"));
                    std::cout << std::string(pem, pem_len) << std::endl;
                    free(pem);
                }
                CHECK(0 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));
                pub->free(pub);
            }
        }

        WHEN("it can be loaded by name") {
            tlsuv_private_key_t pk2{};
            int rc2 = tls->load_keychain_key(&pk2, name.c_str());
            THEN("loaded successfully") {
                REQUIRE(rc2 == 0);
                pk2->free(pk2);
            }
        }

        pk->free(pk);
        REQUIRE(0 == tls->remove_keychain_key(name.c_str()));
    }

    tls->free_ctx(tls);
}

TEST_CASE("keychain-manual", "[.]") {
    auto tls = default_tls_context(nullptr, 0);
    if (tls->load_keychain_key == nullptr) {
        tls->free_ctx(tls);
        SKIP("keychain not supported");
    }

    uv_timeval64_t now;
    uv_gettimeofday(&now);
    auto name = std::string(getenv("TEST_KEYCHAIN_KEY"));

    GIVEN("existing private key") {
        fprintf(stderr, "using name: %s\n", name.c_str());
        tlsuv_private_key_t pk{};
        REQUIRE(tls->load_keychain_key(&pk, name.c_str()) == 0);

        char data[1024];
        uv_random(nullptr, nullptr, data, sizeof(data), 0, nullptr);
        size_t datalen = sizeof(data);

        char sig[512];
        memset(sig, 0, sizeof(sig));
        size_t siglen = sizeof(sig);

        THEN("it can sign data") {
            REQUIRE(0 == pk->sign(pk, hash_SHA256, data, datalen, sig, &siglen));

            AND_THEN("verify with its public key") {
                auto pub = pk->pubkey(pk);
                REQUIRE(pub != nullptr);

                char *pem = nullptr;
                size_t pem_len = 0;
                CHECK(pub->to_pem(pub, &pem, &pem_len) == 0);
                CHECK(pem != nullptr);
                if (pem) {
                    CHECK_THAT(pem, Catch::Matchers::StartsWith("-----BEGIN PUBLIC KEY-----"));
                    std::cout << std::string(pem, pem_len) << std::endl;
                    free(pem);
                }
                CHECK(0 == pub->verify(pub, hash_SHA256, data, datalen, sig, siglen));
                pub->free(pub);
            }
            pk->free(pk);
        }
    }

    tls->free_ctx(tls);
}

TEST_CASE("wraparound buffer test", "[util]") {
    WRAPAROUND_BUFFER(,16) buf{};
    WAB_INIT(buf);

    char *p;
    size_t len;
    WAB_PUT_SPACE(buf, p, len);
    CHECK(p == buf.putp);
    CHECK(len == sizeof(buf.buf));

    WAB_GET_SPACE(buf, p, len);
    CHECK(p == buf.getp);
    CHECK(len == 0);

    WAB_UPDATE_PUT(buf, 10);
    CHECK(buf.putp == buf.buf + 10);
    WAB_PUT_SPACE(buf, p, len);
    CHECK(p == buf.putp);
    CHECK(len == sizeof(buf.buf) - 10);
    WAB_GET_SPACE(buf, p, len);
    CHECK(p == buf.getp);
    CHECK(len == 10);

    WAB_UPDATE_GET(buf, 6);
    CHECK(buf.getp == p + 6);
    WAB_GET_SPACE(buf, p, len);
    CHECK(p == buf.getp);
    CHECK(len == 4);

    // wrap around
    WAB_UPDATE_PUT(buf, 6);
    CHECK(buf.putp - buf.buf == 0);
    WAB_PUT_SPACE(buf, p, len);
    CHECK(p == buf.putp);
    CHECK(len == buf.getp - buf.putp - 1);

    WAB_GET_SPACE(buf, p, len);
    CHECK(p == buf.getp);
    CHECK(len == 10);

    WAB_UPDATE_GET(buf, len);
    CHECK(buf.getp == buf.buf);
}

TEST_CASE("cert-chain", "[key]") {
    auto pem = R"("
-----BEGIN CERTIFICATE-----
MIIDojCCAYqgAwIBAgIDBLfgMA0GCSqGSIb3DQEBCwUAMIGTMQswCQYDVQQGEwJVUzELMAkGA1UE
CBMCTkMxEjAQBgNVBAcTCUNoYXJsb3R0ZTETMBEGA1UEChMKTmV0Rm91bmRyeTEoMCYGA1UEAxMf
Wml0aSBDb250cm9sbGVyIEludGVybWVkaWF0ZSBDQTEkMCIGCSqGSIb3DQEJARYVc3VwcG9ydEBu
ZXRmb3VuZHJ5LmlvMB4XDTI0MDczMTE3MjUzNVoXDTI1MDczMTE3MjYzNVowFDESMBAGA1UEAxMJ
Q2FmU3ZwSHAwMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE92VlMrJv9Ydw33aTCefJtwMHgDKv
mDqJJkH4STzM/+5UJDLtSF5z7TftVFZ2rZvLaDPD1rs4JpWdiscASYdk96NIMEYwDgYDVR0PAQH/
BAQDAgSwMBMGA1UdJQQMMAoGCCsGAQUFBwMCMB8GA1UdIwQYMBaAFGt95MgE1uEQNJoDlUebyTJ/
7/UJMA0GCSqGSIb3DQEBCwUAA4ICAQByEisNy20/2GMaN1q7ILIMiuq1SeDHi9qsmzgOnb3mYGWU
uLuMc0Rs/N5W4+8TXq3ex+GHKWGAx+AB6oNTYtqLsGAEo9c7kCPWOrsFa3TPDG/u7jNBCvC5idIw
RanAfNMQGXWuqbTt9mi5hf115vC6QIRSUo6gPF5+IEMG1RcNMjTYChCFctBzPMqU4Ku9S2jFyYhM
7s1q6MGAQ7LEkXOVWn66dsnpaqkVjFOsjg66xkENFyF0PIwMvbCqUrlzxKij3INfc8Q0QtCLY0Wl
+9p1n27MRQBamiJHVdANJ62X2f3LH2izkV4Ria2sZjqYvs3Oh/1tWc171QKekZDwn+Nz01eXRTVM
ynVPu5CNlcGD4vMUkiXWr282MH51l02aZ0/PUXPdU7NZxJdt+AGsWNgTBjKA0uEhc51rXKmj1iN9
s1/1K4PDrgwt0ikHq5n6NpEk3OXfAVuLWJqPQ/+leouP5S8OCG6n2DrrypC2bT4EMej5VMo8OUbn
XNS8uqbsqU70gKlJdnOh9exoClo9yppNzRdmpyZTBPCrQ637wnhw4VjwsYzZccg8iT6mNn/tRHeR
qfvj87DcOjrqPPRUC9MLvO+1uyPG069mZAu2DJzYtFM7OOmqHEK58syMf4sAR09o7drfliKkz8Mx
9nGcqsXUMNZRl1aiDtplNxnRrEPaqw==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIF6TCCA9GgAwIBAgIGAYekZkqtMA0GCSqGSIb3DQEBCwUAMHIxLTArBgNVBAMMJDUwYTQ5YTkw
LTQ4MDQtNGU0Ni1iMGM0LWU5M2ZjNmJjNTc4ZjETMBEGA1UECgwKTmV0Rm91bmRyeTESMBAGA1UE
BwwJQ2hhcmxvdHRlMQswCQYDVQQIDAJOQzELMAkGA1UEBhMCVVMwHhcNMjMwNDIxMTUxOTM5WhcN
MzMwNDE4MTUxOTM5WjCBkzELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAk5DMRIwEAYDVQQHEwlDaGFy
bG90dGUxEzARBgNVBAoTCk5ldEZvdW5kcnkxKDAmBgNVBAMTH1ppdGkgQ29udHJvbGxlciBJbnRl
cm1lZGlhdGUgQ0ExJDAiBgkqhkiG9w0BCQEWFXN1cHBvcnRAbmV0Zm91bmRyeS5pbzCCAiIwDQYJ
KoZIhvcNAQEBBQADggIPADCCAgoCggIBAJQvbAD2H5aI1fe1z2GbaSopVNx5izA0+QAWTXAKyXZm
aUxJpw10fC1fFPOx0OSi0a/cdDgXMl8TdEs9Qb2w/6qTTduQUGes4SsO02BUHvUDPK4HG6eHpxL6
VSudDDcH8mkGcv1RhsTTvB2400Gxdgp1i9zpfZkt4JYDeEJDhE//+GXZ2pMzX0ShUPzRoRyuzW+O
ha1TgcX8upA/8nDwREEp24/A3Hwk+uJ4Ym+1XOPICT2idgJcjs++3tb++UJitfFELTKUVlzs0kx+
2sEddhjYuEkv21x53eW727sG0cNIVr9SHqS3NgK5w4GE92f73D6IDHWchZHpSSs0atF6Ty7nIElx
CgtlhMbge3yLeLHJHX/jHibb38e7w+jpyyBNk/ty85r+DN5j24it8TOpKgARvywb5vRTIUUGv6wq
jRBSN76yUQxaKUUoSII13b0yJwZncMkNK4hex1W6sS+o4PB2pVQpOAb5dbrtsUtLr+mEfERdQH+y
SQaKjYTvyl9XbA9lw3551pyfJPLuc4otKx+uBVTMWmYIkuiRvUM/Xql29LqM0lrSrw64xqw1sdn7
piKqIBXetnF4KUhvs0mXzwSf+XuRNHTrFAKaQY373hDnVqgesjo87ilP7zhTftX5CBzeywx84e79
chQJ00uV/7V/tDoYYX1Auohf800XZ1UlAgMBAAGjYzBhMB0GA1UdDgQWBBRrfeTIBNbhEDSaA5VH
m8kyf+/1CTAfBgNVHSMEGDAWgBRrfeTIBNbhEDSaA5VHm8kyf+/1CTAPBgNVHRMBAf8EBTADAQH/
MA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG9w0BAQsFAAOCAgEAi8Nr09339b5Hu7S46ce8k4dNO+iT
VLW37Kxwy6ZCwgOHgjoMMR0JTzn1y+dJGv2+SWC8AOQwGsaAh8JBbK5F3I/dGyXLUmth3KsUr1Ex
2cLJM9usF+MRQrKarybW84yWyPXRUNh9z8n/gNsZVLhggLG0C73zfCijhFcOdMf/1s732tUlG3/X
mZb2Rftf94oquLp31499OJXsl554QRDDzST88hyUmBd6q/gfWPjhdESvM/60IIgnPNlXv7q/X1EF
4qqSts8doaMx2XGX5sZA6o7h2fCdgGUtxe1Pm1vohCzKeFZeC9wS18QxGxIWkBJY6RndcfC2CXNC
H9Ck1wlfpUH6snzKw+iyw6+1QaMNLjQU3ZsU5tJBjdFSKDF+auBVX1V3rY2arKfrYBtzq6v98WXW
BYPbtx6Hy30DmwBAQsWSh087O6Atj0ZupgFm58zYrIj4lL5sdeOeJ+yu4rKqI5IYj1ssEkL7ey8c
ddyoEdE1txGHUICkwIlzHbtBNp0vCoI6V5K4IYAEFwpDJxmIrMDxi7z6WBt7i0YZuLfwWV/5Gu26
ClE70UxGSsMjY8Evg0qSemyX/S63aziH1I9+m+3BUF+bg75zTmirgzIPt3B0mbD4Rx99DC6bE9n8
Z8AgrJehwuXYVyJrG5Tc1vnlSUhUrK2812JyXA7tkWj/qzc=
-----END CERTIFICATE-----
)";
    auto tls = default_tls_context(nullptr, 0);
    tlsuv_certificate_t cert = nullptr;
    CHECK(tls->load_cert(&cert, pem, strlen(pem)) == 0);
    cert->free(cert);
    tls->free_ctx(tls);
}
