#include "torrent_handle.h"
#include "torrent_info.h"
#include "torrent_status.h"
#include "peer_info.h"
#include "torrent_session.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <string>

// Include real headers only when not in stub mode
#ifndef TORRENT_STUB_MODE
    #include <libtorrent/torrent_handle.hpp>
    #include <libtorrent/torrent_status.hpp>
    #include <libtorrent/torrent_info.hpp>
    #include <libtorrent/peer_info.hpp>
    #include <libtorrent/hex.hpp>
#endif

using namespace godot;

void TorrentHandle::_bind_methods() {
    ClassDB::bind_method(D_METHOD("pause"), &TorrentHandle::pause);
    ClassDB::bind_method(D_METHOD("resume"), &TorrentHandle::resume);
    ClassDB::bind_method(D_METHOD("is_paused"), &TorrentHandle::is_paused);
    ClassDB::bind_method(D_METHOD("is_valid"), &TorrentHandle::is_valid);
    
    ClassDB::bind_method(D_METHOD("get_torrent_info"), &TorrentHandle::get_torrent_info);
    ClassDB::bind_method(D_METHOD("get_status"), &TorrentHandle::get_status);
    ClassDB::bind_method(D_METHOD("get_name"), &TorrentHandle::get_name);
    ClassDB::bind_method(D_METHOD("get_info_hash"), &TorrentHandle::get_info_hash);
    
    ClassDB::bind_method(D_METHOD("set_piece_priority", "piece_index", "priority"), &TorrentHandle::set_piece_priority);
    ClassDB::bind_method(D_METHOD("get_piece_priority", "piece_index"), &TorrentHandle::get_piece_priority);
    ClassDB::bind_method(D_METHOD("set_file_priority", "file_index", "priority"), &TorrentHandle::set_file_priority);
    ClassDB::bind_method(D_METHOD("get_file_priority", "file_index"), &TorrentHandle::get_file_priority);
    ClassDB::bind_method(D_METHOD("rename_file", "file_index", "new_name"), &TorrentHandle::rename_file);
    ClassDB::bind_method(D_METHOD("get_file_progress"), &TorrentHandle::get_file_progress);

    ClassDB::bind_method(D_METHOD("have_piece", "piece_index"), &TorrentHandle::have_piece);
    ClassDB::bind_method(D_METHOD("read_piece", "piece_index"), &TorrentHandle::read_piece);
    ClassDB::bind_method(D_METHOD("get_piece_availability"), &TorrentHandle::get_piece_availability);

    ClassDB::bind_method(D_METHOD("force_recheck"), &TorrentHandle::force_recheck);
    ClassDB::bind_method(D_METHOD("force_reannounce"), &TorrentHandle::force_reannounce);
    ClassDB::bind_method(D_METHOD("force_dht_announce"), &TorrentHandle::force_dht_announce);
    ClassDB::bind_method(D_METHOD("move_storage", "new_path"), &TorrentHandle::move_storage);
    
    ClassDB::bind_method(D_METHOD("get_peer_info"), &TorrentHandle::get_peer_info);
    
    ClassDB::bind_method(D_METHOD("scrape_tracker"), &TorrentHandle::scrape_tracker);
    ClassDB::bind_method(D_METHOD("flush_cache"), &TorrentHandle::flush_cache);
    ClassDB::bind_method(D_METHOD("clear_error"), &TorrentHandle::clear_error);

    ClassDB::bind_method(D_METHOD("add_tracker", "url", "tier"), &TorrentHandle::add_tracker, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("remove_tracker", "url"), &TorrentHandle::remove_tracker);
    ClassDB::bind_method(D_METHOD("get_trackers"), &TorrentHandle::get_trackers);

    ClassDB::bind_method(D_METHOD("add_url_seed", "url"), &TorrentHandle::add_url_seed);
    ClassDB::bind_method(D_METHOD("remove_url_seed", "url"), &TorrentHandle::remove_url_seed);
    ClassDB::bind_method(D_METHOD("add_http_seed", "url"), &TorrentHandle::add_http_seed);
    ClassDB::bind_method(D_METHOD("remove_http_seed", "url"), &TorrentHandle::remove_http_seed);
    ClassDB::bind_method(D_METHOD("get_url_seeds"), &TorrentHandle::get_url_seeds);
    ClassDB::bind_method(D_METHOD("get_http_seeds"), &TorrentHandle::get_http_seeds);

    ClassDB::bind_method(D_METHOD("save_resume_data"), &TorrentHandle::save_resume_data);
    ClassDB::bind_method(D_METHOD("get_resume_data"), &TorrentHandle::get_resume_data);

    // Mutable torrent methods
    ClassDB::bind_method(D_METHOD("is_mutable"), &TorrentHandle::is_mutable);
    ClassDB::bind_method(D_METHOD("get_public_key"), &TorrentHandle::get_public_key);
    ClassDB::bind_method(D_METHOD("get_sequence_number"), &TorrentHandle::get_sequence_number);
    ClassDB::bind_method(D_METHOD("publish_update", "new_torrent_data"), &TorrentHandle::publish_update);
    ClassDB::bind_method(D_METHOD("check_for_updates"), &TorrentHandle::check_for_updates);
    ClassDB::bind_method(D_METHOD("set_auto_update", "enabled"), &TorrentHandle::set_auto_update);
    ClassDB::bind_method(D_METHOD("is_auto_update_enabled"), &TorrentHandle::is_auto_update_enabled);

    // Internal methods for libtorrent integration
    ClassDB::bind_method(D_METHOD("_set_internal_handle", "handle"), &TorrentHandle::_set_internal_handle);
    ClassDB::bind_method(D_METHOD("_get_internal_handle"), &TorrentHandle::_get_internal_handle);
}

