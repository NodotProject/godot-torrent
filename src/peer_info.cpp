#include "peer_info.h"

#include <godot_cpp/core/class_db.hpp>
#include <libtorrent/peer_info.hpp>
#include <sstream>
#include <iomanip>

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
    ClassDB::bind_method(D_METHOD("get_peer_dictionary"), &PeerInfo::get_peer_dictionary);
}

PeerInfo::PeerInfo() : _peer_info(nullptr) {
}

PeerInfo::~PeerInfo() {
}

String PeerInfo::get_ip() const {
    if (!_peer_info) return "";

    std::stringstream ss;
    ss << _peer_info->ip.address();
    return String(ss.str().c_str());
}

int PeerInfo::get_port() const {
    if (!_peer_info) return 0;
    return _peer_info->ip.port();
}

String PeerInfo::get_client() const {
    if (!_peer_info) return "";
    return String(_peer_info->client.c_str());
}

String PeerInfo::get_peer_id() const {
    if (!_peer_info) return "";

    std::stringstream ss;
    for (int i = 0; i < 20 && i < static_cast<int>(_peer_info->pid.size()); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(static_cast<unsigned char>(_peer_info->pid[i]));
    }
    return String(ss.str().c_str());
}

String PeerInfo::get_connection_type() const {
    if (!_peer_info) return "";
    return connection_type_to_string(_peer_info->connection_type);
}

bool PeerInfo::is_seed() const {
    if (!_peer_info) return false;
    return (_peer_info->flags & libtorrent::peer_info::seed) != 0;
}

bool PeerInfo::is_local() const {
    if (!_peer_info) return false;
    return (_peer_info->flags & libtorrent::peer_info::local_connection) != 0;
}

int PeerInfo::get_download_rate() const {
    if (!_peer_info) return 0;
    return _peer_info->down_speed;
}

int PeerInfo::get_upload_rate() const {
    if (!_peer_info) return 0;
    return _peer_info->up_speed;
}

int64_t PeerInfo::get_total_download() const {
    if (!_peer_info) return 0;
    return _peer_info->total_download;
}

int64_t PeerInfo::get_total_upload() const {
    if (!_peer_info) return 0;
    return _peer_info->total_upload;
}

float PeerInfo::get_progress() const {
    if (!_peer_info) return 0.0f;
    return _peer_info->progress;
}

int PeerInfo::get_pieces_downloaded() const {
    if (!_peer_info) return 0;
    return _peer_info->num_pieces;
}

int PeerInfo::get_last_request() const {
    if (!_peer_info) return 0;
    // Convert time_duration (nanoseconds) to seconds
    return std::chrono::duration_cast<std::chrono::seconds>(_peer_info->last_request).count();
}

int PeerInfo::get_last_active() const {
    if (!_peer_info) return 0;
    // Convert time_duration (nanoseconds) to seconds
    return std::chrono::duration_cast<std::chrono::seconds>(_peer_info->last_active).count();
}

int PeerInfo::get_download_queue_length() const {
    if (!_peer_info) return 0;
    return _peer_info->download_queue_length;
}

int PeerInfo::get_upload_queue_length() const {
    if (!_peer_info) return 0;
    return _peer_info->upload_queue_length;
}

bool PeerInfo::is_interesting() const {
    if (!_peer_info) return false;
    return (_peer_info->flags & libtorrent::peer_info::interesting) != 0;
}

bool PeerInfo::is_choked() const {
    if (!_peer_info) return false;
    return (_peer_info->flags & libtorrent::peer_info::choked) != 0;
}

bool PeerInfo::is_remote_interested() const {
    if (!_peer_info) return false;
    return (_peer_info->flags & libtorrent::peer_info::remote_interested) != 0;
}

bool PeerInfo::is_remote_choked() const {
    if (!_peer_info) return false;
    return (_peer_info->flags & libtorrent::peer_info::remote_choked) != 0;
}

String PeerInfo::get_country() const {
    // Country information requires GeoIP database which may not be available
    // This field is optional in libtorrent and not always present
    return "";
}

void PeerInfo::_set_internal_info(std::shared_ptr<libtorrent::peer_info> info) {
    _peer_info = info;
}

Dictionary PeerInfo::get_peer_dictionary() const {
    Dictionary dict;

    dict["ip"] = get_ip();
    dict["port"] = get_port();
    dict["client"] = get_client();
    dict["peer_id"] = get_peer_id();
    dict["connection_type"] = get_connection_type();
    dict["is_seed"] = is_seed();
    dict["is_local"] = is_local();
    dict["download_rate"] = get_download_rate();
    dict["upload_rate"] = get_upload_rate();
    dict["total_download"] = get_total_download();
    dict["total_upload"] = get_total_upload();
    dict["progress"] = get_progress();
    dict["pieces_downloaded"] = get_pieces_downloaded();
    dict["last_request"] = get_last_request();
    dict["last_active"] = get_last_active();
    dict["download_queue_length"] = get_download_queue_length();
    dict["upload_queue_length"] = get_upload_queue_length();
    dict["is_interesting"] = is_interesting();
    dict["is_choked"] = is_choked();
    dict["is_remote_interested"] = is_remote_interested();
    dict["is_remote_choked"] = is_remote_choked();
    dict["country"] = get_country();

    return dict;
}

String PeerInfo::connection_type_to_string(int type) const {
    switch (type) {
        case libtorrent::peer_info::standard_bittorrent: return "bittorrent";
        case libtorrent::peer_info::web_seed: return "web_seed";
        case libtorrent::peer_info::http_seed: return "http_seed";
        default: return "unknown";
    }
}