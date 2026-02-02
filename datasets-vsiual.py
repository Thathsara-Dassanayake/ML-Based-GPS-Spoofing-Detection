import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import welch
import os 

# --- Configuration ---
BASE_DIR = r'D:\Downloads' 

FILE_PATHS = {
    'Clear-Sky': os.path.join(BASE_DIR, 'clearsky_signal_C-1.bin'),
    'Galileo Spoofer': os.path.join(BASE_DIR, 'Galileo_3_Spoofer_Static_NoMP_TruePosition.bin')
}
SAMPLING_FREQUENCY = 50e6  # 50 MSps (from dataset description)

# --- Sample Sizes for Visualization ---
CHUNK_SIZE_SAMPLES_FOR_VIS = 2048   # Number of complex samples to read for general visualization
CHUNK_OFFSET_SAMPLES = 0            # Offset from the beginning of the file to read the chunk
TIME_PLOT_SAMPLES = 500             # Number of samples for the time-domain plot

# --- Data Cleaning & Scaling Parameters ---
OUTLIER_THRESHOLD = 1e20 # Remove I/Q values (and thus magnitudes) above this
TARGET_SCALE_MAX_ABS = 1.0 # Target maximum absolute value for scaled I/Q components (e.g., -1 to 1)

# --- Helper Function to Read IQ Data ---
def read_iq_data_chunk(filepath, num_samples, offset_samples=0, sample_dtype=np.float32):
    """
    Reads a specified number of interleaved I/Q samples (32-bit float) from a binary file,
    starting at a given offset. Converts them into complex numbers.
    Handles NaN values and extreme outliers by removing them.

    Args:
        filepath (str): Path to the .bin file.
        num_samples (int): Total number of I/Q pairs (complex samples) to read.
        offset_samples (int): Number of I/Q pairs to skip from the beginning of the file.
        sample_dtype: Data type of each I or Q component (e.g., np.float32).

    Returns:
        np.ndarray: A 1D array of complex IQ samples after cleaning.
    """
    bytes_per_sample_component = np.dtype(sample_dtype).itemsize
    bytes_per_complex_sample = 2 * bytes_per_sample_component

    start_byte = offset_samples * bytes_per_complex_sample
    read_bytes = num_samples * bytes_per_complex_sample

    try:
        with open(filepath, 'rb') as f:
            f.seek(start_byte)
            raw_data = f.read(read_bytes)
            
            iq_interleaved = np.frombuffer(raw_data, dtype=sample_dtype)
            
            if iq_interleaved.size % 2 != 0:
                print(f"Warning: Odd number of samples read from {filepath}. Truncating last sample.")
                iq_interleaved = iq_interleaved[:-1]
                
            i_samples = iq_interleaved[0::2]
            q_samples = iq_interleaved[1::2]
            
            complex_iq = i_samples + 1j * q_samples
            
            initial_len = len(complex_iq)
            
            # --- Outlier Removal ---
            # Remove NaN values first
            complex_iq = complex_iq[~np.isnan(complex_iq)]
            
            # Remove samples where the absolute value of I or Q components is excessively large
            valid_indices = (np.abs(complex_iq.real) < OUTLIER_THRESHOLD) & \
                            (np.abs(complex_iq.imag) < OUTLIER_THRESHOLD)
            complex_iq = complex_iq[valid_indices]

            removed_count = initial_len - len(complex_iq)
            if removed_count > 0:
                print(f"Removed {removed_count} NaN/Outlier samples from {filepath}.")
            
            return complex_iq

    except FileNotFoundError:
        print(f"Error: File not found at {filepath}")
        return None
    except Exception as e:
        print(f"An error occurred while reading {filepath}: {e}")
        return None

# --- Helper Function to Scale IQ Data ---
def scale_iq_data(iq_samples, target_max_abs):
    """
    Scales the complex IQ samples such that their maximum absolute real or imaginary
    component (whichever is largest) is equal to target_max_abs.
    This effectively scales the data to the range [-target_max_abs, target_max_abs]
    while preserving the relative I/Q relationships.
    """
    if len(iq_samples) == 0:
        return iq_samples

    # Find the largest absolute value among all I and Q components
    max_val_i = np.max(np.abs(iq_samples.real))
    max_val_q = np.max(np.abs(iq_samples.imag))
    overall_max_abs = max(max_val_i, max_val_q)

    if overall_max_abs == 0:
        return iq_samples # Avoid division by zero if all samples are zero

    # Scale all samples proportionally
    scaled_iq_samples = (iq_samples / overall_max_abs) * target_max_abs
    return scaled_iq_samples

