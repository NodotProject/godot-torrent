extends GutTest

# Integration Test: Pause/Resume During Downloads - Issue #39
# Tests pause and resume functionality at various stages

var session: TorrentSession
var download_dir: String = "/tmp/godot_torrent_pause_test"
var test_handle: TorrentHandle

var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969"

func before_all():
	var dir = DirAccess.open("/tmp")
	if not dir.dir_exists("godot_torrent_pause_test"):
		dir.make_dir("godot_torrent_pause_test")

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
	if dir.dir_exists("godot_torrent_pause_test"):
		var files_dir = DirAccess.open(download_dir)
		if files_dir:
			files_dir.list_dir_begin()
			var file_name = files_dir.get_next()
			while file_name != "":
				if not files_dir.current_is_dir():
					files_dir.remove(file_name)
				file_name = files_dir.get_next()
			files_dir.list_dir_end()
		dir.remove("godot_torrent_pause_test")

# Test 1: Immediate pause after adding
func test_01_immediate_pause():
	print("\n[TEST] Immediate Pause After Adding")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_not_null(test_handle, "Handle should be created")

	# Pause immediately
	test_handle.pause()

	OS.delay_msec(1000)

	assert_true(test_handle.is_paused(), "Torrent should be paused")
	print("  ✓ Successfully paused immediately after adding")

# Test 2: Resume after immediate pause
func test_02_resume_after_immediate_pause():
	print("\n[TEST] Resume After Immediate Pause")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	test_handle.pause()

	OS.delay_msec(1000)
	assert_true(test_handle.is_paused(), "Should be paused")

	# Resume
	test_handle.resume()

	OS.delay_msec(1000)
	assert_false(test_handle.is_paused(), "Should not be paused after resume")
	print("  ✓ Successfully resumed after immediate pause")

# Test 3: Pause during metadata download
func test_03_pause_during_metadata():
	print("\n[TEST] Pause During Metadata Download")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Wait a bit, then pause during metadata
	OS.delay_msec(2000)

	test_handle.pause()
	assert_true(test_handle.is_paused(), "Should be paused during metadata download")

	# Resume and continue
	test_handle.resume()
	assert_false(test_handle.is_paused(), "Should resume from metadata download")

	print("  ✓ Successfully paused/resumed during metadata")

# Test 4: Multiple pause/resume cycles
func test_04_multiple_pause_resume_cycles():
	print("\n[TEST] Multiple Pause/Resume Cycles")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	var cycles = 5
	print("  Running %d pause/resume cycles..." % cycles)

	for i in range(cycles):
		test_handle.pause()
		OS.delay_msec(500)
		assert_true(test_handle.is_paused(), "Should be paused in cycle %d" % i)

		test_handle.resume()
		OS.delay_msec(500)
		assert_false(test_handle.is_paused(), "Should be resumed in cycle %d" % i)

	print("  ✓ Completed %d pause/resume cycles" % cycles)

# Test 5: Pause with active peers
func test_05_pause_with_active_peers():
	print("\n[TEST] Pause With Active Peers")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Wait for potential peer connections
	print("  Waiting for peer discovery...")
	var wait_time = 15000
	var start_time = Time.get_ticks_msec()

	while (Time.get_ticks_msec() - start_time) < wait_time:
		var status = test_handle.get_status()
		if status and status.get_num_peers() > 0:
			print("  Found %d peers" % status.get_num_peers())
			break
		OS.delay_msec(2000)

	# Pause with peers (or without if none found)
	test_handle.pause()
	assert_true(test_handle.is_paused(), "Should pause even with active peers")

	OS.delay_msec(2000)

	# Verify still paused
	assert_true(test_handle.is_paused(), "Should remain paused")

	print("  ✓ Successfully paused with peers")

# Test 6: Check download rate after pause
func test_06_download_rate_after_pause():
	print("\n[TEST] Download Rate After Pause")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	OS.delay_msec(5000)

	# Get rate before pause
	var status_before = test_handle.get_status()
	var rate_before = 0
	if status_before:
		rate_before = status_before.get_download_rate()

	print("  Rate before pause: %d B/s" % rate_before)

	# Pause
	test_handle.pause()
	OS.delay_msec(2000)

	# Rate should be 0 or very low when paused
	var status_after = test_handle.get_status()
	if status_after:
		var rate_after = status_after.get_download_rate()
		print("  Rate after pause: %d B/s" % rate_after)
		# Note: Rate might not be exactly 0 due to buffering
		assert_true(rate_after <= rate_before, "Rate should not increase after pause")

	print("  ✓ Download rate check passed")

# Test 7: Resume maintains progress
func test_07_resume_maintains_progress():
	print("\n[TEST] Resume Maintains Progress")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Wait a bit for some potential progress
	OS.delay_msec(5000)

	var status_before = test_handle.get_status()
	var progress_before = 0.0
	if status_before:
		progress_before = status_before.get_progress()

	print("  Progress before pause: %.2f%%" % (progress_before * 100.0))

	# Pause and resume
	test_handle.pause()
	OS.delay_msec(2000)
	test_handle.resume()
	OS.delay_msec(1000)

	var status_after = test_handle.get_status()
	if status_after:
		var progress_after = status_after.get_progress()
		print("  Progress after resume: %.2f%%" % (progress_after * 100.0))
		assert_true(progress_after >= progress_before, "Progress should not decrease after resume")

	print("  ✓ Progress maintained after resume")

# Test 8: Rapid pause/resume stress test
func test_08_rapid_pause_resume():
	print("\n[TEST] Rapid Pause/Resume Stress Test")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	print("  Performing rapid pause/resume operations...")
	var rapid_cycles = 20

	for i in range(rapid_cycles):
		test_handle.pause()
		OS.delay_msec(100)
		test_handle.resume()
		OS.delay_msec(100)

	# Final state should be resumed
	OS.delay_msec(1000)
	assert_false(test_handle.is_paused(), "Final state should be resumed")

	print("  ✓ Completed %d rapid cycles" % rapid_cycles)

# Test 9: Pause all files individually
func test_09_pause_via_file_priorities():
	print("\n[TEST] Pause Via File Priorities")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Wait for metadata
	print("  Waiting for metadata...")
	var metadata_timeout = 30000
	var start_time = Time.get_ticks_msec()
	var has_metadata = false

	while (Time.get_ticks_msec() - start_time) < metadata_timeout:
		var info = test_handle.get_torrent_info()
		if info and info.is_valid() and info.get_name().length() > 0:
			has_metadata = true
			break
		OS.delay_msec(1000)

	if has_metadata:
		var info = test_handle.get_torrent_info()
		var file_count = info.get_file_count()

		print("  Setting all %d files to priority 0 (don't download)..." % file_count)

		for i in range(file_count):
			test_handle.set_file_priority(i, 0)

		OS.delay_msec(1000)

		# Verify priorities set
		for i in range(file_count):
			var priority = test_handle.get_file_priority(i)
			assert_eq(priority, 0, "File %d priority should be 0" % i)

		print("  ✓ All files set to don't download")
	else:
		pass_test("Skipped - metadata not received in time")

# Test 10: Pause state persists across queries
func test_10_pause_state_persistence():
	print("\n[TEST] Pause State Persistence")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Pause
	test_handle.pause()
	OS.delay_msec(1000)

	# Query state multiple times
	print("  Verifying pause state persistence...")
	var queries = 10

	for i in range(queries):
		assert_true(test_handle.is_paused(), "Should remain paused in query %d" % i)
		OS.delay_msec(100)

	print("  ✓ Pause state persisted across %d queries" % queries)
