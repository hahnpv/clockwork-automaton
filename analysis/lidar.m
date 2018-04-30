% reads LIDAR files
filename = 'lidar002_000.csv';

M = dlmread(filename);
scans = [find([1; diff(M(:,1))<0] == 1); size(M,1)];
x = M(:,2).*cos(M(:,1)*pi()/180);
y = M(:,2).*sin(M(:,1)*pi()/180);

figure
for i = 1:numel(scans)-1
    plot(x(scans(i):scans(i+1)-1),y(scans(i):scans(i+1)-1));
    hold all;
%    pause(1)
end