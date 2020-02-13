[x_m, sr] = audioread('/Users/caspia/Desktop/spring20/MUSI 6106/audio/saxophone_vibrato.wav');
[x, sr] = audioread('/Users/caspia/Desktop/spring20/MUSI 6106/audio/saxophone.wav');
x_m = x_m(1:10000, :);
x = x(1:10000, :);
y1 = vibrato(x(:,1), sr, 21, 0.00023);
y2 = vibrato(x(:,2), sr, 21, 0.00023);
y = [y1 y2];
diff = abs(y-x_m);
subplot(3,1,1);
plot((1:10000)/sr, diff);
title("saxophone.wav");
xlabel("time (s)");

[x_m, sr] = audioread('/Users/caspia/Desktop/spring20/MUSI 6106/audio/violin_vibrato.wav');
[x, sr] = audioread('/Users/caspia/Desktop/spring20/MUSI 6106/audio/violin.wav');
x_m = x_m(1:10000, :);
x = x(1:10000, :);
y1 = vibrato(x(:,1), sr, 21, 0.00023);
y2 = vibrato(x(:,2), sr, 21, 0.00023);
y = [y1 y2];
diff = abs(y-x_m);
subplot(3,1,2);
plot((1:10000)/sr, diff);
title("violin.wav");

[x_m, sr] = audioread('/Users/caspia/Desktop/spring20/MUSI 6106/audio/birdflute_vibrato.wav');
[x, sr] = audioread('/Users/caspia/Desktop/spring20/MUSI 6106/audio/birdflute.wav');
x_m = x_m(1:10000, :);
x = x(1:10000, :);
y1 = vibrato(x(:,1), sr, 21, 0.00023);
y2 = vibrato(x(:,2), sr, 21, 0.00023);
y = [y1 y2];
diff = abs(y-x_m);
subplot(3,1,3);
plot((1:10000)/sr, diff);
title("birdflute.wav");

sgtitle('difference in the first 10000 samples');
xlabel("time (s)");