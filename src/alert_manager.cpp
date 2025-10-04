#include "alert_manager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/alert.hpp>
#include <sstream>

using namespace godot;

void AlertManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_alerts"), &AlertManager::get_alerts);
    ClassDB::bind_method(D_METHOD("get_alerts_by_type", "alert_type"), &AlertManager::get_alerts_by_type);
    ClassDB::bind_method(D_METHOD("clear_alerts"), &AlertManager::clear_alerts);
    
    ClassDB::bind_method(D_METHOD("set_alert_mask", "mask"), &AlertManager::set_alert_mask);
    ClassDB::bind_method(D_METHOD("get_alert_mask"), &AlertManager::get_alert_mask);
    
    ClassDB::bind_method(D_METHOD("enable_error_alerts", "enabled"), &AlertManager::enable_error_alerts);
    ClassDB::bind_method(D_METHOD("enable_status_alerts", "enabled"), &AlertManager::enable_status_alerts);
    ClassDB::bind_method(D_METHOD("enable_progress_alerts", "enabled"), &AlertManager::enable_progress_alerts);
    ClassDB::bind_method(D_METHOD("enable_peer_alerts", "enabled"), &AlertManager::enable_peer_alerts);
    ClassDB::bind_method(D_METHOD("enable_storage_alerts", "enabled"), &AlertManager::enable_storage_alerts);
    ClassDB::bind_method(D_METHOD("enable_tracker_alerts", "enabled"), &AlertManager::enable_tracker_alerts);
    ClassDB::bind_method(D_METHOD("enable_dht_alerts", "enabled"), &AlertManager::enable_dht_alerts);
    
    ClassDB::bind_method(D_METHOD("get_torrent_alerts"), &AlertManager::get_torrent_alerts);
    ClassDB::bind_method(D_METHOD("get_peer_alerts"), &AlertManager::get_peer_alerts);
    ClassDB::bind_method(D_METHOD("get_tracker_alerts"), &AlertManager::get_tracker_alerts);
    ClassDB::bind_method(D_METHOD("get_error_alerts"), &AlertManager::get_error_alerts);
}

AlertManager::AlertManager() {
    // Default mask: all categories enabled
    _alert_mask = libtorrent::alert_category::all;
}

AlertManager::~AlertManager() {
}

void AlertManager::process_alerts(const std::vector<void*>& alerts) {
    for (void* alert_ptr : alerts) {
        if (alert_ptr) {
            Dictionary alert_dict = _convert_alert_to_dictionary(alert_ptr);
            _alerts.append(alert_dict);
        }
    }
}

Array AlertManager::get_alerts() {
    return _alerts;
}

Array AlertManager::get_alerts_by_type(int alert_type) {
    Array filtered_alerts;
    
    for (int i = 0; i < _alerts.size(); i++) {
        Dictionary alert = _alerts[i];
        if (alert.has("type") && alert["type"].operator int() == alert_type) {
            filtered_alerts.append(alert);
        }
    }
    
    return filtered_alerts;
}

void AlertManager::clear_alerts() {
    _alerts.clear();
}

void AlertManager::set_alert_mask(int mask) {
    _alert_mask = mask;
}

int AlertManager::get_alert_mask() const {
    return _alert_mask;
}

void AlertManager::enable_error_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::error;
    } else {
        _alert_mask &= ~libtorrent::alert_category::error;
    }
}

void AlertManager::enable_status_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::status;
    } else {
        _alert_mask &= ~libtorrent::alert_category::status;
    }
}

void AlertManager::enable_progress_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::file_progress;
    } else {
        _alert_mask &= ~libtorrent::alert_category::file_progress;
    }
}

void AlertManager::enable_peer_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::peer;
    } else {
        _alert_mask &= ~libtorrent::alert_category::peer;
    }
}

void AlertManager::enable_storage_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::storage;
    } else {
        _alert_mask &= ~libtorrent::alert_category::storage;
    }
}

void AlertManager::enable_tracker_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::tracker;
    } else {
        _alert_mask &= ~libtorrent::alert_category::tracker;
    }
}

void AlertManager::enable_dht_alerts(bool enabled) {
    if (enabled) {
        _alert_mask |= libtorrent::alert_category::dht;
    } else {
        _alert_mask &= ~libtorrent::alert_category::dht;
    }
}

Array AlertManager::get_torrent_alerts() const {
    Array filtered;
    for (int i = 0; i < _alerts.size(); i++) {
        Dictionary alert = _alerts[i];
        if (alert.has("category")) {
            int category = alert["category"];
            if (category & libtorrent::alert_category::status) {
                filtered.append(alert);
            }
        }
    }
    return filtered;
}

