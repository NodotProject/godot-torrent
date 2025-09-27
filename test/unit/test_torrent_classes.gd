extends GutTest

var session: TorrentSession

func before_each():
    session = TorrentSession.new()
    session.start_session()

func after_each():
    if session:
        session.stop_session()
    session = null

func test_invalid_torrent_handle():
    var handle = TorrentHandle.new()
    
    assert_false(handle.is_valid(), "New handle should not be valid")
    assert_false(handle.is_paused(), "Invalid handle should not be paused")
    
    # These should not crash
    handle.pause()
    handle.resume()
    handle.force_recheck()

func test_torrent_handle_creation():
    # For now, just test that we can create a handle
    # In a real scenario, we would need actual torrent data
    var handle = TorrentHandle.new()
    
    assert_not_null(handle, "Handle should be created")
    assert_false(handle.is_valid(), "Handle should not be valid without internal data")

func test_torrent_info_methods():
    var info = TorrentInfo.new()
    
    assert_not_null(info, "TorrentInfo should be created")
    assert_false(info.is_valid(), "TorrentInfo should not be valid without data")
    assert_eq(info.get_name(), "", "Name should be empty for invalid info")
    assert_eq(info.get_total_size(), 0, "Size should be 0 for invalid info")

func test_torrent_status_methods():
    var status = TorrentStatus.new()
    
    assert_not_null(status, "TorrentStatus should be created")
    assert_eq(status.get_progress(), 0.0, "Progress should be 0 for invalid status")
    assert_false(status.is_paused(), "Should not be paused for invalid status")

func test_peer_info_methods():
    var peer = PeerInfo.new()
    
    assert_not_null(peer, "PeerInfo should be created")
    assert_eq(peer.get_ip(), "", "IP should be empty for invalid peer")
    assert_eq(peer.get_port(), 0, "Port should be 0 for invalid peer")

func test_alert_manager():
    var alert_manager = AlertManager.new()
    
    assert_not_null(alert_manager, "AlertManager should be created")
    
    var alerts = alert_manager.get_alerts()
    assert_not_null(alerts, "Alerts should not be null")
    assert_eq(alerts.size(), 0, "Should have no alerts initially")
    
    # Test alert filtering
    alert_manager.enable_error_alerts(true)
    alert_manager.enable_status_alerts(false)
    
    var mask = alert_manager.get_alert_mask()
    assert_ne(mask, 0, "Alert mask should not be zero")