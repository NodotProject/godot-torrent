extends Control

# P2P File Sharing Example
# Demonstrates creating torrents from local files and sharing them
# Includes support for mutable torrents (BEP 46)

var session: TorrentSession
var shared_files: Dictionary = {}  # torrent_name -> handle
var subscribed_torrents: Dictionary = {}  # public_key_hex -> handle
var mutable_keypair: TorrentKeyPair = null  # Current keypair for mutable torrents
var is_mutable_mode: bool = false  # Toggle between regular and mutable torrents

@onready var file_selector: FileDialog = $FileDialog
@onready var share_button: Button = $VBoxContainer/ShareSection/ShareButton
@onready var file_path_input: LineEdit = $VBoxContainer/ShareSection/HBoxContainer/FilePathInput
@onready var browse_button: Button = $VBoxContainer/ShareSection/HBoxContainer/BrowseButton
@onready var tracker_input: LineEdit = $VBoxContainer/ShareSection/TrackerInput
@onready var mutable_checkbox: CheckBox = $VBoxContainer/ShareSection/MutableCheckbox
@onready var keypair_section: VBoxContainer = $VBoxContainer/ShareSection/KeyPairSection
@onready var public_key_display: LineEdit = $VBoxContainer/ShareSection/KeyPairSection/PublicKeyDisplay
@onready var private_key_display: TextEdit = $VBoxContainer/ShareSection/KeyPairSection/PrivateKeyDisplay
@onready var generate_keys_button: Button = $VBoxContainer/ShareSection/KeyPairSection/GenerateKeysButton
@onready var load_keys_button: Button = $VBoxContainer/ShareSection/KeyPairSection/LoadKeysButton
@onready var update_button: Button = $VBoxContainer/ShareSection/UpdateButton
@onready var shared_list: ItemList = $VBoxContainer/SharedList
@onready var magnet_output: TextEdit = $VBoxContainer/MagnetSection/MagnetOutput
@onready var copy_button: Button = $VBoxContainer/MagnetSection/CopyButton
@onready var status_label: Label = $VBoxContainer/StatusLabel

func _ready():
	# Initialize session
	session = TorrentSession.new()
	session.start_session()

	# Enable DHT for better peer discovery
	var settings = {
		"enable_dht": true,
		"enable_lsd": true,  # Local Service Discovery
	}
	session.start_session_with_settings(settings)

	# Connect signals
	share_button.pressed.connect(_on_share_button_pressed)
	browse_button.pressed.connect(_on_browse_button_pressed)
	copy_button.pressed.connect(_on_copy_button_pressed)
	shared_list.item_selected.connect(_on_shared_item_selected)
	mutable_checkbox.toggled.connect(_on_mutable_toggled)
	generate_keys_button.pressed.connect(_on_generate_keys_pressed)
	load_keys_button.pressed.connect(_on_load_keys_pressed)
	update_button.pressed.connect(_on_update_button_pressed)

	# Setup file dialog
	file_selector.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_selector.file_selected.connect(_on_file_selected)

	# Set default tracker
	tracker_input.text = "udp://tracker.opentrackr.org:1337/announce"

	# Hide mutable torrent UI initially
	keypair_section.visible = false
	update_button.visible = false

	# Start update timer
	var timer = Timer.new()
	timer.wait_time = 2.0
	timer.timeout.connect(_update_shared_files)
	add_child(timer)
	timer.start()

	status_label.text = "Session started with DHT. Ready to share files."

func _on_browse_button_pressed():
	file_selector.popup_centered_ratio(0.7)

func _on_file_selected(path: String):
	file_path_input.text = path

func _on_share_button_pressed():
	var file_path = file_path_input.text.strip_edges()
	var tracker_url = tracker_input.text.strip_edges()

	if file_path.is_empty():
		status_label.text = "Error: Please select a file to share"
		return

	if not FileAccess.file_exists(file_path):
		status_label.text = "Error: File does not exist"
		return

	# Check if mutable mode and keypair is available
	if is_mutable_mode:
		if mutable_keypair == null:
			status_label.text = "Error: Please generate or load a keypair first"
			return
		_share_mutable_torrent(file_path)
	else:
		_share_regular_torrent(file_path, tracker_url)