Array AlertManager::get_peer_alerts() const {
    Array filtered;
    for (int i = 0; i < _alerts.size(); i++) {
        Dictionary alert = _alerts[i];
        if (alert.has("category")) {
            int category = alert["category"];
            if (category & libtorrent::alert_category::peer) {
                filtered.append(alert);
            }
        }
    }
    return filtered;
}

Array AlertManager::get_tracker_alerts() const {
    Array filtered;
    for (int i = 0; i < _alerts.size(); i++) {
        Dictionary alert = _alerts[i];
        if (alert.has("category")) {
            int category = alert["category"];
            if (category & libtorrent::alert_category::tracker) {
                filtered.append(alert);
            }
        }
    }
    return filtered;
}

Array AlertManager::get_error_alerts() const {
    Array filtered;
    for (int i = 0; i < _alerts.size(); i++) {
        Dictionary alert = _alerts[i];
        if (alert.has("category")) {
            int category = alert["category"];
            if (category & libtorrent::alert_category::error) {
                filtered.append(alert);
            }
        }
    }
    return filtered;
}

Dictionary AlertManager::_convert_alert_to_dictionary(void* alert_ptr) {
    libtorrent::alert* alert = static_cast<libtorrent::alert*>(alert_ptr);

    Dictionary dict;
    dict["type"] = alert->type();
    dict["category"] = static_cast<int>(alert->category());
    dict["message"] = String(alert->message().c_str());
    dict["type_name"] = _get_alert_type_name(alert->type());

    // Extract info_hash if available (most alerts have it)
    std::stringstream ss;

    // Torrent alerts
    if (auto* ta = libtorrent::alert_cast<libtorrent::torrent_finished_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::torrent_error_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["error"] = String(ta->error.message().c_str());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::torrent_added_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::torrent_removed_alert>(alert)) {
        ss << ta->info_hash;
        dict["info_hash"] = String(ss.str().c_str());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::torrent_paused_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::torrent_resumed_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::state_changed_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["old_state"] = static_cast<int>(ta->prev_state);
        dict["new_state"] = static_cast<int>(ta->state);
    }
    // Tracker alerts
    else if (auto* ta = libtorrent::alert_cast<libtorrent::tracker_reply_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["num_peers"] = ta->num_peers;
        dict["tracker_url"] = String(ta->tracker_url());
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::tracker_error_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["error"] = String(ta->error.message().c_str());
        dict["tracker_url"] = String(ta->tracker_url());
        dict["times_in_row"] = ta->times_in_row;
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::tracker_announce_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["tracker_url"] = String(ta->tracker_url());
        dict["event"] = static_cast<int>(ta->event);
    }
    else if (auto* ta = libtorrent::alert_cast<libtorrent::tracker_warning_alert>(alert)) {
        ss << ta->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["warning"] = String(ta->warning_message());
        dict["tracker_url"] = String(ta->tracker_url());
    }
    // Peer alerts
    else if (auto* pa = libtorrent::alert_cast<libtorrent::peer_connect_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["peer_id"] = String(pa->pid.to_string().c_str());
        std::stringstream peer_ss;
        peer_ss << pa->endpoint;
        dict["endpoint"] = String(peer_ss.str().c_str());
    }
    else if (auto* pa = libtorrent::alert_cast<libtorrent::peer_disconnected_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["peer_id"] = String(pa->pid.to_string().c_str());
        dict["error"] = String(pa->error.message().c_str());
        dict["reason"] = static_cast<int>(pa->reason);
    }
    else if (auto* pa = libtorrent::alert_cast<libtorrent::peer_error_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["peer_id"] = String(pa->pid.to_string().c_str());
        dict["error"] = String(pa->error.message().c_str());
    }
    else if (auto* pa = libtorrent::alert_cast<libtorrent::peer_ban_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        std::stringstream peer_ss;
        peer_ss << pa->endpoint;
        dict["endpoint"] = String(peer_ss.str().c_str());
    }
    // Piece alerts
    else if (auto* pa = libtorrent::alert_cast<libtorrent::piece_finished_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["piece_index"] = static_cast<int>(pa->piece_index);
    }
    else if (auto* pa = libtorrent::alert_cast<libtorrent::hash_failed_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["piece_index"] = static_cast<int>(pa->piece_index);
    }
    else if (auto* pa = libtorrent::alert_cast<libtorrent::read_piece_alert>(alert)) {
        ss << pa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["piece_index"] = static_cast<int>(pa->piece);
        dict["size"] = pa->size;
    }
    // File alerts
    else if (auto* fa = libtorrent::alert_cast<libtorrent::file_error_alert>(alert)) {
        ss << fa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["file"] = String(fa->filename());
        dict["error"] = String(fa->error.message().c_str());
    }
    else if (auto* fa = libtorrent::alert_cast<libtorrent::file_completed_alert>(alert)) {
        ss << fa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["file_index"] = static_cast<int>(fa->index);
    }
    else if (auto* fa = libtorrent::alert_cast<libtorrent::file_renamed_alert>(alert)) {
        ss << fa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["file_index"] = static_cast<int>(fa->index);
        dict["new_name"] = String(fa->new_name());
    }
    // Metadata alerts
    else if (auto* ma = libtorrent::alert_cast<libtorrent::metadata_received_alert>(alert)) {
        ss << ma->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
    }
    else if (auto* ma = libtorrent::alert_cast<libtorrent::metadata_failed_alert>(alert)) {
        ss << ma->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["error"] = String(ma->error.message().c_str());
    }
    // DHT alerts
    else if (auto* da = libtorrent::alert_cast<libtorrent::dht_reply_alert>(alert)) {
        dict["num_peers"] = da->num_peers;
    }
    else if (auto* da = libtorrent::alert_cast<libtorrent::dht_bootstrap_alert>(alert)) {
        // No additional data
    }
    else if (auto* da = libtorrent::alert_cast<libtorrent::dht_error_alert>(alert)) {
        dict["error"] = String(da->error.message().c_str());
        // operation field is deprecated, skip it
    }
    // Storage/resume data alerts
    else if (auto* sa = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(alert)) {
        ss << sa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        // Resume data is in sa->params, but it's complex to serialize
    }
    else if (auto* sa = libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(alert)) {
        ss << sa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["error"] = String(sa->error.message().c_str());
    }
    else if (auto* sa = libtorrent::alert_cast<libtorrent::storage_moved_alert>(alert)) {
        ss << sa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["storage_path"] = String(sa->storage_path());
    }
    else if (auto* sa = libtorrent::alert_cast<libtorrent::storage_moved_failed_alert>(alert)) {
        ss << sa->handle.info_hash();
        dict["info_hash"] = String(ss.str().c_str());
        dict["error"] = String(sa->error.message().c_str());
    }

    return dict;
}

