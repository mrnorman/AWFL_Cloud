
#ifndef __YAKL_H_
#define __YAKL_H_

#include <iostream>
#include <algorithm>

#ifdef __NVCC__
  #define _YAKL __host__ __device__
#else
  #define _YAKL
#endif


namespace yakl {

  typedef unsigned long ulong;
  typedef unsigned int  uint;


  // Unpack 2D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int &i1, int &i2) {
    i1 = (iGlob/(n2))     ;
    i2 = (iGlob     ) % n2;
  }


  // Unpack 3D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int n3, int &i1, int &i2, int &i3) {
    i1 = (iGlob/(n3*n2))     ;
    i2 = (iGlob/(n3   )) % n2;
    i3 = (iGlob        ) % n3;
  }

  
  // Unpack 4D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int &i1, int &i2, int &i3, int &i4) {
    i1 = (iGlob/(n4*n3*n2))     ;
    i2 = (iGlob/(n4*n3   )) % n2;
    i3 = (iGlob/(n4      )) % n3;
    i4 = (iGlob           ) % n4;
  }

  
  // Unpack 5D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int &i1, int &i2, int &i3, int &i4, int &i5) {
    i1 = (iGlob/(n5*n4*n3*n2))     ;
    i2 = (iGlob/(n5*n4*n3   )) % n2;
    i3 = (iGlob/(n5*n4      )) % n3;
    i4 = (iGlob/(n5         )) % n4;
    i5 = (iGlob              ) % n5;
  }

  
  // Unpack 6D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int n6, int &i1, int &i2, int &i3, int &i4, int &i5, int &i6) {
    i1 = (iGlob/(n6*n5*n4*n3*n2))     ;
    i2 = (iGlob/(n6*n5*n4*n3   )) % n2;
    i3 = (iGlob/(n6*n5*n4      )) % n3;
    i4 = (iGlob/(n6*n5         )) % n4;
    i5 = (iGlob/(n6            )) % n5;
    i6 = (iGlob                 ) % n6;
  }

  
  // Unpack 7D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int &i1, int &i2, int &i3, int &i4, int &i5, int &i6, int &i7) {
    i1 = (iGlob/(n7*n6*n5*n4*n3*n2))     ;
    i2 = (iGlob/(n7*n6*n5*n4*n3   )) % n2;
    i3 = (iGlob/(n7*n6*n5*n4      )) % n3;
    i4 = (iGlob/(n7*n6*n5         )) % n4;
    i5 = (iGlob/(n7*n6            )) % n5;
    i6 = (iGlob/(n7               )) % n6;
    i7 = (iGlob                    ) % n7;
  }

  
  // Unpack 8D indices
  _YAKL void unpackIndices(int iGlob, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int &i1, int &i2, int &i3, int &i4, int &i5, int &i6, int &i7, int &i8) {
    i1 = (iGlob/(n8*n7*n6*n5*n4*n3*n2))     ;
    i2 = (iGlob/(n8*n7*n6*n5*n4*n3   )) % n2;
    i3 = (iGlob/(n8*n7*n6*n5*n4      )) % n3;
    i4 = (iGlob/(n8*n7*n6*n5         )) % n4;
    i5 = (iGlob/(n8*n7*n6            )) % n5;
    i6 = (iGlob/(n8*n7               )) % n6;
    i7 = (iGlob/(n8                  )) % n7;
    i8 = (iGlob                       ) % n8;
  }


  // These unfotunately cannot live inside classes in CUDA :-/
  #ifdef __NVCC__
  template <class F, class... Args> __global__ void parallelForCUDA(ulong const nIter, F &f, Args&&... args) {
    ulong i = blockIdx.x*blockDim.x + threadIdx.x;
    if (i < nIter) {
      f( i , args... );
    }
  }
  #endif


  uint const targetCUDA      = 1;
  uint const targetCPUSerial = 2;


  class Launcher {
    protected:
      uint target;
      uint vectorSize;
      #ifdef __NVCC__
      cudaStream_t myStream;
      #endif

      template <class F, class... Args> inline void parallelForCPU(ulong const nIter, F &f, Args&&... args) {
        for (ulong i=0; i<nIter; i++) {
          f( i , args... );
        }
      }

    public:


      Launcher() {
        target = 0;
        vectorSize = 0;
        init(targetCPUSerial,128);
      }

      Launcher(uint targetIn) {
        target = 0;
        vectorSize = 0;
        init(targetIn,128);
      }

      Launcher(uint targetIn, uint vectorSizeIn) {
        target = 0;
        vectorSize = 0;
        init(targetIn,vectorSizeIn);
      }

      ~Launcher() {
        finalize();
      }

      void init(uint targetIn=targetCPUSerial, uint vectorSizeIn=128) {
        finalize();
        target = targetIn;
        vectorSize = vectorSizeIn;
        if (target == targetCUDA) {
          #ifdef __NVCC__
            // Always create a CUDA stream upon instantiation
            cudaStreamCreate(&myStream);
          #endif
        }
      }

      void finalize() {
        if (target == targetCUDA && target > 0) {
          #ifdef __NVCC__
            // Always destroy a CUDA stream upon finalize
            cudaStreamDestroy(myStream);
          #endif
        }
        target = 0;
        vectorSize = 0;
      }

      template <class F, class... Args> inline void parallelFor(ulong const nIter, F &f, Args&&... args) {
        std::cout << "Got here 1\n";
        if        (target == targetCUDA) {
          #ifdef __NVCC__
            parallelForCUDA <<< (uint) nIter/vectorSize+1 , vectorSize , 0 , myStream >>> ( nIter , f , args... );
          #else
            std::cerr << "ERROR: "<< __FILE__ << ":" << __LINE__ << "\n";
            std::cerr << "Attempting to launch CUDA without nvcc\n";
          #endif
        } else if (target == targetCPUSerial ) {
          parallelForCPU( nIter , f , args... );
        }
      }

      template <class F, class... Args> inline void parallelFor(ulong const nIter, F const &f, Args&&... args) {
        if        (target == targetCUDA) {
          #ifdef __NVCC__
            parallelForCUDA <<< (uint) nIter/vectorSize+1 , vectorSize , 0 , myStream >>> ( nIter , f , args... );
          #else
            std::cerr << "ERROR: "<< __FILE__ << ":" << __LINE__ << "\n";
            std::cerr << "Attempting to launch CUDA without nvcc\n";
          #endif
        } else if (target == targetCPUSerial ) {
          parallelForCPU( nIter , f , args... );
        }
      }

      void synchronizeSelf() {
        if        (target == targetCUDA) {
        #ifdef __NVCC__
          cudaStreamSynchronize(myStream);
        #endif
        } else if (target == targetCPUSerial ) {
        }
      }

      void synchronizeGlobal() {
        if        (target == targetCUDA) {
        #ifdef __NVCC__
          cudaDeviceSynchronize();
        #endif
        } else if (target == targetCPUSerial ) {
        }
      }

  };


  template <uint target> class Atomics {
  protected:
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
  public:
    template <class FP> inline _YAKL void addAtomic(FP &x, FP const val) {
      if (target == targetCUDA) {
        #ifdef __NVCC__
          atomicAdd(&x,val);
        #endif
      } else if (target == targetCPUSerial) {
        x += val;
      }
    }

    template <class FP> inline _YAKL void minAtomic(FP &a, FP const b) {
      if (target == targetCUDA) {
        #ifdef __NVCC__
          atomicMin(&a,b);
        #endif
      } else if (target == targetCPUSerial) {
        a = a < b ? a : b;
      }
    }

    template <class FP> inline _YAKL void maxAtomic(FP &a, FP const b) {
      if (target == targetCUDA) {
        #ifdef __NVCC__
          atomicMax(&a,b);
        #endif
      } else if (target == targetCPUSerial) {
        a = a > b ? a : b;
      }
    }

  };


}


#endif

