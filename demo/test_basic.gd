extends Node

func _ready():
    print("=== Testing Godot-Torrent GDExtension ===")
    
    # Test TorrentSession creation
    var session = TorrentSession.new()
    print("✓ TorrentSession created: ", session != null)
    
    # Test basic session operations
    print("Initial running state: ", session.is_running())
    
    var started = session.start_session()
    print("✓ Session started: ", started)
    print("Running state after start: ", session.is_running())
    
    # Test configuration methods
    session.set_download_rate_limit(1000000)
    session.set_upload_rate_limit(500000)
    session.set_listen_port(8080)
    
    # Test DHT operations
    print("DHT running: ", session.is_dht_running())
    session.start_dht()
    session.stop_dht()
    
    # Test session stats
    var stats = session.get_session_stats()
    print("✓ Session stats: ", stats)
    
    # Test alerts
    var alerts = session.get_alerts()
    print("✓ Alerts: ", alerts.size(), " alerts")
    
    # Test TorrentHandle creation
    var handle = TorrentHandle.new()
    print("✓ TorrentHandle created: ", handle != null)
    print("Handle valid: ", handle.is_valid())
    print("Handle paused: ", handle.is_paused())
    
    # Test handle operations
    handle.pause()
    handle.resume()
    print("Handle name: ", handle.get_name())
    print("Handle hash: ", handle.get_info_hash())
    
    # Test TorrentInfo
    var info = TorrentInfo.new()
    print("✓ TorrentInfo created: ", info != null)
    print("Info name: ", info.get_name())
    print("Info size: ", info.get_total_size())
    print("Info valid: ", info.is_valid())
    
    # Test TorrentStatus
    var status = TorrentStatus.new()
    print("✓ TorrentStatus created: ", status != null)
    print("Status progress: ", status.get_progress())
    print("Status state: ", status.get_state_string())
    
    # Test PeerInfo
    var peer = PeerInfo.new()
    print("✓ PeerInfo created: ", peer != null)
    print("Peer IP: ", peer.get_ip())
    print("Peer port: ", peer.get_port())
    
    # Test AlertManager
    var alert_manager = AlertManager.new()
    print("✓ AlertManager created: ", alert_manager != null)
    print("Alert mask: ", alert_manager.get_alert_mask())
    var manager_alerts = alert_manager.get_alerts()
    print("Manager alerts: ", manager_alerts.size(), " alerts")
    
    # Test adding a dummy torrent
    var dummy_data = PackedByteArray([1, 2, 3, 4, 5])  # Fake torrent data
    var torrent_handle = session.add_torrent_file(dummy_data, "/tmp/test")
    print("✓ Add torrent result: ", torrent_handle != null)
    
    # Test magnet URI
    var magnet_handle = session.add_magnet_uri("magnet:?xt=urn:btih:dummy", "/tmp/test2")
    print("✓ Add magnet result: ", magnet_handle != null)
    
    # Stop session
    session.stop_session()
    print("✓ Session stopped. Running state: ", session.is_running())
    
    print("=== All basic tests completed! ===")
    
    # Exit after testing
    get_tree().quit()