String AlertManager::_get_alert_type_name(int type) {
    // Map libtorrent alert type IDs to human-readable names
    switch (type) {
        case libtorrent::torrent_finished_alert::alert_type: return "torrent_finished";
        case libtorrent::torrent_error_alert::alert_type: return "torrent_error";
        case libtorrent::torrent_added_alert::alert_type: return "torrent_added";
        case libtorrent::torrent_removed_alert::alert_type: return "torrent_removed";
        case libtorrent::torrent_paused_alert::alert_type: return "torrent_paused";
        case libtorrent::torrent_resumed_alert::alert_type: return "torrent_resumed";
        case libtorrent::state_changed_alert::alert_type: return "state_changed";
        case libtorrent::tracker_reply_alert::alert_type: return "tracker_reply";
        case libtorrent::tracker_error_alert::alert_type: return "tracker_error";
        case libtorrent::tracker_announce_alert::alert_type: return "tracker_announce";
        case libtorrent::tracker_warning_alert::alert_type: return "tracker_warning";
        case libtorrent::peer_connect_alert::alert_type: return "peer_connect";
        case libtorrent::peer_disconnected_alert::alert_type: return "peer_disconnected";
        case libtorrent::peer_error_alert::alert_type: return "peer_error";
        case libtorrent::peer_ban_alert::alert_type: return "peer_ban";
        case libtorrent::piece_finished_alert::alert_type: return "piece_finished";
        case libtorrent::hash_failed_alert::alert_type: return "hash_failed";
        case libtorrent::read_piece_alert::alert_type: return "read_piece";
        case libtorrent::file_error_alert::alert_type: return "file_error";
        case libtorrent::file_completed_alert::alert_type: return "file_completed";
        case libtorrent::file_renamed_alert::alert_type: return "file_renamed";
        case libtorrent::metadata_received_alert::alert_type: return "metadata_received";
        case libtorrent::metadata_failed_alert::alert_type: return "metadata_failed";
        case libtorrent::dht_reply_alert::alert_type: return "dht_reply";
        case libtorrent::dht_bootstrap_alert::alert_type: return "dht_bootstrap";
        case libtorrent::dht_error_alert::alert_type: return "dht_error";
        case libtorrent::save_resume_data_alert::alert_type: return "save_resume_data";
        case libtorrent::save_resume_data_failed_alert::alert_type: return "save_resume_data_failed";
        case libtorrent::storage_moved_alert::alert_type: return "storage_moved";
        case libtorrent::storage_moved_failed_alert::alert_type: return "storage_moved_failed";
        default: return "unknown_alert";
    }
}