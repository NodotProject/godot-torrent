extends GutTest

# Integration Test: Multi-Torrent Scenarios - Issue #39
# Tests handling multiple torrents simultaneously

var session: TorrentSession
var download_dir: String = "/tmp/godot_torrent_multi_test"
var test_handles: Array = []

# Use small legal test magnet
var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969"

func before_all():
	var dir = DirAccess.open("/tmp")
	if not dir.dir_exists("godot_torrent_multi_test"):
		dir.make_dir("godot_torrent_multi_test")

func before_each():
	session = TorrentSession.new()
	test_handles.clear()

func after_each():
	for handle in test_handles:
		if handle and handle.is_valid():
			session.remove_torrent(handle, true)

	if session and session.is_running():
		session.stop_session()

	session = null
	test_handles.clear()

func after_all():
	var dir = DirAccess.open("/tmp")
	if dir.dir_exists("godot_torrent_multi_test"):
		var files_dir = DirAccess.open(download_dir)
		if files_dir:
			files_dir.list_dir_begin()
			var file_name = files_dir.get_next()
			while file_name != "":
				if files_dir.current_is_dir() and file_name != "." and file_name != "..":
					_remove_dir_recursive(download_dir + "/" + file_name)
				elif not files_dir.current_is_dir():
					files_dir.remove(file_name)
				file_name = files_dir.get_next()
			files_dir.list_dir_end()
		dir.remove("godot_torrent_multi_test")

func _remove_dir_recursive(path: String):
	var dir = DirAccess.open(path)
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			var full_path = path + "/" + file_name
			if dir.current_is_dir() and file_name != "." and file_name != "..":
				_remove_dir_recursive(full_path)
			elif not dir.current_is_dir():
				dir.remove(file_name)
			file_name = dir.get_next()
		dir.list_dir_end()
	DirAccess.remove_absolute(path)

# Test 1: Add multiple torrents simultaneously
func test_01_add_multiple_torrents():
	print("\n[TEST] Add Multiple Torrents")

	session.start_session()
	session.start_dht()

	var num_torrents = 5
	print("  Adding %d torrents..." % num_torrents)

	for i in range(num_torrents):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/torrent" + str(i))
		if handle:
			assert_true(handle.is_valid(), "Handle %d should be valid" % i)
			test_handles.append(handle)

	assert_eq(test_handles.size(), num_torrents, "All torrents should be added")
	print("  ✓ Successfully added %d torrents" % test_handles.size())

# Test 2: Monitor multiple torrents concurrently
func test_02_monitor_multiple_torrents():
	print("\n[TEST] Monitor Multiple Torrents")

	session.start_session()
	session.start_dht()

	var num_torrents = 3
	for i in range(num_torrents):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/torrent" + str(i))
		if handle and handle.is_valid():
			test_handles.append(handle)

	print("  Monitoring %d torrents for 10 seconds..." % test_handles.size())
	var start_time = Time.get_ticks_msec()
	var monitor_time = 10000

	while (Time.get_ticks_msec() - start_time) < monitor_time:
		for i in range(test_handles.size()):
			var handle = test_handles[i]
			if handle and handle.is_valid():
				var status = handle.get_status()
				assert_not_null(status, "Status should be available for torrent %d" % i)

		OS.delay_msec(2000)

	print("  ✓ Successfully monitored all torrents")

# Test 3: Remove torrents individually
func test_03_remove_individual_torrents():
	print("\n[TEST] Remove Individual Torrents")

	session.start_session()
	session.start_dht()

	for i in range(3):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/torrent" + str(i))
		if handle and handle.is_valid():
			test_handles.append(handle)

	var initial_count = test_handles.size()
	print("  Initial torrent count: %d" % initial_count)

	# Remove middle torrent
	if test_handles.size() > 1:
		var removed = session.remove_torrent(test_handles[1], true)
		assert_true(removed, "Should successfully remove torrent")
		assert_false(test_handles[1].is_valid(), "Removed handle should be invalid")

	print("  ✓ Successfully removed individual torrent")

# Test 4: Pause/resume all torrents
func test_04_pause_resume_all():
	print("\n[TEST] Pause/Resume All Torrents")

	session.start_session()
	session.start_dht()

	for i in range(3):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/torrent" + str(i))
		if handle and handle.is_valid():
			test_handles.append(handle)

	# Pause all
	print("  Pausing all torrents...")
	for handle in test_handles:
		if handle and handle.is_valid():
			handle.pause()

	OS.delay_msec(1000)

	# Verify all paused
	for i in range(test_handles.size()):
		var handle = test_handles[i]
		if handle and handle.is_valid():
			assert_true(handle.is_paused(), "Torrent %d should be paused" % i)

	# Resume all
	print("  Resuming all torrents...")
	for handle in test_handles:
		if handle and handle.is_valid():
			handle.resume()

	OS.delay_msec(1000)

	# Verify all resumed
	for i in range(test_handles.size()):
		var handle = test_handles[i]
		if handle and handle.is_valid():
			assert_false(handle.is_paused(), "Torrent %d should not be paused" % i)

	print("  ✓ Successfully paused/resumed all torrents")

# Test 5: Different save paths for each torrent
func test_05_different_save_paths():
	print("\n[TEST] Different Save Paths")

	session.start_session()
	session.start_dht()

	var paths = []
	for i in range(3):
		var path = download_dir + "/separate_" + str(i)
		paths.append(path)

		var dir = DirAccess.open(download_dir)
		if not dir.dir_exists("separate_" + str(i)):
			dir.make_dir("separate_" + str(i))

		var handle = session.add_magnet_uri(test_magnet, path)
		if handle and handle.is_valid():
			test_handles.append(handle)

	assert_eq(test_handles.size(), 3, "All torrents should be added to different paths")
	print("  ✓ Successfully added torrents to different save paths")

# Test 6: Concurrent status queries
func test_06_concurrent_status_queries():
	print("\n[TEST] Concurrent Status Queries")

	session.start_session()
	session.start_dht()

	for i in range(5):
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/torrent" + str(i))
		if handle and handle.is_valid():
			test_handles.append(handle)

	print("  Performing concurrent status queries...")
	var iterations = 100

	for j in range(iterations):
		for handle in test_handles:
			if handle and handle.is_valid():
				var status = handle.get_status()
				assert_not_null(status, "Status should always be available")

	print("  ✓ Completed %d status queries across %d torrents" % [iterations * test_handles.size(), test_handles.size()])

# Test 7: Sequential add/remove stress test
func test_07_sequential_add_remove():
	print("\n[TEST] Sequential Add/Remove")

	session.start_session()
	session.start_dht()

	var cycles = 5
	print("  Running %d add/remove cycles..." % cycles)

	for cycle in range(cycles):
		# Add torrent
		var handle = session.add_magnet_uri(test_magnet, download_dir + "/cycle" + str(cycle))
		assert_not_null(handle, "Handle should be created in cycle %d" % cycle)
		assert_true(handle.is_valid(), "Handle should be valid in cycle %d" % cycle)

		OS.delay_msec(1000)

		# Remove torrent
		var removed = session.remove_torrent(handle, true)
		assert_true(removed, "Torrent should be removed in cycle %d" % cycle)
		assert_false(handle.is_valid(), "Handle should be invalid after removal in cycle %d" % cycle)

	print("  ✓ Completed %d add/remove cycles" % cycles)
