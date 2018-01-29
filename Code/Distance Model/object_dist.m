close all;
%% Structure for Phone details
field1 = 'Name';
field2 = 'Sp';
field3 = 'Zf';
value1 = ["Iphone SE";"Iphone 7";"LG G2";"MOTO G4"];
value2 = [1.22e-6,1.3e-6,1.12e-6,1.11e-6];
value3 = [4.2e-3,4.1e-3,4.1e-3,3.7e-3];
phone_details = struct(field1,value1,field2,value2,field3,value3);

%% Mesurable values
So_inner = 0.155; %size of internal circle of light in m
So_outer = 0.2; %size of outer circle of light in m

%Different distances from light(in cm) at which measurement were taken
Dist=[30, 40, 50, 60, 70, 80, 90, 100, 150, 200, 250, 300]; 

%Np = ceil(rand(4,12)*1000);
Np=[2053,1477,1163,953,798,714,582,538,362,266,214,180; %% Iphone_SE
    1929,1428,1109,904,769,674,545,508,341,257,202,170; %% Iphone_7
    2097,1546,1190,992,841,719,602,558,380,288,224,190; %% LG_G2
    1914,1383,1104,918,774,633,566,519,347,262,208,173;]; %% MOTO_G4

%% Calculation of object distance from phone
%Formuls =>  Do = ((So / (Np * Sp)) + 1) * Zf

Do = zeros(4,length(Dist));    % Array for caluculated distances (in meters)
error = zeros(4,length(Dist)); % Array for error in calculated values (in cm)
error_per = zeros(4,length(Dist)); % Array for percentage error in calculated values

for i=1:4
    for p=1:length(Dist)
        Do(i,p) = ((So_inner / (Np(i,p) * phone_details.Sp(i))) + 1) * phone_details.Zf(i);
        error(i,p) = Do(i,p) - (Dist(p)/100);
        error_per(i,p) = (abs(error(i,p)) * 100) / Do(i,p);
    end
end

Mean_error_per = mean(error_per,2);
RMS_error = rms(error,2);   % RMS error for each phone (in meters)
%% Figure for calculated distances based on pixel values and formula

figure(1)
hold on;
for i=1:4
    plot(Do(i,:)*100,Np(i,:),'x-','DisplayName',phone_details.Name(i),'LineWidth',2);
end    
hold off;
grid on;
grid minor;
lgd = legend('show');
lgd.FontSize = 15;
title(lgd,'Phone')
title('Calculated distance from Object');
xlabel('Distance from light source (cm)');
ylabel('Size of Image in pixels');

%% Figure for Error in calculated distances and Measured distance

figure(2)
hold on;
for i=1:4
    stem(Dist,error(i,:)*100,'DisplayName',phone_details.Name(i));
end
hold off;
lgd = legend('show');
lgd.FontSize = 15;
title(lgd,'Phone')
title('Absolute error in Distance calculation');
xlabel('Distance from light source (cm)');
ylabel('Error in value (cm)');
grid on;

%% Figure for Percentage Error in Calculated distances

figure(3)
hold on;
for i=1:4
    plot(Dist,error_per(i,:),'x-','DisplayName',phone_details.Name(i));
end
hold off;
lgd = legend('show');
lgd.FontSize = 15;
title(lgd,'Phone')
title('Percentage error in Distance calculation');
xlabel('Distance from light source (cm)');
ylabel('Percentage Error');
grid on;