TorrentHandle::TorrentHandle() {
    _handle_ptr = nullptr;
    _is_valid = false;
    _stub_paused = false;
    _stub_name = "Default Torrent";
    _stub_info_hash = "0123456789abcdef0123456789abcdef01234567";
    _resume_data_ready = false;
    _is_mutable = false;
    _sequence_number = 0;
    _auto_update_enabled = false;
    _parent_session = nullptr;

    detect_build_mode();

    if (_is_stub_mode) {
        log_handle_operation("TorrentHandle initialized in STUB mode");
    } else {
        log_handle_operation("TorrentHandle initialized with REAL libtorrent integration");
    }
}

TorrentHandle::~TorrentHandle() {
    try {
        std::lock_guard<std::mutex> lock(_handle_mutex);
        cleanup_handle();
    } catch (...) {
        // Ignore exceptions in destructor to prevent crashes
    }
}

void TorrentHandle::detect_build_mode() {
#ifdef TORRENT_STUB_MODE
    _is_stub_mode = true;
#else
    _is_stub_mode = false;
#endif
}

void TorrentHandle::cleanup_handle() {
    // NOTE: Caller must hold _handle_mutex lock

    try {
        if (_handle_ptr && !_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            delete static_cast<libtorrent::torrent_handle*>(_handle_ptr);
#endif
        }
    } catch (...) {
        // Ignore cleanup exceptions to prevent crashes
    }
    _handle_ptr = nullptr;
    _is_valid = false;
}

bool TorrentHandle::validate_handle() const {
    if (!_is_valid || !_handle_ptr) {
        return false;
    }
    
    if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            return handle->is_valid();
        } catch (const std::exception& e) {
            return false;
        }
#endif
    }
    
    return _is_valid; // In stub mode, return cached validity
}

void TorrentHandle::pause() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        report_error("pause", "Invalid handle");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->pause();
            log_handle_operation("Torrent paused");
#endif
        } else {
            _stub_paused = true;
            simulate_handle_operation("pause");
        }
    } catch (const std::exception& e) {
        handle_operation_error("pause", e);
    }
}

void TorrentHandle::resume() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        report_error("resume", "Invalid handle");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->resume();
            log_handle_operation("Torrent resumed");
#endif
        } else {
            _stub_paused = false;
            simulate_handle_operation("resume");
        }
    } catch (const std::exception& e) {
        handle_operation_error("resume", e);
    }
}

bool TorrentHandle::is_paused() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        return false;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            libtorrent::torrent_status status = handle->status();
            return status.paused;
