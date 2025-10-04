#include "torrent_error.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void TorrentError::_bind_methods() {
    ClassDB::bind_static_method("TorrentError", D_METHOD("create", "code", "message", "context"), &TorrentError::create, DEFVAL(""), DEFVAL(""));
    ClassDB::bind_static_method("TorrentError", D_METHOD("from_libtorrent_error", "ec_value", "ec_message", "context"), &TorrentError::from_libtorrent_error, DEFVAL(""));

    ClassDB::bind_method(D_METHOD("get_code"), &TorrentError::get_code);
    ClassDB::bind_method(D_METHOD("get_category"), &TorrentError::get_category);
    ClassDB::bind_method(D_METHOD("get_message"), &TorrentError::get_message);
    ClassDB::bind_method(D_METHOD("get_context"), &TorrentError::get_context);
    ClassDB::bind_method(D_METHOD("is_error"), &TorrentError::is_error);
    ClassDB::bind_method(D_METHOD("is_recoverable"), &TorrentError::is_recoverable);
    ClassDB::bind_method(D_METHOD("to_dict"), &TorrentError::to_dict);

    // Bind error categories
    BIND_ENUM_CONSTANT(NONE);
    BIND_ENUM_CONSTANT(SESSION_ERROR);
    BIND_ENUM_CONSTANT(TORRENT_ERROR);
    BIND_ENUM_CONSTANT(NETWORK_ERROR);
    BIND_ENUM_CONSTANT(STORAGE_ERROR);
    BIND_ENUM_CONSTANT(PARSE_ERROR);
    BIND_ENUM_CONSTANT(VALIDATION_ERROR);
    BIND_ENUM_CONSTANT(TRACKER_ERROR);
    BIND_ENUM_CONSTANT(DHT_ERROR);
    BIND_ENUM_CONSTANT(PEER_ERROR);
    BIND_ENUM_CONSTANT(INTERNAL_ERROR);

    // Bind common error codes
    BIND_ENUM_CONSTANT(OK);
    BIND_ENUM_CONSTANT(SESSION_NOT_RUNNING);
    BIND_ENUM_CONSTANT(SESSION_ALREADY_RUNNING);
    BIND_ENUM_CONSTANT(SESSION_START_FAILED);
    BIND_ENUM_CONSTANT(INVALID_TORRENT_HANDLE);
    BIND_ENUM_CONSTANT(INVALID_TORRENT_FILE);
    BIND_ENUM_CONSTANT(INVALID_MAGNET_URI);
    BIND_ENUM_CONSTANT(TORRENT_ADD_FAILED);
    BIND_ENUM_CONSTANT(INVALID_PATH);
    BIND_ENUM_CONSTANT(EMPTY_SAVE_PATH);
    BIND_ENUM_CONSTANT(INVALID_PARAMETER);
    BIND_ENUM_CONSTANT(UNKNOWN_ERROR);
}

TorrentError::TorrentError() {
    _code = OK;
    _category = NONE;
    _recoverable = true;
}

TorrentError::~TorrentError() {
}

Ref<TorrentError> TorrentError::create(Code code, const String& message, const String& context) {
    Ref<TorrentError> error;
    error.instantiate();
    error->_init_error(code, message, context);
    return error;
}

Ref<TorrentError> TorrentError::from_libtorrent_error(int ec_value, const String& ec_message, const String& context) {
    Ref<TorrentError> error;
    error.instantiate();

    // Map libtorrent error codes to our error codes
    Code mapped_code = UNKNOWN_ERROR;

    // Common libtorrent error categories
    if (ec_value == 0) {
        mapped_code = OK;
    } else if (ec_message.contains("invalid torrent file") || ec_message.contains("parse")) {
        mapped_code = INVALID_TORRENT_FILE;
    } else if (ec_message.contains("invalid magnet")) {
        mapped_code = INVALID_MAGNET_URI;
    } else if (ec_message.contains("permission") || ec_message.contains("access denied")) {
        mapped_code = PERMISSION_DENIED;
    } else if (ec_message.contains("disk full") || ec_message.contains("space")) {
        mapped_code = DISK_FULL;
    } else if (ec_message.contains("timeout")) {
        mapped_code = TIMEOUT;
    } else if (ec_message.contains("tracker")) {
        mapped_code = TRACKER_ANNOUNCE_FAILED;
    } else if (ec_message.contains("network") || ec_message.contains("connection")) {
        mapped_code = CONNECTION_FAILED;
    }

    String full_message = ec_message;
    if (!ec_message.is_empty() && ec_value != 0) {
        full_message = "libtorrent error " + String::num(ec_value) + ": " + ec_message;
    }

    error->_init_error(mapped_code, full_message, context);
    return error;
}

int TorrentError::get_code() const {
    return static_cast<int>(_code);
}

int TorrentError::get_category() const {
    return static_cast<int>(_category);
}

String TorrentError::get_message() const {
    return _message;
}

String TorrentError::get_context() const {
    return _context;
}

bool TorrentError::is_error() const {
    return _code != OK;
}

bool TorrentError::is_recoverable() const {
    return _recoverable;
}