func _share_regular_torrent(file_path: String, tracker_url: String):
	# NOTE: TorrentCreator class would be used here in full implementation
	# For now, we demonstrate sharing existing .torrent files

	status_label.text = "Creating torrent (simulated)..."

	# In a real implementation, this would:
	# 1. Create .torrent file using TorrentCreator
	# 2. Add the torrent in seed mode
	# 3. Generate magnet URI

	# Simulated torrent creation
	var file_name = file_path.get_file()
	var torrent_name = file_name + ".torrent"

	# For demonstration, we'll add the file's directory as the torrent
	var directory = file_path.get_base_dir()

	# Create a fake magnet for demonstration
	# In real implementation, use TorrentCreator to generate proper torrent
	var info_hash = _generate_info_hash(file_path)
	var magnet_uri = "magnet:?xt=urn:btih:%s&dn=%s&tr=%s" % [
		info_hash,
		file_name.uri_encode(),
		tracker_url.uri_encode()
	]

	# Add to shared list
	var list_idx = shared_list.add_item("%s [Sharing]" % file_name)
	shared_files[file_name] = {
		"path": file_path,
		"magnet": magnet_uri,
		"list_item": list_idx,
		"handle": null,
		"is_mutable": false
	}

	status_label.text = "Sharing: %s" % file_name

func _share_mutable_torrent(file_path: String):
	status_label.text = "Creating mutable torrent..."

	# Create torrent from file using session
	var torrent_data = session.create_torrent_from_path(file_path)
	if torrent_data.size() == 0:
		status_label.text = "Error: Failed to create torrent from file"
		return

	# Get the directory to save downloads
	var save_path = file_path.get_base_dir() + "/downloads"

	# Add as mutable torrent using the keypair
	var handle = session.add_mutable_torrent(mutable_keypair, save_path, torrent_data)
	if not handle or not handle.is_valid():
		status_label.text = "Error: Failed to add mutable torrent"
		return

	var file_name = file_path.get_file()
	var public_key_hex = mutable_keypair.get_public_key_hex()

	# Add to shared list
	var list_idx = shared_list.add_item("%s [Mutable v1]" % file_name)
	shared_files[file_name] = {
		"path": file_path,
		"public_key": public_key_hex,
		"magnet": "magnet:?xt=urn:btmh:1220%s&dn=%s" % [public_key_hex, file_name.uri_encode()],
		"list_item": list_idx,
		"handle": handle,
		"is_mutable": true,
		"keypair": mutable_keypair,
		"sequence": 1
	}

	# Show update button for this mutable torrent
	update_button.visible = true

	status_label.text = "Mutable torrent published! Share the public key: %s..." % public_key_hex.substr(0, 16)

func _generate_info_hash(file_path: String) -> String:
	# Generate a deterministic hash from file path
	# In real implementation, this would be calculated from torrent metadata
	var hash = file_path.hash()
	return "%040x" % hash  # Format as 40-character hex string

func _on_shared_item_selected(index: int):
	# Find the shared file for this list item
	for file_name in shared_files.keys():
		var file_data = shared_files[file_name]
		if file_data["list_item"] == index:
			magnet_output.text = file_data["magnet"]
			copy_button.disabled = false
			break

func _on_copy_button_pressed():
	var magnet = magnet_output.text
	DisplayServer.clipboard_set(magnet)
	status_label.text = "Magnet URI copied to clipboard!"

func _on_mutable_toggled(enabled: bool):
	is_mutable_mode = enabled
	keypair_section.visible = enabled
	tracker_input.editable = !enabled  # Mutable torrents use DHT, not trackers

	if enabled:
		status_label.text = "Mutable mode enabled. Generate or load a keypair to continue."
	else:
		status_label.text = "Regular torrent mode."
		update_button.visible = false

func _on_generate_keys_pressed():
	# Generate a real Ed25519 keypair using TorrentKeyPair
	mutable_keypair = TorrentKeyPair.generate()

	# Display the keys in hex format
	public_key_display.text = mutable_keypair.get_public_key_hex()
	private_key_display.text = mutable_keypair.get_private_key_hex()

	status_label.text = "Keypair generated! SAVE THE PRIVATE KEY - you'll need it to update the torrent."

func _on_load_keys_pressed():
	# In real implementation, this would load a keypair from file
	status_label.text = "Load keypair from file (not yet implemented)"

func _on_update_button_pressed():
	# Find the currently selected mutable torrent
	var selected = shared_list.get_selected_items()
	if selected.is_empty():
		status_label.text = "Error: Please select a mutable torrent to update"
		return

	var selected_idx = selected[0]
	var selected_file = null
	for file_name in shared_files.keys():
		if shared_files[file_name]["list_item"] == selected_idx:
			selected_file = shared_files[file_name]
			break

	if selected_file == null or !selected_file.get("is_mutable", false):
		status_label.text = "Error: Selected torrent is not mutable"
		return

	# Open file dialog to select new file
	status_label.text = "Select updated file..."
	file_selector.file_selected.connect(_on_update_file_selected.bind(selected_file), CONNECT_ONE_SHOT)
	file_selector.popup_centered_ratio(0.7)

