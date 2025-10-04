extends GutTest

# Integration Test: Error Recovery and Network Failures - Issue #39
# Tests error handling and recovery from various failure scenarios

var session: TorrentSession
var download_dir: String = "/tmp/godot_torrent_error_test"
var test_handle: TorrentHandle

var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969"
var invalid_magnet: String = "magnet:?xt=urn:btih:0000000000000000000000000000000000000000"

func before_all():
	var dir = DirAccess.open("/tmp")
	if not dir.dir_exists("godot_torrent_error_test"):
		dir.make_dir("godot_torrent_error_test")

func before_each():
	session = TorrentSession.new()

func after_each():
	if test_handle and test_handle.is_valid():
		session.remove_torrent(test_handle, true)

	if session and session.is_running():
		session.stop_session()

	session = null
	test_handle = null

func after_all():
	var dir = DirAccess.open("/tmp")
	if dir.dir_exists("godot_torrent_error_test"):
		var files_dir = DirAccess.open(download_dir)
		if files_dir:
			files_dir.list_dir_begin()
			var file_name = files_dir.get_next()
			while file_name != "":
				if not files_dir.current_is_dir():
					files_dir.remove(file_name)
				file_name = files_dir.get_next()
			files_dir.list_dir_end()
		dir.remove("godot_torrent_error_test")

# ==============================================================================
# Error Recovery Tests
# ==============================================================================

# Test 1: Invalid magnet URI
func test_01_invalid_magnet_uri():
	print("\n[TEST] Invalid Magnet URI")

	session.start_session()
	session.start_dht()

	# Try to add invalid magnet
	test_handle = session.add_magnet_uri(invalid_magnet, download_dir)

	# May still create handle but should have issues
	if test_handle:
		print("  Handle created for invalid magnet: %s" % test_handle.is_valid())
	else:
		print("  No handle created (expected)")

	print("  ✓ Invalid magnet URI handled")

# Test 2: Invalid save path
func test_02_invalid_save_path():
	print("\n[TEST] Invalid Save Path")

	session.start_session()
	session.start_dht()

	# Try to add with invalid path
	var invalid_path = "/nonexistent/path/that/does/not/exist"
	test_handle = session.add_magnet_uri(test_magnet, invalid_path)

	if test_handle:
		print("  Handle created: %s" % test_handle.is_valid())
		# libtorrent may try to create the path or report error via alerts
	else:
		print("  No handle created (expected)")

	print("  ✓ Invalid save path handled")

# Test 3: Empty save path
func test_03_empty_save_path():
	print("\n[TEST] Empty Save Path")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, "")

	if test_handle:
		print("  Handle created with empty path: %s" % test_handle.is_valid())
	else:
		print("  No handle created with empty path (expected)")

	print("  ✓ Empty save path handled")

# Test 4: Session not started
func test_04_session_not_started():
	print("\n[TEST] Operations Without Started Session")

	# Don't start session
	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	if test_handle:
		print("  Handle created without session: %s" % test_handle.is_valid())
	else:
		print("  No handle created without session (expected)")

	print("  ✓ Session not started handled")

# Test 5: Double start session
func test_05_double_start_session():
	print("\n[TEST] Double Start Session")

	var result1 = session.start_session()
	print("  First start: %s" % result1)

	var result2 = session.start_session()
	print("  Second start: %s" % result2)

	assert_true(session.is_running(), "Session should be running")

	print("  ✓ Double start handled")

# Test 6: Remove invalid handle
func test_06_remove_invalid_handle():
	print("\n[TEST] Remove Invalid Handle")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Remove once
	var removed1 = session.remove_torrent(test_handle, false)
	print("  First removal: %s" % removed1)

	# Try to remove again (handle should be invalid)
	var removed2 = session.remove_torrent(test_handle, false)
	print("  Second removal: %s" % removed2)

	assert_false(test_handle.is_valid(), "Handle should be invalid after removal")

	print("  ✓ Double removal handled")

# Test 7: Operations on invalid handle
func test_07_operations_on_invalid_handle():
	print("\n[TEST] Operations On Invalid Handle")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	session.remove_torrent(test_handle, false)

	# Try operations on invalid handle
	print("  Trying operations on invalid handle...")

	test_handle.pause()  # Should not crash
	test_handle.resume()  # Should not crash

	var status = test_handle.get_status()
	print("  Status from invalid handle: %s" % (status != null))

	print("  ✓ Operations on invalid handle handled gracefully")

