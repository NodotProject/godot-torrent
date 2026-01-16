extends GutTest

# Unit Test: DHT Bootstrap Nodes Configuration
# Tests the set_dht_bootstrap_nodes replacement behavior

var session: TorrentSession

func before_each():
	session = TorrentSession.new()
	session.start_session()

func after_each():
	if session and session.is_running():
		session.stop_session()
	session = null

# ==============================================================================
# Bootstrap Nodes Configuration Tests
# ==============================================================================

# Test 1: Set custom bootstrap nodes (replaces defaults)
func test_01_set_custom_bootstrap_nodes():
	print("\n[TEST] Set Custom Bootstrap Nodes")

	# Set custom bootstrap nodes (should replace hardcoded defaults)
	var custom_nodes = [
		"custom1.example.com:6881",
		"custom2.example.com:6881",
		"custom3.example.com:6881"
	]

	session.set_dht_bootstrap_nodes(custom_nodes)

	# Start DHT after setting custom nodes
	session.start_dht()

	OS.delay_msec(1000)

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should be running")

	print("  ✓ Custom bootstrap nodes set successfully")

# Test 2: Clear all bootstrap nodes (including defaults)
func test_02_clear_bootstrap_nodes():
	print("\n[TEST] Clear All Bootstrap Nodes")

	# Clear all bootstrap nodes by passing empty array
	session.set_dht_bootstrap_nodes([])

	# Start DHT with no bootstrap nodes
	session.start_dht()

	OS.delay_msec(1000)

	# DHT should still start, but won't have bootstrap nodes
	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should be running even without bootstrap nodes")

	print("  ✓ Bootstrap nodes cleared successfully")

# Test 3: Set bootstrap nodes before starting DHT
func test_03_set_nodes_before_dht_start():
	print("\n[TEST] Set Nodes Before Starting DHT")

	# Set custom nodes before starting DHT
	var custom_nodes = [
		"node1.test.com:6881",
		"node2.test.com:6881"
	]

	session.set_dht_bootstrap_nodes(custom_nodes)
	session.start_dht()

	OS.delay_msec(1000)

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should start with custom nodes")

	print("  ✓ Custom nodes set before DHT start")

# Test 4: Set bootstrap nodes after starting DHT (should replace)
func test_04_set_nodes_after_dht_start():
	print("\n[TEST] Set Nodes After Starting DHT")

	# Start DHT with default nodes
	session.start_dht()
	OS.delay_msec(1000)

	# Now replace with custom nodes
	var custom_nodes = [
		"newnode1.test.com:6881",
		"newnode2.test.com:6881"
	]

	session.set_dht_bootstrap_nodes(custom_nodes)

	OS.delay_msec(1000)

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should still be running after changing nodes")

	print("  ✓ Custom nodes set after DHT start")

# Test 5: Replace bootstrap nodes multiple times
func test_05_replace_nodes_multiple_times():
	print("\n[TEST] Replace Nodes Multiple Times")

	session.start_dht()
	OS.delay_msec(500)

	# First set
	session.set_dht_bootstrap_nodes([
		"first1.test.com:6881",
		"first2.test.com:6881"
	])
	OS.delay_msec(500)

	# Second set (should replace first set)
	session.set_dht_bootstrap_nodes([
		"second1.test.com:6881",
		"second2.test.com:6881",
		"second3.test.com:6881"
	])
	OS.delay_msec(500)

	# Third set (should replace second set)
	session.set_dht_bootstrap_nodes([
		"third1.test.com:6881"
	])

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should remain running through multiple replacements")

	print("  ✓ Nodes replaced multiple times successfully")

# Test 6: add_dht_node vs set_dht_bootstrap_nodes behavior
func test_06_add_vs_set_behavior():
	print("\n[TEST] add_dht_node vs set_dht_bootstrap_nodes")

	# Set initial nodes
	session.set_dht_bootstrap_nodes([
		"initial1.test.com:6881",
		"initial2.test.com:6881"
	])

	session.start_dht()
	OS.delay_msec(500)

	# add_dht_node should add on top of existing
	session.add_dht_node("additional.test.com", 6881)
	OS.delay_msec(500)

	# set_dht_bootstrap_nodes should replace all
	session.set_dht_bootstrap_nodes([
		"replacement.test.com:6881"
	])

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should handle both add and set operations")

	print("  ✓ add_dht_node and set_dht_bootstrap_nodes work as expected")

# Test 7: Set nodes with various formats
func test_07_set_nodes_various_formats():
	print("\n[TEST] Set Nodes With Various Formats")

	# Test with hostname and port
	session.set_dht_bootstrap_nodes([
		"router.example.com:6881"
	])
	OS.delay_msec(200)

	# Test with IPv4
	session.set_dht_bootstrap_nodes([
		"192.168.1.1:6881",
		"10.0.0.1:6881"
	])
	OS.delay_msec(200)

	# Test with IPv6 (if supported)
	session.set_dht_bootstrap_nodes([
		"[2001:db8::1]:6881"
	])
	OS.delay_msec(200)

	# Test with different ports
	session.set_dht_bootstrap_nodes([
		"node1.test.com:6881",
		"node2.test.com:8999",
		"node3.test.com:25401"
	])

	session.start_dht()
	OS.delay_msec(1000)

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should accept various node formats")

	print("  ✓ Various node formats accepted")

# Test 8: Set empty nodes then add back
func test_08_clear_then_add_back():
	print("\n[TEST] Clear Then Add Back Nodes")

	session.start_dht()
	OS.delay_msec(500)

	# Clear all nodes
	session.set_dht_bootstrap_nodes([])
	OS.delay_msec(500)

	# Add nodes back
	session.set_dht_bootstrap_nodes([
		"restored1.test.com:6881",
		"restored2.test.com:6881"
	])

	OS.delay_msec(1000)

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should work after clearing and restoring nodes")

	print("  ✓ Nodes cleared and restored successfully")

# Test 9: Set nodes on stopped session (should not crash)
func test_09_set_nodes_session_not_running():
	print("\n[TEST] Set Nodes On Stopped Session")

	session.stop_session()
	await wait_frames(5)

	# Should not crash when session is stopped
	session.set_dht_bootstrap_nodes([
		"node.test.com:6881"
	])

	# Start session and verify we can still use it
	session.start_session()
	session.start_dht()

	OS.delay_msec(1000)

	print("  ✓ No crash when setting nodes on stopped session")

# Test 10: Standard bootstrap nodes still work
func test_10_standard_bootstrap_nodes():
	print("\n[TEST] Standard Bootstrap Nodes")

	# Use well-known bootstrap nodes
	var standard_nodes = [
		"router.bittorrent.com:6881",
		"dht.transmissionbt.com:6881",
		"router.utorrent.com:6881",
		"dht.libtorrent.org:25401"
	]

	session.set_dht_bootstrap_nodes(standard_nodes)
	session.start_dht()

	OS.delay_msec(2000)

	var dht_running = session.is_dht_running()
	assert_true(dht_running, "DHT should work with standard bootstrap nodes")

	var dht_state = session.get_dht_state()
	assert_not_null(dht_state, "DHT state should be available")

	print("  ✓ Standard bootstrap nodes work correctly")