#endif
        } else {
            return _stub_paused;
        }
    } catch (const std::exception& e) {
        handle_operation_error("is_paused", e);
        return false;
    }
    
    return false;
}

bool TorrentHandle::is_valid() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    return validate_handle();
}

Ref<TorrentInfo> TorrentHandle::get_torrent_info() {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    Ref<TorrentInfo> info;
    info.instantiate();

    if (!validate_handle()) {
        return info;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::shared_ptr<const libtorrent::torrent_info> ti = handle->torrent_file();
            if (ti) {
                // Create a non-const shared_ptr copy for TorrentInfo
                info->_set_internal_info(std::const_pointer_cast<libtorrent::torrent_info>(ti));
                log_handle_operation("Retrieved real torrent info");
            }
#endif
        } else {
            simulate_handle_operation("get_torrent_info");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_torrent_info", e);
    }

    return info;
}

Ref<TorrentStatus> TorrentHandle::get_status() {
    UtilityFunctions::print("DEBUG: get_status() called, about to lock");
    std::lock_guard<std::mutex> lock(_handle_mutex);
    UtilityFunctions::print("DEBUG: lock acquired");

    Ref<TorrentStatus> status;
    status.instantiate();
    UtilityFunctions::print("DEBUG: status instantiated");

    if (!validate_handle()) {
        log_handle_operation("Cannot get status: Invalid handle", false);
        return status;
    }
    UtilityFunctions::print("DEBUG: handle validated");

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            UtilityFunctions::print("DEBUG: About to call lt handle->status()");

            // Get real-time status from libtorrent
            // Use empty status_flags_t{} to get basic cached status (non-blocking)
            libtorrent::torrent_status lt_status = handle->status(libtorrent::status_flags_t{});
            UtilityFunctions::print("DEBUG: lt handle->status() returned");

            // Create a copy of the status to pass to TorrentStatus
            libtorrent::torrent_status* status_copy = new libtorrent::torrent_status(lt_status);
            UtilityFunctions::print("DEBUG: status copied");

            // Pass the pointer directly as uint64_t (Variant will accept it as an int)
            status->_set_internal_status((uint64_t)status_copy);
            UtilityFunctions::print("DEBUG: internal status set");

            log_handle_operation("Real torrent status retrieved");
#endif
        } else {
            // Stub mode - set a dummy status
            Dictionary stub_status;
            stub_status["stub"] = true;
            status->_set_internal_status(stub_status);
            simulate_handle_operation("get_status");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_status", e);
    }

    return status;
}

String TorrentHandle::get_name() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        return "Invalid Handle";
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            libtorrent::torrent_status status = handle->status();
            return String(status.name.c_str());
#endif
        } else {
            return _stub_name;
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_name", e);
        return "Error";
    }
    
    return "Unknown";
}

String TorrentHandle::get_info_hash() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        return "";
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            return String(libtorrent::aux::to_hex(handle->info_hash()).c_str());
#endif
        } else {
            return _stub_info_hash;
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_info_hash", e);
        return "";
    }
    
    return "";
}

void TorrentHandle::set_piece_priority(int piece_index, int priority) {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot set piece priority: Invalid handle[/color]");
        return;
    }
    
    if (!validate_piece_index(piece_index) || !validate_priority(priority)) {
        UtilityFunctions::print_rich("[color=yellow]Invalid piece index or priority value[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->piece_priority(libtorrent::piece_index_t(piece_index), static_cast<libtorrent::download_priority_t>(priority));
            log_handle_operation("Set piece " + String::num(piece_index) + " priority to " + String::num(priority));
#endif
        } else {
            simulate_handle_operation("set_piece_priority");
        }
    } catch (const std::exception& e) {
        handle_operation_error("set_piece_priority", e);
    }
}

int TorrentHandle::get_piece_priority(int piece_index) const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        return 0;
    }
    
    if (!validate_piece_index(piece_index)) {
        return 0;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            return static_cast<int>(handle->piece_priority(libtorrent::piece_index_t(piece_index)));
#endif
        } else {
            return 4; // Default priority in stub mode
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_piece_priority", e);
        return 0;
    }
    
    return 0;
}

