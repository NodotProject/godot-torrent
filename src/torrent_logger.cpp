#include "torrent_logger.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/time.hpp>

using namespace godot;

void TorrentLogger::_bind_methods() {
    // Logging control
    ClassDB::bind_method(D_METHOD("enable_logging", "enabled"), &TorrentLogger::enable_logging);
    ClassDB::bind_method(D_METHOD("is_logging_enabled"), &TorrentLogger::is_logging_enabled);
    ClassDB::bind_method(D_METHOD("set_log_level", "level"), &TorrentLogger::set_log_level);
    ClassDB::bind_method(D_METHOD("get_log_level"), &TorrentLogger::get_log_level);

    // Category filtering
    ClassDB::bind_method(D_METHOD("enable_category", "category", "enabled"), &TorrentLogger::enable_category);
    ClassDB::bind_method(D_METHOD("is_category_enabled", "category"), &TorrentLogger::is_category_enabled);
    ClassDB::bind_method(D_METHOD("enable_all_categories"), &TorrentLogger::enable_all_categories);
    ClassDB::bind_method(D_METHOD("disable_all_categories"), &TorrentLogger::disable_all_categories);

    // File logging
    ClassDB::bind_method(D_METHOD("set_log_file", "file_path"), &TorrentLogger::set_log_file);
    ClassDB::bind_method(D_METHOD("close_log_file"), &TorrentLogger::close_log_file);
    ClassDB::bind_method(D_METHOD("is_log_file_enabled"), &TorrentLogger::is_log_file_enabled);

    // Logging methods
    ClassDB::bind_method(D_METHOD("log_error", "message", "category"), &TorrentLogger::log_error, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("log_warning", "message", "category"), &TorrentLogger::log_warning, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("log_info", "message", "category"), &TorrentLogger::log_info, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("log_debug", "message", "category"), &TorrentLogger::log_debug, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("log_trace", "message", "category"), &TorrentLogger::log_trace, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("log", "level", "message", "category"), &TorrentLogger::log, DEFVAL(""));

    // Statistics
    ClassDB::bind_method(D_METHOD("get_log_stats"), &TorrentLogger::get_log_stats);
    ClassDB::bind_method(D_METHOD("reset_log_stats"), &TorrentLogger::reset_log_stats);

    // Bind log levels
    BIND_ENUM_CONSTANT(NONE);
    BIND_ENUM_CONSTANT(ERROR);
    BIND_ENUM_CONSTANT(WARNING);
    BIND_ENUM_CONSTANT(INFO);
    BIND_ENUM_CONSTANT(DEBUG);
    BIND_ENUM_CONSTANT(TRACE);

    // Bind log categories
    BIND_ENUM_CONSTANT(ALL);
    BIND_ENUM_CONSTANT(SESSION);
    BIND_ENUM_CONSTANT(TORRENT);
    BIND_ENUM_CONSTANT(PEER);
    BIND_ENUM_CONSTANT(TRACKER);
    BIND_ENUM_CONSTANT(DHT);
    BIND_ENUM_CONSTANT(PORT_MAPPING);
    BIND_ENUM_CONSTANT(STORAGE);
    BIND_ENUM_CONSTANT(PERFORMANCE);
    BIND_ENUM_CONSTANT(ALERT);
}

TorrentLogger::TorrentLogger() {
    _enabled = false;
    _log_level = WARNING; // Default to warnings and errors
    _file_logging_enabled = false;

    // Enable all categories by default
    for (int i = 0; i < 10; i++) {
        _category_filters[i] = true;
    }

    // Reset statistics
    reset_log_stats();
}

TorrentLogger::~TorrentLogger() {
    close_log_file();
}

void TorrentLogger::enable_logging(bool enabled) {
    _enabled = enabled;
    if (enabled) {
        log_info("Logging enabled", "LOGGER");
    }
}

bool TorrentLogger::is_logging_enabled() const {
    return _enabled;
}

void TorrentLogger::set_log_level(LogLevel level) {
    _log_level = level;
    log_info("Log level set to: " + get_level_name(level), "LOGGER");
}

