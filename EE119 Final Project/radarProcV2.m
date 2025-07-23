%=============================================================================
%  PEOPLE COUNTING
%=============================================================================
clear all; clc; close all;

%% ===========================
%% 1) HIGH-LEVEL PARAMETERS
%% ===========================
c                = 3e8;
Tf               = 50e-3;
f0               = 60.6e9;
B                = 3.2849e9;
fc               = f0 + B/2;
frameCutoff      = 512;       % how many frames to process
distanceLim      = 6;         % (m) for display x-axis

% — Detection thresholds (global + local) —
threshold_dB     = 32;        % (dB) base RD‐amplitude threshold
atnPerMeter      = 5;         % (dB/decade) attenuation vs. range
thresholdDoppler = 0.02;      % (m/s) to gate static clutter
thresholdWindow  = 21;        % only used inside detectObjects for local window

% — Hyper‐parameters for local-max search —
gridSize         = 21;         % odd integer for local max window
dbMargin         = 3;         % how many dB above (local or global) to call a peak

% — Persistence‐tracking parameters —
gateRange        = 0.5;       % (m) maximum allowed range difference to consider "same" track
gateDoppler      = 1;      % (m/s) maximum allowed Doppler difference for "same1 track
minPersistence   = 4;         % a track must be seen ≥ n frames before we count it
maxMissedAllowed = 7;         % if a track is missed  > n consecutive frames, drop it

% - Extra Plotting to Debug the Object Tracking -
debugMode        = false;

nBG = 50;

%% ===========================
%% 2) LOAD DATA & PRECOMPUTE
%% ===========================
cd '/Users/Vassilis/Desktop/EE219/Final Project mmWave/Matlab Project';
%load('output_5_30');   % first test (in front of and behind the wall)
load('output_5_30v2');   % second test (behind the wall)

Nframes = min(length(radarCube.data), frameCutoff);
[NChirps, NReceivers, NSamples] = size(radarCube.data{1});

% Precompute range & Doppler axes
deltaR  = c/(2*B);
deltaV  = c/(2*Tf*fc);
x_range = (0 : NSamples-1) * deltaR;                                % range (m)
y_range = ((-NChirps/2) : (NChirps/2 - 1)) * deltaV;                % Doppler (m/s)

% Windowing mats for 3D FFT pipeline
winrange3D   = permute(repmat(hann(NSamples), [1, NChirps, NReceivers]), [2,1,3]);
windoppler3D = repmat(hann(NChirps), [1, NSamples, NReceivers]);

%% ------------------------------------------------
%% 3) BUILD STATIC BACKGROUND MAP (BG) – median
%% ------------------------------------------------
BGstack = zeros(length(y_range), length(x_range), nBG);
for ff = 1 : min(nBG, Nframes)
    frm = permute(radarCube.data{ff}, [1,3,2]);
    frm = ifft(frm, NSamples, 2) .* windoppler3D .* winrange3D;
    RDf = fftshift(20*log10(squeeze(mean(abs(fftn(frm)),3)) + eps), 1);
    BGstack(:,:,ff) = RDf;
end
BG = median(BGstack, 3);

%% ===========================
%% 4) PROCESS EACH FRAME & DO TRACKING
%% ===========================
% Initialize an empty track list:
tracks = struct( ...
    'pos',            {}, ...   % [range, doppler]
    'age',            {}, ...   % how many frames in a row this track has been seen
    'totalCount',     {}, ...   % cumulative number of times detected
    'missedCount',     {});     % how many consecutive frames we haven,t seen it

allDetections = cell(Nframes,1);
counts        = zeros(Nframes,1);

