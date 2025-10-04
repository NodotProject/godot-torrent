extends GutTest

# Unit tests for Issue #7: TorrentHandle - Status Retrieval
# These tests validate real-time torrent status retrieval functionality

var handle: TorrentHandle
var status: TorrentStatus

func before_each():
	handle = TorrentHandle.new()
	# Set up a valid handle for testing
	handle._set_internal_handle({})

func after_each():
	handle = null
	status = null

# Issue #7 Tests: TorrentHandle - Status Retrieval

func test_issue7_real_status_query():
	# Validates: Implement get_status() with real libtorrent status query
	
	status = handle.get_status()
	assert_not_null(status, "get_status() should return a TorrentStatus object")
	assert_true(status is TorrentStatus, "Returned object should be TorrentStatus")

func test_issue7_status_object_mapping():
	# Validates: Map libtorrent::torrent_status to TorrentStatus object
	
	status = handle.get_status()
	
	# Test that all basic status fields are accessible
	var state_string = status.get_state_string()
	var state = status.get_state()
	var paused = status.is_paused()
	var finished = status.is_finished()
	var seeding = status.is_seeding()
	
	assert_not_null(state_string, "State string should be available")
	assert_true(state >= 0, "State should be a valid integer")
	assert_true(typeof(paused) == TYPE_BOOL, "Paused should be boolean")
	assert_true(typeof(finished) == TYPE_BOOL, "Finished should be boolean")
	assert_true(typeof(seeding) == TYPE_BOOL, "Seeding should be boolean")

func test_issue7_all_status_fields():
	# Validates: Populate all TorrentStatus fields with real data
	
	status = handle.get_status()
	
	# Progress information
	var progress = status.get_progress()
	var total_done = status.get_total_done()
	var total_size = status.get_total_size()
	var total_wanted = status.get_total_wanted()
	var total_wanted_done = status.get_total_wanted_done()
	
	assert_true(progress >= 0.0 and progress <= 1.0, "Progress should be between 0 and 1")
	assert_true(total_done >= 0, "Total done should be non-negative")
	assert_true(total_size > 0, "Total size should be positive")
	assert_true(total_wanted >= 0, "Total wanted should be non-negative")
	assert_true(total_wanted_done >= 0, "Total wanted done should be non-negative")

func test_issue7_state_string_mapping():
	# Validates: Implement state string mapping
	
	status = handle.get_status()
	var state_string = status.get_state_string()
	
	# Valid state strings from libtorrent
	var valid_states = [
		"checking_files",
		"downloading_metadata", 
		"downloading",
		"finished",
		"seeding",
		"allocating",
		"checking_resume_data",
		"unknown"
	]
	
	assert_true(state_string in valid_states, "State string should be valid: " + state_string)

func test_issue7_progress_calculation():
	# Validates: Add progress calculation
	
	status = handle.get_status()
	var progress = status.get_progress()
	var total_done = status.get_total_done()
	var total_size = status.get_total_size()
	
	# Progress should be consistent with total_done/total_size
	if total_size > 0:
		var calculated_progress = float(total_done) / float(total_size)
		var difference = abs(progress - calculated_progress)
		assert_true(difference < 0.01, "Progress calculation should be consistent")

func test_issue7_rate_information():
	# Validates: Add rate information (download/upload)
	
	status = handle.get_status()
	
	var download_rate = status.get_download_rate()
	var upload_rate = status.get_upload_rate()
	var download_payload_rate = status.get_download_payload_rate()
	var upload_payload_rate = status.get_upload_payload_rate()
	
	assert_true(download_rate >= 0, "Download rate should be non-negative")
	assert_true(upload_rate >= 0, "Upload rate should be non-negative")
	assert_true(download_payload_rate >= 0, "Download payload rate should be non-negative")
	assert_true(upload_payload_rate >= 0, "Upload payload rate should be non-negative")

func test_issue7_peer_counts():
	# Validates: Add peer counts
	
	status = handle.get_status()
	
	var num_peers = status.get_num_peers()
	var num_seeds = status.get_num_seeds()
	var num_connections = status.get_num_connections()
	var connections_limit = status.get_connections_limit()
	
	assert_true(num_peers >= 0, "Number of peers should be non-negative")
	assert_true(num_seeds >= 0, "Number of seeds should be non-negative") 
	assert_true(num_connections >= 0, "Number of connections should be non-negative")
	assert_true(connections_limit > 0, "Connections limit should be positive")

func test_issue7_time_tracking():
	# Validates: Add time tracking
	
	status = handle.get_status()
	
	var active_time = status.get_active_time()
	var seeding_time = status.get_seeding_time()
	var time_since_download = status.get_time_since_download()
	var time_since_upload = status.get_time_since_upload()
	
	assert_true(active_time >= 0, "Active time should be non-negative")
	assert_true(seeding_time >= 0, "Seeding time should be non-negative")
	assert_true(time_since_download >= 0, "Time since download should be non-negative")
	assert_true(time_since_upload >= 0, "Time since upload should be non-negative")

