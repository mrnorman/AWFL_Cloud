
#ifndef _WENO_LIMITER_H_
#define _WENO_LIMITER_H_

#include "const.h"
#include "SArray.h"
#include "TransformMatrices.h"

template <class FP> class WenoLimiter {

protected:


  SArray<FP,hs+2> idl;
  FP sigma;
  FP const eps = 1.0e-20;

public:

  _YAKL WenoLimiter() {
    if        (ord == 3) {
      FP sigma = 0.1_fp;
      idl(0) = 1._fp;
      idl(1) = 1._fp;
      idl(2) = 100._fp;
    } else if (ord == 5) {
      FP sigma = 0.1_fp;
      idl(0) = 1._fp;
      idl(1) = 100._fp;
      idl(2) = 1._fp;
      idl(3) = 1000._fp;
    } else if (ord == 7) {
      FP sigma = 0.01_fp;
      idl(0) = 1._fp;
      idl(1) = 20._fp;
      idl(2) = 20._fp;
      idl(3) = 1._fp;
      idl(4) = 400._fp;
    } else if (ord == 9) {
      sigma = 0.1_fp;
      idl(0) = 1._fp;
      idl(1) = 18._fp;
      idl(2) = 76._fp;
      idl(3) = 18._fp;
      idl(4) = 1._fp;
      idl(5) = 5832._fp;
    }
    convexify( idl );
  }


  inline _YAKL void compute_weno_coefs( SArray<FP,ord,ord,ord> const &recon , SArray<FP,ord> const &u , SArray<FP,ord> &aw ) {
    SArray<FP,hs+2> tv;
    SArray<FP,hs+2> wts;
    SArray<FP,hs+2,ord> a;
    SArray<FP,hs+1> lotmp;
    SArray<FP,ord > hitmp;
    TransformMatrices<FP> transform;
    FP lo_avg;

    // Init to zero
    for (int j=0; j<hs+2; j++) {
      for (int i=0; i<ord; i++) {
        a(j,i) = 0._fp;
      }
    }

    // Compute three quadratic polynomials (left, center, and right) and the high-order polynomial
    for(int i=0; i<hs+1; i++) {
      for (int ii=0; ii<hs+1; ii++) {
        for (int s=0; s<hs+1; s++) {
          a(i,ii) += recon(i,s,ii) * u(i+s);
        }
      }
    }
    for (int ii=0; ii<ord; ii++) {
      for (int s=0; s<ord; s++) {
        a(hs+1,ii) += recon(hs+1,s,ii) * u(s);
      }
    }

    // Compute "bridge" polynomial
    for (int i=0; i<hs+1; i++) {
      for (int ii=0; ii<hs+1; ii++) {
        a(hs+1,ii) -= idl(i)*a(i,ii);
      }
    }
    for (int ii=0; ii<ord; ii++) {
      a(hs+1,ii) /= idl(hs+1);
    }

    // Compute total variation of all candidate polynomials
    for (int i=0; i<hs+1; i++) {
      for (int ii=0; ii<hs+1; ii++) {
        lotmp(ii) = a(i,ii);
      }
      tv(i) = transform.coefs_to_tv(lotmp);
    }
      for (int ii=0; ii<ord; ii++) {
        hitmp(ii) = a(hs+1,ii);
      }
    tv(hs+1) = transform.coefs_to_tv(hitmp);

    // Reduce the bridge polynomial TV to something closer to the other TV values
    lo_avg = 0._fp;
    for (int i=0; i<hs+1; i++) {
      lo_avg += tv(i);
    }
    lo_avg /= hs+1;
    tv(hs+1) = lo_avg + ( tv(hs+1) - lo_avg ) * sigma;

    // WENO weights are proportional to the inverse of TV**2 and then re-confexified
    for (int i=0; i<hs+2; i++) {
      wts(i) = idl(i) / ( tv(i)*tv(i) + eps );
    }
    convexify(wts);

    // Map WENO weights for sharper fronts and less sensitivity to "eps"
    map_weights(idl,wts);
    convexify(wts);

    // WENO polynomial is the weighted sum of candidate polynomials using WENO weights instead of ideal weights
    for (int i=0; i<ord; i++) {
      aw(i) = 0._fp;
    }
    for (int i=0; i<hs+2; i++) {
      for (int ii=0; ii<ord; ii++) {
        aw(ii) += wts(i) * a(i,ii);
      }
    }
  }


  inline _YAKL void map_weights( SArray<FP,hs+2> const &idl , SArray<FP,hs+2> &wts ) {
    // Map the weights for quicker convergence. WARNING: Ideal weights must be (0,1) before mapping
    for (int i=0; i<hs+2; i++) {
      wts(i) = wts(i) * ( idl(i) + idl(i)*idl(i) - 3._fp*idl(i)*wts(i) + wts(i)*wts(i) ) / ( idl(i)*idl(i) + wts(i) * ( 1._fp - 2._fp * idl(i) ) );
    }
  }


  inline _YAKL void convexify( SArray<FP,hs+2> &wts ) {
    FP sum = 0._fp;
    for (int i=0; i<hs+2; i++) { sum += wts(i); }
    for (int i=0; i<hs+2; i++) { wts(i) /= (sum + eps); }
  }

};

#endif
