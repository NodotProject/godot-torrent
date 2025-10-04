extends Control

# Demo scene that downloads a real torrent file
# This demonstrates the full API usage for end-to-end torrent downloading

# UI Elements
@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var progress_bar: ProgressBar = $VBoxContainer/ProgressBar
@onready var progress_label: Label = $VBoxContainer/ProgressBar/ProgressLabel
@onready var download_button: Button = $VBoxContainer/HBoxContainer2/DownloadButton
@onready var pause_button: Button = $VBoxContainer/HBoxContainer2/PauseButton
@onready var cancel_button: Button = $VBoxContainer/HBoxContainer2/CancelButton
@onready var speed_label: Label = $VBoxContainer/HBoxContainer/SpeedLabel
@onready var peers_label: Label = $VBoxContainer/HBoxContainer/PeersLabel
@onready var log_text: TextEdit = $VBoxContainer/LogText

# Torrent session and handle
var session: TorrentSession
var torrent_handle: TorrentHandle

# Download settings
var download_dir: String
var is_downloading: bool = false
var is_paused: bool = false

# Small test torrent - The WIRED CD (Creative Commons, ~73MB)
var test_magnet: String = "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fwired-cd.torrent"

func _ready():
	print("DEBUG GD: _ready() called")
	log_message("=== Godot-Torrent Download Test ===")
	log_message("This demo downloads a small legal torrent to your user directory")

	print("DEBUG GD: About to get user data dir")
	# Set download directory to user://downloads/
	download_dir = OS.get_user_data_dir() + "/downloads"
	log_message("Download directory: " + download_dir)

	print("DEBUG GD: About to create directory")
	# Create download directory if it doesn't exist
	var dir = DirAccess.open(OS.get_user_data_dir())
	if not dir.dir_exists("downloads"):
		dir.make_dir("downloads")
		log_message("Created downloads directory")

	print("DEBUG GD: About to initialize UI")
	# Initialize UI
	progress_bar.value = 0
	pause_button.disabled = true
	cancel_button.disabled = true

	print("DEBUG GD: About to connect signals")
	# Connect button signals
	download_button.pressed.connect(_on_download_pressed)
	pause_button.pressed.connect(_on_pause_pressed)
	cancel_button.pressed.connect(_on_cancel_pressed)

	print("DEBUG GD: About to create TorrentSession")
	# Initialize session
	session = TorrentSession.new()
	print("DEBUG GD: TorrentSession created")
	log_message("TorrentSession created")
	print("DEBUG GD: _ready() complete")

func _process(delta):
	if is_downloading and torrent_handle and torrent_handle.is_valid():
		update_progress()

func _on_download_pressed():
	if is_downloading:
		return

	log_message("\n=== Starting Download ===")

	# Start session
	if not session.start_session():
		log_message("ERROR: Failed to start session")
		return

	log_message("Session started")

	# Enable DHT for magnet link support
	session.start_dht()
	log_message("DHT enabled")

	# Add torrent
	log_message("Adding magnet URI...")
	torrent_handle = session.add_magnet_uri(test_magnet, download_dir)

	if not torrent_handle or not torrent_handle.is_valid():
		log_message("ERROR: Failed to add torrent")
		return

	log_message("Torrent added successfully")
	log_message("Waiting for metadata...")

	is_downloading = true
	download_button.disabled = true
	pause_button.disabled = false
	cancel_button.disabled = false

	status_label.text = "Status: Downloading metadata..."

func _on_pause_pressed():
	if not torrent_handle or not torrent_handle.is_valid():
		return

	if is_paused:
		torrent_handle.resume()
		pause_button.text = "Pause"
		log_message("Download resumed")
		is_paused = false
	else:
		torrent_handle.pause()
		pause_button.text = "Resume"
		log_message("Download paused")
		is_paused = true

func _on_cancel_pressed():
	if not torrent_handle or not torrent_handle.is_valid():
		return

	log_message("\n=== Canceling Download ===")

	# Remove torrent (delete files)
	session.remove_torrent(torrent_handle, true)
	log_message("Torrent removed")

	cleanup()

