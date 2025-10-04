#include "torrent_result.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TorrentResult::_bind_methods() {
    ClassDB::bind_static_method("TorrentResult", D_METHOD("ok", "value"), &TorrentResult::ok, DEFVAL(Variant()));
    ClassDB::bind_static_method("TorrentResult", D_METHOD("error", "error"), &TorrentResult::error);
    ClassDB::bind_static_method("TorrentResult", D_METHOD("error_code", "code", "message", "context"), &TorrentResult::error_code, DEFVAL(""), DEFVAL(""));

    ClassDB::bind_method(D_METHOD("is_ok"), &TorrentResult::is_ok);
    ClassDB::bind_method(D_METHOD("is_error"), &TorrentResult::is_error);
    ClassDB::bind_method(D_METHOD("get_value"), &TorrentResult::get_value);
    ClassDB::bind_method(D_METHOD("get_error"), &TorrentResult::get_error);
    ClassDB::bind_method(D_METHOD("to_dict"), &TorrentResult::to_dict);
    ClassDB::bind_method(D_METHOD("to_string"), &TorrentResult::to_string);
    ClassDB::bind_method(D_METHOD("unwrap"), &TorrentResult::unwrap);
    ClassDB::bind_method(D_METHOD("unwrap_or", "default_value"), &TorrentResult::unwrap_or);
}

TorrentResult::TorrentResult() {
    _ok = true;
    _value = Variant();
    _error.instantiate();
}

TorrentResult::~TorrentResult() {
}

Ref<TorrentResult> TorrentResult::ok(const Variant& value) {
    Ref<TorrentResult> result;
    result.instantiate();
    result->_ok = true;
    result->_value = value;
    result->_error = TorrentError::create(TorrentError::OK);
    return result;
}

Ref<TorrentResult> TorrentResult::error(Ref<TorrentError> error) {
    Ref<TorrentResult> result;
    result.instantiate();
    result->_ok = false;
    result->_value = Variant();
    result->_error = error;
    return result;
}

Ref<TorrentResult> TorrentResult::error_code(TorrentError::Code code, const String& message, const String& context) {
    return error(TorrentError::create(code, message, context));
}

bool TorrentResult::is_ok() const {
    return _ok;
}

bool TorrentResult::is_error() const {
    return !_ok;
}

Variant TorrentResult::get_value() const {
    return _value;
}

Ref<TorrentError> TorrentResult::get_error() const {
    return _error;
}

Dictionary TorrentResult::to_dict() const {
    Dictionary dict;
    dict["ok"] = _ok;
    if (_ok) {
        dict["value"] = _value;
    } else {
        dict["error"] = _error->to_dict();
    }
    return dict;
}

String TorrentResult::to_string() const {
    if (_ok) {
        return "TorrentResult::Ok(" + _value.stringify() + ")";
    } else {
        return "TorrentResult::Error(" + _error->get_message() + ")";
    }
}

Variant TorrentResult::unwrap() {
    if (_ok) {
        return _value;
    } else {
        UtilityFunctions::push_error("Attempted to unwrap error result: " + _error->get_message());
        return Variant();
    }
}

Variant TorrentResult::unwrap_or(const Variant& default_value) {
    if (_ok) {
        return _value;
    } else {
        return default_value;
    }
}
