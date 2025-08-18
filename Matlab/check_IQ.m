fname = 'D:/Downloads/clearsky_signal_C-1_trimmed.bin'; 
nsamp = 2e6;   % ~2 million complex samples probe

% Try float32 (IQ interleaved)
fid = fopen(fname,'rb');
fseek(fid,0,'bof');
iq_f32 = fread(fid, 2*nsamp, 'float32=>single', 0, 'ieee-le');
fclose(fid);
I32 = iq_f32(1:2:end);
Q32 = iq_f32(2:2:end);
fprintf('float32:  I rms=%g, Q rms=%g, corr(I,Q)=%g\n', ...
        rms(I32), rms(Q32), corr(I32(1:1e5),Q32(1:1e5)));

% Try int16 (IQ interleaved)
fid = fopen(fname,'rb');
iq_i16 = fread(fid, 2*nsamp, 'int16=>single', 0, 'ieee-le');
fclose(fid);
I16 = iq_i16(1:2:end);
Q16 = iq_i16(2:2:end);
fprintf('int16:    I rms=%g, Q rms=%g, corr(I,Q)=%g\n', ...
        rms(I16), rms(Q16), corr(I16(1:1e5),Q16(1:1e5)));
