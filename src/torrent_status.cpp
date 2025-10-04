#include "torrent_status.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/time.hpp>
#include <string>
#include <chrono>

// Include real headers only when not in stub mode
#ifndef TORRENT_STUB_MODE
    #include <libtorrent/torrent_status.hpp>
    #include <libtorrent/hex.hpp>
#endif

using namespace godot;

void TorrentStatus::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_state_string"), &TorrentStatus::get_state_string);
    ClassDB::bind_method(D_METHOD("get_state"), &TorrentStatus::get_state);
    ClassDB::bind_method(D_METHOD("is_paused"), &TorrentStatus::is_paused);
    ClassDB::bind_method(D_METHOD("is_finished"), &TorrentStatus::is_finished);
    ClassDB::bind_method(D_METHOD("is_seeding"), &TorrentStatus::is_seeding);
    
    ClassDB::bind_method(D_METHOD("get_progress"), &TorrentStatus::get_progress);
    ClassDB::bind_method(D_METHOD("get_total_done"), &TorrentStatus::get_total_done);
    ClassDB::bind_method(D_METHOD("get_total_size"), &TorrentStatus::get_total_size);
    ClassDB::bind_method(D_METHOD("get_total_wanted"), &TorrentStatus::get_total_wanted);
    ClassDB::bind_method(D_METHOD("get_total_wanted_done"), &TorrentStatus::get_total_wanted_done);
    
    ClassDB::bind_method(D_METHOD("get_download_rate"), &TorrentStatus::get_download_rate);
    ClassDB::bind_method(D_METHOD("get_upload_rate"), &TorrentStatus::get_upload_rate);
    ClassDB::bind_method(D_METHOD("get_download_payload_rate"), &TorrentStatus::get_download_payload_rate);
    ClassDB::bind_method(D_METHOD("get_upload_payload_rate"), &TorrentStatus::get_upload_payload_rate);
    
    ClassDB::bind_method(D_METHOD("get_num_peers"), &TorrentStatus::get_num_peers);
    ClassDB::bind_method(D_METHOD("get_num_seeds"), &TorrentStatus::get_num_seeds);
    ClassDB::bind_method(D_METHOD("get_num_connections"), &TorrentStatus::get_num_connections);
    ClassDB::bind_method(D_METHOD("get_connections_limit"), &TorrentStatus::get_connections_limit);
    
    ClassDB::bind_method(D_METHOD("get_active_time"), &TorrentStatus::get_active_time);
    ClassDB::bind_method(D_METHOD("get_seeding_time"), &TorrentStatus::get_seeding_time);
    ClassDB::bind_method(D_METHOD("get_time_since_download"), &TorrentStatus::get_time_since_download);
    ClassDB::bind_method(D_METHOD("get_time_since_upload"), &TorrentStatus::get_time_since_upload);
    
    ClassDB::bind_method(D_METHOD("get_num_pieces"), &TorrentStatus::get_num_pieces);
    ClassDB::bind_method(D_METHOD("get_pieces_downloaded"), &TorrentStatus::get_pieces_downloaded);
    
    ClassDB::bind_method(D_METHOD("get_queue_position"), &TorrentStatus::get_queue_position);
    
    ClassDB::bind_method(D_METHOD("get_error"), &TorrentStatus::get_error);
    ClassDB::bind_method(D_METHOD("get_save_path"), &TorrentStatus::get_save_path);
    ClassDB::bind_method(D_METHOD("get_name"), &TorrentStatus::get_name);
    ClassDB::bind_method(D_METHOD("get_distributed_copies"), &TorrentStatus::get_distributed_copies);
    
    // Enhanced status information
    ClassDB::bind_method(D_METHOD("get_all_time_download"), &TorrentStatus::get_all_time_download);
    ClassDB::bind_method(D_METHOD("get_all_time_upload"), &TorrentStatus::get_all_time_upload);
    ClassDB::bind_method(D_METHOD("get_availability"), &TorrentStatus::get_availability);
    ClassDB::bind_method(D_METHOD("get_block_size"), &TorrentStatus::get_block_size);
    ClassDB::bind_method(D_METHOD("get_list_peers"), &TorrentStatus::get_list_peers);
    ClassDB::bind_method(D_METHOD("get_list_seeds"), &TorrentStatus::get_list_seeds);
    ClassDB::bind_method(D_METHOD("get_connect_candidates"), &TorrentStatus::get_connect_candidates);
    ClassDB::bind_method(D_METHOD("get_downloading_piece_index"), &TorrentStatus::get_downloading_piece_index);
    ClassDB::bind_method(D_METHOD("get_downloading_block_index"), &TorrentStatus::get_downloading_block_index);
    ClassDB::bind_method(D_METHOD("get_downloading_progress"), &TorrentStatus::get_downloading_progress);
    ClassDB::bind_method(D_METHOD("get_downloading_total"), &TorrentStatus::get_downloading_total);
    
    // Internal methods
    ClassDB::bind_method(D_METHOD("_set_internal_status", "status"), &TorrentStatus::_set_internal_status);
    ClassDB::bind_method(D_METHOD("get_status_dictionary"), &TorrentStatus::get_status_dictionary);
}

