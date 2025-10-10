#include "torrent_key_pair.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <libtorrent/aux_/ed25519.hpp>
#include <cstring>
#include <random>

using namespace godot;

TorrentKeyPair::TorrentKeyPair() : _has_private_key(false) {
    _public_key.fill(0);
    _private_key.fill(0);
    _seed.fill(0);
}

TorrentKeyPair::~TorrentKeyPair() {
    // Zero out sensitive data
    _private_key.fill(0);
    _seed.fill(0);
}

void TorrentKeyPair::_bind_methods() {
    // Factory methods
    ClassDB::bind_static_method("TorrentKeyPair", D_METHOD("generate"), &TorrentKeyPair::generate);
    ClassDB::bind_static_method("TorrentKeyPair", D_METHOD("from_seed", "seed"), &TorrentKeyPair::from_seed);
    ClassDB::bind_static_method("TorrentKeyPair", D_METHOD("from_keys", "public_key", "private_key"), &TorrentKeyPair::from_keys);

    // Getters
    ClassDB::bind_method(D_METHOD("get_public_key"), &TorrentKeyPair::get_public_key);
    ClassDB::bind_method(D_METHOD("get_private_key"), &TorrentKeyPair::get_private_key);
    ClassDB::bind_method(D_METHOD("get_seed"), &TorrentKeyPair::get_seed);
    ClassDB::bind_method(D_METHOD("get_public_key_hex"), &TorrentKeyPair::get_public_key_hex);
    ClassDB::bind_method(D_METHOD("can_sign"), &TorrentKeyPair::can_sign);

    // Cryptographic operations
    ClassDB::bind_method(D_METHOD("sign", "data"), &TorrentKeyPair::sign);
    ClassDB::bind_static_method("TorrentKeyPair", D_METHOD("verify", "signature", "data", "public_key"), &TorrentKeyPair::verify);
}

Ref<TorrentKeyPair> TorrentKeyPair::generate() {
    Ref<TorrentKeyPair> keypair;
    keypair.instantiate();

    // Generate random seed
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (size_t i = 0; i < 32; ++i) {
        keypair->_seed[i] = static_cast<char>(dis(gen));
    }

    // Generate keypair from seed using libtorrent's Ed25519
    std::array<char, 32> pk;
    std::array<char, 64> sk;

    libtorrent::aux::ed25519_create_keypair(
        reinterpret_cast<unsigned char*>(pk.data()),
        reinterpret_cast<unsigned char*>(sk.data()),
        reinterpret_cast<const unsigned char*>(keypair->_seed.data())
    );

    keypair->_public_key = pk;
    keypair->_private_key = sk;
    keypair->_has_private_key = true;

    return keypair;
}

Ref<TorrentKeyPair> TorrentKeyPair::from_seed(PackedByteArray seed) {
    if (seed.size() != 32) {
        ERR_PRINT("Seed must be exactly 32 bytes");
        return Ref<TorrentKeyPair>();
    }

    Ref<TorrentKeyPair> keypair;
    keypair.instantiate();

    // Copy seed
    std::memcpy(keypair->_seed.data(), seed.ptr(), 32);

    // Generate keypair from seed
    std::array<char, 32> pk;
    std::array<char, 64> sk;

    libtorrent::aux::ed25519_create_keypair(
        reinterpret_cast<unsigned char*>(pk.data()),
        reinterpret_cast<unsigned char*>(sk.data()),
        reinterpret_cast<const unsigned char*>(keypair->_seed.data())
    );

    keypair->_public_key = pk;
    keypair->_private_key = sk;
    keypair->_has_private_key = true;

    return keypair;
}

Ref<TorrentKeyPair> TorrentKeyPair::from_keys(PackedByteArray public_key, PackedByteArray private_key) {
    if (public_key.size() != 32) {
        ERR_PRINT("Public key must be exactly 32 bytes");
        return Ref<TorrentKeyPair>();
    }

    if (private_key.size() != 64) {
        ERR_PRINT("Private key must be exactly 64 bytes");
        return Ref<TorrentKeyPair>();
    }

    Ref<TorrentKeyPair> keypair;
    keypair.instantiate();

    // Copy keys
    std::memcpy(keypair->_public_key.data(), public_key.ptr(), 32);
    std::memcpy(keypair->_private_key.data(), private_key.ptr(), 64);
    keypair->_has_private_key = true;

    // Note: We don't have the seed when loading from keys
    // This means get_seed() will return zeros

    return keypair;
}

PackedByteArray TorrentKeyPair::get_public_key() const {
    PackedByteArray result;
    result.resize(32);
    std::memcpy(result.ptrw(), _public_key.data(), 32);
    return result;
}

PackedByteArray TorrentKeyPair::get_private_key() const {
    if (!_has_private_key) {
        ERR_PRINT("This keypair does not have a private key");
        return PackedByteArray();
    }

    PackedByteArray result;
    result.resize(64);
    std::memcpy(result.ptrw(), _private_key.data(), 64);
    return result;
}

PackedByteArray TorrentKeyPair::get_seed() const {
    PackedByteArray result;
    result.resize(32);
    std::memcpy(result.ptrw(), _seed.data(), 32);
    return result;
}

String TorrentKeyPair::get_public_key_hex() const {
    String result;
    for (size_t i = 0; i < 32; ++i) {
        result += String::num_int64(static_cast<unsigned char>(_public_key[i]), 16, false).pad_zeros(2);
    }
    return result;
}

bool TorrentKeyPair::can_sign() const {
    return _has_private_key;
}

PackedByteArray TorrentKeyPair::sign(PackedByteArray data) {
    if (!_has_private_key) {
        ERR_PRINT("Cannot sign: this keypair does not have a private key");
        return PackedByteArray();
    }

    if (data.size() == 0) {
        ERR_PRINT("Cannot sign empty data");
        return PackedByteArray();
    }

    // Create signature
    std::array<char, 64> signature;

    libtorrent::aux::ed25519_sign(
        reinterpret_cast<unsigned char*>(signature.data()),
        reinterpret_cast<const unsigned char*>(data.ptr()),
        static_cast<std::ptrdiff_t>(data.size()),
        reinterpret_cast<const unsigned char*>(_public_key.data()),
        reinterpret_cast<const unsigned char*>(_private_key.data())
    );

    // Convert to PackedByteArray
    PackedByteArray result;
    result.resize(64);
    std::memcpy(result.ptrw(), signature.data(), 64);

    return result;
}

bool TorrentKeyPair::verify(PackedByteArray signature, PackedByteArray data, PackedByteArray public_key) {
    if (signature.size() != 64) {
        ERR_PRINT("Signature must be exactly 64 bytes");
        return false;
    }

    if (public_key.size() != 32) {
        ERR_PRINT("Public key must be exactly 32 bytes");
        return false;
    }

    if (data.size() == 0) {
        ERR_PRINT("Cannot verify empty data");
        return false;
    }

    // Verify signature
    int result = libtorrent::aux::ed25519_verify(
        reinterpret_cast<const unsigned char*>(signature.ptr()),
        reinterpret_cast<const unsigned char*>(data.ptr()),
        static_cast<std::ptrdiff_t>(data.size()),
        reinterpret_cast<const unsigned char*>(public_key.ptr())
    );

    return result == 1;
}
