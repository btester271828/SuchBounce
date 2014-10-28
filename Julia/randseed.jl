function randseed(r::Array{Float64,2},v::Array{Float64,2},w::Array{Float64,2},q::Array{Float64,2},rad::Array{Float64,2})

const n = size(q,2);
r[:,1] = 3*rand(3,1);

for i = 2:n
	
    global occupied = bool(ones(i-1,1));
    while any(occupied)
        r[:,i] = 2*rand(3,1);        
	local d = zeros((i-1),1);        
	for j = 1:(i-1)
		d[j] = norm(r[:,i] - r[:,j]);
			if (d[j] > rad[i] + rad[j] + 0.0001)
				occupied[j] = bool(0);
			end			
        end
    end
end    

for i = 1:n
    v[:,i] = 5e-3*(0.5-rand(3,1));
    q[i] = 1e-8*(0.5-rand());
end
return r, v, w, q

end