func update_progress():
	# Request status updates via alerts (non-blocking)
	session.post_torrent_updates()

	# Get alerts (also non-blocking)
	var alerts = session.get_alerts()

	for alert in alerts:
		# Check if this alert contains torrent status
		if alert.has("torrent_status"):
			var statuses = alert["torrent_status"]
			for status_dict in statuses:
				# Match our handle by info hash
				if status_dict["info_hash"] == torrent_handle.get_info_hash():
					# Get progress info
					var progress = status_dict.get("progress", 0.0)
					var download_rate = status_dict.get("download_rate", 0)
					var upload_rate = status_dict.get("upload_rate", 0)
					var num_peers = status_dict.get("num_peers", 0)
					var state = status_dict.get("state", 0)
					var is_finished = status_dict.get("is_finished", false)
					var is_seeding = status_dict.get("is_seeding", false)

					# Update UI
					progress_bar.value = progress * 100.0
					progress_label.text = "%.1f%%" % (progress * 100.0)

					# Format speeds
					var dl_speed = format_bytes(download_rate) + "/s"
					var ul_speed = format_bytes(upload_rate) + "/s"

					speed_label.text = "↓ %s  ↑ %s" % [dl_speed, ul_speed]
					peers_label.text = "Peers: %d" % num_peers

					# Update status
					if is_finished:
						status_label.text = "Status: Complete! ✓"
						log_message("\n=== Download Complete! ===")

						# Get torrent name
						var name = torrent_handle.get_name()
						log_message("Torrent: " + name)
						log_message("Location: " + download_dir)

						# List downloaded files
						log_message("\nDownloaded files:")
						list_files(download_dir)

						cleanup()
					elif is_seeding:
						status_label.text = "Status: Seeding"
					else:
						# Map state int to string
						var state_str = get_state_string(state)
						status_label.text = "Status: %s (%.1f%%)" % [state_str, progress * 100.0]

func get_state_string(state: int) -> String:
	match state:
		0: return "Queued for checking"
		1: return "Checking files"
		2: return "Downloading metadata"
		3: return "Downloading"
		4: return "Finished"
		5: return "Seeding"
		6: return "Allocating"
		7: return "Checking resume data"
		_: return "Unknown"

func cleanup():
	is_downloading = false
	download_button.disabled = false
	pause_button.disabled = true
	cancel_button.disabled = true

	if session and session.is_running():
		session.stop_session()
		log_message("Session stopped")

func list_files(path: String, indent: String = "  "):
	var dir = DirAccess.open(path)
	if not dir:
		log_message(indent + "(Cannot access directory)")
		return

	var found_files = false

	dir.list_dir_begin()
	var file_name = dir.get_next()

	while file_name != "":
		if not file_name.begins_with("."):  # Skip hidden files
			var file_path = path + "/" + file_name
			if dir.current_is_dir():
				# Recursively list subdirectories
				log_message(indent + "[" + file_name + "/]")
				list_files(file_path, indent + "  ")
				found_files = true
			else:
				var file = FileAccess.open(file_path, FileAccess.READ)
				if file:
					var size = file.get_length()
					log_message(indent + "- %s (%s)" % [file_name, format_bytes(size)])
					file.close()
					found_files = true
		file_name = dir.get_next()

	dir.list_dir_end()

	if not found_files and indent == "  ":
		log_message(indent + "(No files found)")

func format_bytes(bytes: int) -> String:
	if bytes < 1024:
		return str(bytes) + " B"
	elif bytes < 1024 * 1024:
		return "%.1f KB" % (bytes / 1024.0)
	elif bytes < 1024 * 1024 * 1024:
		return "%.1f MB" % (bytes / (1024.0 * 1024.0))
	else:
		return "%.1f GB" % (bytes / (1024.0 * 1024.0 * 1024.0))

func log_message(message: String):
	print(message)
	if log_text and is_node_ready():
		log_text.text += message + "\n"
		# Auto-scroll to bottom
		await get_tree().process_frame  # Wait one frame for text to update
		log_text.scroll_vertical = log_text.get_line_count()

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		# Clean up on exit
		if torrent_handle and torrent_handle.is_valid():
			session.remove_torrent(torrent_handle, false)
		if session and session.is_running():
			session.stop_session()
		get_tree().quit()
