#include "alert_manager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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
    _alert_mask = 7; // STUB: basic mask
}

AlertManager::~AlertManager() {
}

// STUB implementations
void AlertManager::process_alerts(const std::vector<void*>& alerts) {
    // STUB: Convert void* alerts when libtorrent is integrated
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

// STUB implementations for alert categories
void AlertManager::enable_error_alerts(bool enabled) {
    // STUB
}

void AlertManager::enable_status_alerts(bool enabled) {
    // STUB
}

void AlertManager::enable_progress_alerts(bool enabled) {
    // STUB
}

void AlertManager::enable_peer_alerts(bool enabled) {
    // STUB
}

void AlertManager::enable_storage_alerts(bool enabled) {
    // STUB
}

void AlertManager::enable_tracker_alerts(bool enabled) {
    // STUB
}

void AlertManager::enable_dht_alerts(bool enabled) {
    // STUB
}

Array AlertManager::get_torrent_alerts() const {
    return Array(); // STUB
}

Array AlertManager::get_peer_alerts() const {
    return Array(); // STUB
}

Array AlertManager::get_tracker_alerts() const {
    return Array(); // STUB
}

Array AlertManager::get_error_alerts() const {
    return Array(); // STUB
}

Dictionary AlertManager::_convert_alert_to_dictionary(void* alert) {
    Dictionary alert_dict;
    
    // STUB: Will convert libtorrent alerts when integrated
    alert_dict["type"] = 0;
    alert_dict["category"] = 0;
    alert_dict["message"] = "STUB: Alert message";
    alert_dict["timestamp"] = 0;
    alert_dict["type_name"] = "stub_alert";
    
    return alert_dict;
}

String AlertManager::_get_alert_type_name(int type) {
    return "stub_alert"; // STUB
}