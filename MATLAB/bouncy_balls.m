clear all

n = 12;
rad = 0.1*ones(n,1);
m = ones(n,1);
w = zeros(n,3);
random_seed;

%% Settings
delta_t = 0.0004; % Delta t for integration
grad_correct = 1; % Use gradient term in integration in addition to linear
k = (4*pi*8.85419e-12)^-1; % Strength of Electrostatic (Coulomb) interactions
G = 6.67384e-11; % Strength of Gravitiational (Newtonian) interactions
epsilon = 1e-6; % Strength of LJ interactions
r_m = 0.05; % Range of LJ
max_step = 1e4; % Number of time steps simulated
mu_s = 0.5; % Coefficient of Static Friction
mu_d = 0.4; % Coefficient of Dynamic Friction
diss = 1; % Coefficient of Restitution
%% Render options
do_render = 1;
warp = 0.5; % Ratio of simulated time to rendered video time
do_translate = 0;
frame = 2;

%% Simulation!

F_total = Inf;
t_step = 0;

E_start = [0 0];
for i = 1 :size(r,1)
    
    I(i) = (2*m(i)*(rad(i)^2)/5);
    E_start = E_start + 0.5*(dot(v(i,:),v(i,:))*m(i)*[1 0] + dot(w(i,:),w(i,:))*I(i)*[0 1]);
    for ii = 1:size(r,1)
        mass(i,ii) = m(i)*m(ii)*(1 - (i == ii));
        charge(i,ii) = q(i)*q(ii)*(1 - (i == ii));
    end
end


h = waitbar(0,'Simulating...');

while (t_step < max_step)
    
    waitbar(t_step/max_step)
    
    t_step = t_step + 1;
    
    F = zeros(size(r));
    d = zeros(size(r,1));
    R = zeros(3,size(r,1),size(r,1));
    F_part = zeros(3,size(r,1),size(r,1));
    F_part_grad = zeros(3,size(r,1),size(r,1));    
    j_part = zeros(3,size(r,1),size(r,1));
    j_f_part = zeros(3,size(r,1),size(r,1));
    
    for i = 1:size(r,1)
        
        for ii = 1:(i-1)
            
            R(:,i,ii) = r(ii,:) - r(i,:);
            R(:,ii,i) = -R(:,i,ii);
            d(i,ii) = norm(R(:,i,ii));
            d(ii,i) = d(i,ii);
            collision_flag = heaviside(rad(ii) + rad(i) - d(i,ii)); % 1/2 + (rad(ii) + rad(i) - d(i,ii))/(2*norm(rad(ii) + rad(i) - d(i,ii)))
            
            %% Force from ranged particle interactions
            F_part(:,i,ii) = (-charge(i,ii)*k/d(i,ii)^2 + mass(i,ii)*G/d(i,ii)^2 + epsilon*(r_m^12/d(i,ii)^13 - 2*r_m^6/d(i,ii)^7))*R(:,i,ii)*(1-collision_flag)/d(i,ii);
         
                F_part_grad(:,i,ii) = (2*charge(i,ii)*k/d(i,ii)^3 - 2*mass(i,ii)*G/d(i,ii)^3 - epsilon*(-13*r_m^12/d(i,ii)^14 + 14*r_m^6/d(i,ii)^8))*...
                    R(:,i,ii)*(1-collision_flag)/d(i,ii)*dot((v(ii,:) - v(i,:)),R(:,i,ii)/d(i,ii));
                      
            F_part(:,ii,i) = -F_part(:,i,ii);
            F_part_grad(:,ii,i) = -F_part_grad(:,i,ii);
            
            %% Impulses for particle collisions
            dir_length = norm((v(ii,:) - v(i,:))' - dot((v(ii,:) - v(i,:))',R(:,i,ii)/d(i,ii))*R(:,i,ii)/d(i,ii));
            if dir_length ~= 0
                fric_dir = ((v(ii,:) - v(i,:))' - dot((v(ii,:) - v(i,:))',R(:,i,ii)/d(i,ii))*R(:,i,ii)/d(i,ii))/dir_length;
            else
                fric_dir = [0 0 0]';
            end
            v_rel = fric_dir'*dir_length + cross(w(ii,:),R(:,ii,i)*rad(ii)/d(i,ii)) - cross(w(i,:),R(:,i,ii)*rad(i)/d(i,ii));
            j_f_part(:,i,ii) = -v_rel*collision_flag/(rad(ii)^2/I(ii) + rad(i)^2/I(i) + 1/m(i) + 1/m(ii));
            j_part(:,i,ii) = -(1+diss)*R(:,i,ii)*collision_flag*dot((v(ii,:)-v(i,:)),R(:,i,ii))/(d(i,ii)^2*(m(i)^-1 + m(ii)^-1));
            
            j_f_part(:,ii,i) = -j_f_part(:,i,ii);
            j_part(:,ii,i) = -j_part(:,i,ii);
            
            
        end
        
    end
    
    F = sum(F_part,3);
    F_grad = sum(F_part_grad,3);
    j = sum(j_part,3);
    
    %% Update particle positions and velocities
    
    for i = 1:size(r,1)
        v(i,:) = v(i,:) + F(:,i)'/m(i)*delta_t + F_grad(:,i)'/m(i)*delta_t^2  - j(:,i)'/m(i);
        for ii = 1:size(r,1);
            if i ~= ii
                w(i,:) = w(i,:) - cross(R(:,i,ii),j_f_part(:,i,ii))'*rad(i)/(d(i,ii)*I(i));
                v(i,:) = v(i,:) - j_f_part(:,i,ii)'/m(i);
            end
        end
        
        r(i,:) = r(i,:) + v(i,:)*delta_t;
        r_tracker(t_step,i,:) = r(i,:);
    end
    
    t(t_step) = t_step * delta_t;
    
end

E_end = [0 0];
for i = 1:size(r,1)
    E_end = E_end + 0.5*(dot(v(i,:),v(i,:))*m(i)*[1 0] + dot(w(i,:),w(i,:))*I(i)*[0 1]);
end

close(h)

if do_translate
    for i=1:size(r_tracker,1)
        for j=1:size(r_tracker,3)
            r_tracker(i,:,j) = r_tracker(i,:,j) - r_tracker(i,frame,j);
        end
    end
end

if do_render
    sim_render
end