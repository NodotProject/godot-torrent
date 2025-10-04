extends Node

# Seeding Server Example
# Demonstrates running a headless seeding server for content distribution

var session: TorrentSession
var seeding_torrents: Dictionary = {}  # info_hash -> handle
var config_file_path: String = "user://seeding_server_config.json"

# Server statistics
var total_uploaded: int = 0
var server_start_time: int = 0

func _ready():
	print("=== Godot-Torrent Seeding Server ===")
	print("Initializing...")

	# Initialize session with server-optimized settings
	session = TorrentSession.new()

	var settings = {
		"user_agent": "Godot-Torrent-Seeding-Server/1.0",
		"enable_dht": true,
		"enable_lsd": true,
		"enable_upnp": true,
		"enable_natpmp": true,
		"upload_rate_limit": 50 * 1024 * 1024,  # 50 MB/s upload
		"connections_limit": 500,  # Support many peers
		"unchoke_slots_limit": 50,  # Serve many peers simultaneously
	}

	session.start_session_with_settings(settings)
	session.set_listen_port_range(6881, 6889)

	print("Session started with DHT, UPnP, and NAT-PMP")
	print("Listening on ports 6881-6889")

	# Load and start seeding torrents
	_load_config()

	# Setup update timer
	var timer = Timer.new()
	timer.wait_time = 10.0  # Update every 10 seconds
	timer.timeout.connect(_update_statistics)
	add_child(timer)
	timer.start()

	server_start_time = Time.get_unix_time_from_system()

	print("\nServer running. Press Ctrl+C to stop.\n")

func _load_config():
	# Load seeding configuration
	if not FileAccess.file_exists(config_file_path):
		print("No config file found. Creating default configuration...")
		_create_default_config()
		return

	var file = FileAccess.open(config_file_path, FileAccess.READ)
	if not file:
		print("Error: Could not open config file")
		return

	var json_string = file.get_as_text()
	file.close()

	var json = JSON.new()
	var parse_result = json.parse(json_string)

	if parse_result != OK:
		print("Error: Invalid JSON in config file")
		return

	var config = json.get_data()

	# Load torrents from config
	if config.has("torrents"):
		for torrent_config in config["torrents"]:
			_load_torrent(torrent_config)

	print("Loaded %d torrents from config" % seeding_torrents.size())

func _create_default_config():
	var default_config = {
		"torrents": [
			{
				"type": "magnet",
				"uri": "magnet:?xt=urn:btih:example1...",
				"path": "/path/to/content1"
			},
			{
				"type": "file",
				"path": "/path/to/torrent_file.torrent",
				"content_path": "/path/to/content2"
			}
		],
		"settings": {
			"upload_limit_mbps": 50,
			"max_connections": 500
		}
	}

	var json_string = JSON.stringify(default_config, "\t")
	var file = FileAccess.open(config_file_path, FileAccess.WRITE)
	if file:
		file.store_string(json_string)
		file.close()
		print("Created default config at: " + config_file_path)
		print("Please edit this file and restart the server")
	else:
		print("Error: Could not create config file")

func _load_torrent(config: Dictionary):
	var handle = null

	if config["type"] == "magnet":
		handle = session.add_magnet_uri(config["uri"], config["path"])
	elif config["type"] == "file":
		if FileAccess.file_exists(config["path"]):
			var torrent_data = FileAccess.get_file_as_bytes(config["path"])
			handle = session.add_torrent_file(torrent_data, config["content_path"])
		else:
			print("Warning: Torrent file not found: " + config["path"])
			return

	if handle and handle.is_valid():
		var info_hash = handle.get_info_hash()
		seeding_torrents[info_hash] = handle

		# Force to seeding mode if content is complete
		handle.resume()

		var info = handle.get_torrent_info()
		var name = info.get_name() if info and info.is_valid() else "Unknown"
		print("  ✓ Added: " + name)
	else:
		print("  ✗ Failed to add torrent")

func _update_statistics():
	print("\n--- Server Statistics ---")
	print("Uptime: %d seconds" % (Time.get_unix_time_from_system() - server_start_time))
	print("Active torrents: %d" % seeding_torrents.size())

	var total_upload_rate: float = 0.0
	var total_peers: int = 0
	var total_uploaded_session: int = 0

	for info_hash in seeding_torrents.keys():
		var handle = seeding_torrents[info_hash]
		if not handle or not handle.is_valid():
			continue

		var status = handle.get_status()
		if not status:
			continue

		var info = handle.get_torrent_info()
		var name = info.get_name() if info and info.is_valid() else "Unknown"

		var upload_rate = status.get_upload_rate() / 1024.0 / 1024.0  # MB/s
		var num_peers = status.get_num_peers()
		var uploaded = status.get_total_upload()
		var progress = status.get_progress()

		total_upload_rate += upload_rate
		total_peers += num_peers
		total_uploaded_session += uploaded

		print("\n%s:" % name)
		print("  Progress: %.1f%%" % (progress * 100.0))
		print("  Upload rate: %.2f MB/s" % upload_rate)
		print("  Connected peers: %d" % num_peers)
		print("  Total uploaded: %.2f MB" % (uploaded / 1024.0 / 1024.0))

	print("\n--- Totals ---")
	print("Total upload rate: %.2f MB/s" % total_upload_rate)
	print("Total connected peers: %d" % total_peers)
	print("Total uploaded (session): %.2f GB" % (total_uploaded_session / 1024.0 / 1024.0 / 1024.0))
	print("------------------------\n")

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		print("\nShutting down server...")

		# Gracefully stop all torrents
		for info_hash in seeding_torrents.keys():
			var handle = seeding_torrents[info_hash]
			if handle and handle.is_valid():
				var status = handle.get_status()
				if status:
					var uploaded = status.get_total_upload()
					print("  Final upload for %s: %.2f MB" % [
						info_hash, uploaded / 1024.0 / 1024.0
					])

		if session:
			session.stop_session()

		print("Server stopped.")
		get_tree().quit()
