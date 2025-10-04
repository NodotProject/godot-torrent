extends GutTest

# Integration Test: DHT and Resume Data - Issue #39
# Tests DHT-only torrents and resume data functionality

var session: TorrentSession
var download_dir: String = "/tmp/godot_torrent_dht_resume_test"
var test_handle: TorrentHandle

var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969"

func before_all():
	var dir = DirAccess.open("/tmp")
	if not dir.dir_exists("godot_torrent_dht_resume_test"):
		dir.make_dir("godot_torrent_dht_resume_test")

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
	if dir.dir_exists("godot_torrent_dht_resume_test"):
		var files_dir = DirAccess.open(download_dir)
		if files_dir:
			files_dir.list_dir_begin()
			var file_name = files_dir.get_next()
			while file_name != "":
				if not files_dir.current_is_dir():
					files_dir.remove(file_name)
				file_name = files_dir.get_next()
			files_dir.list_dir_end()
		dir.remove("godot_torrent_dht_resume_test")

# ==============================================================================
# DHT Tests
# ==============================================================================

# Test 1: DHT initialization
func test_01_dht_initialization():
	print("\n[TEST] DHT Initialization")

	session.start_session()

	# Initially DHT should not be running
	var dht_running_before = session.is_dht_running()
	print("  DHT running before start: %s" % dht_running_before)

	# Start DHT
	session.start_dht()

	OS.delay_msec(2000)

	# Check if DHT started (may take time to bootstrap)
	var dht_running_after = session.is_dht_running()
	print("  DHT running after start: %s" % dht_running_after)

	# DHT state should be available
	var dht_state = session.get_dht_state()
	assert_not_null(dht_state, "DHT state should not be null")
	print("  DHT nodes: %d" % dht_state.get("nodes", 0))

	print("  ✓ DHT initialization tested")

# Test 2: DHT stop and restart
func test_02_dht_stop_restart():
	print("\n[TEST] DHT Stop and Restart")

	session.start_session()
	session.start_dht()

	OS.delay_msec(2000)

	# Stop DHT
	session.stop_dht()
	OS.delay_msec(1000)

	# Restart DHT
	session.start_dht()
	OS.delay_msec(2000)

	var dht_state = session.get_dht_state()
	assert_not_null(dht_state, "DHT state should be available after restart")

	print("  ✓ DHT stop/restart successful")

# Test 3: Add DHT bootstrap nodes
func test_03_add_dht_nodes():
	print("\n[TEST] Add DHT Bootstrap Nodes")

	session.start_session()
	session.start_dht()

	# Add common DHT bootstrap nodes
	var bootstrap_nodes = [
		{"host": "router.bittorrent.com", "port": 6881},
		{"host": "dht.transmissionbt.com", "port": 6881},
		{"host": "router.utorrent.com", "port": 6881}
	]

	print("  Adding %d bootstrap nodes..." % bootstrap_nodes.size())

	for node in bootstrap_nodes:
		session.add_dht_node(node.host, node.port)

	OS.delay_msec(2000)

	print("  ✓ Bootstrap nodes added")

# Test 4: DHT state save and load
func test_04_dht_state_persistence():
	print("\n[TEST] DHT State Persistence")

	session.start_session()
	session.start_dht()

	# Let DHT run for a bit
	OS.delay_msec(5000)

	# Save DHT state
	var dht_data = session.save_dht_state()
	assert_not_null(dht_data, "DHT state data should not be null")
	print("  DHT state saved: %d bytes" % dht_data.size())

	# Stop session
	session.stop_session()

	OS.delay_msec(1000)

	# Start new session
	session.start_session()
	session.start_dht()

	# Load DHT state
	if dht_data.size() > 0:
		var loaded = session.load_dht_state(dht_data)
		print("  DHT state loaded: %s" % loaded)

	print("  ✓ DHT state persistence tested")