TorrentStatus::TorrentStatus() {
    _status_ptr = nullptr;
    _is_valid = false;
    _last_update_time = 0;
    
    // Initialize cached status with defaults
    _cached_status = {};
    _cached_status.state_string = "unknown";
    _cached_status.state = 0;
    _cached_status.paused = false;
    _cached_status.finished = false;
    _cached_status.seeding = false;
    _cached_status.progress = 0.0f;
    _cached_status.total_size = 1024 * 1024 * 100; // 100MB default for stub
    _cached_status.connections_limit = 50;
    _cached_status.num_pieces = 100;
    _cached_status.distributed_copies = 1.0f;
    _cached_status.block_size = 16384; // 16KB default
    _cached_status.downloading_piece_index = -1;
    _cached_status.downloading_block_index = -1;
    _cached_status.save_path = "/tmp";
    _cached_status.name = "Unknown Torrent";
    
    detect_build_mode();
    
    if (_is_stub_mode) {
        create_stub_status();
        log_status_operation("TorrentStatus initialized in STUB mode");
    } else {
        log_status_operation("TorrentStatus initialized with REAL libtorrent integration");
    }
}

TorrentStatus::~TorrentStatus() {
    try {
        cleanup_status();
    } catch (...) {
        // Ignore exceptions in destructor to prevent crashes
    }
}

void TorrentStatus::detect_build_mode() {
#ifdef TORRENT_STUB_MODE
    _is_stub_mode = true;
#else
    _is_stub_mode = false;
#endif
}

void TorrentStatus::cleanup_status() {
    std::lock_guard<std::mutex> lock(_status_mutex);
    
    try {
        if (_status_ptr && !_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            delete static_cast<libtorrent::torrent_status*>(_status_ptr);
#endif
        }
    } catch (...) {
        // Ignore cleanup exceptions to prevent crashes
    }
    _status_ptr = nullptr;
    _is_valid = false;
}

bool TorrentStatus::validate_status() const {
    return _is_valid && (_status_ptr != nullptr || _is_stub_mode);
}

bool TorrentStatus::is_cache_valid() const {
    auto current_time = Time::get_singleton()->get_ticks_msec();
    return (current_time - _last_update_time) < CACHE_VALIDITY_MS;
}

void TorrentStatus::update_cached_status() {
    std::lock_guard<std::mutex> lock(_status_mutex);
    
    if (!validate_status()) {
        return;
    }
    
    // Check if cache is still valid to optimize performance
    if (is_cache_valid()) {
        return;
    }
    
    try {
        if (!_is_stub_mode) {
            map_libtorrent_status();
        } else {
            create_stub_status();
        }
        
        _last_update_time = Time::get_singleton()->get_ticks_msec();
    } catch (const std::exception& e) {
        handle_status_error("update_cached_status", e);
    }
}

