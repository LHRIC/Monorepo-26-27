# How often each board is sampled, in Hz. Higher rates give finer
# resolution (useful for fast-changing signals like wheel speed) at
# the cost of more CAN bus traffic.
sample_rate_hz = 100

# ADC resolution in bits. Higher values give finer measurement
# granularity but require more capable hardware.
adc_bits = 12

# Smoothing factor (0-1) for the exponential moving average filter
# applied to raw samples. Lower values smooth more but respond slower
# to real changes.
smoothing_factor = 0.3

# If no new sample is produced for a board within this window (ms),
# downstream consumers should treat its last value as stale rather
# than trusting a frozen reading.
stale_timeout_ms = 200

# Per-signal engineering-unit ranges used to flag out-of-range or
# implausible readings (e.g. a sensor fault) rather than passing
# them through as valid data.
valid_ranges = {
    "brake_pressure_psi": {"min": 0, "max": 2000},
    "tire_temp_c": {"min": -20, "max": 150},
    "suspension_travel_mm": {"min": 0, "max": 100},
}

# CAN transmit priority/arbitration base offset for this board group,
# so multiple sensor clusters don't collide on the same ID range.
can_id_base = 0x200

# Whether to log every transmitted CAN frame to disk for later
# debugging or telemetry replay.
enable_data_logging = False

# Simulated/demo mode: if True, ignore real sensing elements and
# generate synthetic values instead. Useful for bench testing
# consumers of the CAN data without live hardware attached.
demo_mode = False