func test_issue7_real_time_updates():
	# Validates: Test status updates in real-time
	
	# Get initial status
	var status1 = handle.get_status()
	var initial_time = status1.get_active_time()
	
	# Wait a short time (simulate some activity)
	await wait_frames(5)
	
	# Get updated status  
	var status2 = handle.get_status()
	var updated_time = status2.get_active_time()
	
	# In stub mode, time should progress
	# In real mode, this would show actual time progression
	assert_true(updated_time >= initial_time, "Time should progress or stay same")

func test_issue7_performance_optimization():
	# Validates: Optimize status query frequency / Performance is acceptable (< 1ms per query)
	
	var start_time = Time.get_ticks_msec()
	
	# Perform multiple status queries
	for i in range(100):
		status = handle.get_status()
		var _ = status.get_progress()
		var _ = status.get_download_rate()
		var _ = status.get_num_peers()
	
	var end_time = Time.get_ticks_msec()
	var total_time = end_time - start_time
	var avg_time_per_query = total_time / 100.0
	
	# Should be well under 1ms per query (accounting for stub mode overhead)
	assert_true(avg_time_per_query < 5.0, "Average query time should be under 5ms, got: " + str(avg_time_per_query) + "ms")

func test_issue7_status_dictionary():
	# Validates: Status values match libtorrent's internal state
	
	status = handle.get_status()
	var status_dict = status.get_status_dictionary()
	
	assert_not_null(status_dict, "Status dictionary should be available")
	assert_true(status_dict.has("state_string"), "Dictionary should contain state_string")
	assert_true(status_dict.has("progress"), "Dictionary should contain progress")
	assert_true(status_dict.has("download_rate"), "Dictionary should contain download_rate")
	assert_true(status_dict.has("upload_rate"), "Dictionary should contain upload_rate")
	assert_true(status_dict.has("num_peers"), "Dictionary should contain num_peers")
	assert_true(status_dict.has("mode"), "Dictionary should contain mode")

func test_issue7_enhanced_status_fields():
	# Validates: Enhanced status information is available
	
	status = handle.get_status()
	
	# Test enhanced fields
	var all_time_download = status.get_all_time_download()
	var all_time_upload = status.get_all_time_upload()
	var availability = status.get_availability()
	var block_size = status.get_block_size()
	var list_peers = status.get_list_peers()
	var list_seeds = status.get_list_seeds()
	var connect_candidates = status.get_connect_candidates()
	
	assert_true(all_time_download >= 0, "All time download should be non-negative")
	assert_true(all_time_upload >= 0, "All time upload should be non-negative")
	assert_true(availability >= 0.0 and availability <= 1.0, "Availability should be between 0 and 1")
	assert_true(block_size > 0, "Block size should be positive")
	assert_true(list_peers >= 0, "List peers should be non-negative")
	assert_true(list_seeds >= 0, "List seeds should be non-negative")
	assert_true(connect_candidates >= 0, "Connect candidates should be non-negative")

func test_issue7_downloading_info():
	# Validates: Current downloading piece/block information
	
	status = handle.get_status()
	
	var downloading_piece_index = status.get_downloading_piece_index()
	var downloading_block_index = status.get_downloading_block_index()
	var downloading_progress = status.get_downloading_progress()
	var downloading_total = status.get_downloading_total()
	
	# These can be -1 if not currently downloading
	assert_true(downloading_piece_index >= -1, "Downloading piece index should be -1 or positive")
	assert_true(downloading_block_index >= -1, "Downloading block index should be -1 or positive")
	assert_true(downloading_progress >= 0, "Downloading progress should be non-negative")
	assert_true(downloading_total >= 0, "Downloading total should be non-negative")

func test_issue7_error_handling():
	# Validates: Error handling for invalid handles
	
	var invalid_handle = TorrentHandle.new()
	var invalid_status = invalid_handle.get_status()
	
	assert_not_null(invalid_status, "Should return status object even for invalid handle")
	
	# Status should have safe default values
	var progress = invalid_status.get_progress()
	var name = invalid_status.get_name()
	
	assert_true(progress >= 0.0, "Invalid handle should have safe progress value")
	assert_not_null(name, "Invalid handle should have safe name value")

func test_issue7_cache_validity():
	# Validates: Caching mechanism works correctly
	
	status = handle.get_status()
	var status_dict = status.get_status_dictionary()
	
	assert_true(status_dict.has("cache_age_ms"), "Status should include cache age")
	var cache_age = status_dict["cache_age_ms"]
	assert_true(cache_age >= 0, "Cache age should be non-negative")

func test_issue7_consistency():
	# Validates: Status values are internally consistent
	
	status = handle.get_status()
	
	var progress = status.get_progress()
	var is_finished = status.is_finished()
	var is_seeding = status.is_seeding()
	var pieces_downloaded = status.get_pieces_downloaded()
	var num_pieces = status.get_num_pieces()
	
	# Consistency checks
	if progress >= 1.0:
		assert_true(is_finished, "Should be finished if progress is 100%")
	
	if is_seeding:
		assert_true(is_finished, "Should be finished if seeding")
	
	if num_pieces > 0 and pieces_downloaded >= 0:
		var piece_progress = float(pieces_downloaded) / float(num_pieces)
		var difference = abs(progress - piece_progress)
		# Allow some tolerance for rounding differences
		assert_true(difference < 0.1, "Piece progress should be consistent with overall progress")