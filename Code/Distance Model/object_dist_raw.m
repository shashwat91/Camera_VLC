%% Constants of mobile phone
Zf = 3.6e-3; %Focal distance
Sp = 1.12e-6; %Size of 1 pixel

%% Mesurable values
So = 0.2; %size of object in m
Np = [1760,1300,1080,780,676,588,548,368,280,234,184]; %size of image in terms of pixels

%% Calculation of object distance from phone
%Formuls =>  Do = ((So / (Np * Sp)) + 1) * Zf

Do = zeros(1,length(Np));
for i=1:length(Np)
    Do(i) = ((So / (Np(i) * Sp)) + 1) * Zf;
end

d=[30    50    60    70    80    90   100   150   200   250   300];

figure(2)
hold on;
plot(d,Np,'x-');
plot(Do*100,Np,'x-');
hold off;
legend('Measured Distance','Calculated Distance');