void TorrentHandle::set_file_priority(int file_index, int priority) {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot set file priority: Invalid handle[/color]");
        return;
    }
    
    if (!validate_file_index(file_index) || !validate_priority(priority)) {
        UtilityFunctions::print_rich("[color=yellow]Invalid file index or priority value[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->file_priority(libtorrent::file_index_t(file_index), static_cast<libtorrent::download_priority_t>(priority));
            log_handle_operation("Set file " + String::num(file_index) + " priority to " + String::num(priority));
#endif
        } else {
            simulate_handle_operation("set_file_priority");
        }
    } catch (const std::exception& e) {
        handle_operation_error("set_file_priority", e);
    }
}

int TorrentHandle::get_file_priority(int file_index) const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        return 0;
    }
    
    if (!validate_file_index(file_index)) {
        return 0;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            return static_cast<int>(handle->file_priority(libtorrent::file_index_t(file_index)));
#endif
        } else {
            return 4; // Default priority in stub mode
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_file_priority", e);
        return 0;
    }
    
    return 0;
}

void TorrentHandle::force_recheck() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot force recheck: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->force_recheck();
            log_handle_operation("Force recheck initiated");
#endif
        } else {
            simulate_handle_operation("force_recheck");
        }
    } catch (const std::exception& e) {
        handle_operation_error("force_recheck", e);
    }
}

void TorrentHandle::force_reannounce() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot force reannounce: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->force_reannounce();
            log_handle_operation("Force reannounce initiated");
#endif
        } else {
            simulate_handle_operation("force_reannounce");
        }
    } catch (const std::exception& e) {
        handle_operation_error("force_reannounce", e);
    }
}

void TorrentHandle::force_dht_announce() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot force DHT announce: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->force_dht_announce();
            log_handle_operation("Force DHT announce initiated");
#endif
        } else {
            simulate_handle_operation("force_dht_announce");
        }
    } catch (const std::exception& e) {
        handle_operation_error("force_dht_announce", e);
    }
}

void TorrentHandle::move_storage(String new_path) {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot move storage: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->move_storage(new_path.utf8().get_data());
            log_handle_operation("Move storage to: " + new_path);
#endif
        } else {
            simulate_handle_operation("move_storage to " + new_path);
        }
    } catch (const std::exception& e) {
        handle_operation_error("move_storage", e);
    }
}

Array TorrentHandle::get_peer_info() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    Array peers;
    
    if (!validate_handle()) {
        return peers;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::vector<libtorrent::peer_info> peer_list;
            handle->get_peer_info(peer_list);

            for (const auto& peer : peer_list) {
                Ref<PeerInfo> peer_info;
                peer_info.instantiate();

                // Create a shared_ptr copy of peer data
                auto peer_copy = std::make_shared<libtorrent::peer_info>(peer);
                peer_info->_set_internal_info(peer_copy);

                peers.append(peer_info);
            }

            log_handle_operation("Retrieved " + String::num(peers.size()) + " peer info entries");
#endif
        } else {
            // Stub mode - return empty array
            simulate_handle_operation("get_peer_info");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_peer_info", e);
    }
    
    return peers;
}

void TorrentHandle::scrape_tracker() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot scrape tracker: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->scrape_tracker();
            log_handle_operation("Tracker scrape initiated");
#endif
        } else {
            simulate_handle_operation("scrape_tracker");
        }
    } catch (const std::exception& e) {
        handle_operation_error("scrape_tracker", e);
    }
}

void TorrentHandle::flush_cache() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot flush cache: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->flush_cache();
            log_handle_operation("Cache flushed");
#endif
        } else {
            simulate_handle_operation("flush_cache");
        }
    } catch (const std::exception& e) {
        handle_operation_error("flush_cache", e);
    }
}

void TorrentHandle::clear_error() {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    
    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot clear error: Invalid handle[/color]");
        return;
    }
    
    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->clear_error();
            log_handle_operation("Error cleared");
#endif
        } else {
            simulate_handle_operation("clear_error");
        }
    } catch (const std::exception& e) {
        handle_operation_error("clear_error", e);
    }
}

