extends GutTest

# Integration Test: Priority Changes During Downloads - Issue #39
# Tests changing file and piece priorities dynamically

var session: TorrentSession
var download_dir: String = "/tmp/godot_torrent_priority_test"
var test_handle: TorrentHandle

var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969"

func before_all():
	var dir = DirAccess.open("/tmp")
	if not dir.dir_exists("godot_torrent_priority_test"):
		dir.make_dir("godot_torrent_priority_test")

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
	if dir.dir_exists("godot_torrent_priority_test"):
		var files_dir = DirAccess.open(download_dir)
		if files_dir:
			files_dir.list_dir_begin()
			var file_name = files_dir.get_next()
			while file_name != "":
				if not files_dir.current_is_dir():
					files_dir.remove(file_name)
				file_name = files_dir.get_next()
			files_dir.list_dir_end()
		dir.remove("godot_torrent_priority_test")

func _wait_for_metadata(timeout_seconds: int) -> bool:
	var start_time = Time.get_ticks_msec()
	var timeout_ms = timeout_seconds * 1000

	while (Time.get_ticks_msec() - start_time) < timeout_ms:
		var info = test_handle.get_torrent_info()
		if info and info.is_valid():
			var name = info.get_name()
			if name and name.length() > 0:
				return true
		OS.delay_msec(1000)

	return false

# Test 1: Change file priority before download starts
func test_01_file_priority_before_download():
	print("\n[TEST] File Priority Before Download")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()
	print("  File count: %d" % file_count)

	if file_count > 0:
		# Set first file to high priority
		test_handle.set_file_priority(0, 7)
		var priority = test_handle.get_file_priority(0)
		assert_eq(priority, 7, "File 0 should have priority 7")
		print("  ✓ Set file 0 to high priority: %d" % priority)

		# Set remaining files to low priority
		for i in range(1, file_count):
			test_handle.set_file_priority(i, 1)
			var p = test_handle.get_file_priority(i)
			assert_eq(p, 1, "File %d should have priority 1" % i)

		print("  ✓ Successfully set priorities before download")

# Test 2: Change file priority during download
func test_02_file_priority_during_download():
	print("\n[TEST] File Priority During Download")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	if file_count > 1:
		# Start with all normal priority
		for i in range(file_count):
			test_handle.set_file_priority(i, 4)

		# Let it download a bit
		OS.delay_msec(5000)

		# Change priorities mid-download
		print("  Changing priorities during download...")
		test_handle.set_file_priority(0, 7)  # High
		if file_count > 1:
			test_handle.set_file_priority(1, 0)  # Don't download

		OS.delay_msec(1000)

		# Verify changes applied
		assert_eq(test_handle.get_file_priority(0), 7, "File 0 should be high priority")
		if file_count > 1:
			assert_eq(test_handle.get_file_priority(1), 0, "File 1 should be don't download")

		print("  ✓ Successfully changed priorities during download")

# Test 3: Toggle file download on/off
func test_03_toggle_file_download():
	print("\n[TEST] Toggle File Download On/Off")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	if file_count > 0:
		print("  Toggling file 0 download...")

		# Disable
		test_handle.set_file_priority(0, 0)
		assert_eq(test_handle.get_file_priority(0), 0, "Should be disabled")

		OS.delay_msec(1000)

		# Enable
		test_handle.set_file_priority(0, 4)
		assert_eq(test_handle.get_file_priority(0), 4, "Should be enabled")

		OS.delay_msec(1000)

		# Disable again
		test_handle.set_file_priority(0, 0)
		assert_eq(test_handle.get_file_priority(0), 0, "Should be disabled again")

		print("  ✓ Successfully toggled file download")

# Test 4: Set all priorities at once
func test_04_set_all_priorities():
	print("\n[TEST] Set All File Priorities")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	print("  Setting priorities for all %d files..." % file_count)

	# Set alternating priorities
	for i in range(file_count):
		var priority = 7 if i % 2 == 0 else 1
		test_handle.set_file_priority(i, priority)

	# Verify all set correctly
	for i in range(file_count):
		var expected = 7 if i % 2 == 0 else 1
		var actual = test_handle.get_file_priority(i)
		assert_eq(actual, expected, "File %d priority should be %d" % [i, expected])

	print("  ✓ Successfully set all file priorities")