hFig = figure('Position',[50 50 1000 600]);
for ff = 1 : Nframes
    % (a) compute BG‐subtracted RD for this frame
    frm = permute(radarCube.data{ff}, [1,3,2]);
    frm = ifft(frm, NSamples, 2) .* windoppler3D .* winrange3D;
    RD_raw = fftshift(20*log10(squeeze(mean(abs(fftn(frm)),3)) + eps), 1);
    RD     = max(RD_raw - BG, 0);

    % (b) DETECT OBJECTS (unchanged)
    dets = detectObjects( ...
      RD, x_range, y_range, ...
      threshold_dB, thresholdDoppler, atnPerMeter, ...
      gridSize, dbMargin, thresholdWindow, debugMode );

    allDetections{ff} = dets;  % store for possible debugging

    % (c) UPDATE TRACKS with new detections
    tracks = updateTracks(tracks, dets, gateRange, gateDoppler, maxMissedAllowed);

    % (d) COUNT only those tracks that have persisted long enough
    stillCountable = [tracks.totalCount] >= minPersistence;
    counts(ff) = sum(stillCountable);

    % (e) VISUALIZE (RD + current raw detections + persisted‐tracks in green)
    figure(hFig); clf;
    imagesc(x_range, y_range, RD);
    axis xy; axis image; colormap(jet); colorbar;
    xlim([0, distanceLim]); hold on;

    % show all raw detections in red
    if ~isempty(dets)
      scatter(dets(:,1), dets(:,2), 80, 'r', 'filled');
    end
    % overlay persisted‐tracks (range,doppler) in green
    for t = find(stillCountable)
      scatter(tracks(t).pos(1), tracks(t).pos(2), 120, 'g', 'filled', 'MarkerEdgeColor','k');
    end
    hold off;

    title(sprintf('Frame %d/%d — Active People: %d', ff, Nframes, counts(ff)));
    xlabel('Range (m)'); ylabel('Doppler (m/s)');
    drawnow;
end

%% ===========================
%% 5) PLOT COUNT OVER TIME
%% ===========================
figure;
plot(1:Nframes, counts, '-o','LineWidth',1.5);
xlabel('Frame'); ylabel('Persisted People Count');
title('People Count w/ Persistence Filtering');
grid on;


%% ==============================================================================
%% 6) DETECTION FUNCTION (unchanged except for the extra thresholdWindow)
%% ==============================================================================
function detections = detectObjects( ...
    RD, x_range, y_range, threshold_dB, thresholdDoppler, atnPerMeter, ...
    gridSize, dbMargin, thresholdWindow, debugMode)

    % 1) build 1D attenuation curve + replicate to 2D
    attenuationCurve = threshold_dB - atnPerMeter * log10(x_range + 0.1);
    globalThreshMat  = repmat(attenuationCurve, length(y_range), 1);

    % 2) amplitude mask (global)
    RD_mask    = RD > globalThreshMat;
    RD_masked  = RD .* RD_mask;

    % 3) Doppler gating (remove static clutter)
    validRows    = abs(y_range) >= thresholdDoppler;
    RD_filt      = RD_masked(validRows, :);
    yr           = y_range(validRows);
    globalTh_filt = globalThreshMat(validRows, :);

    % 4) compute local adaptive threshold via moving‐mean
    localWin     = max(gridSize, thresholdWindow);
    kern         = ones(localWin) / (localWin^2);
    localMean    = conv2(RD_filt, kern, 'same');
    localThreshMat = localMean + dbMargin;

    % combine global vs. local; pick the more conservative
    adaptThresh = max(globalTh_filt, localThreshMat);

    [NY, NX] = size(RD_filt);
    detections = [];
    if NY < gridSize || NX < gridSize
        return;
    end

    halfW = floor(gridSize/2);

    % 5) Find local maxima above the adaptive threshold
    for r = (1+halfW) : (NY-halfW)
      for c = (1+halfW) : (NX-halfW)
        centerVal = RD_filt(r,c);
        localWnd  = RD_filt(r-halfW:r+halfW, c-halfW:c+halfW);
        % if this pixel is the max in its local window AND above threshold
        if centerVal == max(localWnd(:)) && centerVal > adaptThresh(r,c)
          detections(end+1, :) = [ x_range(c), yr(r) ]; %#ok<AGROW>
        end
      end
    end

    % (Optional) DEBUG PLOTS
    if debugMode
      figure(99); clf;
      set(gcf,'Name','detectObjects Adaptive Debug','NumberTitle','off');
      subplot(2,2,1);
      imagesc(x_range, y_range(validRows), RD_filt);     axis xy; colorbar;
      title('Filtered RD');
      subplot(2,2,2);
      imagesc(x_range, y_range(validRows), globalTh_filt); axis xy; colorbar;
      title('Global Threshold');
      subplot(2,2,3);
      imagesc(x_range, y_range(validRows), localThreshMat); axis xy; colorbar;
      title('Local Adaptive Threshold');
      subplot(2,2,4);
      imagesc(x_range, y_range(validRows), RD_filt); axis xy; hold on;
      if ~isempty(detections)
        scatter(detections(:,1), detections(:,2), 50, 'r','filled');
      end
      title(sprintf('Detections (N=%d)', size(detections,1)));
      hold off; drawnow; pause(0.1);
    end