# --- Main Plotting Logic ---
if __name__ == "__main__":
    plt.style.use('ggplot') 

    data = {}
    for name, path in FILE_PATHS.items():
        print(f"\nProcessing '{name}'...")
        # 1. Read the chunk of data
        iq_chunk = read_iq_data_chunk(path, CHUNK_SIZE_SAMPLES_FOR_VIS, CHUNK_OFFSET_SAMPLES)
        
        if iq_chunk is None or len(iq_chunk) == 0:
            print(f"Skipping plotting for {name} due to data loading issues or all samples removed.")
            data[name] = None 
            continue
        
        # 2. Scale the data
        scaled_iq_chunk = scale_iq_data(iq_chunk, TARGET_SCALE_MAX_ABS)
        data[name] = scaled_iq_chunk # Store the scaled data

        print(f"Loaded and scaled {len(data[name])} samples for '{name}'.")


    # --- Generate Plots ---
    
    # Filter out files that failed to load/process
    valid_data_keys = [key for key, value in data.items() if value is not None]
    if not valid_data_keys:
        print("No valid data found to plot. Exiting.")
        exit()

    # 1. Time Domain Magnitude Plot
    fig1, axes1 = plt.subplots(len(valid_data_keys), 1, figsize=(12, 4 * len(valid_data_keys)), sharex=True)
    if len(valid_data_keys) == 1: axes1 = [axes1]
    fig1.suptitle(f'IQ Signal Magnitude (First {TIME_PLOT_SAMPLES} Cleaned & Scaled Samples)', fontsize=16)

    for i, name in enumerate(valid_data_keys):
        iq_samples = data[name]
        if len(iq_samples) < TIME_PLOT_SAMPLES:
            print(f"Not enough cleaned & scaled samples for time-domain plot for {name}. (Requires {TIME_PLOT_SAMPLES}, has {len(iq_samples)})")
            continue
        time_axis = np.arange(TIME_PLOT_SAMPLES) / SAMPLING_FREQUENCY
        axes1[i].plot(time_axis, np.abs(iq_samples[:TIME_PLOT_SAMPLES]), label=name, alpha=0.8)
        axes1[i].set_ylabel('Magnitude')
        axes1[i].set_ylim(0, TARGET_SCALE_MAX_ABS * np.sqrt(2) * 1.1) 
        axes1[i].legend()
        axes1[i].set_title(name)
    axes1[-1].set_xlabel('Time (s)')
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

    # 2. Constellation Diagram (I vs. Q)
    fig2, axes2 = plt.subplots(1, len(valid_data_keys), figsize=(6 * len(valid_data_keys), 6), sharex=True, sharey=True)
    if len(valid_data_keys) == 1: axes2 = [axes2]
    fig2.suptitle(f'IQ Constellation Diagram (First {CHUNK_SIZE_SAMPLES_FOR_VIS} Cleaned & Scaled Samples)', fontsize=16)

    for i, name in enumerate(valid_data_keys):
        iq_samples = data[name]
        axes2[i].scatter(iq_samples.real, iq_samples.imag, s=0.5, alpha=0.6, label=name) 
        axes2[i].set_xlabel('I (In-phase)')
        axes2[i].set_ylabel('Q (Quadrature)')
        axes2[i].set_title(name)
        axes2[i].set_aspect('equal', adjustable='box') 
        axes2[i].grid(True)
        axes2[i].legend()
        # Set symmetric limits based on target scale
        axes2[i].set_xlim(-TARGET_SCALE_MAX_ABS * 1.1, TARGET_SCALE_MAX_ABS * 1.1)
        axes2[i].set_ylim(-TARGET_SCALE_MAX_ABS * 1.1, TARGET_SCALE_MAX_ABS * 1.1)
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

    # 3. Power Spectral Density (PSD)
    fig3, axes3 = plt.subplots(len(valid_data_keys), 1, figsize=(12, 4 * len(valid_data_keys)), sharex=True)
    if len(valid_data_keys) == 1: axes3 = [axes3]
    fig3.suptitle(f'Power Spectral Density (PSD) - {CHUNK_SIZE_SAMPLES_FOR_VIS} Cleaned & Scaled Samples', fontsize=16)

    for i, name in enumerate(valid_data_keys):
        iq_samples = data[name]
        nperseg_val = min(2048, len(iq_samples)) 
        if nperseg_val < 2: 
            print(f"Not enough cleaned & scaled samples ({len(iq_samples)}) for PSD for {name}. Skipping.")
            continue

        freqs, psd = welch(iq_samples, fs=SAMPLING_FREQUENCY, nperseg=nperseg_val, return_onesided=False, scaling='density')
        freqs = np.fft.fftshift(freqs)
        
        # <<<<<<< CONVERT PSD TO dB/Hz >>>>>>>
        # Handle potential zero or very small values before log10
        psd_db = 10 * np.log10(np.maximum(psd, 1e-20)) # Add a small floor to avoid log(0) issues
        
        axes3[i].plot(freqs / 1e6, psd_db, label=name) # Changed to plot from semilogy
        axes3[i].set_ylabel('PSD (dB/Hz)') # Updated label
        axes3[i].set_title(name)
        axes3[i].legend()
        axes3[i].grid(True) # Grid is fine on linear dB scale
    axes3[-1].set_xlabel('Frequency (MHz)')
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

    # 4. Histograms of I, Q, and Magnitude
    fig4, axes4 = plt.subplots(len(valid_data_keys), 3, figsize=(18, 4 * len(valid_data_keys)), sharey='row')
    if len(valid_data_keys) == 1: axes4 = [axes4]

    fig4.suptitle(f'Histograms of I, Q, and Magnitude (First {CHUNK_SIZE_SAMPLES_FOR_VIS} Cleaned & Scaled Samples)', fontsize=16)

    for i, name in enumerate(valid_data_keys):
        iq_samples = data[name]

        axes4[i, 0].hist(iq_samples.real, bins=50, density=True, alpha=0.7, color='skyblue', label=name + ' I')
        axes4[i, 0].set_title(f'{name} - I Component')
        axes4[i, 0].set_xlabel('I Value')
        axes4[i, 0].set_ylabel('Density')
        axes4[i, 0].set_xlim(-TARGET_SCALE_MAX_ABS * 1.1, TARGET_SCALE_MAX_ABS * 1.1) 
        axes4[i, 0].legend()
        axes4[i, 0].grid(True)

        axes4[i, 1].hist(iq_samples.imag, bins=50, density=True, alpha=0.7, color='lightcoral', label=name + ' Q')
        axes4[i, 1].set_title(f'{name} - Q Component')
        axes4[i, 1].set_xlabel('Q Value')
        axes4[i, 1].set_xlim(-TARGET_SCALE_MAX_ABS * 1.1, TARGET_SCALE_MAX_ABS * 1.1) 
        axes4[i, 1].legend()
        axes4[i, 1].grid(True)

        axes4[i, 2].hist(np.abs(iq_samples), bins=50, density=True, alpha=0.7, color='lightgreen', label=name + ' Mag')
        axes4[i, 2].set_title(f'{name} - Magnitude')
        axes4[i, 2].set_xlabel('Magnitude Value')
        axes4[i, 2].set_xlim(0, TARGET_SCALE_MAX_ABS * np.sqrt(2) * 1.1) 
        axes4[i, 2].legend()
        axes4[i, 2].grid(True)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

    print("\nSmall sample comparison plots generated with outlier removal, scaling, and PSD in dB.")
    print(f"Data scaled such that maximum absolute I or Q component is {TARGET_SCALE_MAX_ABS}.")
    print("These plots are for demonstration. For meaningful analysis, increase CHUNK_SIZE_SAMPLES_FOR_VIS.")
    print("Due to very small sample sizes, PSDs and histograms may look less smooth or representative.")
    print(f"Outliers were removed based on an absolute threshold of {OUTLIER_THRESHOLD}.")
    