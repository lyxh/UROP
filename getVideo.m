function SK
RemoveInvalidPixel=false;
AverageFilter=true;
drawPath=true;
UpdateNFrames=5; 
%need to average the data, so that the path is smoother?
AveragingFrames=5;
RunLoop=true;
function KeyPressFcn(~,~)
   RunLoop=false;
end

hFig=figure(1);
set(hFig,'NumberTitle','off','Name','Press any key to exit!');
set(hFig,'KeyPressFcn',@KeyPressFcn);
cmap=jet(256);
colormap(cmap);
dist=zeros(320,240);
dist1=getSKFrame();
    for i=1:240
         for j=1:320
             dist(j,i)=dist1(321-j,241-i);
         end
    end
amp=zeros(320,240);
path=ones(320,240);

%First plot for distances
subplot(1,2,1);
hdist=imagesc(dist');
%setappdata(gcf, 'SubplotDefaultAxesLocation', [0, 0, 1, 1]);
axis image;
set(gca,'YDIR','normal');
title('Distances');

%Could also plot the path
subplot(1,2,2);
hpath=imagesc(path');
axis image;
set(gca,'YDIR','normal');
title('Path');

hFPS=annotation('textbox',[0,0,1,.1],...
    'String','FPS:',...
    'FitBoxToText','off','LineStyle','none');

set(hFig,'Renderer','painters');

NFrames=0;
FPS=0;
tic; % for FPS 

while(RunLoop)
    % fetch new frame, good!
    data = getSKFrame();
    for i=1:240
         for j=1:320
             dist1(j,i)=data(321-j,241-i);
         end
    end
    amp=dist;
    
    NFrames=NFrames+1;   
    if (~mod(NFrames,100))
        FPS=int2str(100/toc);
        set(hFPS,'String',['FPS: ',FPS]);
        tic;
    end

    if (mod(NFrames,UpdateNFrames)==UpdateNFrames-1)   
        %for every 5 frame, average,process and draw the data
         dist(:,:)=dist(:,:)/UpdateNFrames;
         [m,n]=size(dist);        
         dist_processed(:,:)=dist(:,:);   
         amp_processed=amp(:,:);  
         
         %remove invalid pixels
         if (RemoveInvalidPixel)
           for i=1:m
             for j=1:n
                  if dist_processed(i,j)==32001
                     dist_processed(i,j)=5000;
                  end
             end
           end      
          
         %averaging filter of the data by every column
         if (AverageFilter)
             a = 1;
             b = [1/8 3/8 3/8 1/8];
             for j=1:n
                x = dist_processed(:,j);           
                dist_processed(:,j) = filter(b,a,x);
             end
             for i=1:n
                x = dist_processed(i,:);
                dist_processed(i,:) = filter(b,a,x);
             end
             %Remove the pixel on the edges since it is too noisy             
             for i=1:n
                 for j=1:n
                     if(i==1)||(i==2)||(i==3)||(i==4)||(j==1)||(j==2)||(j==3)||(j==4)
                         dist_processed(i,j)=32001;
                     end
                 end
             end             
         end

         %for distance, get the pixel with minimum value;
         %for amplitude, get the pixel with maximum value.
         [col,row_indices] = min(dist_processed,[],1) ;
         [maximum,dist_min_col]=min(col,[],2);
         dist_min_row=row_indices(dist_min_col);
         [col,row_indices] = max(amp_processed,[],1);
         [maximum,amp_max_col]=max(col,[],2);
         amp_max_row=row_indices(amp_max_col);
         amp_max_val=amp_processed(amp_max_row,amp_max_col);
 
         %draw a box around the closest point 
         for i=-10:10
             for j=-10:10
                 row=min([m max([dist_min_row+i 1])]);
                 col=min([n max([dist_min_col+j 1])]);
                 dist_processed(row,col)=0.01;               
                 row=min([m max([amp_max_row+i 1])]);
                 col=min([n max([amp_max_col+j 1])]);
                 amp_processed(row,col)=amp_max_val;
             end
         end
         %add the point to the path
         path(dist_min_row,dist_min_col)=0;
       
         %show the path on the graph
         if(drawPath)
            set(hpath,'CData',path');
         end    
         set(hdist,'CData',dist_processed');
         drawnow;
         end  
    elseif (mod(NFrames,UpdateNFrames)==0)   
       %clear the data
       dist=zeros(320,240);
       for i=1:320 
           for j=1:240
              dist(i,j)=dist(i,j) + dist1(i,j);
           end
       end
    else
       for i=1:320 
           for j=1:240
              dist(i,j)=dist(i,j) + dist1(i,j);
           end
       end
    end
end
set(hFig,'Name','Finished');
end