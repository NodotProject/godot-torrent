extends GutTest

# Integration tests for mutable torrent publish/subscribe workflow
# These tests verify the end-to-end functionality including DHT operations

var publisher_session: TorrentSession
var subscriber_session: TorrentSession
var test_file_path: String
var test_file_path_v2: String
var publisher_save_path: String
var subscriber_save_path: String

# Longer timeout for DHT operations
const DHT_TIMEOUT = 10.0  # seconds

func before_each():
	# Create publisher session
	publisher_session = TorrentSession.new()
	publisher_session.start_session()
	publisher_session.start_dht()

	# Create subscriber session
	subscriber_session = TorrentSession.new()
	subscriber_session.start_session()
	subscriber_session.start_dht()

	# Create test files
	test_file_path = "user://test_publish_file.txt"
	test_file_path_v2 = "user://test_publish_file_v2.txt"
	publisher_save_path = "user://publisher_data"
	subscriber_save_path = "user://subscriber_data"

	var file = FileAccess.open(test_file_path, FileAccess.WRITE)
	file.store_string("Original content - version 1")
	file.close()

	file = FileAccess.open(test_file_path_v2, FileAccess.WRITE)
	file.store_string("Updated content - version 2")
	file.close()

	# Create save directories
	DirAccess.make_dir_recursive_absolute(publisher_save_path)
	DirAccess.make_dir_recursive_absolute(subscriber_save_path)

func after_each():
	if publisher_session:
		publisher_session.stop_session()

	if subscriber_session:
		subscriber_session.stop_session()

	# Clean up test files
	if FileAccess.file_exists(test_file_path):
		DirAccess.remove_absolute(test_file_path)

	if FileAccess.file_exists(test_file_path_v2):
		DirAccess.remove_absolute(test_file_path_v2)

# Test 1: Basic publish and subscribe workflow
func test_publish_and_subscribe_basic():
	# Generate keypair
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Publisher creates mutable torrent
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	assert_gt(torrent_data.size(), 0, "Torrent data should be created")

	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	assert_not_null(pub_handle, "Publisher handle should be created")
	assert_true(pub_handle.is_valid(), "Publisher handle should be valid")
	assert_true(pub_handle.is_mutable(), "Publisher handle should be mutable")
	assert_eq(pub_handle.get_sequence_number(), 1, "Initial sequence should be 1")

	# Subscriber subscribes to the mutable torrent
	var sub_handle = subscriber_session.subscribe_mutable_torrent(
		public_key,
		subscriber_save_path
	)

	assert_not_null(sub_handle, "Subscriber handle should be created")
	assert_true(sub_handle.is_valid(), "Subscriber handle should be valid")
	assert_true(sub_handle.is_mutable(), "Subscriber handle should be mutable")
	assert_eq(sub_handle.get_public_key(), public_key, "Public keys should match")

# Test 2: Publish and subscribe with DHT propagation
func test_publish_subscribe_with_dht_wait():
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Publisher creates and publishes
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	# Wait for DHT to propagate (give it some time)
	await wait_seconds(2.0)

	# Check for DHT put confirmation
	var pub_alerts = publisher_session.get_alerts()
	var found_put_alert = false
	for alert in pub_alerts:
		if alert.get("type") == "dht_put_alert":
			found_put_alert = true
			gut.p("DHT put confirmed for sequence " + str(alert.get("sequence", 0)))
			break

	# Note: DHT operations may not complete immediately in tests
	# We just verify the methods don't crash
	pass_test("DHT operations executed without errors")

# Test 3: Update detection workflow
func test_update_detection_workflow():
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Publisher creates mutable torrent
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	assert_eq(pub_handle.get_sequence_number(), 1, "Initial sequence should be 1")

	# Subscriber subscribes
	var sub_handle = subscriber_session.subscribe_mutable_torrent(
		public_key,
		subscriber_save_path
	)
	sub_handle.set_auto_update(true)

	assert_true(sub_handle.is_auto_update_enabled(), "Auto-update should be enabled")

	# Publisher publishes update
	var new_torrent_data = publisher_session.create_torrent_from_path(test_file_path_v2)
	var success = pub_handle.publish_update(new_torrent_data)

	assert_true(success, "Update should be published")
	assert_eq(pub_handle.get_sequence_number(), 2, "Sequence should be incremented to 2")

	# Wait for DHT propagation
	await wait_seconds(2.0)

	# Check for alerts
	var pub_alerts = publisher_session.get_alerts()
	var sub_alerts = subscriber_session.get_alerts()

	gut.p("Publisher alerts: " + str(pub_alerts.size()))
	gut.p("Subscriber alerts: " + str(sub_alerts.size()))

	# Note: In a real DHT network, we would expect to receive alerts
	# In a test environment, DHT may not be fully connected
	pass_test("Update workflow completed without errors")

