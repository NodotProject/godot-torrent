#ifndef TORRENT_KEY_PAIR_H
#define TORRENT_KEY_PAIR_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <libtorrent/aux_/ed25519.hpp>
#include <array>

namespace godot {

/**
 * @class TorrentKeyPair
 * @brief Manages Ed25519 cryptographic keypairs for mutable torrents (BEP 46)
 *
 * This class provides Ed25519 key generation, serialization, and cryptographic
 * operations for implementing mutable torrents. The keypair consists of:
 * - 32-byte public key (can be shared)
 * - 64-byte private key (must be kept secret)
 * - 32-byte seed (can regenerate the keypair)
 */
class TorrentKeyPair : public RefCounted {
    GDCLASS(TorrentKeyPair, RefCounted)

private:
    std::array<char, 32> _public_key;
    std::array<char, 64> _private_key;
    std::array<char, 32> _seed;
    bool _has_private_key;

protected:
    static void _bind_methods();

public:
    TorrentKeyPair();
    ~TorrentKeyPair();

    // Factory methods

    /**
     * @brief Generates a new random keypair
     * @return A new TorrentKeyPair with a randomly generated keypair
     */
    static Ref<TorrentKeyPair> generate();

    /**
     * @brief Creates a keypair from a 32-byte seed (deterministic)
     * @param seed A 32-byte seed for key generation
     * @return A new TorrentKeyPair derived from the seed
     */
    static Ref<TorrentKeyPair> from_seed(PackedByteArray seed);

    /**
     * @brief Creates a keypair from existing keys (for loading saved keys)
     * @param public_key The 32-byte public key
     * @param private_key The 64-byte private key
     * @return A new TorrentKeyPair with the provided keys
     */
    static Ref<TorrentKeyPair> from_keys(PackedByteArray public_key, PackedByteArray private_key);

    // Getters

    /**
     * @brief Gets the 32-byte public key
     * @return PackedByteArray containing the public key
     */
    PackedByteArray get_public_key() const;

    /**
     * @brief Gets the 64-byte private key (SECRET - must be protected!)
     * @return PackedByteArray containing the private key
     */
    PackedByteArray get_private_key() const;

    /**
     * @brief Gets the 32-byte seed (SECRET - can regenerate the keypair)
     * @return PackedByteArray containing the seed
     */
    PackedByteArray get_seed() const;

    /**
     * @brief Gets the public key as a hexadecimal string
     * @return Hex-encoded public key (64 characters)
     */
    String get_public_key_hex() const;

    /**
     * @brief Checks if this keypair has a private key (can sign)
     * @return true if private key is available, false otherwise
     */
    bool can_sign() const;

    // Cryptographic operations

    /**
     * @brief Signs arbitrary data with the private key
     * @param data The data to sign
     * @return 64-byte signature
     */
    PackedByteArray sign(PackedByteArray data);

    /**
     * @brief Verifies a signature against data and public key
     * @param signature The 64-byte signature to verify
     * @param data The data that was signed
     * @param public_key The 32-byte public key to verify against
     * @return true if signature is valid, false otherwise
     */
    static bool verify(PackedByteArray signature, PackedByteArray data, PackedByteArray public_key);

    // Internal accessors for C++ code
    const std::array<char, 32>& get_lt_public_key() const { return _public_key; }
    const std::array<char, 64>& get_lt_private_key() const { return _private_key; }
    const std::array<char, 32>& get_lt_seed() const { return _seed; }
};

} // namespace godot

#endif // TORRENT_KEY_PAIR_H
