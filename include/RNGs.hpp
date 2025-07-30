#pragma once

//
// Generate random number
// using a linear congruential
// unsigned int 32-bit generator
//
uint32_t randqd_uint32(uint32_t rqd_seed){    
    rqd_seed = (uint32_t) (1664525UL * rqd_seed + 1013904223UL);
    return rqd_seed;
};