// Internal methods for libtorrent integration
void TorrentHandle::_set_internal_handle(const Variant& handle) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    try {
        // Clean up existing handle
        cleanup_handle();

        if (handle.get_type() != Variant::NIL) {
            if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
                // Extract handle pointer from Dictionary
                if (handle.get_type() == Variant::DICTIONARY) {
                    Dictionary handle_dict = handle;
                    if (handle_dict.has("libtorrent_ptr")) {
                        uint64_t ptr_value = handle_dict["libtorrent_ptr"];
                        _handle_ptr = reinterpret_cast<void*>(ptr_value);
                        _is_valid = (_handle_ptr != nullptr);
                        log_handle_operation("Real libtorrent handle set from pointer");
                    } else {
                        _handle_ptr = nullptr;
                        _is_valid = false;
                        log_handle_operation("Invalid handle data: missing libtorrent_ptr", false);
                    }
                } else {
                    _handle_ptr = nullptr;
                    _is_valid = false;
                    log_handle_operation("Invalid handle data: not a Dictionary", false);
                }
#endif
            } else {
                // In stub mode, any non-null Variant makes it valid
                _handle_ptr = reinterpret_cast<void*>(1); // Non-null marker for stub mode
                _is_valid = true;
                log_handle_operation("Stub handle set");
            }
        } else {
            _handle_ptr = nullptr;
            _is_valid = false;
            log_handle_operation("Handle cleared (set to null)");
        }
    } catch (const std::exception& e) {
        handle_operation_error("_set_internal_handle", e);
        _handle_ptr = nullptr;
        _is_valid = false;
    }
}

Variant TorrentHandle::_get_internal_handle() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (_handle_ptr && _is_valid) {
        if (!_is_stub_mode) {
            // Return handle pointer as Dictionary
            Dictionary handle_dict;
            handle_dict["libtorrent_ptr"] = (uint64_t)_handle_ptr;
            return handle_dict;
        } else {
            // In stub mode, return a dummy object
            Dictionary stub_handle;
            stub_handle["type"] = "stub_handle";
            stub_handle["valid"] = _is_valid;
            return stub_handle;
        }
    }

    return Variant(); // Return null variant for invalid handle
}

// Validation helpers
bool TorrentHandle::validate_piece_index(int piece_index) const {
    if (piece_index < 0) {
        return false;
    }
    
    if (!_is_stub_mode && _handle_ptr) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            if (handle->torrent_file()) {
                return piece_index < handle->torrent_file()->num_pieces();
            }
        } catch (...) {
            return false;
        }
#endif
    }
    
    // In stub mode or when validation fails, allow reasonable indices
    return piece_index < 10000; // Reasonable upper bound
}

bool TorrentHandle::validate_file_index(int file_index) const {
    if (file_index < 0) {
        return false;
    }
    
    if (!_is_stub_mode && _handle_ptr) {
#ifndef TORRENT_STUB_MODE
        try {
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            if (handle->torrent_file()) {
                return file_index < handle->torrent_file()->num_files();
            }
        } catch (...) {
            return false;
        }
#endif
    }
    
    // In stub mode or when validation fails, allow reasonable indices
    return file_index < 1000; // Reasonable upper bound
}

bool TorrentHandle::validate_priority(int priority) const {
    // libtorrent priorities: 0 (don't download) to 7 (highest priority)
    return priority >= 0 && priority <= 7;
}

// Error handling
void TorrentHandle::handle_operation_error(const std::string& operation, const std::exception& e) const {
    String error_msg = "[TorrentHandle::" + String(operation.c_str()) + "] Exception: " + String(e.what());
    UtilityFunctions::push_error(error_msg);
}

void TorrentHandle::report_error(const String& operation, const String& message) const {
    String error_msg = "[TorrentHandle::" + operation + "] " + message;
    UtilityFunctions::push_error(error_msg);
}

void TorrentHandle::log_handle_operation(const String& operation, bool success) const {
    String mode_prefix = _is_stub_mode ? "STUB HANDLE" : "REAL HANDLE";
    
    if (success) {
        UtilityFunctions::print(mode_prefix + ": " + operation);
    } else {
        UtilityFunctions::print_rich("[color=yellow]" + mode_prefix + ": " + operation + "[/color]");
    }
}

void TorrentHandle::simulate_handle_operation(const String& operation) const {
    UtilityFunctions::print("STUB HANDLE: " + operation + " (real implementation active)");
}

