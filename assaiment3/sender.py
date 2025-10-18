import threading
import time
import random
from channel import channel_busy, channel_lock

# Shared, thread-safe collision counter (using a mutable list)
collision_count = [0]
collision_lock = threading.Lock()

def sender(sender_id, p, num_frames, frame_queue, collision_prob=0.1, max_backoff=10, time_slot=0.01):
    global channel_busy, collision_count

    for frame_num in range(1, num_frames + 1):
        transmitted = False
        while not transmitted:
            with channel_lock:
                busy = channel_busy
            if not busy:
                if random.random() <= p:
                    with channel_lock:
                        channel_busy = True

                    # Attempt transmission
                    if random.random() < collision_prob:
                        print(f"Frame {frame_num} collided (Sender {sender_id})")
                        with collision_lock:
                            collision_count[0] += 1   # safely increment shared counter
                        with channel_lock:
                            channel_busy = False
                        backoff_slots = random.randint(0, max_backoff)
                        time.sleep(backoff_slots * time_slot)
                    else:
                        start_time = time.time()
                        frame_queue.append((frame_num, start_time))
                        with channel_lock:
                            channel_busy = False
                        transmitted = True
                else:
                    time.sleep(time_slot)
            else:
                time.sleep(time_slot)
