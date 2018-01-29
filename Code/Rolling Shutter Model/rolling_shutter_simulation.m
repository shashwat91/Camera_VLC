close all;

te = 161.6e-6;
tr = 10.5e-6;

rows = 100;
frame1 = zeros(rows,3);
values = zeros(rows,1);

%% Calculating Start and End time of each row 
%  according to te and tr

for i=rows:-1:1
    frame1(i,1) = double((i - 1)) * tr;
    frame1(i,2) = frame1(i,1) + te;
    frame1(i,3) = frame1(i,2) + tr;
end

%% Generating a square wave(INPUT) of given frequency with 1ns resolution

f=10000;
t = 0:1e-7:max(frame1(:));
sq=0.5*(square(pi*f*t)+1);

%% Simulating samples based on input and rolling shutter
% Formula for getting values from 1 point to another
% seq = sq(vpa(ceil(frame1(9,2)*1e7))+1 : vpa(ceil(frame1(9,3)*1e7))+1);

% seq = zeros(rows,1);
% for i=1:rows
%     seq(i,1) = mean(sq(vpa(ceil(frame1(i,1)*1e7))+1 : vpa(ceil(frame1(i,2)*1e7))+1));
% end

samples = zeros(rows,1);
for i=1:rows
    sum=0;
    count=0.0;
    for j=frame1(i,1):1e-7:frame1(i,2)
    	sum = sum + sq(vpa(ceil(j*1e7))+1);
        count = count+1;
    end
    samples(i,1) = sum/count;
end

%% Plotting the results

% Plot for Rolling Shutter times
figure(1);
hold on;
for i=rows:-1:1
   plot(frame1(rows-i+1,:),[i i i],'rx-');
end
plot(t,(10*sq)+rows+10,'bo-');
hold off;
grid on;
grid minor;
title('Generated Waveform and Timing of rolling shutter');
xlabel('Time (sec)');
ylabel('Row Number');

% Plot for received signal
figure(2)
plot(samples,'x-');
ylim([0 1]);
title('Calculated symbols using Rolling Shutter effect');
xlabel('Pixel Number (sample number)');
ylabel('Signal Intensity');