void TorrentStatus::map_libtorrent_status() {
#ifndef TORRENT_STUB_MODE
    if (!_status_ptr) return;
    
    try {
        libtorrent::torrent_status* lt_status = static_cast<libtorrent::torrent_status*>(_status_ptr);
        
        // Basic status
        _cached_status.state = static_cast<int>(lt_status->state);
        _cached_status.state_string = map_state_to_string(_cached_status.state);
        _cached_status.paused = lt_status->paused;
        _cached_status.finished = (lt_status->state == libtorrent::torrent_status::finished || 
                                 lt_status->state == libtorrent::torrent_status::seeding);
        _cached_status.seeding = (lt_status->state == libtorrent::torrent_status::seeding);
        
        // Progress information
        _cached_status.progress = lt_status->progress;
        _cached_status.total_done = lt_status->total_done;
        _cached_status.total_size = lt_status->total;
        _cached_status.total_wanted = lt_status->total_wanted;
        _cached_status.total_wanted_done = lt_status->total_wanted_done;
        
        // Rate information
        _cached_status.download_rate = lt_status->download_rate;
        _cached_status.upload_rate = lt_status->upload_rate;
        _cached_status.download_payload_rate = lt_status->download_payload_rate;
        _cached_status.upload_payload_rate = lt_status->upload_payload_rate;
        
        // Peer information
        _cached_status.num_peers = lt_status->num_peers;
        _cached_status.num_seeds = lt_status->num_seeds;
        _cached_status.num_connections = lt_status->connections_limit;
        _cached_status.connections_limit = lt_status->connections_limit;
        
        // Time information
        _cached_status.active_time = lt_status->active_time.count();
        _cached_status.seeding_time = lt_status->seeding_time.count();
        _cached_status.time_since_download = lt_status->time_since_download.count();
        _cached_status.time_since_upload = lt_status->time_since_upload.count();
        
        // Piece information
        _cached_status.num_pieces = lt_status->num_pieces;
        _cached_status.pieces_downloaded = lt_status->num_pieces - lt_status->pieces_left;
        
        // Queue information
        _cached_status.queue_position = lt_status->queue_position;
        
        // Error information
        _cached_status.error = String(lt_status->error.message().c_str());
        
        // Additional information
        _cached_status.save_path = String(lt_status->save_path.c_str());
        _cached_status.name = String(lt_status->name.c_str());
        _cached_status.distributed_copies = lt_status->distributed_copies;
        
        // Enhanced status information
        _cached_status.all_time_download = lt_status->all_time_download;
        _cached_status.all_time_upload = lt_status->all_time_upload;
        _cached_status.availability = lt_status->pieces_left > 0 ? 
            static_cast<float>(lt_status->num_pieces - lt_status->pieces_left) / lt_status->num_pieces : 1.0f;
        _cached_status.block_size = lt_status->block_size;
        _cached_status.list_peers = lt_status->list_peers;
        _cached_status.list_seeds = lt_status->list_seeds;
        _cached_status.connect_candidates = lt_status->connect_candidates;
        _cached_status.downloading_piece_index = static_cast<int>(lt_status->current_tracker.size() > 0 ? 0 : -1); // Simplified
        _cached_status.downloading_block_index = -1; // Not directly available
        _cached_status.downloading_progress = 0; // Not directly available
        _cached_status.downloading_total = 0; // Not directly available
        
        log_status_operation("Real status mapped from libtorrent");
    } catch (const std::exception& e) {
        handle_status_error("map_libtorrent_status", e);
    }
#endif
}

