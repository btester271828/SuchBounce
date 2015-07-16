i = 1;
 root = ['../Setup_fine_' num2str(i) '/Outputs/']; %/Output_Data/rotation_dependence/pi
    multiWaitbar('Total',0);
    [Tv,l] = fileread([root 'T_v_tracks.dat']);
    n = size(Tv,2);
    multiWaitbar('Total',0.25);
    [Tw,~]=fileread([root 'T_w_tracks.dat'],l,0,n);
    multiWaitbar('Total',0.5);
    [V,~]=fileread([root 'V_tracks.dat'],l,0,n);
    multiWaitbar('Total',0.75);
    [E,~]=fileread([root 'E_int_tracks.dat'],l,0,n);
    multiWaitbar('Total',1);
    multiWaitbar('Total','Close');
    ii=size(V,1);
    % Tv(:,512) = zeros(size(Tv,1),1);
    % Tw(:,512) = zeros(size(Tv,1),1);
    % V(:,512) = zeros(size(Tv,1),1);
    % E(:,512) = zeros(size(Tv,1),1);
    T0(:,i+1) = sum(E,2)+sum(V,2)+sum(Tv,2)+sum(Tw,2);
    for ii = 1:size(T0,1)
        T0(ii,i+1) = T0(ii,i+1) - T0(1,i+1);
    end
    
    T_parts(:,:,i+1) = 2*E + 2*V + Tv - (6.67384e-11*1e4*1e4/0.4);
    
for i = 2:5
    root = ['../Setup_fine_' num2str(i) '/Outputs/'];
    multiWaitbar('Total',0);
    [Tv,~] = fileread([root 'T_v_tracks.dat'],l,0,n);
    multiWaitbar('Total',0.25);
    [Tw,~]=fileread([root 'T_w_tracks.dat'],l,0,n);
    multiWaitbar('Total',0.5);
    [V,~]=fileread([root 'V_tracks.dat'],l,0,n);
    multiWaitbar('Total',0.75);
    [E,~]=fileread([root 'E_int_tracks.dat'],l,0,n);
    multiWaitbar('Total',1);
    multiWaitbar('Total','Close');
    ii=size(V,1);
    % Tv(:,512) = zeros(size(Tv,1),1);
    % Tw(:,512) = zeros(size(Tv,1),1);
    % V(:,512) = zeros(size(Tv,1),1);
    % E(:,512) = zeros(size(Tv,1),1);
    T0(:,i+1) = sum(E,2)+sum(V,2)+sum(Tv,2)+sum(Tw,2);
    for ii = 1:size(T0,1)
        T0(ii,i+1) = T0(ii,i+1) - T0(1,i+1);
    end
    T_parts(:,:,i+1) = 2*E + 2*V + Tv - (6.67384e-11*1e4*1e4/0.4);
end
plot(2:ii,T_parts(2:ii,1,2)-T0(2:ii,2)/2,2:ii,T_parts(2:ii,1,3)-T0(2:ii,3)/2,2:ii,T_parts(2:ii,1,4)-T0(2:ii,4)/2,2:ii,T_parts(2:ii,1,5)-T0(2:ii,5)/2); %,2:ii,T_parts(2:ii,1,6)-T0(2:ii,6)/2)
