
#ifndef _YAKL_H_
#define _YAKL_H_

#include <iostream>
#include <algorithm>

#ifdef __NVCC__
  #define YAKL_LAMBDA [=] __host__ __device__
  #define YAKL_INLINE __host__ __device__
#elif defined(__USE_HIP__)
  #define YAKL_LAMBDA [=] __host__ __device__
  #define YAKL_INLINE __host__ __device__
  #include "hip_runtime.h"
#else
  #define YAKL_LAMBDA [=]
  #define YAKL_INLINE 
#endif


namespace yakl {

  
  int const functorBufSize = 1024*128;
  void *functorBuffer;
  int vectorSize = 128;


  void init(int vectorSize_in=128) {
    vectorSize = vectorSize_in;
    #if defined(__NVCC__)
      cudaMalloc(&functorBuffer,functorBufSize);
    #endif
    #if defined(__USE_HIP__)
      hipMalloc(&functorBuffer,functorBufSize);
    #endif
  }


  void finalize() {
    #if defined(__NVCC__)
      cudaFree(functorBuffer);
    #endif
    #if defined(__USE_HIP__)
      hipFree(functorBuffer);
    #endif
  }


  typedef unsigned long ulong;
  typedef unsigned int  uint;


  // Unpack 2D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int &i1, int &i2) {
    i1 = (iGlob/(n2))     ;
    i2 = (iGlob     ) % n2;
  }


  // Unpack 3D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int n3, int &i1, int &i2, int &i3) {
    i1 = (iGlob/(n3*n2))     ;
    i2 = (iGlob/(n3   )) % n2;
    i3 = (iGlob        ) % n3;
  }

  
  // Unpack 4D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int &i1, int &i2, int &i3, int &i4) {
    i1 = (iGlob/(n4*n3*n2))     ;
    i2 = (iGlob/(n4*n3   )) % n2;
    i3 = (iGlob/(n4      )) % n3;
    i4 = (iGlob           ) % n4;
  }

  
  // Unpack 5D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int &i1, int &i2, int &i3, int &i4, int &i5) {
    i1 = (iGlob/(n5*n4*n3*n2))     ;
    i2 = (iGlob/(n5*n4*n3   )) % n2;
    i3 = (iGlob/(n5*n4      )) % n3;
    i4 = (iGlob/(n5         )) % n4;
    i5 = (iGlob              ) % n5;
  }

  
  // Unpack 6D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int n6, int &i1, int &i2, int &i3, int &i4, int &i5, int &i6) {
    i1 = (iGlob/(n6*n5*n4*n3*n2))     ;
    i2 = (iGlob/(n6*n5*n4*n3   )) % n2;
    i3 = (iGlob/(n6*n5*n4      )) % n3;
    i4 = (iGlob/(n6*n5         )) % n4;
    i5 = (iGlob/(n6            )) % n5;
    i6 = (iGlob                 ) % n6;
  }

  
  // Unpack 7D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int &i1, int &i2, int &i3, int &i4, int &i5, int &i6, int &i7) {
    i1 = (iGlob/(n7*n6*n5*n4*n3*n2))     ;
    i2 = (iGlob/(n7*n6*n5*n4*n3   )) % n2;
    i3 = (iGlob/(n7*n6*n5*n4      )) % n3;
    i4 = (iGlob/(n7*n6*n5         )) % n4;
    i5 = (iGlob/(n7*n6            )) % n5;
    i6 = (iGlob/(n7               )) % n6;
    i7 = (iGlob                    ) % n7;
  }

  
  // Unpack 8D indices
  YAKL_INLINE void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int &i1, int &i2, int &i3, int &i4, int &i5, int &i6, int &i7, int &i8) {
    i1 = (iGlob/(n8*n7*n6*n5*n4*n3*n2))     ;
    i2 = (iGlob/(n8*n7*n6*n5*n4*n3   )) % n2;
    i3 = (iGlob/(n8*n7*n6*n5*n4      )) % n3;
    i4 = (iGlob/(n8*n7*n6*n5         )) % n4;
    i5 = (iGlob/(n8*n7*n6            )) % n5;
    i6 = (iGlob/(n8*n7               )) % n6;
    i7 = (iGlob/(n8                  )) % n7;
    i8 = (iGlob                       ) % n8;
  }


  #ifdef __NVCC__
    template <class F> __global__ void cudaKernelVal(ulong const nIter, F f) {
      ulong i = blockIdx.x*blockDim.x + threadIdx.x;
      if (i < nIter) {
        f( i );
      }
    }
    template <class F> __global__ void cudaKernelRef(ulong const nIter, F const &f) {
      ulong i = blockIdx.x*blockDim.x + threadIdx.x;
      if (i < nIter) {
        f( i );
      }
    }
    template<class F , typename std::enable_if< sizeof(F) <= 4000 , int >::type = 0> void parallel_for_cuda( int const nIter , F const &f ) {
      cudaKernelVal <<< (uint) (nIter-1)/vectorSize+1 , vectorSize >>> ( nIter , f );
    }
    template<class F , typename std::enable_if< sizeof(F) >= 4001 , int >::type = 0> void parallel_for_cuda( int const nIter , F const &f ) {
      F *fp = (F *) functorBuffer;
      cudaMemcpy(fp,&f,sizeof(F),cudaMemcpyHostToDevice);
      cudaKernelRef <<< (uint) (nIter-1)/vectorSize+1 , vectorSize >>> ( nIter , *fp );
    }
  #endif


  #ifdef __USE_HIP__
    template <class F> __global__ void cudaKernelVal(ulong const nIter, F f) {
      ulong i = hipBlockIdx_x*hipBlockDim_x + hipThreadIdx_x;
      if (i < nIter) {
        f( i );
      }
    }
    template <class F> __global__ void cudaKernelRef(ulong const nIter, F const &f) {
      ulong i = hipBlockIdx_x*hipBlockDim_x + hipThreadIdx_x;
      if (i < nIter) {
        f( i );
      }
    }
    // template<class F , typename std::enable_if< sizeof(F) <= 4000 , int >::type = 0> void parallel_for_hip( int const nIter , F const &f ) {
    //   hipLaunchKernelGGL( cudaKernelVal , dim3((nIter-1)/vectorSize+1) , dim3(vectorSize) , (std::uint32_t) 0 , (hipStream_t) 0 , nIter , f );
    // }
    // template<class F , typename std::enable_if< sizeof(F) >= 4001 , int >::type = 0> void parallel_for_hip( int const nIter , F const &f ) {
    //   F *fp = (F *) functorBuffer;
    //   hipMemcpy(fp,&f,sizeof(F),hipMemcpyHostToDevice);
    //   hipLaunchKernelGGL( cudaKernelVal , dim3((nIter-1)/vectorSize+1) , dim3(vectorSize) , (std::uint32_t) 0 , (hipStream_t) 0 , nIter , *fp );
    // }
    template<class F> void parallel_for_hip( int const nIter , F const &f ) {
      hipLaunchKernelGGL( cudaKernelVal , dim3((nIter-1)/vectorSize+1) , dim3(vectorSize) , (std::uint32_t) 0 , (hipStream_t) 0 , nIter , f );
    }
  #endif

  template <class F> void parallel_for_cpu_serial( int const nIter , F const &f ) {
    for (int i=0; i<nIter; i++) {
      f(i);
    }
  }


  template <class F> void parallel_for( int const nIter , F const &f ) {
    #ifdef __NVCC__
      parallel_for_cuda( nIter , f );
    #elif defined(__USE_HIP__)
      parallel_for_hip( nIter , f );
    #else
      parallel_for_cpu_serial( nIter , f );
    #endif
  }


  template <class F> void parallel_for( char const * str , int const nIter , F const &f ) {
    parallel_for( nIter , f );
  }


  void fence() {
    #ifdef __NVCC__
      cudaDeviceSynchronize();
    #endif
    #ifdef __USE_HIP__
      hipDeviceSynchronize();
    #endif
  }


  #ifdef __NVCC__
    __device__ __forceinline__ void atomicMin(float *address , float value) {
      int oldval, newval, readback;
      oldval = __float_as_int(*address);
      newval = __float_as_int( __int_as_float(oldval) < value ? __int_as_float(oldval) : value );
      while ( ( readback = atomicCAS( (int *) address , oldval , newval ) ) != oldval ) {
        oldval = readback;
        newval = __float_as_int( __int_as_float(oldval) < value ? __int_as_float(oldval) : value );
      }
    }

    __device__ __forceinline__ void atomicMin(double *address , double value) {
      unsigned long long oldval, newval, readback;
      oldval = __double_as_longlong(*address);
      newval = __double_as_longlong( __longlong_as_double(oldval) < value ? __longlong_as_double(oldval) : value );
      while ( ( readback = atomicCAS( (unsigned long long *) address , oldval , newval ) ) != oldval ) {
        oldval = readback;
        newval = __double_as_longlong( __longlong_as_double(oldval) < value ? __longlong_as_double(oldval) : value );
      }
    }

    __device__ __forceinline__ void atomicMax(float *address , float value) {
      int oldval, newval, readback;
      oldval = __float_as_int(*address);
      newval = __float_as_int( __int_as_float(oldval) > value ? __int_as_float(oldval) : value );
      while ( ( readback = atomicCAS( (int *) address , oldval , newval ) ) != oldval ) {
        oldval = readback;
        newval = __float_as_int( __int_as_float(oldval) > value ? __int_as_float(oldval) : value );
      }
    }

    __device__ __forceinline__ void atomicMax(double *address , double value) {
      unsigned long long oldval, newval, readback;
      oldval = __double_as_longlong(*address);
      newval = __double_as_longlong( __longlong_as_double(oldval) > value ? __longlong_as_double(oldval) : value );
      while ( ( readback = atomicCAS( (unsigned long long *) address , oldval , newval ) ) != oldval ) {
        oldval = readback;
        newval = __double_as_longlong( __longlong_as_double(oldval) > value ? __longlong_as_double(oldval) : value );
      }
    }
  #endif
  template <class FP> inline YAKL_INLINE void addAtomic(FP &x, FP const val) {
    #ifdef __NVCC__
      atomicAdd(&x,val);
    #else
      x += val;
    #endif
  }

  template <class FP> inline YAKL_INLINE void minAtomic(FP &a, FP const b) {
    #ifdef __NVCC__
      atomicMin(&a,b);
    #else
      a = a < b ? a : b;
    #endif
  }

  template <class FP> inline YAKL_INLINE void maxAtomic(FP &a, FP const b) {
    #ifdef __NVCC__
      atomicMax(&a,b);
    #else
      a = a > b ? a : b;
    #endif
  }

}


#endif

