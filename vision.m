% matlab script in process to develop coplotting capability

fname = 'motion003_000.mp4';

%% Make the MP4 on the fly
if ~exist(fname,'file')
    disp('Generating mp4')
    [~,h264,~] = fileparts(fname);
    h264 = [h264 '.h264'];
    cmd = ['"' fullfile('/usr/bin/ffmpeg') '" -r 30 -i ' h264 ' -vcodec copy ' fname];
    [status, message] = system(cmd);
    if ~exist(fname,'file')
        error('mp4 generation failed')
    end
end

vid  = VideoReader(fname);

[~,datname,~] = fileparts(fname);
datname = [datname '.data'];
m    = memmapfile(datname,'Format',{'int8',[1],'x'; 'int8',[1],'y';'uint16',[1],'SAD'});

cols   = (vid.Width/16) + 1;    % 16+1, there's always an extra column
rows   = ceil(vid.Height/16);   % ceil if not even
frames = 30*30-1;               % 30 fps, 30 seconds

% make mask
centerx = 30;
centery = 43;
maxrad  = 25;                   % outer null
minrad  =  5;                   % center null
mask = zeros(rows,cols);
for i = 1:cols
    for j = 1:rows
        dist =sqrt(abs(i - centery)^2 + abs(j - centerx)^2);
        if dist < maxrad && dist > minrad
            mask(j,i) = 1.;
        end
    end
end
maski = int8(mask);
maskd = mask;
figure
colormap bone
imagesc(mask)

h = figure;
for i = 1:frames                % frame 1 is baseline frame and is all zeros

    j = i * (cols * rows);
    k = (i+1) * (cols * rows) - 1;
    frame = m.Data(j:k);
    x    = reshape([frame.x],   [cols, rows])';  % FIXME
    y    = reshape([frame.y],   [cols, rows])';  % FIXME
    SAD  = reshape([frame.SAD], [cols, rows])'; % OK

    % FIXME X, Y, are rotated relative to video
    subplot(2,3,1)
    cla;
    quiver(x.*maski,y.*maski);                    % Vectors
    axis([0, cols, 0, rows]);
    set(gca,'ydir','reverse');
    subplot(2,3,2)
    imagesc(SAD)                    % Sum of Absolute Differences
    subplot(2,3,3)
    image(read(vid, i));            % Visual
    subplot(2,3,4)
    imagesc((x>0)-(x<0))            % might be useful for upright camera
    subplot(2,3,5)
    divv = divergence(x.*maski, y.*maski);
    div(i) = mean(mean(divv));
    imagesc(divv);
    imagesc(mask);
    subplot(2,3,6)
    cavv = curl(x, y);
    cavv = cavv .* maskd;
    cav(i) = mean(mean(cavv));
    imagesc(cavv);
    drawnow;
    
    % TODO null out the non-interesting parts
    
    disp(['frame ' num2str(i) ' mean curl ' num2str(cav(i))...
          ' mean div ' num2str(div(i))]);
    if ~isvalid(h)
        return
    end
end

%{
Richard's example code:
xyloObj = VideoReader('testvideo.mp4');
% xyloObj = VideoReader('testvideo_one_MECO.mp4');
nFrames = xyloObj.NumberOfFrames;
vidHeight = xyloObj.Height;
vidWidth = xyloObj.Width;
total=vidHeight*vidWidth;
vidtime=1/29.48*[1:nFrames];
writerObj = VideoWriter('H1R1b.avi');
open(writerObj);
set(gca,'nextplot','replacechildren');
set(gcf,'Renderer','zbuffer');
buildframes=[nFrames/2:5:nFrames*3/4];
for i=1:size(buildframes,2),
    frm=read(xyloObj,buildframes(i));
    tt(i,1)=1.0/total*sum(sum(frm(:,:,1))');
    tt(i,2)=1.0/total*sum(sum(frm(:,:,2))');
    tt(i,3)=1.0/total*sum(sum(frm(:,:,3))');
subplot(4,1,[1:3])
h=image(frm); hold all;axis off;
subplot(4,1,4)
plot(tt(1:i,:));xlim([0,size(buildframes,2)]);ylim([65,85]);axis on;
xlabel('frames/5');
frame=getframe(gcf);
writeVideo(writerObj,frame);
end;
close(writerObj);
% save('movie_data');
% movie2avi(M,'test_movie'
);
%}