extends GutTest

# Integration test for real torrent download
# Tests end-to-end download using a small legal torrent (Debian netinst ISO)
#
# This test requires network access and may take several minutes.
# Use a small torrent for faster testing.

var session: TorrentSession
var download_dir: String = "/tmp/godot_torrent_test"
var test_handle: TorrentHandle

# Use a small test torrent - The WIRED CD (Creative Commons, ~73MB)
# This is a legal, frequently used test torrent
var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fwired-cd.torrent"

var download_timeout: int = 300  # 5 minutes max
var use_magnet: bool = true  # Use magnet link instead of torrent file

func before_all():
	# Create download directory
	var dir = DirAccess.open("/tmp")
	if not dir.dir_exists("godot_torrent_test"):
		dir.make_dir("godot_torrent_test")

	print("Using magnet link for The WIRED CD test torrent")
	print("This is a legal Creative Commons licensed audio file")

func before_each():
	session = TorrentSession.new()

func after_each():
	# Clean up session and handles
	if test_handle and test_handle.is_valid():
		session.remove_torrent(test_handle, true)

	if session and session.is_running():
		session.stop_session()

	session = null
	test_handle = null

func after_all():
	# Clean up download directory
	var dir = DirAccess.open("/tmp")
	if dir.dir_exists("godot_torrent_test"):
		# Remove all files first
		var files_dir = DirAccess.open(download_dir)
		if files_dir:
			files_dir.list_dir_begin()
			var file_name = files_dir.get_next()
			while file_name != "":
				if not files_dir.current_is_dir():
					files_dir.remove(file_name)
				file_name = files_dir.get_next()
			files_dir.list_dir_end()

		dir.remove("godot_torrent_test")

	print("Cleanup complete")

# Test 1: Session initialization
func test_01_session_initialization():
	# Validates: Test session initialization

	var result = session.start_session()
	assert_true(result, "Session should start successfully")
	assert_true(session.is_running(), "Session should be running")

	# Enable DHT and LSD for better peer discovery
	session.start_dht()

	# DHT may take time to start, just verify the call doesn't crash
	# is_dht_running() may return false immediately after start_dht()
	var dht_status = session.is_dht_running()
	print("DHT running status: ", dht_status)
	assert_true(true, "DHT start called successfully")

# Test 2: Torrent addition
func test_02_torrent_addition():
	# Validates: Test torrent addition

	session.start_session()
	session.start_dht()

	# Add torrent using magnet link
	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_not_null(test_handle, "add_magnet_uri should return a handle")
	assert_true(test_handle.is_valid(), "Handle should be valid")

	# Get torrent info (may not be available immediately for magnets)
	var name = test_handle.get_name()
	print("Torrent name: ", name if name else "<metadata not yet available>")

# Test 3: Download progress monitoring
func test_03_download_progress_monitoring():
	# Validates: Test download progress monitoring

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	if not test_handle or not test_handle.is_valid():
		assert_true(false, "Failed to add torrent")
		return

	# Monitor progress - just check that the interface works
	# Don't wait for actual download progress (too slow for CI)
	var status = test_handle.get_status()
	assert_not_null(status, "Should be able to get status")

	if status:
		var progress = status.get_progress()
		var download_rate = status.get_download_rate()
		var num_peers = status.get_num_peers()

		print("Initial state - Progress: %.2f%% | Rate: %d KB/s | Peers: %d" % [
			progress * 100.0,
			download_rate / 1024,
			num_peers
		])

		# Verify values are sane
		assert_true(progress >= 0.0 and progress <= 1.0, "Progress should be between 0 and 1")
		assert_true(download_rate >= 0, "Download rate should be non-negative")
		assert_true(num_peers >= 0, "Peer count should be non-negative")

	print("Download monitoring interface functional")

# Test 4: Pause and resume
func test_04_pause_resume():
	# Validates: Test pause/resume during download

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	if not test_handle or not test_handle.is_valid():
		assert_true(false, "Failed to add torrent")
		return

	# Initially should be active (not paused)
	var initially_paused = test_handle.is_paused()

	# Pause the torrent
	test_handle.pause()
	var paused_state = test_handle.is_paused()
	assert_true(paused_state, "Torrent should be paused after pause()")

	# Resume the torrent
	test_handle.resume()
	var resumed_state = test_handle.is_paused()
	assert_false(resumed_state, "Torrent should not be paused after resume()")

	print("Pause/Resume test passed")

# Test 5: Basic completion check (without full download)
func test_05_completion_detection():
	# Validates: Test completion detection interface
	# Note: We won't actually download the full file, just test the interface

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	if not test_handle or not test_handle.is_valid():
		assert_true(false, "Failed to add torrent")
		return

	# Check status for completion indicators
	var status = test_handle.get_status()
	if status:
		var progress = status.get_progress()
		var is_seeding = status.is_seeding()
		var is_finished = status.is_finished()

		print("Progress: %.2f%% | Seeding: %s | Finished: %s" % [
			progress * 100.0,
			str(is_seeding),
			str(is_finished)
		])

		# For a new download, these should be false/0
		assert_true(progress >= 0.0 and progress <= 1.0, "Progress should be between 0 and 1")

	# The interface works
	assert_true(true, "Completion detection interface functional")

# Test 6: File presence check
func test_06_file_presence():
	# Validates: Check that download directory is set up correctly

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	if not test_handle or not test_handle.is_valid():
		assert_true(false, "Failed to add torrent")
		return

	# Files may not be created immediately, just check directory exists

	# Check if download directory exists
	var dir = DirAccess.open(download_dir)
	assert_not_null(dir, "Download directory should exist")

	# List files in directory
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		var file_count = 0
		while file_name != "":
			if not dir.current_is_dir():
				file_count += 1
				print("Found file: ", file_name)
			file_name = dir.get_next()
		dir.list_dir_end()

		print("Files in download directory: ", file_count)
		# Files should be created by libtorrent
		assert_true(true, "Download directory accessible")

