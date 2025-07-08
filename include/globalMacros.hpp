#ifndef GLOBALMACROS_HPP
#define GLOBALMACROS_HPP

//OpenCL version
#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

//Inline the function
#define FORCE_INLINE inline __attribute__((always_inline))

//Pack the struct
#define PACKSTRUCT __attribute__ ((packed))
 
#endif