void TorrentHandle::save_resume_data() {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot save resume data: Invalid handle[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->save_resume_data(libtorrent::torrent_handle::save_info_dict);
            log_handle_operation("Resume data save requested");
#endif
        } else {
            simulate_handle_operation("save_resume_data");
        }
    } catch (const std::exception& e) {
        handle_operation_error("save_resume_data", e);
    }
}

PackedByteArray TorrentHandle::get_resume_data() {
    std::lock_guard<std::mutex> lock(_resume_data_mutex);
    return _resume_data;
}

void TorrentHandle::rename_file(int file_index, String new_name) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot rename file: Invalid handle[/color]");
        return;
    }

    if (!validate_file_index(file_index)) {
        UtilityFunctions::print_rich("[color=yellow]Invalid file index[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->rename_file(libtorrent::file_index_t(file_index), new_name.utf8().get_data());
            log_handle_operation("Rename file " + String::num(file_index) + " to " + new_name);
#endif
        } else {
            simulate_handle_operation("rename_file");
        }
    } catch (const std::exception& e) {
        handle_operation_error("rename_file", e);
    }
}

Array TorrentHandle::get_file_progress() {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    Array progress;

    if (!validate_handle()) {
        return progress;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::vector<int64_t> file_progress;
            handle->file_progress(file_progress, libtorrent::torrent_handle::piece_granularity);

            for (int64_t bytes : file_progress) {
                progress.append(bytes);
            }

            log_handle_operation("Retrieved file progress for " + String::num(progress.size()) + " files");
#endif
        } else {
            simulate_handle_operation("get_file_progress");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_file_progress", e);
    }

    return progress;
}

bool TorrentHandle::have_piece(int piece_index) const {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        return false;
    }

    if (!validate_piece_index(piece_index)) {
        return false;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            return handle->have_piece(libtorrent::piece_index_t(piece_index));
#endif
        } else {
            return false; // Stub mode
        }
    } catch (const std::exception& e) {
        handle_operation_error("have_piece", e);
        return false;
    }

    return false;
}

void TorrentHandle::read_piece(int piece_index) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot read piece: Invalid handle[/color]");
        return;
    }

    if (!validate_piece_index(piece_index)) {
        UtilityFunctions::print_rich("[color=yellow]Invalid piece index[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->read_piece(libtorrent::piece_index_t(piece_index));
            log_handle_operation("Read piece " + String::num(piece_index) + " requested");
#endif
        } else {
            simulate_handle_operation("read_piece");
        }
    } catch (const std::exception& e) {
        handle_operation_error("read_piece", e);
    }
}

Array TorrentHandle::get_piece_availability() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    Array availability;

    if (!validate_handle()) {
        return availability;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::vector<int> piece_availability;
            handle->piece_availability(piece_availability);

            for (int avail : piece_availability) {
                availability.append(avail);
            }

            log_handle_operation("Retrieved piece availability for " + String::num(availability.size()) + " pieces");
#endif
        } else {
            simulate_handle_operation("get_piece_availability");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_piece_availability", e);
    }

    return availability;
}

void TorrentHandle::add_tracker(String url, int tier) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot add tracker: Invalid handle[/color]");
        return;
    }

    if (url.is_empty()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot add tracker: URL is empty[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            libtorrent::announce_entry ae(url.utf8().get_data());
            ae.tier = tier;
            std::vector<libtorrent::announce_entry> trackers;
            trackers.push_back(ae);
            handle->add_tracker(ae);
            log_handle_operation("Added tracker: " + url + " (tier " + String::num(tier) + ")");
#endif
        } else {
            simulate_handle_operation("add_tracker");
        }
    } catch (const std::exception& e) {
        handle_operation_error("add_tracker", e);
    }
}