String TorrentStatus::map_state_to_string(int state) const {
#ifndef TORRENT_STUB_MODE
    switch (static_cast<libtorrent::torrent_status::state_t>(state)) {
        case libtorrent::torrent_status::checking_files: return "checking_files";
        case libtorrent::torrent_status::downloading_metadata: return "downloading_metadata";
        case libtorrent::torrent_status::downloading: return "downloading";
        case libtorrent::torrent_status::finished: return "finished";
        case libtorrent::torrent_status::seeding: return "seeding";
        case libtorrent::torrent_status::allocating: return "allocating";
        case libtorrent::torrent_status::checking_resume_data: return "checking_resume_data";
        default: return "unknown";
    }
#else
    switch (state) {
        case 0: return "checking_files";
        case 1: return "downloading_metadata";
        case 2: return "downloading";
        case 3: return "finished";
        case 4: return "seeding";
        case 5: return "allocating";
        case 6: return "checking_resume_data";
        default: return "unknown";
    }
#endif
}

void TorrentStatus::create_stub_status() {
    // Create realistic stub data that changes over time
    auto current_time = Time::get_singleton()->get_ticks_msec();
    float time_factor = (current_time % 60000) / 60000.0f; // 1 minute cycle
    
    _cached_status.state = 2; // downloading
    _cached_status.state_string = "downloading";
    _cached_status.paused = false;
    _cached_status.progress = time_factor * 0.5f; // Progress from 0 to 50%
    _cached_status.finished = _cached_status.progress >= 1.0f;
    _cached_status.seeding = _cached_status.finished;
    
    _cached_status.total_size = 1024LL * 1024 * 100; // 100MB
    _cached_status.total_done = static_cast<int64_t>(_cached_status.total_size * _cached_status.progress);
    _cached_status.total_wanted = _cached_status.total_size;
    _cached_status.total_wanted_done = _cached_status.total_done;
    
    // Simulate varying rates
    _cached_status.download_rate = static_cast<int>(512000 * (0.5f + 0.5f * sin(time_factor * 6.28f))); // 256-768 KB/s
    _cached_status.upload_rate = static_cast<int>(128000 * (0.3f + 0.7f * sin(time_factor * 3.14f))); // 38-166 KB/s
    _cached_status.download_payload_rate = static_cast<int>(_cached_status.download_rate * 0.9f);
    _cached_status.upload_payload_rate = static_cast<int>(_cached_status.upload_rate * 0.9f);
    
    // Peer information
    _cached_status.num_peers = 15 + static_cast<int>(10 * sin(time_factor * 4.71f));
    _cached_status.num_seeds = 5 + static_cast<int>(3 * cos(time_factor * 3.14f));
    _cached_status.num_connections = _cached_status.num_peers + _cached_status.num_seeds;
    
    // Time information (in seconds)
    _cached_status.active_time = static_cast<int>(current_time / 1000);
    _cached_status.seeding_time = _cached_status.finished ? static_cast<int>(current_time / 2000) : 0;
    _cached_status.time_since_download = 5;
    _cached_status.time_since_upload = 10;
    
    // Piece information
    _cached_status.pieces_downloaded = static_cast<int>(_cached_status.num_pieces * _cached_status.progress);
    
    // Additional information
    _cached_status.all_time_download = _cached_status.total_done + 1024 * 1024; // Extra downloaded data
    _cached_status.all_time_upload = static_cast<int64_t>(_cached_status.total_done * 0.3f); // 30% ratio
    _cached_status.availability = 0.95f + 0.05f * sin(time_factor * 6.28f);
    _cached_status.list_peers = _cached_status.num_peers * 2;
    _cached_status.list_seeds = _cached_status.num_seeds * 2;
    _cached_status.connect_candidates = 50;
    _cached_status.downloading_piece_index = _cached_status.pieces_downloaded % _cached_status.num_pieces;
    _cached_status.downloading_block_index = static_cast<int>(time_factor * 16);
    _cached_status.downloading_progress = static_cast<int>(time_factor * _cached_status.block_size);
    _cached_status.downloading_total = _cached_status.block_size;
    
    _cached_status.name = "Stub Test Torrent";
}

