%% ===================== FGI-GSRx Galileo E1B Full Receiver =====================
clc; clear; close all;

%% -------------------- Add FGI-GSRx to MATLAB Path --------------------
fgirxPath = 'D:\UoM\Final Year Project\FGI-GSRx MatLab Open Source multi-GNSS software receiver\FGI-GSRx';
addpath(genpath(fgirxPath));

%% -------------------- Input binary file --------------------
rfFile = 'D:/Downloads/clearsky_signal_C-1_trimmed.bin';

%% -------------------- Determine max available milliseconds --------------------
fileInfo = dir(rfFile);
fileBytes = fileInfo.bytes;   
sampleSizeBytes = 2;        % 16-bit per I or Q
numComplexSamples = fileBytes / (sampleSizeBytes*2);  % I+Q
samplingFreq = 5e6;         % 5 MHz
msAvailable = floor(numComplexSamples / samplingFreq * 1000); 
fprintf('Maximum milliseconds available in file: %d ms\n', msAvailable);

%% -------------------- Create Parameter File --------------------
paramFile = fullfile(fgirxPath,'param','tuni_E1_run.txt');

fid = fopen(paramFile,'w');
fprintf(fid, [
    'sys,enabledSignals,[{[''gale1b'']}],\n' ...
    sprintf('sys,msToProcess,%d,\n', msAvailable) ...  % auto-adjust
    'sys,msToSkip,0,\n' ...
    'sys,loadDataFile,false,\n' ...
    'sys,saveDataFile,false,\n' ...
    'sys,plotSpectra,true,\n' ...
    'sys,plotAcquisition,true,\n' ...
    'sys,plotTracking,true,\n' ...
    'sys,showTrackingOutput,true,\n' ...
    sprintf('gale1b,rfFileName,''%s'',\n', rfFile) ...
    'gale1b,centerFrequency,1575.42e6,\n' ...
    'gale1b,samplingFreq,5e6,\n' ...
    'gale1b,bandWidth,4.2e6,\n' ...
    'gale1b,sampleSize,16,\n' ...
    'gale1b,complexData,true,\n' ...
    'gale1b,iqSwap,false,\n' ...
    'gale1b,modType,''CBOC'',\n' ...
    'gale1b,acqSatelliteList,[1:36],\n' ...
    'gale1b,nonCohIntNumber,2,\n' ...
    'gale1b,cohIntNumber,1,\n' ...
    'gale1b,acqThreshold,6,\n' ...
    'gale1b,maxSearchFreq,6000\n' ...
    ]);
fclose(fid);

%% -------------------- Run Receiver --------------------
disp('Running FGI-GSRx Galileo E1B Receiver...');
try
    gsrx(paramFile);   % Run the receiver
catch ME
    error('Error running GSRx: %s', ME.message);
end

%% -------------------- Frame Decoding (Safe) --------------------
disp('Decoding frames for valid PRNs...');

try
    obsData = [];
    ephData = [];
    
    % Check if trackData exists
    if exist('trackData','var') && isfield(trackData,'gale1b')
        allPRNs = fieldnames(trackData.gale1b);  % PRNs tracked
        for i = 1:length(allPRNs)
            prnStr = allPRNs{i};
            prnData = trackData.gale1b.(prnStr);

            % Only attempt decoding if there is a valid preamble
            try
                if isfield(prnData,'I_P') && ~isempty(prnData.I_P) && any(prnData.I_P)
                    [obsTmp, ephTmp] = doFrameDecoding(trackData, str2double(prnStr), prnData.settings);
                    obsData.(sprintf('PRN_%s', prnStr)) = obsTmp;
                    ephData.(sprintf('PRN_%s', prnStr)) = ephTmp;
                    fprintf('Decoded PRN %s successfully.\n', prnStr);
                else
                    fprintf('Skipping PRN %s: no valid preamble detected.\n', prnStr);
                end
            catch
                fprintf('Skipping PRN %s: error during decoding.\n', prnStr);
            end
        end
    else
        warning('No tracked PRNs found in trackData.gale1b.');
    end
catch ME
    warning('Frame decoding skipped due to error: %s', ME.message);
end

disp('FGI-GSRx run completed successfully.');
