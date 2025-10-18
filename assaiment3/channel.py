import threading

# Shared channel state
channel_lock = threading.Lock()
channel_busy = False