void TorrentHandle::remove_tracker(String url) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot remove tracker: Invalid handle[/color]");
        return;
    }

    if (url.is_empty()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot remove tracker: URL is empty[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::vector<libtorrent::announce_entry> trackers = handle->trackers();

            // Find and remove the tracker
            bool found = false;
            for (auto it = trackers.begin(); it != trackers.end(); ++it) {
                if (it->url == url.utf8().get_data()) {
                    trackers.erase(it);
                    found = true;
                    break;
                }
            }

            if (found) {
                handle->replace_trackers(trackers);
                log_handle_operation("Removed tracker: " + url);
            } else {
                UtilityFunctions::print_rich("[color=yellow]Tracker not found: " + url + "[/color]");
            }
#endif
        } else {
            simulate_handle_operation("remove_tracker");
        }
    } catch (const std::exception& e) {
        handle_operation_error("remove_tracker", e);
    }
}

Array TorrentHandle::get_trackers() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    Array trackers;

    if (!validate_handle()) {
        return trackers;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::vector<libtorrent::announce_entry> tracker_list = handle->trackers();

            for (const auto& tracker : tracker_list) {
                Dictionary tracker_info;
                tracker_info["url"] = String(tracker.url.c_str());
                tracker_info["tier"] = tracker.tier;
                tracker_info["fail_limit"] = tracker.fail_limit;
                tracker_info["source"] = static_cast<int>(tracker.source);
                tracker_info["verified"] = tracker.verified;

                // Get endpoint information
                Array endpoints;
                for (const auto& endpoint : tracker.endpoints) {
                    Dictionary ep_info;
                    ep_info["fails"] = endpoint.fails;
                    ep_info["updating"] = endpoint.updating;
                    ep_info["start_sent"] = endpoint.start_sent;
                    ep_info["complete_sent"] = endpoint.complete_sent;

                    if (endpoint.message.size() > 0) {
                        ep_info["message"] = String(endpoint.message.c_str());
                    }

                    endpoints.append(ep_info);
                }
                tracker_info["endpoints"] = endpoints;

                trackers.append(tracker_info);
            }

            log_handle_operation("Retrieved " + String::num(trackers.size()) + " trackers");
#endif
        } else {
            simulate_handle_operation("get_trackers");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_trackers", e);
    }

    return trackers;
}

void TorrentHandle::add_url_seed(String url) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot add URL seed: Invalid handle[/color]");
        return;
    }

    if (url.is_empty()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot add URL seed: URL is empty[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->add_url_seed(url.utf8().get_data());
            log_handle_operation("Added URL seed: " + url);
#endif
        } else {
            simulate_handle_operation("add_url_seed");
        }
    } catch (const std::exception& e) {
        handle_operation_error("add_url_seed", e);
    }
}

void TorrentHandle::remove_url_seed(String url) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot remove URL seed: Invalid handle[/color]");
        return;
    }

    if (url.is_empty()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot remove URL seed: URL is empty[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->remove_url_seed(url.utf8().get_data());
            log_handle_operation("Removed URL seed: " + url);
#endif
        } else {
            simulate_handle_operation("remove_url_seed");
        }
    } catch (const std::exception& e) {
        handle_operation_error("remove_url_seed", e);
    }
}

void TorrentHandle::add_http_seed(String url) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot add HTTP seed: Invalid handle[/color]");
        return;
    }

    if (url.is_empty()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot add HTTP seed: URL is empty[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->add_http_seed(url.utf8().get_data());
            log_handle_operation("Added HTTP seed: " + url);
#endif
        } else {
            simulate_handle_operation("add_http_seed");
        }
    } catch (const std::exception& e) {
        handle_operation_error("add_http_seed", e);
    }
}

void TorrentHandle::remove_http_seed(String url) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!validate_handle()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot remove HTTP seed: Invalid handle[/color]");
        return;
    }

    if (url.is_empty()) {
        UtilityFunctions::print_rich("[color=yellow]Cannot remove HTTP seed: URL is empty[/color]");
        return;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            handle->remove_http_seed(url.utf8().get_data());
            log_handle_operation("Removed HTTP seed: " + url);
#endif
        } else {
            simulate_handle_operation("remove_http_seed");
        }
    } catch (const std::exception& e) {
        handle_operation_error("remove_http_seed", e);
    }
}

Array TorrentHandle::get_url_seeds() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    Array seeds;

    if (!validate_handle()) {
        return seeds;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::set<std::string> url_seeds = handle->url_seeds();

            for (const auto& seed : url_seeds) {
                seeds.append(String(seed.c_str()));
            }

            log_handle_operation("Retrieved " + String::num(seeds.size()) + " URL seeds");
#endif
        } else {
            simulate_handle_operation("get_url_seeds");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_url_seeds", e);
    }

    return seeds;
}

