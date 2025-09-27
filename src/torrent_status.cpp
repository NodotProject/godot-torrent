#include "torrent_status.h"

#include <godot_cpp/core/class_db.hpp>

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
}

TorrentStatus::TorrentStatus() {
    _valid = false;
    _state = 0;
    _paused = false;
    _progress = 0.0f;
}

TorrentStatus::~TorrentStatus() {
}

String TorrentStatus::get_state_string() const {
    return "downloading"; // STUB
}

int TorrentStatus::get_state() const {
    return _state;
}

bool TorrentStatus::is_paused() const {
    return _paused;
}

bool TorrentStatus::is_finished() const {
    return _progress >= 1.0f;
}

bool TorrentStatus::is_seeding() const {
    return is_finished();
}

float TorrentStatus::get_progress() const {
    return _progress;
}

int64_t TorrentStatus::get_total_done() const {
    return static_cast<int64_t>(get_total_size() * _progress);
}

int64_t TorrentStatus::get_total_size() const {
    return 1024 * 1024 * 100; // 100MB stub
}

int64_t TorrentStatus::get_total_wanted() const {
    return get_total_size();
}

int64_t TorrentStatus::get_total_wanted_done() const {
    return get_total_done();
}

int TorrentStatus::get_download_rate() const {
    return 0; // STUB
}

int TorrentStatus::get_upload_rate() const {
    return 0; // STUB
}

int TorrentStatus::get_download_payload_rate() const {
    return 0; // STUB
}

int TorrentStatus::get_upload_payload_rate() const {
    return 0; // STUB
}

int TorrentStatus::get_num_peers() const {
    return 0; // STUB
}

int TorrentStatus::get_num_seeds() const {
    return 0; // STUB
}

int TorrentStatus::get_num_connections() const {
    return 0; // STUB
}

int TorrentStatus::get_connections_limit() const {
    return 50; // STUB
}

int TorrentStatus::get_active_time() const {
    return 0; // STUB
}

int TorrentStatus::get_seeding_time() const {
    return 0; // STUB
}

int TorrentStatus::get_time_since_download() const {
    return 0; // STUB
}

int TorrentStatus::get_time_since_upload() const {
    return 0; // STUB
}

int TorrentStatus::get_num_pieces() const {
    return 100; // STUB
}

int TorrentStatus::get_pieces_downloaded() const {
    return static_cast<int>(_progress * 100);
}

int TorrentStatus::get_queue_position() const {
    return 0; // STUB
}

String TorrentStatus::get_error() const {
    return ""; // STUB: No error
}

String TorrentStatus::get_save_path() const {
    return "/tmp"; // STUB
}

String TorrentStatus::get_name() const {
    return "STUB: Test Torrent";
}

float TorrentStatus::get_distributed_copies() const {
    return 1.0f; // STUB
}

void TorrentStatus::_set_internal_status(const void* status) {
    _valid = (status != nullptr);
}