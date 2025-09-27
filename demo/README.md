# Demo Applications for Godot-Torrent GDExtension

This directory contains demonstration applications that showcase the functionality of the Godot-Torrent GDExtension.

## 📁 Contents

### Basic Demo (`test_basic.tscn` + `test_basic.gd`)
A simple demonstration covering core functionality:
- Session creation and management
- Basic torrent operations
- Status monitoring
- Core class instantiation

**Run with:**
```bash
godot --headless demo/test_basic.tscn
```

### Advanced Demo (`demo_advanced.tscn` + `demo_advanced.gd`)
A comprehensive demonstration showcasing advanced features:
- Session management with custom configurations
- Torrent operations (files and magnet URIs)
- Real-time progress monitoring
- Alert system demonstration
- Peer management showcase
- File and piece priority management
- Advanced torrent operations

**Run with:**
```bash
godot --headless demo/demo_advanced.tscn
```

## 🚀 What the Demos Show

### Core Functionality Demonstrated:
1. **Session Management**
   - Starting/stopping sessions
   - Custom configuration settings
   - Bandwidth limiting
   - Port configuration
   - DHT management

2. **Torrent Operations**
   - Adding torrents via .torrent files
   - Adding torrents via magnet URIs
   - Pause/resume operations
   - Force recheck and reannounce

3. **Information Access**
   - Real-time status monitoring
   - Torrent metadata access
   - Peer information retrieval
   - Session statistics

4. **Advanced Features**
   - File and piece priority management
   - Alert system processing
   - Comprehensive error handling
   - Resource cleanup

### Sample Output:
```
=== Advanced Godot-Torrent Demo ===

--- Phase 1: Session Management ---
✓ Session created: true
✓ Session started: true
✓ Session stats retrieved: 6 metrics

--- Phase 2: Configuration ---
✓ Bandwidth limits configured
✓ Port range configured: 6881-6889
✓ Custom session started: true

--- Phase 3: Torrent Operations ---
✓ Torrent file added: true
✓ Magnet URI added: true
✓ Testing pause/resume...

--- Phase 4: Alert System ---
✓ Alert manager created
✓ Alert categories configured

--- Phase 5: Peer Management ---
✓ Peer info retrieved: 0 peers

--- Phase 6: Advanced Features ---
✓ Torrent info retrieved
✓ Testing priorities
✓ Testing advanced operations
```

## 🎯 Usage in Your Project

These demos serve as:
1. **Integration Examples** - How to use the API in real applications
2. **Testing Tools** - Verify functionality without GUT
3. **Learning Resources** - Understand best practices
4. **Debugging Aid** - Identify issues during development

## 📝 Notes

- Demos use stub implementations that simulate real behavior
- All API calls are functional and tested
- Output includes "STUB:" prefixes to indicate simulation mode
- Once libtorrent integration is complete, demos will work with real torrents