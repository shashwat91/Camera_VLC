te = 161.5e-6;
tr = 10.5e-6;

rows = 200;
frame1 = zeros(rows,3);
values = zeros(rows,1);

%% Calculating Start and End time of each row 
%  according to te and tr

for i=rows:-1:1
    frame1(i,1) = double((i - 1)) * tr;
    frame1(i,2) = frame1(i,1) + te;
    frame1(i,3) = frame1(i,2) + tr;
end

%% Generating a square wave(INPUT) of given frequency with 1ns(10^-9sec) resolution

f=6100:100:10000;
samples = zeros(rows,length(f));
for itteration=1:length(f)
    f(itteration)
    clear 'sq','t';
    t = 0:1e-7:max(frame1(:));
    sq=0.5*(square(pi*f(itteration)*t)+1);

    %% Simulating samples based on input and rolling shutter
    % Formula for getting values from 1 point to another
    % seq = sq(vpa(ceil(frame1(9,2)*1e7))+1 : vpa(ceil(frame1(9,3)*1e7))+1);

    % seq = zeros(rows,1);
    % for i=1:rows
    %     seq(i,1) = mean(sq(vpa(ceil(frame1(i,1)*1e7))+1 : vpa(ceil(frame1(i,2)*1e7))+1));
    % end

    for i=1:rows
        sum=0;
        count=0.0;
        for j=frame1(i,1):1e-7:frame1(i,2)
            sum = sum + sq(vpa(ceil(j*1e7))+1);
            count = count+1;
        end
        samples(i,itteration) = sum/count;
    end

    filename = strcat('run_',num2str(itteration),'_freq_',num2str(f(itteration)));
    save(filename);
end