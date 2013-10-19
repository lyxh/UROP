yp = getSKFrame();
[m,n]=size(yp);
d_m=double(m);
d_size=double(double(320.00)*double(240.00));
frame_num=floor(double(d_m)/double(d_size));
yp=yp(1:240*320);
data = reshape(yp, 320, 240);%,frame_num
for i=1:240
   depth_data(:,i)=data(:,241-i);
end

cmap=jet(256);
colormap(cmap);
subplot(1,1,1);
hdist=imagesc(depth_data');
setappdata(gcf, 'SubplotDefaultAxesLocation', [0, 0, 1, 1]);
axis image;
set(gca,'YDIR','normal');
title('Distances');
