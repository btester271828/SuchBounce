global setup = "Setup/setup0.vec";
global out_n = 0;
include("sim_func.jl")

setup = "Setup/setup1.vec";
out_n = 1;
include("sim_func.jl")

setup = "Setup/setup2.vec";
out_n = 2;
include("sim_func.jl")

setup = "Setup/setup3.vec";
out_n = 3;
include("sim_func.jl")
