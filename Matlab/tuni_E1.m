% param/tuni_E1.m - compatible with your GSRx version
settings.fileName         = 'D:/Downloads/clearsky_signal_C-1_trimmed.bin';
settings.dataType         = 'int16';
settings.iqOrder          = 'IQ';
settings.samplingFreq     = 50e6;
settings.IF               = 0;
settings.constellation    = 'Galileo';
settings.signal           = 'E1B';
settings.msToProcess      = 1000;
settings.acqSatelliteList = [1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 ...
                             21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36];
settings.enableTracking   = 1;
settings.enableNavData    = 1;
