from channel import channel_signal

def receiver(receiver_id, walsh_code, decoded_results):
    """Decodes data from the channel using the receiver's Walsh code."""
    # Calculate inner product (correlation)
    inner_product = sum(c * s for c, s in zip(walsh_code, channel_signal))

    # Normalize to get bipolar value
    decoded_value = inner_product / len(walsh_code)

    # Convert back to bit or silence
    if decoded_value > 0:
        original_bit = 1
    elif decoded_value < 0:
        original_bit = 0
    else:
        original_bit = 'silence'  # No transmission

    print(f"Receiver {receiver_id}: Decoded '{original_bit}'")
    decoded_results[receiver_id] = original_bit