// Internal methods for libtorrent integration
void TorrentStatus::_set_internal_status(const Variant& status) {
    std::lock_guard<std::mutex> lock(_status_mutex);
    
    try {
        // Clean up existing status
        cleanup_status();
        
        if (status.get_type() != Variant::NIL) {
            if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
                // In real mode, this would need proper conversion from Variant
                // For now, we'll treat any non-null Variant as valid
                _status_ptr = nullptr; // TODO: Implement proper status conversion
                _is_valid = false; // TODO: Set based on actual status validity
                log_status_operation("Real libtorrent status set (conversion needed)");
#endif
            } else {
                // In stub mode, any non-null Variant makes it valid
                _status_ptr = reinterpret_cast<void*>(1); // Non-null marker for stub mode
                _is_valid = true;
                log_status_operation("Stub status set");
            }
        } else {
            _status_ptr = nullptr;
            _is_valid = false;
            log_status_operation("Status cleared (set to null)");
        }
        
        // Force cache update
        _last_update_time = 0;
        update_cached_status();
    } catch (const std::exception& e) {
        handle_status_error("_set_internal_status", e);
        _status_ptr = nullptr;
        _is_valid = false;
    }
}

Dictionary TorrentStatus::get_status_dictionary() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    
    Dictionary status_dict;
    status_dict["state_string"] = _cached_status.state_string;
    status_dict["state"] = _cached_status.state;
    status_dict["paused"] = _cached_status.paused;
    status_dict["finished"] = _cached_status.finished;
    status_dict["seeding"] = _cached_status.seeding;
    status_dict["progress"] = _cached_status.progress;
    status_dict["total_done"] = _cached_status.total_done;
    status_dict["total_size"] = _cached_status.total_size;
    status_dict["total_wanted"] = _cached_status.total_wanted;
    status_dict["total_wanted_done"] = _cached_status.total_wanted_done;
    status_dict["download_rate"] = _cached_status.download_rate;
    status_dict["upload_rate"] = _cached_status.upload_rate;
    status_dict["download_payload_rate"] = _cached_status.download_payload_rate;
    status_dict["upload_payload_rate"] = _cached_status.upload_payload_rate;
    status_dict["num_peers"] = _cached_status.num_peers;
    status_dict["num_seeds"] = _cached_status.num_seeds;
    status_dict["num_connections"] = _cached_status.num_connections;
    status_dict["connections_limit"] = _cached_status.connections_limit;
    status_dict["active_time"] = _cached_status.active_time;
    status_dict["seeding_time"] = _cached_status.seeding_time;
    status_dict["time_since_download"] = _cached_status.time_since_download;
    status_dict["time_since_upload"] = _cached_status.time_since_upload;
    status_dict["num_pieces"] = _cached_status.num_pieces;
    status_dict["pieces_downloaded"] = _cached_status.pieces_downloaded;
    status_dict["queue_position"] = _cached_status.queue_position;
    status_dict["error"] = _cached_status.error;
    status_dict["save_path"] = _cached_status.save_path;
    status_dict["name"] = _cached_status.name;
    status_dict["distributed_copies"] = _cached_status.distributed_copies;
    status_dict["all_time_download"] = _cached_status.all_time_download;
    status_dict["all_time_upload"] = _cached_status.all_time_upload;
    status_dict["availability"] = _cached_status.availability;
    status_dict["block_size"] = _cached_status.block_size;
    status_dict["list_peers"] = _cached_status.list_peers;
    status_dict["list_seeds"] = _cached_status.list_seeds;
    status_dict["connect_candidates"] = _cached_status.connect_candidates;
    status_dict["downloading_piece_index"] = _cached_status.downloading_piece_index;
    status_dict["downloading_block_index"] = _cached_status.downloading_block_index;
    status_dict["downloading_progress"] = _cached_status.downloading_progress;
    status_dict["downloading_total"] = _cached_status.downloading_total;
    status_dict["mode"] = _is_stub_mode ? "stub" : "real";
    status_dict["cache_age_ms"] = Time::get_singleton()->get_ticks_msec() - _last_update_time;
    
    return status_dict;
}

