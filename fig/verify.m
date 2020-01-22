
% first file: sweep.wav in ex1
orig_wav = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep.wav";
fir_wav = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep_fir.wav";
iir_wav = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep_iir.wav";

[y_orig, sr] = audioread(orig_wav);
[y_fir, ~] = audioread(fir_wav);
[y_iir, ~] = audioread(iir_wav);

% only take the first 1000 samples
y_orig = y_orig(1:1000, :);
y_fir = y_fir(1:1000, :);
y_iir = y_iir(1:1000, :);

y_fir_t = fircomb(y_orig, 0.5, 10);
y_iir_t = iircomb(y_orig, 0.5, 10);

diff_fir = abs(y_fir-y_fir_t);
diff_iir = abs(y_iir-y_iir_t);

idx = 1:size(y_orig, 1);
idx = idx/sr;

subplot(2,1,1);
plot(idx, diff_fir);
xlabel('time');
ylim([0, 1e-4]);
title('Difference for FIR');

subplot(2,1,2);
plot(idx, diff_iir);
xlabel('time');
ylim([0, 1e-4]);
title('Difference for IIR');

sgtitle('first 1000 samples in sweep.wav');


function y = fircomb(x, gain, delayLen)
[nSample, nChannel] = size(x);
Delayline=zeros(delayLen, nChannel);
for n=1:nSample
	y(n, :)=x(n, :)+gain*Delayline(end, :);
	Delayline=[x(n, :);Delayline(1:end-1, :)];
end
end

function y = iircomb(x, gain, delayLen)
[nSample, nChannel] = size(x);
Delayline=zeros(delayLen, nChannel);
for n=1:nSample
	y(n, :)=x(n, :)+gain*Delayline(end, :);
	Delayline=[y(n, :);Delayline(1:end-1, :)];
end
end