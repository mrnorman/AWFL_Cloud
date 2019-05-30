
#ifndef _TIMEINTEGRATOR_H_
#define _TIMEINTEGRATOR_H_

#include "const.h"
#include "Parallel.h"
#include "Domain.h"
#include "State.h"
#include "Tendencies.h"

class TimeIntegrator {

  real4d stateTmp;
  real4d tendArr;
  real4d tendArrTmp;
  Tendencies tend;
  int dsSwitch;

public :


  inline void initialize(Domain &dom) {
    if (timeMethod == TIME_SSPRK3) {
      stateTmp   = real4d("stateTmp"  ,numState,dom.nz+2*hs,dom.ny+2*hs,dom.nx+2*hs);
      tendArrTmp = real4d("tendArrTmp",numState,dom.nz,dom.ny,dom.nx);
    }
    tendArr = real4d("tendArr",numState,dom.nz,dom.ny,dom.nx);
    tend.initialize(dom);
    dsSwitch = 1;
  }


  inline void stepForward(State &state, Domain &dom, Exchange &exch, Parallel const &par) {
    if (timeMethod == TIME_SSPRK3) {
      stepForwardSSPRK3(state, dom, exch, par);
    } else if (timeMethod == TIME_ADER) {
      stepForwardADER(state, dom, exch, par);
    } else {
      std::cout << "Error: Unrecognized timeMethod\n";
      exit(-1);
    }
  }


  inline void stepForwardADER(State &state, Domain &dom, Exchange &exch, Parallel const &par) {
    if (dsSwitch) {
      dsSwitch = 0;
      tend.compEulerTendADER_X(state.state, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr);
      applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      if (!dom.run2d) {
        tend.compEulerTendADER_Y(state.state, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr);
        applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      }
      dom.dt /= 2;
      tend.compEulerTendADER_Z(state.state, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArr);
      applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      tend.compEulerTendADER_Z(state.state, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArr);
      applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      dom.dt *= 2;
    } else {
      dsSwitch = 1;
      dom.dt /= 2;
      tend.compEulerTendADER_Z(state.state, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArr);
      applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      tend.compEulerTendADER_Z(state.state, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArr);
      applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      dom.dt *= 2;
      if (!dom.run2d) {
        tend.compEulerTendADER_Y(state.state, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr);
        applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
      }
      tend.compEulerTendADER_X(state.state, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr);
      applyTendencies( state.state , 1._fp , state.state , 0._fp , state.state , 1._fp , tendArr, dom);
    }
    applyHeatingCooling(state.state,state.hyDensCells,dom);
  }


  inline void stepForwardSSPRK3(State &state, Domain const &dom, Exchange &exch, Parallel const &par) {
    // Stage 1
    tend.compEulerTendSD_X(state.state, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr   );
    if (!dom.run2d) {
      tend.compEulerTendSD_Y(state.state, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    }
    tend.compEulerTendSD_Z(state.state, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    tend.compEulerTendSD_S(state.state,                                        dom,            tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    applyTendencies( stateTmp , 1._fp , state.state , 0._fp , stateTmp , 1._fp , tendArr, dom);

    // Stage 2
    tend.compEulerTendSD_X(stateTmp, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr   );
    if (!dom.run2d) {
      tend.compEulerTendSD_Y(stateTmp, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    }
    tend.compEulerTendSD_Z(stateTmp, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    tend.compEulerTendSD_S(stateTmp,                                        dom,            tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    applyTendencies( stateTmp , 0.75_fp , state.state , 0.25_fp , stateTmp , 0.25_fp , tendArr, dom);

    // Stage 3
    tend.compEulerTendSD_X(stateTmp, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArr   );
    if (!dom.run2d) {
      tend.compEulerTendSD_Y(stateTmp, state.hyDensCells, state.hyDensThetaCells, dom, exch, par, tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    }
    tend.compEulerTendSD_Z(stateTmp, state.hyDensGLL, state.hyDensThetaGLL, dom, exch, par, tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    tend.compEulerTendSD_S(stateTmp,                                        dom,            tendArrTmp); appendTendencies(tendArr, tendArrTmp, dom);
    applyTendencies( state.state , 1._fp/3._fp , state.state , 2._fp/3._fp , stateTmp , 2._fp/3._fp , tendArr , dom);
  }


  inline void applyTendencies(real4d &state2, real const c0, real4d const &state0,
                                              real const c1, real4d const &state1,
                                              real const ct, real4d const &tend, Domain const &dom) {
    // for (int l=0; l<numState; l++) {
    //   for (int k=0; k<dom.nz; k++) {
    //     for (int j=0; j<dom.ny; j++) {
    //       for (int i=0; i<dom.nx; i++) {
    Kokkos::parallel_for( numState*dom.nz*dom.ny*dom.nx , KOKKOS_LAMBDA (int const iGlob) {
      int l, k, j, i;
      unpackIndices(iGlob,numState,dom.nz,dom.ny,dom.nx,l,k,j,i);
      state2(l,hs+k,hs+j,hs+i) = c0 * state0(l,hs+k,hs+j,hs+i) + c1 * state1(l,hs+k,hs+j,hs+i) + ct * dom.dt * tend(l,k,j,i);
    });
  }


  inline void appendTendencies(real4d &tend, real4d const &tendTmp, Domain const &dom) {
    // for (int l=0; l<numState; l++) {
    //   for (int k=0; k<dom.nz; k++) {
    //     for (int j=0; j<dom.ny; j++) {
    //       for (int i=0; i<dom.nx; i++) {
    Kokkos::parallel_for( numState*dom.nz*dom.ny*dom.nx , KOKKOS_LAMBDA (int const iGlob) {
      int l, k, j, i;
      unpackIndices(iGlob,numState,dom.nz,dom.ny,dom.nx,l,k,j,i);
      tend(l,k,j,i) += tendTmp(l,k,j,i);
    });
  }


  inline void applyHeatingCooling(real4d &state, real1d const &hyDens, Domain const &dom) {
    // for (int k=0; k<dom.nz; k++) {
    //   for (int j=0; j<dom.ny; j++) {
    //     for (int i=0; i<dom.nx; i++) {
    Kokkos::parallel_for( dom.nz*dom.ny*dom.nx , KOKKOS_LAMBDA (int const iGlob) {
      int k, j, i;
      unpackIndices(iGlob,dom.nz,dom.ny,dom.nx,k,j,i);
      real xloc = (i+0.5_fp)*dom.dx;
      real yloc = (j+0.5_fp)*dom.dy;
      real zloc = (k+0.5_fp)*dom.dz;
      if (dom.run2d) {yloc = dom.ylen/2;}
      state(idRT,hs+k,hs+j,hs+i) += dom.dt * ellipsoid_linear(xloc, yloc, zloc, dom.xlen/2, dom.ylen/2, 0.      , 2000.,2000.,2000.,  0.05);
      state(idRT,hs+k,hs+j,hs+i) += dom.dt * ellipsoid_linear(xloc, yloc, zloc, dom.xlen/2, dom.ylen/2, dom.zlen, 2000.,2000.,2000., -0.05);
    });
  }


  inline _HOSTDEV real ellipsoid_linear(real const x   , real const y   , real const z ,
                                        real const x0  , real const y0  , real const z0,
                                        real const xrad, real const yrad, real const zrad, real const amp) {
    real xn = (x-x0)/xrad;
    real yn = (y-y0)/yrad;
    real zn = (z-z0)/zrad;
    real dist = mysqrt( xn*xn + yn*yn + zn*zn );
    return amp * max( 1._fp - dist , 0._fp );
  }

};

#endif
