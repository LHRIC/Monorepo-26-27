# How often the display redraws, in Hz. Higher values feel more responsive
# but burn more CPU/refresh cycles for no real benefit past ~30.
refresh_rate_hz = 30

# Units shown on screen. Affects speed, temperature, and pressure readouts.
unit_system = "imperial"  # "imperial" or "metric"

# Visual theme applied at boot.
theme = "Midnight Blue"

# Screen brightness, 0-100. Some displays also support "auto" to follow
# an ambient light sensor if one is present.
brightness = 80

# If no new CAN frame is received for a value within this window (ms),
# the display shows it as stale (e.g. grayed out or flashing) instead
# of a silently frozen last-known reading.
stale_timeout_ms = 500

# The display only accepts measurements whose CAN IDs appear in this list.
# Find the available IDs with ecu_protocol.
allow_list = [3, 7, 8]

# Values below/above these thresholds trigger a visual warning state
# (e.g. red text, flashing icon) rather than a plain readout.
warning_thresholds = {
    "coolant_temp_c": {"max": 115},
    "battery_voltage_v": {"min": 11.0},
    "engine_rpm": {"max": 6500},
}

# Whether to log every accepted CAN frame to disk for later debugging.
# Off by default since it adds I/O overhead on every tick.
enable_data_logging = False

# Simulated/demo mode: if True, ignore real CAN input and generate
# synthetic values instead. Useful for bench testing the UI alone.
demo_mode = False