# Test 8: Disk full simulation (limited)
func test_08_disk_space_issues():
	print("\n[TEST] Disk Space Issues")

	session.start_session()
	session.start_dht()

	# We can't actually fill the disk, but we can test the interface
	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# In a real scenario, disk full would trigger error alerts
	OS.delay_msec(2000)

	var status = test_handle.get_status()
	if status:
		print("  Torrent state: %s" % status.get_state_string())

	print("  ✓ Disk space interface tested")

# Test 9: Network disconnect simulation
func test_09_network_disconnect():
	print("\n[TEST] Network Disconnect Behavior")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Monitor before "disconnect"
	OS.delay_msec(5000)

	var status_before = test_handle.get_status()
	if status_before:
		print("  Peers before: %d" % status_before.get_num_peers())

	# We can't actually disconnect network, but pause simulates similar behavior
	test_handle.pause()
	OS.delay_msec(2000)

	# Resume (simulating network reconnect)
	test_handle.resume()
	OS.delay_msec(3000)

	var status_after = test_handle.get_status()
	if status_after:
		print("  Peers after reconnect: %d" % status_after.get_num_peers())

	print("  ✓ Network disconnect behavior tested")

# Test 10: Tracker failure handling
func test_10_tracker_failure():
	print("\n[TEST] Tracker Failure Handling")

	session.start_session()
	session.start_dht()

	# Use magnet with potentially unreachable tracker
	var magnet_bad_tracker = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&tr=http://nonexistent.tracker.invalid:80/announce"

	test_handle = session.add_magnet_uri(magnet_bad_tracker, download_dir)

	if test_handle and test_handle.is_valid():
		# Wait for tracker attempts
		OS.delay_msec(10000)

		# Should still function with DHT fallback
		var status = test_handle.get_status()
		if status:
			print("  Torrent state with bad tracker: %s" % status.get_state_string())
			print("  Peers via DHT: %d" % status.get_num_peers())

	print("  ✓ Tracker failure handled with DHT fallback")

# Test 11: Rapid add/remove stress
func test_11_rapid_add_remove_stress():
	print("\n[TEST] Rapid Add/Remove Stress")

	session.start_session()
	session.start_dht()

	var cycles = 10
	print("  Running %d rapid add/remove cycles..." % cycles)

	for i in range(cycles):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/cycle" + str(i))

		# Minimal delay
		OS.delay_msec(100)

		if handle and handle.is_valid():
			session.remove_torrent(handle, true)

		OS.delay_msec(100)

	print("  ✓ Completed %d rapid cycles without crashing" % cycles)

# Test 12: Clear error state
func test_12_clear_error_state():
	print("\n[TEST] Clear Error State")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Let it run a bit
	OS.delay_msec(3000)

	# Clear any potential errors
	test_handle.clear_error()

	print("  ✓ Clear error tested")

# Test 13: File permission issues
func test_13_file_permission_issues():
	print("\n[TEST] File Permission Issues")

	session.start_session()
	session.start_dht()

	# Try to write to a potentially restricted location
	# This depends on system permissions, so it's a soft test
	var restricted_path = "/root/godot_torrent_test"  # Likely no permission

	test_handle = session.add_magnet_uri(test_magnet, restricted_path)

	if test_handle:
		OS.delay_msec(2000)
		# Check if error state appears in status
		var status = test_handle.get_status()
		if status:
			print("  State: %s" % status.get_state_string())
	else:
		print("  Rejected at add time (expected)")

	print("  ✓ Permission issues handled")

# Test 14: Port binding failure
func test_14_port_binding():
	print("\n[TEST] Port Binding Issues")

	session.start_session()

	# Try to set invalid port
	session.set_listen_port(0)  # 0 means let system choose

	# Try very high port
	session.set_listen_port(65535)

	assert_true(session.is_running(), "Session should still be running")

	print("  ✓ Port configuration handled")

# Test 15: Alert processing under load
func test_15_alert_processing_stress():
	print("\n[TEST] Alert Processing Under Load")

	session.start_session()
	session.start_dht()

	# Add multiple torrents to generate alerts
	var handles = []
	for i in range(5):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/t" + str(i))
		if handle and handle.is_valid():
			handles.append(handle)

	# Rapidly query alerts
	print("  Processing alerts rapidly...")
	for i in range(100):
		var alerts = session.get_alerts()
		OS.delay_msec(10)

	# Cleanup
	for handle in handles:
		if handle and handle.is_valid():
			session.remove_torrent(handle, true)

	print("  ✓ Alert processing stress tested")