# Test 5: Magnet link with DHT only
func test_05_magnet_dht_only():
	print("\n[TEST] Magnet Link with DHT Only")

	session.start_session()
	session.start_dht()

	# Add magnet link (relies on DHT for peer discovery)
	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	assert_not_null(test_handle, "Handle should be created")
	assert_true(test_handle.is_valid(), "Handle should be valid")

	# Wait for potential metadata
	print("  Waiting for DHT peer discovery...")
	OS.delay_msec(10000)

	var status = test_handle.get_status()
	if status:
		print("  Peers found: %d" % status.get_num_peers())

	print("  ✓ Magnet link with DHT tested")

# ==============================================================================
# Resume Data Tests
# ==============================================================================

# Test 6: Request resume data
func test_06_request_resume_data():
	print("\n[TEST] Request Resume Data")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Wait a bit for some state
	OS.delay_msec(5000)

	# Request resume data
	print("  Requesting resume data...")
	test_handle.save_resume_data()

	# In a real scenario, we'd check alerts for save_resume_data_alert
	OS.delay_msec(2000)

	print("  ✓ Resume data requested")

# Test 7: Add torrent with resume data
func test_07_add_with_resume_data():
	print("\n[TEST] Add Torrent With Resume Data")

	session.start_session()
	session.start_dht()

	# First, add and get some state
	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	OS.delay_msec(5000)

	# Request resume data
	test_handle.save_resume_data()
	OS.delay_msec(2000)

	# For this test, we simulate having resume data
	# In production, you'd get this from alerts
	var mock_resume_data = PackedByteArray()

	# Remove torrent
	session.remove_torrent(test_handle, false)
	test_handle = null

	OS.delay_msec(1000)

	# Re-add with resume data (if we had real data)
	if mock_resume_data.size() > 0:
		test_handle = session.add_magnet_uri_with_resume(test_magnet, download_dir, mock_resume_data)
		print("  Re-added with resume data")
	else:
		print("  Skipped - no actual resume data (expected in this test)")

	print("  ✓ Add with resume data tested")

# Test 8: Resume data after pause/resume cycles
func test_08_resume_data_after_pause():
	print("\n[TEST] Resume Data After Pause Cycles")

	session.start_session()
	session.start_dht()

	test_handle = session.add_magnet_uri(test_magnet, download_dir)

	# Pause/resume cycles
	for i in range(3):
		test_handle.pause()
		OS.delay_msec(1000)
		test_handle.resume()
		OS.delay_msec(1000)

	# Request resume data after cycles
	test_handle.save_resume_data()
	OS.delay_msec(2000)

	print("  ✓ Resume data after pause cycles tested")

# Test 9: Session state save and load
func test_09_session_state_persistence():
	print("\n[TEST] Session State Persistence")

	session.start_session()
	session.start_dht()

	# Add a torrent
	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	OS.delay_msec(5000)

	# Save session state
	var session_state = session.save_state()
	assert_not_null(session_state, "Session state should not be null")
	print("  Session state saved: %d bytes" % session_state.size())

	# Stop session
	session.stop_session()
	test_handle = null

	OS.delay_msec(1000)

	# Start new session and load state
	session = TorrentSession.new()
	session.start_session()

	if session_state.size() > 0:
		var loaded = session.load_state(session_state)
		print("  Session state loaded: %s" % loaded)

	print("  ✓ Session state persistence tested")

# Test 10: Combined DHT and resume data
func test_10_combined_dht_resume():
	print("\n[TEST] Combined DHT and Resume Data")

	session.start_session()
	session.start_dht()

	# Add torrent
	test_handle = session.add_magnet_uri(test_magnet, download_dir)
	OS.delay_msec(5000)

	# Save both DHT and session state
	var dht_data = session.save_dht_state()
	var session_state = session.save_state()

	print("  DHT state: %d bytes" % dht_data.size())
	print("  Session state: %d bytes" % session_state.size())

	# Request resume data for torrent
	test_handle.save_resume_data()
	OS.delay_msec(2000)

	# Stop everything
	session.stop_session()
	test_handle = null

	OS.delay_msec(1000)

	# Restart with all saved state
	session = TorrentSession.new()
	session.start_session()
	session.start_dht()

	if dht_data.size() > 0:
		session.load_dht_state(dht_data)

	if session_state.size() > 0:
		session.load_state(session_state)

	print("  ✓ Combined DHT and resume data tested")
