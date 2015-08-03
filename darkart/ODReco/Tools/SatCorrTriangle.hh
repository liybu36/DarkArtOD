/*author: Yann Guardincerri translating S. Davini <stefano.davini@ge.infn.it>
 * Maintainer: Stefano Davini <stefano.davini@ge.infn.it>
 * 
 * Helper class for storing and computing basic channel (pulse) saturation information
 * and correction
*/

#ifndef __ODREC_SATURATION_TRIANGLE_H
#define __ODREC_SATURATION_TRIANGLE_H

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

class sat_corr_triangle_t{
public:
  sat_corr_triangle_t() {}
  virtual ~sat_corr_triangle_t() {}
  typedef std::pair <double, double> coordinate; // <time (ns), voltage>
  typedef std::pair <double, double> line_coeff; // <intercept, angular coeff>   
  
  void setP0(coordinate p0) {_p0 = p0;}
  void setP1(coordinate p1) {_p1 = p1;}
  void setP2(coordinate p2) {_p2 = p2;}
  void setP3(coordinate p3) {_p3 = p3;}
  bool checkDiffX(const coordinate& pa, const coordinate& pb) const {return pa.first!=pb.first;} // used to avoid division by zero
  bool checkP0P1() const {return checkDiffX(_p0, _p1);}
  bool checkP2P3() const {return checkDiffX(_p2, _p3);}
  line_coeff computeLine(const coordinate& pa, const coordinate& pb){
    const bool safe = checkDiffX(pa, pb);
    if (!safe) {
      const line_coeff lc_def (-9999., -9999.);
      return lc_def;
    } // I need to find default value to return
    
    const double par1 = (pb.second - pa.second)/(pb.first - pa.first);
    const double par0 = (pa.second - par1*pa.first);
    const line_coeff lc_tmp (par0, par1);
    return lc_tmp;
  }
  void computeLine10() {_l01 = computeLine(_p0, _p1);}
  void computeLine23() {_l23 = computeLine(_p2, _p3);}
  coordinate computeInterception (const line_coeff& l0, const line_coeff& l2) const {
    const bool safe = (l0.second!=l2.second);
    if (!safe) {
      const coordinate pdef (-9999., -9999.);
      return pdef;
    }
    coordinate pi;
    pi.first = (l2.first - l0.first)/(l0.second - l2.second);
    pi.second = l0.first + l0.second*  pi.first;
    return pi;
  }
  void computePt() {_pt = computeInterception(_l01, _l23);}
  double computeArea (const coordinate& a, const coordinate& b, const coordinate& c) const {
    return 0.5 * std::abs( (c.first - a.first)*(b.second - a.second) - (c.second - a.second)*(b.first - a.first)) ;
  }
  double computeCorrection(){
    const line_coeff lc_default (-9999., -9999.);
    computeLine10();
    if (_l01== lc_default) return 0;
    computeLine23();
    if (_l23== lc_default) return 0;
    const coordinate cr_default (-9999., -9999.);
    computePt();
    if (_pt == cr_default) return 0;
    return computeArea(_p1, _p2, _pt);
  }
  double computeY(double x) {
    if(x<_pt.first) return _l01.second*x+_l01.first; 
    else return _l23.second*x+_l23.first; 
  }
  
  
private:
  coordinate _p0, _p1, _p2, _p3, _pt;
  line_coeff _l01, _l23;

        /*      Description of coordinates:
 
                                   pt (saturation top)
                                   +
                p1 (begin sat.)                         p2 (end sat.)
                  ***************************************
                 *                                       *
                *                                         *
        p0 (begin - few samples)                                p3(end + few samples)
        *                                                       *
 
        */


};


#endif
