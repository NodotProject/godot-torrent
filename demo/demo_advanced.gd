# Enhanced demo implementation that simulates libtorrent functionality
# This version provides realistic behavior without actual torrent operations

extends Node

var torrent_session: TorrentSession
var active_torrents = []
var demo_timer: Timer

func _ready():
	print("=== Advanced Godot-Torrent Demo ===")
	setup_demo()
	run_comprehensive_demo()

func setup_demo():
	# Create demo timer for simulating progress
	demo_timer = Timer.new()
	demo_timer.wait_time = 1.0
	demo_timer.timeout.connect(_on_demo_timer_timeout)
	add_child(demo_timer)
	demo_timer.start()

func run_comprehensive_demo():
	print("\n--- Phase 1: Session Management ---")
	test_session_management()
	
	print("\n--- Phase 2: Configuration ---")
	test_configuration()
	
	print("\n--- Phase 3: Torrent Operations ---")
	test_torrent_operations()
	
	print("\n--- Phase 4: Alert System ---")
	test_alert_system()
	
	print("\n--- Phase 5: Peer Management ---")
	test_peer_management()
	
	print("\n--- Phase 6: Advanced Features ---")
	test_advanced_features()

func test_session_management():
	torrent_session = TorrentSession.new()
	print("✓ Session created: ", torrent_session != null)
	
	# Test default startup
	var started = torrent_session.start_session()
	print("✓ Session started: ", started)
	print("  Running state: ", torrent_session.is_running())
	
	# Test DHT
	print("  DHT initially running: ", torrent_session.is_dht_running())
	torrent_session.start_dht()
	torrent_session.stop_dht()
	
	# Get session statistics
	var stats = torrent_session.get_session_stats()
	print("✓ Session stats retrieved: ", stats.keys().size(), " metrics")

func test_configuration():
	# Test bandwidth limits
	torrent_session.set_download_rate_limit(5 * 1024 * 1024)  # 5 MB/s
	torrent_session.set_upload_rate_limit(1 * 1024 * 1024)    # 1 MB/s
	print("✓ Bandwidth limits configured")
	
	# Test port configuration
	torrent_session.set_listen_port_range(6881, 6889)
	print("✓ Port range configured: 6881-6889")
	
	# Test custom settings
	var custom_settings = {
		"user_agent": "Godot-Torrent-Demo/1.0",
		"enable_dht": true,
		"enable_lsd": true,
		"download_rate_limit": 2 * 1024 * 1024,
		"upload_rate_limit": 512 * 1024
	}
	
	var session2 = TorrentSession.new()
	var custom_started = session2.start_session_with_settings(custom_settings)
	print("✓ Custom session started: ", custom_started)
	session2.stop_session()

func test_torrent_operations():
	# Test adding torrents
	var dummy_torrent_data = create_dummy_torrent_data()
	var handle1 = torrent_session.add_torrent_file(dummy_torrent_data, "/tmp/downloads/test1")
	print("✓ Torrent file added: ", handle1 != null)
	
	if handle1:
		active_torrents.append(handle1)
		print("  Torrent name: ", handle1.get_name())
		print("  Info hash: ", handle1.get_info_hash())
		print("  Valid: ", handle1.is_valid())
	
	# Test magnet URI
	var magnet_uri = "magnet:?xt=urn:btih:1234567890abcdef1234567890abcdef12345678&dn=Test+Torrent"
	var handle2 = torrent_session.add_magnet_uri(magnet_uri, "/tmp/downloads/test2")
	print("✓ Magnet URI added: ", handle2 != null)
	
	if handle2:
		active_torrents.append(handle2)
	
	# Test torrent control
	if handle1:
		print("  Testing pause/resume...")
		handle1.pause()
		print("    Paused: ", handle1.is_paused())
		handle1.resume()
		print("    Resumed: ", handle1.is_paused())

func test_alert_system():
	var alert_manager = AlertManager.new()
	print("✓ Alert manager created")
	print("  Initial alert mask: ", alert_manager.get_alert_mask())
	
	# Configure alert categories
	alert_manager.enable_error_alerts(true)
	alert_manager.enable_status_alerts(true)
	alert_manager.enable_tracker_alerts(true)
	print("✓ Alert categories configured")
	
	# Get alerts from session
	var session_alerts = torrent_session.get_alerts()
	print("✓ Session alerts retrieved: ", session_alerts.size(), " alerts")
	
	# Get categorized alerts
	var error_alerts = alert_manager.get_error_alerts()
	var status_alerts = alert_manager.get_torrent_alerts()
	print("  Error alerts: ", error_alerts.size())
	print("  Status alerts: ", status_alerts.size())

