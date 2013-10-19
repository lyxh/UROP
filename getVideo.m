function SK
RemoveInvalidPixel=false;
AverageFilter=true;
drawPath=true;
smoothPath=false;
biggerPath=false;
AverageFrames=true;
UpdateNFrames=2; 
%need to average the data, so that the path is smoother(like a build in average)
CU=struct; % CleanUp struct for closing the pmd camera etc.
path_num_points=0;

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
    NFrames=NFrames+1;   
    if (~mod(NFrames,100))
        FPS=int2str(100/toc);
        set(hFPS,'String',['FPS: ',FPS]);
        tic;
    end

    if (AverageFrames)
        ratio=(UpdateNFrames-1)/UpdateNFrames;
        for i=1:320
            for j=1:240
             dist(i,j)=dist(i,j)*ratio+dist1(i,j)*(1-ratio);
            end
        end
    end
    [m,n]=size(dist);        
    dist_processed=dist;   
    amp_processed=amp;  

    %remove invalid pixels
    if (RemoveInvalidPixel)
       for i=1:m
           for j=1:n
               if dist_processed(i,j)==32001
                   dist_processed(i,j)=5000;
               end
           end
        end      
    end
     
     %averaging filter of the data by every column
     if (AverageFilter)
         a = 1;
         b = [2/8 2/8 2/8 2/8];
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
                     dist_processed(i,j)=1000;
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
     
     path_num_points=path_num_points+1;
     path_points(path_num_points,1)=dist_min_row;
     path_points(path_num_points,2)=dist_min_col;

     
     %add the point to the path
     path(dist_min_row,dist_min_col)=0;
     %also smooth the path
     smooth_path=path;
     if (biggerPath)
         path(dist_min_row-1,dist_min_col)=0;
         path(dist_min_row+1,dist_min_col)=0;
         path(dist_min_row,dist_min_col-1)=0;
         path(dist_min_row,dist_min_col+1)=0;        
     end
     
     if (smoothPath)
         a = 1;
         b = [1/8 3/8 3/8 1/8];
         for j=1:n
            x = path(:,j);           
            smooth_path(:,j) = filter(b,a,x);
         end
         for i=1:n
            x = path(i,:);
            smooth_path(i,:) = filter(b,a,x);
         end
     end 

     %show the path on the graph
     if(drawPath)
        set(hpath,'CData',smooth_path');
     end    
       set(hdist,'CData',dist_processed');
     drawnow;
end
CU.CleanThreads=[];
set(hFig,'Name','Finished');
clear CU; % Force cleanup (GUI may keep cleanup variable present)
end