for i = 1:100
    d = (0.7+i/100);
    r = 0.1;
    V1(i) = (1/30*(2*r+d))*((30*r^2+10*r+d^2)/d^7 + 2*(26*r^2-4*r-d^2)/(2*r+d)^7 + (14*2*r^2+58*r^2+18*r*d+d^2)/(4*r+d)^7);
end
plot(0.7+0.01*(1:100),V1)