func test_peer_management():
	if active_torrents.size() > 0:
		var handle = active_torrents[0]
		var peer_info_list = handle.get_peer_info()
		print("✓ Peer info retrieved: ", peer_info_list.size(), " peers")
		
		# Test individual peer info
		if peer_info_list.size() > 0:
			var peer = peer_info_list[0]
			print("  Sample peer:")
			print("    IP: ", peer.get_ip())
			print("    Port: ", peer.get_port())
			print("    Client: ", peer.get_client())
			print("    Connection type: ", peer.get_connection_type())
			print("    Is seed: ", peer.is_seed())
			print("    Download rate: ", peer.get_download_rate(), " bytes/s")
			print("    Upload rate: ", peer.get_upload_rate(), " bytes/s")

func test_advanced_features():
	if active_torrents.size() > 0:
		var handle = active_torrents[0]
		
		# Test torrent info
		var info = handle.get_torrent_info()
		if info:
			print("✓ Torrent info retrieved:")
			print("  Name: ", info.get_name())
			print("  Total size: ", format_bytes(info.get_total_size()))
			print("  File count: ", info.get_file_count())
			print("  Piece count: ", info.get_piece_count())
			print("  Piece size: ", format_bytes(info.get_piece_size()))
			print("  Is private: ", info.is_private())
			print("  Trackers: ", info.get_trackers().size())
			
			# Test file operations
			if info.get_file_count() > 0:
				var file_info = info.get_file_at(0)
				print("  First file: ", file_info.get("path", "unknown"))
				print("  File size: ", format_bytes(file_info.get("size", 0)))
		
		# Test status
		var status = handle.get_status()
		if status:
			print("✓ Torrent status:")
			print("  State: ", status.get_state_string())
			print("  Progress: ", "%.1f%%" % (status.get_progress() * 100))
			print("  Download rate: ", format_bytes(status.get_download_rate()), "/s")
			print("  Upload rate: ", format_bytes(status.get_upload_rate()), "/s")
			print("  Peers: ", status.get_num_peers())
			print("  Seeds: ", status.get_num_seeds())
		
		# Test piece and file priorities
		print("✓ Testing priorities:")
		handle.set_piece_priority(0, 7)  # High priority
		handle.set_file_priority(0, 6)   # Medium priority
		print("  Piece 0 priority: ", handle.get_piece_priority(0))
		print("  File 0 priority: ", handle.get_file_priority(0))
		
		# Test advanced operations
		print("✓ Testing advanced operations:")
		handle.force_recheck()
		handle.force_reannounce()
		handle.scrape_tracker()

func create_dummy_torrent_data() -> PackedByteArray:
	# Create a minimal fake torrent data structure
	var torrent_dict = {
		"announce": "http://tracker.example.com:8080/announce",
		"info": {
			"name": "Demo Torrent",
			"piece length": 1048576,  # 1MB pieces
			"pieces": "dummy_pieces_hash_data_here",
			"files": [
				{"length": 104857600, "path": ["demo_file_1.txt"]},
				{"length": 52428800, "path": ["demo_file_2.txt"]}
			]
		}
	}
	
	# Convert to bencode-like format (simplified)
	var data = "d8:announce41:http://tracker.example.com:8080/announce4:info"
	data += "d4:name11:Demo Torrent12:piece lengthi1048576e6:pieces32:dummy_pieces_hash_data_here"
	data += "5:filesl"
	data += "d6:lengthi104857600e4:pathl13:demo_file_1.txtee"
	data += "d6:lengthi52428800e4:pathl13:demo_file_2.txtee"
	data += "eee"
	
	return data.to_utf8_buffer()

func format_bytes(bytes: int) -> String:
	var units = ["B", "KB", "MB", "GB", "TB"]
	var size = float(bytes)
	var unit_index = 0
	
	while size >= 1024.0 and unit_index < units.size() - 1:
		size /= 1024.0
		unit_index += 1
	
	return "%.1f %s" % [size, units[unit_index]]

func _on_demo_timer_timeout():
	# Simulate download progress for active torrents
	for handle in active_torrents:
		if handle:
			var status = handle.get_status()
			if status:
				# Simulate progress updates
				var current_progress = status.get_progress()
				if current_progress < 1.0:
					print("  📊 ", handle.get_name(), " progress: %.1f%%" % (current_progress * 100))

func _exit_tree():
	if torrent_session and torrent_session.is_running():
		print("\n=== Cleaning up session ===")
		
		# Remove all torrents
		for handle in active_torrents:
			if handle:
				torrent_session.remove_torrent(handle, false)
		
		# Stop session
		torrent_session.stop_session()
		print("✓ Session stopped and cleaned up")
	
	if demo_timer:
		demo_timer.queue_free()
