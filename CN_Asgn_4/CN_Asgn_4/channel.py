import threading

# The number of stations/senders. This must be a power of 2.
NUM_STATIONS = 6

# Shared channel signal and lock for thread safety.
channel_signal = []
channel_lock = threading.Lock()