Dictionary TorrentError::to_dict() const {
    Dictionary dict;
    dict["code"] = static_cast<int>(_code);
    dict["category"] = get_category_name(_category);
    dict["message"] = _message;
    dict["context"] = _context;
    dict["is_error"] = is_error();
    dict["is_recoverable"] = _recoverable;
    return dict;
}

TorrentError::Category TorrentError::get_category_for_code(Code code) {
    if (code >= 100 && code < 200) return SESSION_ERROR;
    if (code >= 200 && code < 300) return TORRENT_ERROR;
    if (code >= 300 && code < 400) return NETWORK_ERROR;
    if (code >= 400 && code < 500) return STORAGE_ERROR;
    if (code >= 500 && code < 600) return PARSE_ERROR;
    if (code >= 600 && code < 700) return VALIDATION_ERROR;
    if (code >= 700 && code < 800) return TRACKER_ERROR;
    if (code >= 800 && code < 900) return DHT_ERROR;
    if (code >= 900 && code < 1000) return PEER_ERROR;
    if (code >= 1000) return INTERNAL_ERROR;
    return NONE;
}

String TorrentError::get_category_name(Category category) {
    switch (category) {
        case NONE: return "none";
        case SESSION_ERROR: return "session";
        case TORRENT_ERROR: return "torrent";
        case NETWORK_ERROR: return "network";
        case STORAGE_ERROR: return "storage";
        case PARSE_ERROR: return "parse";
        case VALIDATION_ERROR: return "validation";
        case TRACKER_ERROR: return "tracker";
        case DHT_ERROR: return "dht";
        case PEER_ERROR: return "peer";
        case INTERNAL_ERROR: return "internal";
        default: return "unknown";
    }
}

String TorrentError::get_default_message(Code code) {
    switch (code) {
        case OK: return "Success";
        case SESSION_NOT_RUNNING: return "Session is not running";
        case SESSION_ALREADY_RUNNING: return "Session is already running";
        case SESSION_START_FAILED: return "Failed to start session";
        case SESSION_STOP_FAILED: return "Failed to stop session";
        case INVALID_TORRENT_HANDLE: return "Invalid torrent handle";
        case INVALID_TORRENT_FILE: return "Invalid torrent file";
        case INVALID_MAGNET_URI: return "Invalid magnet URI";
        case TORRENT_ADD_FAILED: return "Failed to add torrent";
        case TORRENT_REMOVE_FAILED: return "Failed to remove torrent";
        case TORRENT_NOT_FOUND: return "Torrent not found";
        case NETWORK_INIT_FAILED: return "Network initialization failed";
        case PORT_BINDING_FAILED: return "Failed to bind port";
        case CONNECTION_FAILED: return "Connection failed";
        case TIMEOUT: return "Operation timed out";
        case INVALID_PATH: return "Invalid path";
        case PATH_NOT_FOUND: return "Path not found";
        case PERMISSION_DENIED: return "Permission denied";
        case DISK_FULL: return "Disk full";
        case STORAGE_MOVE_FAILED: return "Failed to move storage";
        case FILE_RENAME_FAILED: return "Failed to rename file";
        case BENCODE_PARSE_ERROR: return "Bencode parsing error";
        case TORRENT_INFO_PARSE_ERROR: return "Torrent info parsing error";
        case RESUME_DATA_PARSE_ERROR: return "Resume data parsing error";
        case DHT_STATE_PARSE_ERROR: return "DHT state parsing error";
        case INVALID_PARAMETER: return "Invalid parameter";
        case EMPTY_SAVE_PATH: return "Save path is empty";
        case INVALID_PIECE_INDEX: return "Invalid piece index";
        case INVALID_FILE_INDEX: return "Invalid file index";
        case INVALID_PRIORITY: return "Invalid priority value";
        case INVALID_URL: return "Invalid URL";
        case TRACKER_ANNOUNCE_FAILED: return "Tracker announce failed";
        case TRACKER_SCRAPE_FAILED: return "Tracker scrape failed";
        case TRACKER_INVALID_RESPONSE: return "Invalid tracker response";
        case DHT_START_FAILED: return "Failed to start DHT";
        case DHT_BOOTSTRAP_FAILED: return "DHT bootstrap failed";
        case PEER_CONNECTION_FAILED: return "Peer connection failed";
        case PEER_BANNED: return "Peer is banned";
        case INTERNAL_EXCEPTION: return "Internal exception occurred";
        case UNKNOWN_ERROR: return "Unknown error occurred";
        default: return "Unspecified error";
    }
}

void TorrentError::_init_error(Code code, const String& message, const String& context) {
    _code = code;
    _category = get_category_for_code(code);
    _context = context;

    // Use provided message or default
    if (message.is_empty()) {
        _message = get_default_message(code);
    } else {
        _message = message;
    }

    // Determine if error is recoverable
    _recoverable = true;
    switch (_category) {
        case VALIDATION_ERROR:
        case PARSE_ERROR:
            _recoverable = false; // User needs to fix input
            break;
        case INTERNAL_ERROR:
            _recoverable = false; // System error
            break;
        case STORAGE_ERROR:
            if (code == DISK_FULL || code == PERMISSION_DENIED) {
                _recoverable = false; // User needs to fix system
            }
            break;
        default:
            _recoverable = true; // Most errors are recoverable (retry, etc.)
            break;
    }
}