TorrentLogger::LogLevel TorrentLogger::get_log_level() const {
    return _log_level;
}

void TorrentLogger::enable_category(LogCategory category, bool enabled) {
    if (category >= 0 && category < 10) {
        _category_filters[category] = enabled;
    }
}

bool TorrentLogger::is_category_enabled(LogCategory category) const {
    if (category >= 0 && category < 10) {
        return _category_filters[category];
    }
    return false;
}

void TorrentLogger::enable_all_categories() {
    for (int i = 0; i < 10; i++) {
        _category_filters[i] = true;
    }
}

void TorrentLogger::disable_all_categories() {
    for (int i = 0; i < 10; i++) {
        _category_filters[i] = false;
    }
}

void TorrentLogger::set_log_file(const String& file_path) {
    std::lock_guard<std::mutex> lock(_log_mutex);

    close_log_file();

    _log_file = FileAccess::open(file_path, FileAccess::WRITE);
    if (_log_file.is_valid()) {
        _log_file_path = file_path;
        _file_logging_enabled = true;

        // Write header
        String header = "=== godot-torrent Log File ===\n";
        header += "Started: " + get_timestamp() + "\n";
        header += "==============================\n\n";
        _log_file->store_string(header);
        _log_file->flush();

        log_info("Log file opened: " + file_path, "LOGGER");
    } else {
        UtilityFunctions::push_error("Failed to open log file: " + file_path);
    }
}

void TorrentLogger::close_log_file() {
    std::lock_guard<std::mutex> lock(_log_mutex);

    if (_log_file.is_valid()) {
        _log_file->close();
        _log_file.unref();
        _file_logging_enabled = false;
        _log_file_path = "";
    }
}

bool TorrentLogger::is_log_file_enabled() const {
    return _file_logging_enabled;
}

void TorrentLogger::log_error(const String& message, const String& category) {
    log(ERROR, message, category);
}

void TorrentLogger::log_warning(const String& message, const String& category) {
    log(WARNING, message, category);
}

void TorrentLogger::log_info(const String& message, const String& category) {
    log(INFO, message, category);
}

void TorrentLogger::log_debug(const String& message, const String& category) {
    log(DEBUG, message, category);
}

void TorrentLogger::log_trace(const String& message, const String& category) {
    log(TRACE, message, category);
}

void TorrentLogger::log(LogLevel level, const String& message, const String& category) {
    if (!should_log(level, category)) {
        return;
    }

    // Update statistics
    switch (level) {
        case ERROR: _log_count_error++; break;
        case WARNING: _log_count_warning++; break;
        case INFO: _log_count_info++; break;
        case DEBUG: _log_count_debug++; break;
        case TRACE: _log_count_trace++; break;
        default: break;
    }

    String formatted = format_log_message(level, message, category);
    write_to_console(level, formatted);

    if (_file_logging_enabled) {
        write_to_file(formatted);
    }
}

void TorrentLogger::process_libtorrent_alert(int alert_type, const String& alert_message) {
    if (!_enabled) {
        return;
    }

    LogLevel level = get_alert_log_level(alert_type);
    String category = get_alert_category(alert_type);

    log(level, alert_message, category);
}

Dictionary TorrentLogger::get_log_stats() const {
    Dictionary stats;
    stats["enabled"] = _enabled;
    stats["log_level"] = static_cast<int>(_log_level);
    stats["file_logging"] = _file_logging_enabled;
    stats["log_file_path"] = _log_file_path;
    stats["error_count"] = _log_count_error;
    stats["warning_count"] = _log_count_warning;
    stats["info_count"] = _log_count_info;
    stats["debug_count"] = _log_count_debug;
    stats["trace_count"] = _log_count_trace;
    stats["total_count"] = _log_count_error + _log_count_warning + _log_count_info +
                          _log_count_debug + _log_count_trace;
    return stats;
}

