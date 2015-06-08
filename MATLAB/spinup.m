m = 1e-2;
rad1 = 0.01;
v_0 = sqrt(6.374e-11*m/rad1)/2;
w = v_0/rad1;
for i = 1:512
v_temp = cross((reshape(r(512,:),3,1)),w*[1 0 0]);
v_temp = cross((reshape(r(i,:),3,1)),w*[1 0 0]);
v(1,i,:) = reshape(v_temp,1,1,3);
end
%filewrite('v.vec',v,'Julia')
%vfilewrite('r.vec',r,'Julia')