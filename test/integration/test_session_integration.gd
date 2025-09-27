extends GutTest

var session: TorrentSession

func before_each():
    session = TorrentSession.new()

func after_each():
    if session and session.is_running():
        session.stop_session()
    session = null

func test_session_with_custom_settings():
    var settings = {
        "user_agent": "Godot-Torrent-Test/1.0",
        "enable_dht": true,
        "enable_lsd": false,
        "download_rate_limit": 100000,
        "upload_rate_limit": 50000
    }
    
    var result = session.start_session_with_settings(settings)
    assert_true(result, "Session should start with custom settings")
    assert_true(session.is_running(), "Session should be running")

func test_alert_processing():
    session.start_session()
    
    # Get alerts (should be empty initially)
    var alerts = session.get_alerts()
    assert_not_null(alerts, "Alerts should not be null")
    
    # Clear alerts
    session.clear_alerts()
    
    # Get alerts again
    alerts = session.get_alerts()
    assert_not_null(alerts, "Alerts should still not be null after clearing")

func test_port_configuration():
    session.start_session()
    
    # Test setting single port
    session.set_listen_port(8080)
    
    # Test setting port range
    session.set_listen_port_range(8080, 8090)
    
    # Verify session is still running
    assert_true(session.is_running(), "Session should still be running after port configuration")

func test_multiple_sessions():
    var session2 = TorrentSession.new()
    
    # Start both sessions
    var result1 = session.start_session()
    var result2 = session2.start_session()
    
    assert_true(result1, "First session should start")
    assert_true(result2, "Second session should start")
    assert_true(session.is_running(), "First session should be running")
    assert_true(session2.is_running(), "Second session should be running")
    
    # Cleanup
    session2.stop_session()
    session2 = null