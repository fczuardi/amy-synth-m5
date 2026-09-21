export PLATFORMIO_SETTING_ENABLE_TELEMETRY := "no"
export PLATFORMIO_CORE_DIR := "/home/fcz/dev/m5stick/.platformio-home"

juno_app := "apps/ble-midi-amy-juno"
juno_env := "m5stack-core-gray"
drum_app := "apps/manual-drum-browser"
drum_env := "m5stack-core-gray"

# Show available commands.
default:
    @just --list

# Upload the 16-channel BLE MIDI Juno demo to an M5Stack Core Gray.
upload-juno:
    pio run -d {{juno_app}} -e {{juno_env}} --target upload

# Build the manual AMY drum-kit browser for an M5Stack Core Gray.
build-drums:
    pio run -d {{drum_app}} -e {{drum_env}}

# Upload the manual AMY drum-kit browser to an M5Stack Core Gray.
upload-drums:
    pio run -d {{drum_app}} -e {{drum_env}} --target upload
