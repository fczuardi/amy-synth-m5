export PLATFORMIO_SETTING_ENABLE_TELEMETRY := "no"

juno_app := "apps/ble-midi-amy-juno"
juno_env := "m5stack-core-gray"

# Show available commands.
default:
    @just --list

# Upload the 16-channel BLE MIDI Juno demo to an M5Stack Core Gray.
upload-juno:
    pio run -d {{juno_app}} -e {{juno_env}} --target upload
