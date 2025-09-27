#include "peer_info.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void PeerInfo::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_ip"), &PeerInfo::get_ip);
    ClassDB::bind_method(D_METHOD("get_port"), &PeerInfo::get_port);
    ClassDB::bind_method(D_METHOD("get_client"), &PeerInfo::get_client);
    ClassDB::bind_method(D_METHOD("get_peer_id"), &PeerInfo::get_peer_id);
    
    ClassDB::bind_method(D_METHOD("get_connection_type"), &PeerInfo::get_connection_type);
    ClassDB::bind_method(D_METHOD("is_seed"), &PeerInfo::is_seed);
    ClassDB::bind_method(D_METHOD("is_local"), &PeerInfo::is_local);
    
    ClassDB::bind_method(D_METHOD("get_download_rate"), &PeerInfo::get_download_rate);
    ClassDB::bind_method(D_METHOD("get_upload_rate"), &PeerInfo::get_upload_rate);
    ClassDB::bind_method(D_METHOD("get_total_download"), &PeerInfo::get_total_download);
    ClassDB::bind_method(D_METHOD("get_total_upload"), &PeerInfo::get_total_upload);
    
    ClassDB::bind_method(D_METHOD("get_progress"), &PeerInfo::get_progress);
    ClassDB::bind_method(D_METHOD("get_pieces_downloaded"), &PeerInfo::get_pieces_downloaded);
    
    ClassDB::bind_method(D_METHOD("get_last_request"), &PeerInfo::get_last_request);
    ClassDB::bind_method(D_METHOD("get_last_active"), &PeerInfo::get_last_active);
    
    ClassDB::bind_method(D_METHOD("get_download_queue_length"), &PeerInfo::get_download_queue_length);
    ClassDB::bind_method(D_METHOD("get_upload_queue_length"), &PeerInfo::get_upload_queue_length);
    
    ClassDB::bind_method(D_METHOD("is_interesting"), &PeerInfo::is_interesting);
    ClassDB::bind_method(D_METHOD("is_choked"), &PeerInfo::is_choked);
    ClassDB::bind_method(D_METHOD("is_remote_interested"), &PeerInfo::is_remote_interested);
    ClassDB::bind_method(D_METHOD("is_remote_choked"), &PeerInfo::is_remote_choked);
    
    ClassDB::bind_method(D_METHOD("get_country"), &PeerInfo::get_country);
}

PeerInfo::PeerInfo() {
    _valid = false;
}

PeerInfo::~PeerInfo() {
}

// STUB implementations
String PeerInfo::get_ip() const { 
    if (!_valid) return "";
    return "127.0.0.1"; 
}
int PeerInfo::get_port() const { 
    if (!_valid) return 0;
    return 6881; 
}
String PeerInfo::get_client() const { return "STUB-1.0"; }
String PeerInfo::get_peer_id() const { return "1234567890abcdef1234"; }
String PeerInfo::get_connection_type() const { return "bittorrent"; }
bool PeerInfo::is_seed() const { return false; }
bool PeerInfo::is_local() const { return true; }
int PeerInfo::get_download_rate() const { return 0; }
int PeerInfo::get_upload_rate() const { return 0; }
int64_t PeerInfo::get_total_download() const { return 0; }
int64_t PeerInfo::get_total_upload() const { return 0; }
float PeerInfo::get_progress() const { return 0.0f; }
int PeerInfo::get_pieces_downloaded() const { return 0; }
int PeerInfo::get_last_request() const { return 0; }
int PeerInfo::get_last_active() const { return 0; }
int PeerInfo::get_download_queue_length() const { return 0; }
int PeerInfo::get_upload_queue_length() const { return 0; }
bool PeerInfo::is_interesting() const { return false; }
bool PeerInfo::is_choked() const { return false; }
bool PeerInfo::is_remote_interested() const { return false; }
bool PeerInfo::is_remote_choked() const { return false; }
String PeerInfo::get_country() const { return "US"; }

void PeerInfo::_set_internal_info(const void* info) {
    _valid = (info != nullptr);
}