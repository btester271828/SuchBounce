#define __CL_ENABLE_EXCEPTIONS
#define linux

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <CL/cl.hpp>
#include <array>
#include <stdio.h>

//Settings:
const int n = 512;
const double settings[9] = {pow(2,9), pow(2.0,-5), 0, 8e-19, 0, 0, 0, 1e-02, pow(2.0,4)};
const double max_time = pow(2,9);
const double warp = pow(2.0,4);