// Public interface methods - all use cached data for performance
String TorrentStatus::get_state_string() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.state_string;
}

int TorrentStatus::get_state() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.state;
}

bool TorrentStatus::is_paused() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.paused;
}

bool TorrentStatus::is_finished() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.finished;
}

bool TorrentStatus::is_seeding() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.seeding;
}

float TorrentStatus::get_progress() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.progress;
}

int64_t TorrentStatus::get_total_done() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.total_done;
}

int64_t TorrentStatus::get_total_size() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.total_size;
}

int64_t TorrentStatus::get_total_wanted() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.total_wanted;
}

int64_t TorrentStatus::get_total_wanted_done() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.total_wanted_done;
}

int TorrentStatus::get_download_rate() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.download_rate;
}

int TorrentStatus::get_upload_rate() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.upload_rate;
}

int TorrentStatus::get_download_payload_rate() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.download_payload_rate;
}

int TorrentStatus::get_upload_payload_rate() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.upload_payload_rate;
}

int TorrentStatus::get_num_peers() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.num_peers;
}

int TorrentStatus::get_num_seeds() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.num_seeds;
}

int TorrentStatus::get_num_connections() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.num_connections;
}

int TorrentStatus::get_connections_limit() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.connections_limit;
}

int TorrentStatus::get_active_time() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.active_time;
}

int TorrentStatus::get_seeding_time() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.seeding_time;
}

int TorrentStatus::get_time_since_download() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.time_since_download;
}

int TorrentStatus::get_time_since_upload() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.time_since_upload;
}

int TorrentStatus::get_num_pieces() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.num_pieces;
}

int TorrentStatus::get_pieces_downloaded() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.pieces_downloaded;
}

int TorrentStatus::get_queue_position() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.queue_position;
}

String TorrentStatus::get_error() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.error;
}

String TorrentStatus::get_save_path() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.save_path;
}

String TorrentStatus::get_name() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.name;
}

float TorrentStatus::get_distributed_copies() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.distributed_copies;
}

// Enhanced status information
int64_t TorrentStatus::get_all_time_download() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.all_time_download;
}

int64_t TorrentStatus::get_all_time_upload() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.all_time_upload;
}

float TorrentStatus::get_availability() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.availability;
}

int TorrentStatus::get_block_size() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.block_size;
}

int TorrentStatus::get_list_peers() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.list_peers;
}

int TorrentStatus::get_list_seeds() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.list_seeds;
}

int TorrentStatus::get_connect_candidates() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.connect_candidates;
}

int TorrentStatus::get_downloading_piece_index() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.downloading_piece_index;
}

int TorrentStatus::get_downloading_block_index() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.downloading_block_index;
}

int TorrentStatus::get_downloading_progress() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.downloading_progress;
}

int TorrentStatus::get_downloading_total() const {
    const_cast<TorrentStatus*>(this)->update_cached_status();
    return _cached_status.downloading_total;
}

// Error handling
void TorrentStatus::handle_status_error(const std::string& operation, const std::exception& e) const {
    String error_msg = "Status error in " + String(operation.c_str()) + ": " + String(e.what());
    UtilityFunctions::print_rich("[color=red]" + error_msg + "[/color]");
}

void TorrentStatus::log_status_operation(const String& operation, bool success) const {
    String mode_prefix = _is_stub_mode ? "STUB STATUS" : "REAL STATUS";
    
    if (success) {
        UtilityFunctions::print(mode_prefix + ": " + operation);
    } else {
        UtilityFunctions::print_rich("[color=yellow]" + mode_prefix + ": " + operation + "[/color]");
    }
}