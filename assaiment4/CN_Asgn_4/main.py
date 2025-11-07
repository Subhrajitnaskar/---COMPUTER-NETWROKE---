import threading
import random
from channel import channel_signal, NUM_STATIONS
from sender import sender
from receiver import receiver  # renamed to avoid import error

def generate_walsh_table(n):
    """Generates a Walsh-Hadamard matrix of size n x n (n must be a power of 2)."""
    if n == 1:
        return [[1]]
    
    W_half = generate_walsh_table(n // 2)
    
    W_full = []
    for i in range(n // 2):
        W_full.append(W_half[i] + W_half[i])
    for i in range(n // 2):
        neg_W_half_row = [-x for x in W_half[i]]
        W_full.append(W_half[i] + neg_W_half_row)
        
    return W_full

def next_power_of_2(x):
    """Returns the next power of 2 greater than or equal to x."""
    power = 1
    while power < x:
        power *= 2
    return power

def main():
    """Runs the CDMA simulation."""

    actual_stations = NUM_STATIONS
    N = next_power_of_2(actual_stations)  # next power of 2
    print(f"Actual Stations: {actual_stations}, Using Walsh size: {N}")

    # Initialize channel
    channel_signal.extend([0] * N)

    # Generate Walsh codes
    walsh_table = generate_walsh_table(N)

    # Assign random 0/1/None (None = silence) for actual stations
    data_to_send = [random.choice([0, 1, None]) for _ in range(actual_stations)]

    # Pad extra stations with None (silence)
    for _ in range(N - actual_stations):
        data_to_send.append(None)

    decoded_data = [None] * N

    # Display setup
    print("--- CDMA Simulation Setup ---")
    for i in range(N):
        print(f"Station {i}: Walsh Code = {walsh_table[i]}, Data to Send = {data_to_send[i]}")
    print("-" * 30 + "\n")

    # Transmission phase
    print("--- Transmission Phase ---")
    sender_threads = []
    for i in range(N):
        thread = threading.Thread(target=sender, args=(i, walsh_table[i], data_to_send[i]))
        sender_threads.append(thread)
        thread.start()

    for thread in sender_threads:
        thread.join()

    print(f"\nFinal Combined Signal on Channel: {channel_signal}\n")

    # Reception phase
    print("--- Reception Phase ---")
    receiver_threads = []
    for i in range(N):
        thread = threading.Thread(target=receiver, args=(i, walsh_table[i], decoded_data))
        receiver_threads.append(thread)
        thread.start()

    for thread in receiver_threads:
        thread.join()

    # Verification
    print("\n" + "-" * 30)
    print("--- Verification ---")

    correct_decodes = 0
    for i in range(N):
        sent = data_to_send[i]
        decoded = decoded_data[i]
        if sent == decoded or (sent is None and decoded == 'silence'):
            print(f"Station {i}: SUCCESS! Sent: {sent}, Decoded: {decoded}")
            correct_decodes += 1
        else:
            print(f"Station {i}: FAILURE! Sent: {sent}, Decoded: {decoded}")

    print("-" * 30)
    if correct_decodes == N:
        print("All data was transmitted and received successfully!")
    else:
        print(f"Simulation failed. {N - correct_decodes} error(s) occurred.")

if __name__ == "__main__":
    main()