end


%% =========================================================================
%% 7) TRACK UPDATE FUNCTION (fixed: no “matched” field in the struct)
%%    - Associates detections → existing tracks using simple gating
%%    - Updates track fields, prunes dead tracks
%%    - Any detection that fails to match becomes a new track
%% =========================================================================
function tracks = updateTracks(tracks, detections, gateRange, gateDoppler, maxMissedAllowed)
    % tracks: array of structs with fields 
    %    pos         = [range, doppler]
    %    age         = how many consecutive frames it has been matched
    %    totalCount  = total times it’s been seen
    %    missedCount = how many consecutive frames it’s been missed
    %
    % detections: M×2 array of [range, doppler]

    numTracks = length(tracks);
    numDets   = size(detections,1);

    % 1) Prepare arrays to mark which tracks/detections get matched
    isMatched   = false(numTracks, 1);
    detAssigned = false(numDets, 1);

    % 2) Attempt to associate each detection to an existing track
    for d = 1:numDets
        dr_new = detections(d,1);
        dv_new = detections(d,2);

        bestT    = 0;
        bestDist = inf;

        % Find the closest unmatched track within gating thresholds
        for t = 1:numTracks
            if ~isMatched(t)
                drDiff = abs(tracks(t).pos(1) - dr_new);
                dvDiff = abs(tracks(t).pos(2) - dv_new);
                if drDiff < gateRange && dvDiff < gateDoppler
                    dist = sqrt(drDiff^2 + dvDiff^2);
                    if dist < bestDist
                        bestDist = dist;
                        bestT    = t;
                    end
                end
            end
        end

        if bestT > 0
            % 2a) Update that existing track
            tracks(bestT).pos        = [dr_new, dv_new];
            tracks(bestT).age        = tracks(bestT).age + 1;
            tracks(bestT).totalCount = tracks(bestT).totalCount + 1;
            tracks(bestT).missedCount= 0;
            isMatched(bestT)         = true;
            detAssigned(d)           = true;
        end
    end

    % 3) Any detections that never matched → create brand‐new tracks
    for d = 1:numDets
        if ~detAssigned(d)
            newTrack.pos        = detections(d, :);
            newTrack.age        = 1;
            newTrack.totalCount = 1;
            newTrack.missedCount= 0;
            tracks(end+1)       = newTrack;  %#ok<AGROW>
        end
    end

    % 4) For every existing track that was not matched, increment its missedCount
    for t = 1:numTracks
        if ~isMatched(t)
            tracks(t).missedCount = tracks(t).missedCount + 1;
        end
    end

    % 5) Prune any track whose missedCount > maxMissedAllowed
    if ~isempty(tracks)
        aliveMask = [tracks.missedCount] <= maxMissedAllowed;
        tracks    = tracks(aliveMask);
    end
end