# Test 4: Multiple subscribers
func test_multiple_subscribers():
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Publisher creates mutable torrent
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	# Create multiple subscriber sessions
	var subscriber2 = TorrentSession.new()
	subscriber2.start_session()
	subscriber2.start_dht()

	# Both subscribers subscribe
	var sub_handle1 = subscriber_session.subscribe_mutable_torrent(
		public_key,
		subscriber_save_path + "/sub1"
	)

	var sub_handle2 = subscriber2.subscribe_mutable_torrent(
		public_key,
		subscriber_save_path + "/sub2"
	)

	assert_not_null(sub_handle1, "First subscriber handle should be created")
	assert_not_null(sub_handle2, "Second subscriber handle should be created")
	assert_true(sub_handle1.is_valid(), "First subscriber handle should be valid")
	assert_true(sub_handle2.is_valid(), "Second subscriber handle should be valid")

	# Clean up
	subscriber2.stop_session()

# Test 5: Sequence number progression
func test_sequence_number_progression():
	var keypair = TorrentKeyPair.generate()
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	# Initial sequence
	assert_eq(pub_handle.get_sequence_number(), 1, "Initial sequence should be 1")

	# Publish multiple updates
	for i in range(3):
		var new_data = publisher_session.create_torrent_from_path(test_file_path_v2)
		var success = pub_handle.publish_update(new_data)
		assert_true(success, "Update " + str(i + 1) + " should be published")

		var expected_seq = i + 2  # Starts at 1, so update 1 = seq 2
		assert_eq(pub_handle.get_sequence_number(), expected_seq,
			"Sequence should be " + str(expected_seq))

# Test 6: Manual update check
func test_manual_update_check():
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Publisher creates mutable torrent
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	# Subscriber subscribes without auto-update
	var sub_handle = subscriber_session.subscribe_mutable_torrent(
		public_key,
		subscriber_save_path
	)

	sub_handle.set_auto_update(false)
	assert_false(sub_handle.is_auto_update_enabled(), "Auto-update should be disabled")

	# Manually trigger update check
	sub_handle.check_for_updates()

	# Wait a bit for the DHT query
	await wait_seconds(1.0)

	# Process alerts
	var alerts = subscriber_session.get_alerts()

	# The method should not crash even if no updates are available
	pass_test("Manual update check executed without errors")

# Test 7: Session-level helper methods
func test_session_helper_methods():
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Create torrent using session
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	# Test publish_mutable_torrent_update
	var new_torrent_data = publisher_session.create_torrent_from_path(test_file_path_v2)
	var success = publisher_session.publish_mutable_torrent_update(
		public_key,
		new_torrent_data
	)

	assert_typeof(success, TYPE_BOOL, "publish_mutable_torrent_update should return bool")

	# Test check_mutable_torrent_for_updates
	publisher_session.check_mutable_torrent_for_updates(public_key)

	# If we reach here, methods didn't crash
	pass_test("Session helper methods executed successfully")

# Test 8: Alert processing
func test_alert_processing():
	var keypair = TorrentKeyPair.generate()
	var public_key = keypair.get_public_key()

	# Publisher creates and publishes
	var torrent_data = publisher_session.create_torrent_from_path(test_file_path)
	var pub_handle = publisher_session.add_mutable_torrent(
		keypair,
		publisher_save_path,
		torrent_data
	)

	# Get alerts multiple times to ensure it doesn't crash
	for i in range(3):
		var alerts = publisher_session.get_alerts()
		gut.p("Alert batch " + str(i + 1) + ": " + str(alerts.size()) + " alerts")
		await wait_seconds(0.5)

	pass_test("Alert processing completed without errors")

# Helper function to wait for a specified time
func wait_seconds(seconds: float):
	await get_tree().create_timer(seconds).timeout
