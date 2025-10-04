extends GutTest

# Tests for TorrentSession - Pure libtorrent wrapper

var session: TorrentSession

func before_each():
	session = TorrentSession.new()

func after_each():
	if session and session.is_running():
		session.stop_session()
	session = null

func test_session_creation():
	assert_not_null(session, "Session should be created")
	assert_false(session.is_running(), "Session should not be running initially")

func test_session_start_stop():
	var result = session.start_session()
	assert_true(result, "Session should start successfully")
	assert_true(session.is_running(), "Session should be running after start")

	session.stop_session()
	await wait_frames(5)
	assert_false(session.is_running(), "Session should stop")

func test_session_stats():
	session.start_session()
	var stats = session.get_session_stats()
	assert_not_null(stats, "Stats should not be null")
	# Stats may be empty depending on libtorrent implementation

func test_bandwidth_limits():
	session.start_session()

	# These should not crash
	session.set_download_rate_limit(1000000)  # 1MB/s
	session.set_upload_rate_limit(500000)     # 500KB/s

	assert_true(session.is_running(), "Session should still be running after setting limits")

func test_dht_operations():
	session.start_session()

	# Test DHT operations
	session.start_dht()
	var dht_running = session.is_dht_running()
	assert_true(typeof(dht_running) == TYPE_BOOL, "is_dht_running should return bool")

	session.stop_dht()

	assert_true(session.is_running(), "Session should still be running after DHT operations")