func _on_update_file_selected(path: String, torrent_data: Dictionary):
	status_label.text = "Publishing update to mutable torrent..."

	# Create new torrent from updated file
	var new_torrent_data = session.create_torrent_from_path(path)
	if new_torrent_data.size() == 0:
		status_label.text = "Error: Failed to create torrent from updated file"
		return

	# Get the handle and publish the update
	var handle = torrent_data["handle"]
	if not handle or not handle.is_valid():
		status_label.text = "Error: Invalid torrent handle"
		return

	# Publish the update using the handle
	var success = handle.publish_update(new_torrent_data)
	if not success:
		status_label.text = "Error: Failed to publish update"
		return

	# Update the sequence number
	var new_sequence = handle.get_sequence_number()
	torrent_data["sequence"] = new_sequence
	torrent_data["path"] = path

	# Update the list item text
	var file_name = path.get_file()
	var list_idx = torrent_data["list_item"]
	shared_list.set_item_text(list_idx, "%s [Mutable v%d]" % [file_name, new_sequence])

	status_label.text = "Update published! Sequence number: %d" % new_sequence

func subscribe_to_mutable_torrent(public_key_hex: String, auto_update: bool = true):
	"""Subscribe to a mutable torrent using its public key."""
	status_label.text = "Subscribing to mutable torrent..."

	# Decode the hex public key
	var public_key = public_key_hex.hex_decode()
	if public_key.size() != 32:
		status_label.text = "Error: Invalid public key (must be 64 hex characters)"
		return

	# Create downloads directory
	var save_path = "user://mutable_downloads"
	DirAccess.make_dir_recursive_absolute(save_path)

	# Subscribe to the mutable torrent
	var handle = session.subscribe_mutable_torrent(public_key, save_path)
	if not handle or not handle.is_valid():
		status_label.text = "Error: Failed to subscribe to mutable torrent"
		return

	# Enable auto-update if requested
	handle.set_auto_update(auto_update)

	# Add to subscribed list
	subscribed_torrents[public_key_hex] = {
		"handle": handle,
		"public_key": public_key_hex,
		"auto_update": auto_update,
		"sequence": 0
	}

	status_label.text = "Subscribed to mutable torrent! Waiting for DHT response..."

func _process_alerts():
	"""Process alerts from the torrent session, including mutable torrent updates."""
	var alerts = session.get_alerts()
	for alert in alerts:
		var alert_type = alert.get("type", "")

		# Handle mutable torrent update alerts
		if alert_type == "mutable_torrent_update_alert":
			var public_key_hex = alert.get("public_key", "")
			var old_seq = alert.get("old_sequence", 0)
			var new_seq = alert.get("new_sequence", 0)
			var new_info_hash = alert.get("new_info_hash", "")

			print("Mutable torrent update detected!")
			print("  Public key: ", public_key_hex.substr(0, 16), "...")
			print("  Old version: ", old_seq)
			print("  New version: ", new_seq)

			# Update sequence number in tracked torrents
			if subscribed_torrents.has(public_key_hex):
				subscribed_torrents[public_key_hex]["sequence"] = new_seq
				status_label.text = "Update available! v%d -> v%d for %s..." % [old_seq, new_seq, public_key_hex.substr(0, 16)]

		# Handle DHT mutable item alerts (when we receive DHT data)
		elif alert_type == "dht_mutable_item_alert":
			var public_key_hex = alert.get("public_key", "")
			var sequence = alert.get("sequence", 0)
			print("DHT mutable item received: ", public_key_hex.substr(0, 16), "... seq=", sequence)

			# Update sequence number
			if subscribed_torrents.has(public_key_hex):
				subscribed_torrents[public_key_hex]["sequence"] = sequence
				status_label.text = "Received mutable torrent data (v%d)" % sequence

		# Handle DHT put confirmation alerts
		elif alert_type == "dht_put_alert":
			var public_key_hex = alert.get("public_key", "")
			var sequence = alert.get("sequence", 0)
			print("DHT put confirmed: ", public_key_hex.substr(0, 16), "... seq=", sequence)

func _update_shared_files():
	# Process alerts first
	_process_alerts()

	# Update shared file statistics
	for file_name in shared_files.keys():
		var file_data = shared_files[file_name]
		var list_idx = file_data["list_item"]
		var handle = file_data["handle"]

		if handle and handle.is_valid():
			var status = handle.get_status()
			if status:
				var upload_rate = status.get_upload_rate() / 1024.0 / 1024.0
				var num_peers = status.get_num_peers()
				var total_uploaded = status.get_total_upload() / 1024.0 / 1024.0

				var mode = "[Mutable]" if file_data.get("is_mutable", false) else "[Sharing]"
				var text = "%s %s - U: %.2f MB/s - %d peers - %.2f MB uploaded" % [
					file_name, mode, upload_rate, num_peers, total_uploaded
				]

				# Add sequence number for mutable torrents
				if file_data.get("is_mutable", false) and file_data.has("sequence"):
					text += " (v%d)" % file_data["sequence"]

				shared_list.set_item_text(list_idx, text)

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if session:
			session.stop_session()
