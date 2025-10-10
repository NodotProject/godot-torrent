extends GutTest

# Unit tests for TorrentKeyPair class

func test_key_generation():
	var kp = TorrentKeyPair.generate()
	assert_not_null(kp, "KeyPair should be generated")
	assert_eq(kp.get_public_key().size(), 32, "Public key should be 32 bytes")
	assert_eq(kp.get_private_key().size(), 64, "Private key should be 64 bytes")
	assert_eq(kp.get_seed().size(), 32, "Seed should be 32 bytes")
	assert_true(kp.can_sign(), "Generated keypair should be able to sign")

func test_public_key_hex():
	var kp = TorrentKeyPair.generate()
	var hex = kp.get_public_key_hex()
	assert_eq(hex.length(), 64, "Hex public key should be 64 characters")
	# Verify it's valid hex
	assert_true(hex.is_valid_hex_number(false), "Should be valid hex string")

func test_seed_derivation():
	# Create a deterministic seed
	var seed = PackedByteArray()
	seed.resize(32)
	for i in range(32):
		seed[i] = 0x42

	var kp1 = TorrentKeyPair.from_seed(seed)
	var kp2 = TorrentKeyPair.from_seed(seed)

	assert_not_null(kp1, "First keypair should be created")
	assert_not_null(kp2, "Second keypair should be created")
	assert_eq(kp1.get_public_key(), kp2.get_public_key(), "Same seed should produce same public key")
	assert_eq(kp1.get_private_key(), kp2.get_private_key(), "Same seed should produce same private key")

func test_invalid_seed_size():
	# Test with wrong seed size
	var bad_seed = PackedByteArray()
	bad_seed.resize(16)  # Wrong size

	var kp = TorrentKeyPair.from_seed(bad_seed)
	assert_null(kp, "Should return null for invalid seed size")

func test_signature_creation_and_verification():
	var kp = TorrentKeyPair.generate()
	var message = "Hello, World!".to_utf8_buffer()

	var signature = kp.sign(message)
	assert_eq(signature.size(), 64, "Signature should be 64 bytes")

	# Verify the signature
	var is_valid = TorrentKeyPair.verify(signature, message, kp.get_public_key())
	assert_true(is_valid, "Signature should be valid")

func test_signature_verification_fails_with_wrong_message():
	var kp = TorrentKeyPair.generate()
	var message = "Hello, World!".to_utf8_buffer()
	var wrong_message = "Goodbye, World!".to_utf8_buffer()

	var signature = kp.sign(message)

	# Verify with wrong message
	var is_valid = TorrentKeyPair.verify(signature, wrong_message, kp.get_public_key())
	assert_false(is_valid, "Signature should be invalid for different message")

func test_signature_verification_fails_with_wrong_key():
	var kp1 = TorrentKeyPair.generate()
	var kp2 = TorrentKeyPair.generate()
	var message = "Hello, World!".to_utf8_buffer()

	var signature = kp1.sign(message)

	# Verify with different keypair's public key
	var is_valid = TorrentKeyPair.verify(signature, message, kp2.get_public_key())
	assert_false(is_valid, "Signature should be invalid for different public key")

func test_from_keys():
	var kp1 = TorrentKeyPair.generate()
	var public_key = kp1.get_public_key()
	var private_key = kp1.get_private_key()

	# Create new keypair from saved keys
	var kp2 = TorrentKeyPair.from_keys(public_key, private_key)
	assert_not_null(kp2, "Should create keypair from keys")
	assert_eq(kp2.get_public_key(), public_key, "Public keys should match")
	assert_eq(kp2.get_private_key(), private_key, "Private keys should match")
	assert_true(kp2.can_sign(), "Loaded keypair should be able to sign")

	# Test signing with loaded keypair
	var message = "Test message".to_utf8_buffer()
	var sig1 = kp1.sign(message)
	var sig2 = kp2.sign(message)
	assert_eq(sig1, sig2, "Same keys should produce same signature")

func test_cannot_sign_empty_data():
	var kp = TorrentKeyPair.generate()
	var empty = PackedByteArray()

	var signature = kp.sign(empty)
	assert_eq(signature.size(), 0, "Should return empty array for empty data")

func test_seed_round_trip():
	var kp1 = TorrentKeyPair.generate()
	var seed = kp1.get_seed()

	# Recreate from seed
	var kp2 = TorrentKeyPair.from_seed(seed)
	assert_eq(kp2.get_public_key(), kp1.get_public_key(), "Should recover same public key from seed")
	assert_eq(kp2.get_private_key(), kp1.get_private_key(), "Should recover same private key from seed")
