% load the wave
[y, sr] = audioread('sweep.wav');

% load the text
t = fopen('sweep.txt', 'r');
c = textscan(t, '%f%f');

% absolute difference: 0
disp(sum(abs(c{1}-y(:,1))) + sum(abs(c{2}-y(:,2))));