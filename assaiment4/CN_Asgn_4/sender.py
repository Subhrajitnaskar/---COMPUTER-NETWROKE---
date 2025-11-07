from channel import channel_lock, channel_signal

def sender(sender_id, walsh_code, data_bit):
    """Encodes a data bit and adds it to the channel."""
    # Convert data bit to bipolar (+1, -1) or 0 for silence
    if data_bit == 1:
        data_bipolar = 1
    elif data_bit == 0:
        data_bipolar = -1
    else:  # None or 's' indicates silence
        data_bipolar = 0

    # Create the sender's signal by multiplying data with its Walsh code
    encoded_signal = [chip * data_bipolar for chip in walsh_code]

    print(f"Sender {sender_id}: Sending '{data_bit}' as {encoded_signal}")

    # Add the signal to the shared channel safely
    with channel_lock:
        for i in range(len(channel_signal)):
            channel_signal[i] += encoded_signal[i]