Array TorrentHandle::get_http_seeds() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    Array seeds;

    if (!validate_handle()) {
        return seeds;
    }

    try {
        if (!_is_stub_mode) {
#ifndef TORRENT_STUB_MODE
            libtorrent::torrent_handle* handle = static_cast<libtorrent::torrent_handle*>(_handle_ptr);
            std::set<std::string> http_seeds = handle->http_seeds();

            for (const auto& seed : http_seeds) {
                seeds.append(String(seed.c_str()));
            }

            log_handle_operation("Retrieved " + String::num(seeds.size()) + " HTTP seeds");
#endif
        } else {
            simulate_handle_operation("get_http_seeds");
        }
    } catch (const std::exception& e) {
        handle_operation_error("get_http_seeds", e);
    }

    return seeds;
}

// Mutable Torrent Methods

bool TorrentHandle::is_mutable() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    return _is_mutable;
}

PackedByteArray TorrentHandle::get_public_key() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    return _public_key;
}

int64_t TorrentHandle::get_sequence_number() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    return _sequence_number;
}

bool TorrentHandle::publish_update(PackedByteArray new_torrent_data) {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!_is_mutable) {
        report_error("publish_update", "This handle is not a mutable torrent");
        return false;
    }

    if (!validate_handle()) {
        report_error("publish_update", "Invalid handle");
        return false;
    }

    if (!_parent_session) {
        report_error("publish_update", "No parent session available");
        return false;
    }

    if (new_torrent_data.size() == 0) {
        report_error("publish_update", "Empty torrent data");
        return false;
    }

    if (_public_key.size() != 32) {
        report_error("publish_update", "Invalid public key");
        return false;
    }

#ifndef TORRENT_STUB_MODE
    try {
        // Cast parent session pointer and call the update method
        libtorrent::session* session = static_cast<libtorrent::session*>(_parent_session);
        TorrentSession* torrent_session = reinterpret_cast<TorrentSession*>(_parent_session);

        // Call the session method to publish the update
        bool success = torrent_session->publish_mutable_torrent_update(_public_key, new_torrent_data);

        if (success) {
            _sequence_number++;
            log_handle_operation("Published mutable torrent update");
        }

        return success;
    } catch (const std::exception& e) {
        report_error("publish_update", String("Exception: ") + e.what());
        return false;
    }
#else
    report_error("publish_update", "Not available in stub mode");
    return false;
#endif
}

void TorrentHandle::check_for_updates() {
    std::lock_guard<std::mutex> lock(_handle_mutex);

    if (!_is_mutable) {
        report_error("check_for_updates", "This handle is not a mutable torrent");
        return;
    }

    if (!validate_handle()) {
        report_error("check_for_updates", "Invalid handle");
        return;
    }

    if (!_parent_session) {
        report_error("check_for_updates", "No parent session available");
        return;
    }

    if (_public_key.size() != 32) {
        report_error("check_for_updates", "Invalid public key");
        return;
    }

#ifndef TORRENT_STUB_MODE
    try {
        // Cast parent session pointer to TorrentSession
        TorrentSession* torrent_session = static_cast<TorrentSession*>(_parent_session);

        // Call the session method to check for updates
        torrent_session->check_mutable_torrent_for_updates(_public_key);

        log_handle_operation("Checking for mutable torrent updates");
    } catch (const std::exception& e) {
        report_error("check_for_updates", String("Exception: ") + e.what());
    }
#else
    report_error("check_for_updates", "Not available in stub mode");
#endif
}

void TorrentHandle::set_auto_update(bool enabled) {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    _auto_update_enabled = enabled;
    log_handle_operation(String("Auto-update ") + (enabled ? "enabled" : "disabled"));
}

bool TorrentHandle::is_auto_update_enabled() const {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    return _auto_update_enabled;
}

void TorrentHandle::_set_parent_session(void* session_ptr) {
    std::lock_guard<std::mutex> lock(_handle_mutex);
    _parent_session = session_ptr;
}