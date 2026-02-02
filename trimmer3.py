# trimmer_corrected.py
import os

# ----- User parameters -----
orig_file = r"D:\Downloads\clearsky_signal_C-1.bin"      # Original large file
trim_file = r"D:\Downloads\clearsky_signal_C-1_trimmed.bin"  # Output trimmed file

sampling_freq = 5_000_000       # Hz
bytes_per_sample = 2 * 2        # I+Q, int16 (2 bytes each) → 4 bytes per complex sample

trim_bytes_target = 2 * 1024**3  # 2 GB output

# ----- Compute trim duration -----
bytes_per_sec = sampling_freq * bytes_per_sample
trim_duration_sec = trim_bytes_target / bytes_per_sec
num_complex_samples = int(sampling_freq * trim_duration_sec)
num_bytes_to_copy = num_complex_samples * bytes_per_sample

print(f"Trim duration: {trim_duration_sec:.2f} seconds")
print(f"Number of complex samples: {num_complex_samples}")
print(f"Total bytes to copy: {num_bytes_to_copy / 1024**2:.2f} MB")

# ----- Optional: start offset (middle of file recommended for reliable signals) -----
file_size = os.path.getsize(orig_file)
start_offset = max(0, file_size // 2 - num_bytes_to_copy // 2)
print(f"Start offset: {start_offset / 1024**3:.2f} GB")

# ----- Copy data in chunks -----
chunk_size = 50 * 1024**2  # 50 MB
bytes_copied = 0

with open(orig_file, 'rb') as f_in, open(trim_file, 'wb') as f_out:
    f_in.seek(start_offset)
    while bytes_copied < num_bytes_to_copy:
        bytes_left = num_bytes_to_copy - bytes_copied
        read_size = min(chunk_size, bytes_left)
        chunk = f_in.read(read_size)
        if not chunk:
            break
        f_out.write(chunk)
        bytes_copied += len(chunk)

print(f"\nTrimmed IQ file saved: {trim_file}")
print(f"Total bytes written: {bytes_copied / 1024**3:.2f} GB")
