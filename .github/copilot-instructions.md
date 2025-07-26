# OBS Plugin Development Instructions

This is an OBS Studio plugin template based on CMake, Qt6, and the OBS frontend API. The plugin is named "PlayFame" and demonstrates best practices for OBS plugin architecture.

## Architecture Overview

### Core Components
- **plugin-main.cpp**: Entry point with `obs_module_load()` and `obs_module_unload()` - handles plugin lifecycle
- **PlayFameDock**: Qt-based dockable widget (`plugin-dock.cpp/h`) - main UI component
- **OBSConfigHelper**: Configuration wrapper (`obs-config-helper.cpp/h`) - manages settings with OBS data APIs
- **ConfigDialog**: Settings UI (`config-dialog.cpp/h`) - modal configuration interface

### Thread Safety Pattern
The plugin uses Qt's thread-safe patterns:
- UI destruction in `destroy_dock_safe()` uses `QMetaObject::invokeMethod` with `Qt::BlockingQueuedConnection`
- Dock creation happens via `Qt::QueuedConnection` to ensure UI thread execution
- Frontend event callbacks handle cleanup during `OBS_FRONTEND_EVENT_EXIT`

### Configuration Pattern
Uses OBS's native `obs_data_t` APIs rather than raw JSON files:
- `OBSConfigHelper` wraps `obs_data_create()`, `obs_data_get_*()`, `obs_data_set_*()`
- Supports sections, type validation, and min/max constraints
- Automatically saves to OBS config directory

## Build System

### Local Development (macOS)
```bash
./cmakeagain.sh  # Clean build with Xcode generator
```

### Key CMake Variables
- `ENABLE_FRONTEND_API=ON` - Required for dock functionality
- `ENABLE_QT=ON` - Required for Qt widgets
- `OBS_STUDIO_PATH` - Points to OBS SDK location
- `CMAKE_OSX_ARCHITECTURES="arm64"` - Target architecture

### Dependencies
- **libobs**: Core OBS functionality
- **obs-frontend-api**: UI integration (`obs_frontend_add_dock_by_id`)
- **Qt6**: Widgets and core functionality
- **Firebase SDK**: External static libraries in `external/firebase_cpp_sdk/`

### Xcode Integration
The CMakeLists.txt includes post-build steps that automatically:
1. Copy built plugin to `~/Library/Application Support/obs-studio/plugins/`
2. Code sign the plugin bundle for local testing

## Code Patterns

### Plugin Registration
```cpp
// In obs_module_load():
QMetaObject::invokeMethod(mainWindow, [mainWindow]() {
    auto *dock = new PlayFameDock(g_plugin_config, mainWindow);
    if (!dock->registerDock()) {
        dock->deleteLater();
    } else {
        g_main_dock = dock;
    }
}, Qt::QueuedConnection);
```

### Configuration Usage
```cpp
// Type-safe configuration with validation
config->setValue("user", "volume", 75, QMetaType::Int, 0, 100);
int volume = config->getValue("user", "volume", 50).toInt();
```

### OBS Frontend Integration
- Use `obs_frontend_get_main_window()` for parent widget
- Register docks with `obs_frontend_add_dock_by_id()`
- Listen for `OBS_FRONTEND_EVENT_EXIT` to clean up UI safely

## Development Workflow

### File Structure
- `src/`: All plugin source code
- `cmake/`: Build system configuration per platform
- `external/`: Third-party dependencies (Firebase)
- `data/locale/`: Internationalization files
- `.github/`: CI/CD workflows and build scripts

### Common Tasks
- **Add new UI elements**: Extend `PlayFameDock` constructor
- **Add configuration**: Use `OBSConfigHelper` with validation
- **Platform-specific builds**: Use GitHub Actions workflows or local scripts
- **Testing**: Build outputs to OBS plugin directory automatically on macOS

### Debugging
- Use `obs_log(LOG_INFO, "[playfame] message")` for plugin logging
- Qt debugging works normally when plugin is loaded in OBS
- Check OBS log files for plugin initialization errors

## External Dependencies
- Firebase SDK linked as static libraries from `external/firebase_cpp_sdk/libs/darwin/`
- Requires macOS frameworks: CoreFoundation, Foundation, Security, SystemConfiguration
- buildspec.json defines OBS Studio and Qt6 dependency versions
