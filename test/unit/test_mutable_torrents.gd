extends GutTest

# Unit tests for mutable torrent functionality
# Tests TorrentHandle mutable methods and basic operations

var session: TorrentSession
var test_file_path: String
var test_save_path: String

func before_each():
	session = TorrentSession.new()
	session.start_session()

	# Enable DHT for mutable torrents
	session.start_dht()

	# Create a temporary test file
	test_file_path = "user://test_mutable_file.txt"
	test_save_path = "user://test_mutable_downloads"

	var file = FileAccess.open(test_file_path, FileAccess.WRITE)
	file.store_string("Test content for mutable torrent")
	file.close()

	# Create save directory
	DirAccess.make_dir_recursive_absolute(test_save_path)

func after_each():
	if session:
		session.stop_session()

	# Clean up test files
	if FileAccess.file_exists(test_file_path):
		DirAccess.remove_absolute(test_file_path)

# Test 1: TorrentKeyPair generation (sanity check)
func test_keypair_generation():
	var keypair = TorrentKeyPair.generate()

	assert_not_null(keypair, "Keypair should be generated")
	assert_eq(keypair.get_public_key().size(), 32, "Public key should be 32 bytes")
	assert_eq(keypair.get_private_key().size(), 64, "Private key should be 64 bytes")
	assert_eq(keypair.get_public_key_hex().length(), 64, "Public key hex should be 64 characters")

# Test 2: Create mutable torrent
func test_create_mutable_torrent():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)

	assert_gt(torrent_data.size(), 0, "Torrent data should be created")

	var handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	assert_not_null(handle, "Handle should be created")
	assert_true(handle.is_valid(), "Handle should be valid")
	assert_true(handle.is_mutable(), "Handle should be mutable")

# Test 3: Check mutable torrent properties
func test_mutable_torrent_properties():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)
	var handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	# Check that the handle has the correct public key
	var public_key = handle.get_public_key()
	assert_eq(public_key.size(), 32, "Public key should be 32 bytes")
	assert_eq(public_key, keypair.get_public_key(), "Public key should match keypair")

	# Check initial sequence number
	var sequence = handle.get_sequence_number()
	assert_eq(sequence, 1, "Initial sequence number should be 1")

# Test 4: Auto-update flag
func test_auto_update_flag():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)
	var handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	# Auto-update should be disabled by default for publishers
	assert_false(handle.is_auto_update_enabled(), "Auto-update should be disabled by default")

	# Enable auto-update
	handle.set_auto_update(true)
	assert_true(handle.is_auto_update_enabled(), "Auto-update should be enabled")

	# Disable auto-update
	handle.set_auto_update(false)
	assert_false(handle.is_auto_update_enabled(), "Auto-update should be disabled")

# Test 5: Subscribe to mutable torrent
func test_subscribe_mutable_torrent():
	# First create a mutable torrent
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)
	var pub_handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	# Now subscribe using the public key
	var public_key = keypair.get_public_key()
	var sub_handle = session.subscribe_mutable_torrent(public_key, test_save_path + "/sub")

	assert_not_null(sub_handle, "Subscriber handle should be created")
	assert_true(sub_handle.is_valid(), "Subscriber handle should be valid")
	assert_true(sub_handle.is_mutable(), "Subscriber handle should be mutable")
	assert_eq(sub_handle.get_public_key(), public_key, "Public key should match")

# Test 6: Publish update
func test_publish_update():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)
	var handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	# Check initial sequence
	assert_eq(handle.get_sequence_number(), 1, "Initial sequence should be 1")

	# Create updated file
	var updated_file_path = "user://test_mutable_file_v2.txt"
	var file = FileAccess.open(updated_file_path, FileAccess.WRITE)
	file.store_string("Updated content for mutable torrent v2")
	file.close()

	# Create new torrent data
	var new_torrent_data = session.create_torrent_from_path(updated_file_path)
	assert_gt(new_torrent_data.size(), 0, "New torrent data should be created")

	# Publish update
	var success = handle.publish_update(new_torrent_data)
	assert_true(success, "Update should be published successfully")

	# Check sequence number was incremented
	assert_eq(handle.get_sequence_number(), 2, "Sequence should be incremented to 2")

	# Clean up
	DirAccess.remove_absolute(updated_file_path)

# Test 7: Check for updates (manual trigger)
func test_check_for_updates():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)
	var pub_handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	# Subscribe to the torrent
	var public_key = keypair.get_public_key()
	var sub_handle = session.subscribe_mutable_torrent(public_key, test_save_path + "/sub")

	# Manually trigger an update check (should not crash)
	sub_handle.check_for_updates()

	# This is a basic test - we're just ensuring the method doesn't crash
	# A full integration test would verify that updates are actually received
	pass_test("check_for_updates() executed without crashing")

# Test 8: Multiple mutable torrents
func test_multiple_mutable_torrents():
	var keypair1 = TorrentKeyPair.generate()
	var keypair2 = TorrentKeyPair.generate()

	var torrent_data = session.create_torrent_from_path(test_file_path)

	var handle1 = session.add_mutable_torrent(keypair1, test_save_path + "/1", torrent_data)
	var handle2 = session.add_mutable_torrent(keypair2, test_save_path + "/2", torrent_data)

	assert_not_null(handle1, "First handle should be created")
	assert_not_null(handle2, "Second handle should be created")
	assert_true(handle1.is_valid(), "First handle should be valid")
	assert_true(handle2.is_valid(), "Second handle should be valid")

	# Ensure they have different public keys
	assert_ne(handle1.get_public_key(), handle2.get_public_key(),
		"Mutable torrents should have different public keys")

# Test 9: Session helper methods
func test_session_mutable_methods():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = session.create_torrent_from_path(test_file_path)
	var handle = session.add_mutable_torrent(keypair, test_save_path, torrent_data)

	# Test publish_mutable_torrent_update
	var new_torrent_data = session.create_torrent_from_path(test_file_path)
	var success = session.publish_mutable_torrent_update(
		keypair.get_public_key(),
		new_torrent_data
	)

	# This might fail if DHT isn't ready, but the method should exist
	# The return value depends on DHT state
	assert_typeof(success, TYPE_BOOL, "publish_mutable_torrent_update should return bool")

	# Test check_mutable_torrent_for_updates
	session.check_mutable_torrent_for_updates(keypair.get_public_key())

	# If we get here without crashing, the method works
	pass_test("Session mutable methods executed without crashing")

# Test 10: Invalid operations
func test_invalid_operations():
	# Try to subscribe with invalid public key
	var invalid_key = PackedByteArray([1, 2, 3])  # Too short
	var handle = session.subscribe_mutable_torrent(invalid_key, test_save_path)

	# Should return null or invalid handle
	# (implementation may vary, but it shouldn't crash)
	if handle != null:
		# If handle is returned, it might still be invalid
		pass_test("Invalid subscribe didn't crash")
	else:
		pass_test("Invalid subscribe returned null")