void TorrentLogger::reset_log_stats() {
    _log_count_error = 0;
    _log_count_warning = 0;
    _log_count_info = 0;
    _log_count_debug = 0;
    _log_count_trace = 0;
}

bool TorrentLogger::should_log(LogLevel level, const String& category) const {
    if (!_enabled || level == NONE) {
        return false;
    }

    // Check log level
    if (level > _log_level) {
        return false;
    }

    // Check category filter (if category is specified)
    if (!category.is_empty()) {
        // Simple category matching
        if (category == "SESSION" && !_category_filters[SESSION]) return false;
        if (category == "TORRENT" && !_category_filters[TORRENT]) return false;
        if (category == "PEER" && !_category_filters[PEER]) return false;
        if (category == "TRACKER" && !_category_filters[TRACKER]) return false;
        if (category == "DHT" && !_category_filters[DHT]) return false;
        if (category == "PORT_MAPPING" && !_category_filters[PORT_MAPPING]) return false;
        if (category == "STORAGE" && !_category_filters[STORAGE]) return false;
        if (category == "PERFORMANCE" && !_category_filters[PERFORMANCE]) return false;
        if (category == "ALERT" && !_category_filters[ALERT]) return false;
    }

    return true;
}

String TorrentLogger::format_log_message(LogLevel level, const String& message, const String& category) const {
    String formatted = "[" + get_timestamp() + "] ";
    formatted += "[" + get_level_name(level) + "]";

    if (!category.is_empty()) {
        formatted += " [" + category + "]";
    }

    formatted += " " + message;
    return formatted;
}

String TorrentLogger::get_level_name(LogLevel level) const {
    switch (level) {
        case NONE: return "NONE";
        case ERROR: return "ERROR";
        case WARNING: return "WARN";
        case INFO: return "INFO";
        case DEBUG: return "DEBUG";
        case TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}

String TorrentLogger::get_timestamp() const {
    Dictionary time_dict = Time::get_singleton()->get_datetime_dict_from_system();

    int year = time_dict["year"];
    int month = time_dict["month"];
    int day = time_dict["day"];
    int hour = time_dict["hour"];
    int minute = time_dict["minute"];
    int second = time_dict["second"];

    return String::num(year) + "-" +
           String::num(month).pad_zeros(2) + "-" +
           String::num(day).pad_zeros(2) + " " +
           String::num(hour).pad_zeros(2) + ":" +
           String::num(minute).pad_zeros(2) + ":" +
           String::num(second).pad_zeros(2);
}

void TorrentLogger::write_to_console(LogLevel level, const String& formatted_message) {
    switch (level) {
        case ERROR:
            UtilityFunctions::push_error(formatted_message);
            break;
        case WARNING:
            UtilityFunctions::push_warning(formatted_message);
            break;
        case INFO:
        case DEBUG:
        case TRACE:
            UtilityFunctions::print(formatted_message);
            break;
        default:
            break;
    }
}

void TorrentLogger::write_to_file(const String& formatted_message) {
    std::lock_guard<std::mutex> lock(_log_mutex);

    if (_log_file.is_valid()) {
        _log_file->store_line(formatted_message);
        _log_file->flush();
    }
}

TorrentLogger::LogLevel TorrentLogger::get_alert_log_level(int alert_type) const {
    // Map alert types to log levels
    // This is a simplified mapping - can be expanded based on libtorrent alert types
    // Alert types from libtorrent::alert

    // Error alerts (0-50 range typically)
    if (alert_type >= 0 && alert_type <= 10) {
        return ERROR;
    }
    // Warning alerts (50-100 range)
    else if (alert_type > 10 && alert_type <= 50) {
        return WARNING;
    }
    // Info alerts (100-200 range)
    else if (alert_type > 50 && alert_type <= 150) {
        return INFO;
    }
    // Debug alerts (200+ range)
    else {
        return DEBUG;
    }
}

String TorrentLogger::get_alert_category(int alert_type) const {
    // This is a simplified categorization
    // In practice, you'd need to check specific alert type values from libtorrent
    return "ALERT";
}
