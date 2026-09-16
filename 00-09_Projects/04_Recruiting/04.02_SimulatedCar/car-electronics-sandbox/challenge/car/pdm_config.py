# Simplified PDM shutdown-loop configuration.
# True means the loop is continuous; False means a break is detected.
shutdown_continuous = False

# How often the control loop services all channels, in Hz. Real fault
# response time is roughly 1000 / control_loop_hz milliseconds.
control_loop_hz = 100

# Hardware-level instantaneous trip current, in amps. This mirrors a
# fixed hardware comparator threshold and generally shouldn't be
# changed without board-level justification.
hardware_trip_current_a = 40.0

# Software-level trip current, in amps. Lower than the hardware
# threshold so logic-driven protection can act before it does.
software_trip_current_a = 25.0

# How many consecutive over-threshold samples are required before
# tripping a channel. Guards against false trips from sensor noise
# or momentary inrush current (e.g. motor/pump startup).
trip_debounce_samples = 3

# Whether a channel that trips stays open until a manual reset command,
# or automatically retries closing after retry_delay_ms.
auto_retry_on_trip = False
retry_delay_ms = 2000

# Max automatic retries before a channel is latched open permanently
# and requires a manual reset. Prevents rapid trip/retry cycling.
max_auto_retries = 3

# Whether to log every trip event (channel, current, timestamp) to
# disk for later fault analysis.
enable_fault_logging = True

# Simulated/demo mode: if True, ignore real current sensing and
# generate synthetic values instead. Useful for bench testing logic
# without live 12V hardware.
demo_mode = False