# Test 5: Piece priority changes
func test_05_piece_priority_changes():
	print("\n[TEST] Piece Priority Changes")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var piece_count = info.get_piece_count()
	print("  Piece count: %d" % piece_count)

	if piece_count > 0:
		# Set first piece to high priority
		test_handle.set_piece_priority(0, 7)
		var priority = test_handle.get_piece_priority(0)
		assert_eq(priority, 7, "Piece 0 should have priority 7")
		print("  ✓ Set piece 0 to priority %d" % priority)

		# Set to normal
		test_handle.set_piece_priority(0, 4)
		priority = test_handle.get_piece_priority(0)
		assert_eq(priority, 4, "Piece 0 should have priority 4")
		print("  ✓ Changed piece 0 to priority %d" % priority)

# Test 6: Sequential piece priorities
func test_06_sequential_piece_priorities():
	print("\n[TEST] Sequential Piece Priorities")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var piece_count = info.get_piece_count()

	# Set first N pieces to high priority for sequential download
	var sequential_count = min(10, piece_count)
	print("  Setting first %d pieces to high priority..." % sequential_count)

	for i in range(sequential_count):
		test_handle.set_piece_priority(i, 7)

	# Verify
	for i in range(sequential_count):
		var priority = test_handle.get_piece_priority(i)
		assert_eq(priority, 7, "Piece %d should be high priority" % i)

	print("  ✓ Sequential priorities set")

# Test 7: Priority change stress test
func test_07_priority_change_stress():
	print("\n[TEST] Priority Change Stress Test")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	if file_count > 0:
		print("  Performing rapid priority changes...")
		var cycles = 20

		for cycle in range(cycles):
			for i in range(file_count):
				var priority = randi() % 8  # Random priority 0-7
				test_handle.set_file_priority(i, priority)

		print("  ✓ Completed %d priority change cycles" % cycles)

# Test 8: Priority and pause interaction
func test_08_priority_pause_interaction():
	print("\n[TEST] Priority and Pause Interaction")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	if file_count > 0:
		# Set priorities while active
		test_handle.set_file_priority(0, 7)

		# Pause
		test_handle.pause()
		OS.delay_msec(1000)

		# Change priority while paused
		test_handle.set_file_priority(0, 1)
		var priority = test_handle.get_file_priority(0)
		assert_eq(priority, 1, "Priority should change while paused")

		# Resume
		test_handle.resume()
		OS.delay_msec(1000)

		# Verify priority persisted
		priority = test_handle.get_file_priority(0)
		assert_eq(priority, 1, "Priority should persist after resume")

		print("  ✓ Priority changes work with pause/resume")

# Test 9: Get file progress with different priorities
func test_09_file_progress_priorities():
	print("\n[TEST] File Progress With Priorities")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	if file_count > 0:
		# Set first file high, others low
		test_handle.set_file_priority(0, 7)
		for i in range(1, file_count):
			test_handle.set_file_priority(i, 1)

		# Wait a bit
		OS.delay_msec(5000)

		# Check progress
		var progress = test_handle.get_file_progress()
		assert_not_null(progress, "Should get file progress")
		assert_eq(progress.size(), file_count, "Progress array size should match file count")

		print("  File progress:")
		for i in range(progress.size()):
			print("    File %d: %d bytes (priority %d)" % [i, progress[i], test_handle.get_file_priority(i)])

		print("  ✓ File progress retrieved with priorities")

# Test 10: Priority persistence across status queries
func test_10_priority_persistence():
	print("\n[TEST] Priority Persistence")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_true(_wait_for_metadata(30), "Metadata should be received")

	var info = test_handle.get_torrent_info()
	var file_count = info.get_file_count()

	if file_count > 0:
		# Set specific priorities
		var priorities = []
		for i in range(file_count):
			var p = (i % 8)  # 0-7
			test_handle.set_file_priority(i, p)
			priorities.append(p)

		# Query multiple times
		print("  Verifying priority persistence...")
		var queries = 10

		for q in range(queries):
			for i in range(file_count):
				var actual = test_handle.get_file_priority(i)
				assert_eq(actual, priorities[i], "File %d priority should persist in query %d" % [i, q])
			OS.delay_msec(100)

		print("  ✓ Priorities persisted across %d queries" % queries)