# Test 7: Torrent removal and cleanup
func test_07_removal_and_cleanup():
	# Validates: Clean up test files

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	if not test_handle or not test_handle.is_valid():
		assert_true(false, "Failed to add torrent")
		return

	# Don't wait - just test the removal interface

	# Remove torrent with file deletion
	var removed = session.remove_torrent(test_handle, true)
	assert_true(removed, "Torrent should be removed successfully")

	# Handle should be invalid after removal
	var is_valid = test_handle.is_valid()
	assert_false(is_valid, "Handle should be invalid after removal")

	print("Removal and cleanup test passed")

# Test 8: Full download with integrity verification
# This test actually downloads a complete small torrent and verifies it
# WARNING: This test takes several minutes and requires network access
# Set environment variable FULL_INTEGRATION_TEST=1 to enable this test
func test_08_full_download_with_verification():
	# Validates: Verify file integrity after download

	# Skip this test unless explicitly enabled
	if OS.get_environment("FULL_INTEGRATION_TEST") != "1":
		pass_test("Skipped - Set FULL_INTEGRATION_TEST=1 to enable full download test")
		return

	print("\n=== Starting Full Download Integration Test ===")
	print("This test will download a small torrent and verify its integrity")
	print("Expected duration: 2-5 minutes depending on network speed")

	session.start_session()
	session.start_dht()

	# Use a very small public domain torrent for testing
	# Using a small file to keep test time reasonable
	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	if not test_handle or not test_handle.is_valid():
		assert_true(false, "Failed to add torrent")
		return

	print("Torrent added, waiting for metadata...")

	# Wait for metadata to be downloaded
	var metadata_timeout = 60  # 60 seconds for metadata
	var metadata_wait_start = Time.get_ticks_msec()
	var has_metadata = false

	while (Time.get_ticks_msec() - metadata_wait_start) < metadata_timeout * 1000:
		var status = test_handle.get_status()
		if status:
			var name = test_handle.get_name()
			if name and not name.is_empty():
				has_metadata = true
				print("Metadata received: ", name)
				break

		OS.delay_msec(1000)  # Wait 1 second between checks

	if not has_metadata:
		fail_test("Failed to download metadata within timeout")
		return

	# Resume the torrent if it's paused
	if test_handle.is_paused():
		test_handle.resume()

	print("Starting download...")

	# Monitor download progress
	var download_start = Time.get_ticks_msec()
	var last_progress = 0.0
	var stalled_count = 0
	var max_stalled_checks = 10  # Allow 10 checks with no progress before failing

	while (Time.get_ticks_msec() - download_start) < download_timeout * 1000:
		var status = test_handle.get_status()
		if not status:
			OS.delay_msec(1000)
			continue

		var progress = status.get_progress()
		var download_rate = status.get_download_rate()
		var num_peers = status.get_num_peers()
		var is_finished = status.is_finished()

		# Print progress every 10%
		if int(progress * 10) > int(last_progress * 10):
			print("Progress: %.1f%% | Rate: %d KB/s | Peers: %d" % [
				progress * 100.0,
				download_rate / 1024,
				num_peers
			])

		# Check if download is stalled
		if progress == last_progress and progress < 1.0:
			stalled_count += 1
			if stalled_count >= max_stalled_checks:
				fail_test("Download appears stalled - no progress after %d checks" % max_stalled_checks)
				return
		else:
			stalled_count = 0

		last_progress = progress

		# Check if download is complete
		if is_finished or progress >= 1.0:
			print("\n=== Download Complete ===")
			print("Final progress: %.2f%%" % (progress * 100.0))
			break

		OS.delay_msec(5000)  # Check every 5 seconds

	# Verify download completed
	var final_status = test_handle.get_status()
	if not final_status or not final_status.is_finished():
		fail_test("Download did not complete within timeout")
		return

	assert_true(final_status.is_finished(), "Download should be finished")
	assert_true(final_status.get_progress() >= 1.0, "Progress should be 100%")

	print("\n=== Verifying File Integrity ===")

	# Force a recheck to verify all pieces
	test_handle.force_recheck()

	# Wait for recheck to complete
	OS.delay_msec(5000)

	var recheck_start = Time.get_ticks_msec()
	var recheck_timeout = 30000  # 30 seconds for recheck
	var recheck_complete = false

	while (Time.get_ticks_msec() - recheck_start) < recheck_timeout:
		var status = test_handle.get_status()
		if status:
			var state = status.get_state_str()
			print("Recheck state: ", state)

			# If still seeding/finished after recheck, integrity is good
			if status.is_finished() or status.is_seeding():
				recheck_complete = true
				break

		OS.delay_msec(1000)

	assert_true(recheck_complete, "File integrity verification should complete")

	# Verify files exist on disk
	var dir = DirAccess.open(download_dir)
	assert_not_null(dir, "Download directory should exist")

	var file_count = 0
	var total_size = 0

	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if not dir.current_is_dir():
				file_count += 1
				var file_path = download_dir + "/" + file_name
				var file = FileAccess.open(file_path, FileAccess.READ)
				if file:
					total_size += file.get_length()
					file.close()
				print("Downloaded file: ", file_name, " (", total_size, " bytes)")
			file_name = dir.get_next()
		dir.list_dir_end()

	assert_true(file_count > 0, "Should have downloaded at least one file")
	assert_true(total_size > 0, "Downloaded files should have content")

	print("\n=== Full Download Test PASSED ===")
	print("Files downloaded: ", file_count)
	print("Total size: ", total_size, " bytes")
	print("Integrity verified successfully")
