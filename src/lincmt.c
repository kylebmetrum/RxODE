#include <R.h>
#include <Rinternals.h>

/* extern SEXP RxODE_ode_dosing(); */
// extern "C" double getLinDeriv(int ncmt, int diff1, int diff2, double rate, double tinf, double Dose, double ka, double tlag, double T, double tT, mat par);
extern double RxODE_solveLinB(double t, int linCmt, int diff1, int diff2, double A, double alpha, double B, double beta, double C, double gamma, double ka, double tlag);
extern unsigned int nDoses();
extern double rxDosingTime(int i);
extern int rxDosingEvid(int i);
double rxDose(int i);

int locateDoseIndex(const double obs_time){
  // Uses bisection for slightly faster lookup of dose index.
  int i, j, ij;
  i = 0;
  j = nDoses() - 1;
  if (obs_time <= rxDosingTime(i)){
    return i;
  }
  if (obs_time >= rxDosingTime(j)){
    return j;
  }
  while(i < j - 1) { /* x[i] <= obs_time <= x[j] */
    ij = (i + j)/2; /* i+1 <= ij <= j-1 */
    if(obs_time < rxDosingTime(ij))
      j = ij;
    else
      i = ij;
  }
  return i;
}//subscript of dose

double RxODE_solveLinB(double t, int linCmt, int diff1, int diff2, double d_A, double d_alpha, double d_B, double d_beta, double d_C, double d_gamma, double d_ka, double d_tlag){
  unsigned int ncmt = 1;
  if (d_C > 0 && d_gamma > 0){
    ncmt = 3;
  } else if (d_B > 0 && d_beta > 0){
    ncmt = 2;
  } else if (d_A > 0 && d_alpha > 0){
    ncmt = 1;
  } else {
    error("You need to specify at least A(=%f) and alpha (=%f). (@t=%f, d1=%d, d2=%d)", d_A, d_alpha, t, diff1, diff2);
  }

  double alpha = d_alpha;
  double A = d_A;
  double beta = d_beta;
  double B = d_B;
  double gamma = d_gamma;
  double C = d_C;
  double ka = d_ka;
  double tlag = d_tlag;
  
  int oral, cmt;
  oral = (ka > 0) ? 1 : 0;
  double ret = 0;
  unsigned int m = 0, l = 0, p = 0;
  int evid;
  double thisT = 0.0, tT = 0.0, res, t1, t2, tinf, dose = 0;
  double rate;
  m = locateDoseIndex(t);
  for(l=0; l <= m; l++){
    //superpostion
    evid = rxDosingEvid(l);
    dose = rxDose(l);
    cmt = (evid%10000)/100 - 1;
    if (cmt != linCmt) continue;
    if (evid > 10000) {
      if (dose > 0){
	// During infusion
	tT = t - rxDosingTime(l);
	thisT = tT - tlag;
	p = l+1;
	while (p < nDoses() && rxDose(p) != -dose){
	  p++;
	}
	if (rxDose(p) != -dose){
	  error("Could not find a error to the infusion.  Check the event table.");
	}
	tinf  = rxDosingTime(p)-rxDosingTime(l);
	rate  = dose;
	if (tT >= tinf) continue;
      } else {
	// After  infusion
	p = l-1;
	while (p >= 0 && rxDose(p) != -dose){
	  p--;
	}
	if (rxDose(p) != -dose){
	  error("Could not find a start to the infusion.  Check the event table.");
	}
	tinf  = rxDosingTime(l) - rxDosingTime(p) - tlag;
	
	tT = t - rxDosingTime(p);
        thisT = tT- tlag;       

	rate  = -dose;
      }
      t1 = ((thisT < tinf) ? thisT : tinf);        //during infusion
      t2 = ((thisT > tinf) ? thisT - tinf : 0.0);  // after infusion
      switch(diff1){
      case 0: // Solved equation -- A
	////////////////////////////////////////////////////////////////////////////////
	ret += rate*A / alpha * (1 - exp(-alpha * t1)) * exp(-alpha * t2);
        break;
      case 1: // dA
	switch(diff2){
	case 0:
	  ret += rate/alpha * (1 - exp(-alpha * t1)) * exp(-alpha * t2);
	  break;
	case 2: // dA, dAlpha
	  ret += (rate/alpha * (exp(-alpha * t1) * t1) - rate/(alpha*alpha) *  (1 - exp(-alpha * t1))) * exp(-alpha * t2) -
	    rate/alpha * (1 - exp(-alpha * t1)) * (exp(-alpha * t2) * t2);
          break;
	case 8: // dA, dTlag
	  if (thisT < tinf){
            // During infusion
            // t1=thisT
            // t2=0;
	    ret -= (rate/alpha * (exp(-alpha * (tT - tlag)) * alpha));
	  } else {
	    ret += rate/alpha * (1 - exp(-alpha * tinf)) * (exp(-alpha * (tT - tinf - tlag)) * alpha);
          }
	  break;
	}
	break;
      case 2: // dAlpha
	switch(diff2){
        case 0:
	  ret += (rate * A/alpha * (exp(-alpha * t1) * t1) - rate * A/(alpha*alpha) * (1 - exp(-alpha * t1))) * exp(-alpha * t2) -
	    rate * A/alpha * (1 - exp(-alpha * t1)) * (exp(-alpha * t2) * t2);
	  break;
	case 1: // dAlpha, dA
	  ret += (rate/alpha * (exp(-alpha * t1) * t1) - rate/(alpha*alpha) *  (1 - exp(-alpha * t1))) * exp(-alpha * t2) -
            rate/alpha * (1 - exp(-alpha * t1)) * (exp(-alpha * t2) * t2);
          break;
	case 2: // dAlpha, dAlpha
	  ret -= ((rate * A/alpha * exp(-alpha * t1) * t1 - (rate * A/(alpha * alpha)) * (1 - exp(-alpha * t1))) * exp(-alpha * t2) * t2 +
		  (rate * A/alpha * (exp(-alpha * t1) * t1 * t1) +
		   rate * A/(alpha*alpha) * exp(-alpha * t1) * t1 +
		   ((rate * A/(alpha * alpha)) * exp(-alpha * t1) * t1 - rate * A *  (alpha + alpha)/pow(alpha,4) * (1 - exp(-alpha * t1)))) * exp(-alpha * t2) +
		  ((rate * A/alpha * exp(-alpha * t1) * t1 -  rate * A/(alpha*alpha) * (1 - exp(-alpha * t1))) * exp(-alpha * t2) * t2 -
		   rate * A/alpha * (1 - exp(-alpha * t1)) * (exp(-alpha * t2) * t2 * t2)));
	  break;
	case 8: // dAlpha, dTlag
	  if (thisT < tinf){
            // During infusion
            // t1=thisT
            // t2=0;
	    ret += rate * A/alpha * (exp(-alpha * (tT - tlag)) * alpha * (tT - tlag) - exp(-alpha * (tT - tlag))) +
	      rate * A/(alpha * alpha) * exp(-alpha * (tT - tlag)) * alpha;
          } else {
	    // After infusion
            // t1=tinf
            // t2 = thisT-tinf
	    ret += (rate * A/alpha * ((exp(-alpha * tinf)) * tinf) - rate * A/(alpha * alpha) * (1 - (exp(-alpha * tinf)))) * exp(-alpha * (tT - tinf - tlag)) * alpha -
	      rate * A/alpha * (1 - (exp(-alpha * tinf))) * (exp(-alpha * (tT - tinf - tlag)) * alpha *  (tT - tinf - tlag) - exp(-alpha * (tT - tinf - tlag)));
	  }
	  break;
	}
        break;
      case 8: //dTlag
	if (thisT < tinf){
	  // During infusion
	  // t1=thisT
	  // t2=0;
	  switch(diff2){
	  case 0:
	    ret -= (rate * A/alpha * (exp(-alpha * (tT - tlag)) * alpha));
	    break;
	  case 1: // dTlag, dA
	    ret -= (rate/alpha * exp(-alpha * (tT - tlag)) * alpha);
	    break;
	  case 2: // dTlag, dAlpha
	    ret -= (rate * A/alpha * (exp(-alpha * (tT - tlag)) - exp(-alpha * (tT - tlag)) * (tT - tlag) * alpha) -
		    rate * A/(alpha*alpha) * exp(-alpha * (tT - tlag)) * alpha);
	    break;
	  case 8: // dTlag dTlag
	    ret -= (rate * A/alpha * (exp(-alpha * (tT - tlag)) * alpha * alpha));
	    break;
          }
	} else {
	  // After infusion
	  // t1=tinf
	  // t2 = thisT-tinf
	  switch(diff2){
	  case 0:
	    ret += rate * A/alpha * (1 - exp(-alpha * tinf)) * (exp(-alpha * (tT - tlag - tinf)) * alpha);
	    break;
	  case 1: //dTlag, dA
	    ret += rate/alpha * (1 - exp(-alpha * tinf)) *  exp(-alpha * (tT - tlag - tinf)) * alpha;
	    break;
	  case 2: //dTlag, dAlpha
	    ret += (rate * A/alpha * (exp(-alpha * tinf) * tinf) - rate * A/(alpha*alpha) *  (1 - exp(-alpha * tinf))) * exp(-alpha * (tT - tlag - tinf)) * alpha + rate * A/alpha * (1 - exp(-alpha * tinf)) * (exp(-alpha * (tT - tlag - tinf)) - exp(-alpha * (tT - tlag - tinf)) * (tT - tlag - tinf) * alpha);
	    break;
          case 8: //dTlag, dTlag
	    ret += rate * A/alpha * (1 - exp(-alpha * tinf)) * (exp(-alpha * (tT - tlag - tinf)) * alpha * alpha);
	    break;
          }
	}
      }
      ////////////////////////////////////////////////////////////////////////////////
      if (ncmt >= 2){
	switch(diff1){
        case 0: // Solved equation -- B
          ////////////////////////////////////////////////////////////////////////////////
          ret += rate*B / beta * (1 - exp(-beta * t1)) * exp(-beta * t2);
          break;
        case 3: // dA
          switch(diff2){
          case 0:
            ret += rate/beta * (1 - exp(-beta * t1)) * exp(-beta * t2);
            break;
          case 4: // dA, dBeta
            ret += (rate/beta * (exp(-beta * t1) * t1) - rate/(beta*beta) *  (1 - exp(-beta * t1))) * exp(-beta * t2) -
              rate/beta * (1 - exp(-beta * t1)) * (exp(-beta * t2) * t2);
            break;
          case 8: // dA, dTlag
            if (thisT < tinf){
              // During infusion
              // t1=thisT
              // t2=0;
              ret -= (rate/beta * (exp(-beta * (tT - tlag)) * beta));
            } else {
              ret += rate/beta * (1 - exp(-beta * tinf)) * (exp(-beta * (tT - tinf - tlag)) * beta);
            }
            break;
          }
          break;
        case 4: // dBeta
          switch(diff2){
          case 0:
            ret += (rate * B/beta * (exp(-beta * t1) * t1) - rate * B/(beta*beta) * (1 - exp(-beta * t1))) * exp(-beta * t2) -
              rate * B/beta * (1 - exp(-beta * t1)) * (exp(-beta * t2) * t2);
            break;
          case 3: // dBeta, dA
            ret += (rate/beta * (exp(-beta * t1) * t1) - rate/(beta*beta) *  (1 - exp(-beta * t1))) * exp(-beta * t2) -
              rate/beta * (1 - exp(-beta * t1)) * (exp(-beta * t2) * t2);
            break;
          case 4: // dBeta, dBeta
            ret -= ((rate * B/beta * exp(-beta * t1) * t1 - (rate * B/(beta * beta)) * (1 - exp(-beta * t1))) * exp(-beta * t2) * t2 +
                    (rate * B/beta * (exp(-beta * t1) * t1 * t1) +
                     rate * B/(beta*beta) * exp(-beta * t1) * t1 +
                     ((rate * B/(beta * beta)) * exp(-beta * t1) * t1 - rate * B *  (beta + beta)/pow(beta,4) * (1 - exp(-beta * t1)))) * exp(-beta * t2) +
                    ((rate * B/beta * exp(-beta * t1) * t1 -  rate * B/(beta*beta) * (1 - exp(-beta * t1))) * exp(-beta * t2) * t2 -
                     rate * B/beta * (1 - exp(-beta * t1)) * (exp(-beta * t2) * t2 * t2)));
            break;
          case 8: // dBeta, dTlag
            if (thisT < tinf){
              // During infusion
              // t1=thisT
              // t2=0;
              ret += rate * B/beta * (exp(-beta * (tT - tlag)) * beta * (tT - tlag) - exp(-beta * (tT - tlag))) +
                rate * B/(beta * beta) * exp(-beta * (tT - tlag)) * beta;
            } else {
              // After infusion
              // t1=tinf
              // t2 = thisT-tinf
              ret += (rate * B/beta * ((exp(-beta * tinf)) * tinf) - rate * B/(beta * beta) * (1 - (exp(-beta * tinf)))) * exp(-beta * (tT - tinf - tlag)) * beta -
                rate * B/beta * (1 - (exp(-beta * tinf))) * (exp(-beta * (tT - tinf - tlag)) * beta *  (tT - tinf - tlag) - exp(-beta * (tT - tinf - tlag)));
            }
            break;
          }
          break;
        case 8: //dTlag
          if (thisT < tinf){
            // During infusion
            // t1=thisT
            // t2=0;
            switch(diff2){
            case 0:
              ret -= (rate * B/beta * (exp(-beta * (tT - tlag)) * beta));
              break;
            case 3: // dTlag, dA
              ret -= (rate/beta * exp(-beta * (tT - tlag)) * beta);
              break;
            case 4: // dTlag, dBeta
              ret -= (rate * B/beta * (exp(-beta * (tT - tlag)) - exp(-beta * (tT - tlag)) * (tT - tlag) * beta) -
                      rate * B/(beta*beta) * exp(-beta * (tT - tlag)) * beta);
              break;
            case 8: // dTlag dTlag
              ret -= (rate * B/beta * (exp(-beta * (tT - tlag)) * beta * beta));
              break;
            }
          } else {
            // After infusion
            // t1=tinf
            // t2 = thisT-tinf
            switch(diff2){
            case 0:
              ret += rate * B/beta * (1 - exp(-beta * tinf)) * (exp(-beta * (tT - tlag - tinf)) * beta);
              break;
            case 3: //dTlag, dA
              ret += rate/beta * (1 - exp(-beta * tinf)) *  exp(-beta * (tT - tlag - tinf)) * beta;
              break;
            case 4: //dTlag, dBeta
              ret += (rate * B/beta * (exp(-beta * tinf) * tinf) - rate * B/(beta*beta) *  (1 - exp(-beta * tinf))) * exp(-beta * (tT - tlag - tinf)) * beta + rate * B/beta * (1 - exp(-beta * tinf)) * (exp(-beta * (tT - tlag - tinf)) - exp(-beta * (tT - tlag - tinf)) * (tT - tlag - tinf) * beta);
              break;
            case 8: //dTlag, dTlag
              ret += rate * B/beta * (1 - exp(-beta * tinf)) * (exp(-beta * (tT - tlag - tinf)) * beta * beta);
              break;
	    }
	  }
	}
        if (ncmt >= 3){
	  switch(diff1){
          case 0: // Solved equation -- C
            ////////////////////////////////////////////////////////////////////////////////
            ret += rate*C / gamma * (1 - exp(-gamma * t1)) * exp(-gamma * t2);
            break;
          case 5: // dA
            switch(diff2){
            case 0:
              ret += rate/gamma * (1 - exp(-gamma * t1)) * exp(-gamma * t2);
              break;
            case 6: // dA, dGamma
              ret += (rate/gamma * (exp(-gamma * t1) * t1) - rate/(gamma*gamma) *  (1 - exp(-gamma * t1))) * exp(-gamma * t2) -
                rate/gamma * (1 - exp(-gamma * t1)) * (exp(-gamma * t2) * t2);
              break;
            case 8: // dA, dTlag
              if (thisT < tinf){
                // During infusion
                // t1=thisT
                // t2=0;
                ret -= (rate/gamma * (exp(-gamma * (tT - tlag)) * gamma));
              } else {
                ret += rate/gamma * (1 - exp(-gamma * tinf)) * (exp(-gamma * (tT - tinf - tlag)) * gamma);
              }
              break;
            }
            break;
          case 6: // dGamma
            switch(diff2){
            case 0:
              ret += (rate * C/gamma * (exp(-gamma * t1) * t1) - rate * C/(gamma*gamma) * (1 - exp(-gamma * t1))) * exp(-gamma * t2) -
                rate * C/gamma * (1 - exp(-gamma * t1)) * (exp(-gamma * t2) * t2);
              break;
            case 5: // dGamma, dA
              ret += (rate/gamma * (exp(-gamma * t1) * t1) - rate/(gamma*gamma) *  (1 - exp(-gamma * t1))) * exp(-gamma * t2) -
                rate/gamma * (1 - exp(-gamma * t1)) * (exp(-gamma * t2) * t2);
              break;
            case 6: // dGamma, dGamma
              ret -= ((rate * C/gamma * exp(-gamma * t1) * t1 - (rate * C/(gamma * gamma)) * (1 - exp(-gamma * t1))) * exp(-gamma * t2) * t2 +
                      (rate * C/gamma * (exp(-gamma * t1) * t1 * t1) +
                       rate * C/(gamma*gamma) * exp(-gamma * t1) * t1 +
                       ((rate * C/(gamma * gamma)) * exp(-gamma * t1) * t1 - rate * C *  (gamma + gamma)/pow(gamma,4) * (1 - exp(-gamma * t1)))) * exp(-gamma * t2) +
                      ((rate * C/gamma * exp(-gamma * t1) * t1 -  rate * C/(gamma*gamma) * (1 - exp(-gamma * t1))) * exp(-gamma * t2) * t2 -
                       rate * C/gamma * (1 - exp(-gamma * t1)) * (exp(-gamma * t2) * t2 * t2)));
              break;
            case 8: // dGamma, dTlag
              if (thisT < tinf){
                // During infusion
                // t1=thisT
                // t2=0;
                ret += rate * C/gamma * (exp(-gamma * (tT - tlag)) * gamma * (tT - tlag) - exp(-gamma * (tT - tlag))) +
                  rate * C/(gamma * gamma) * exp(-gamma * (tT - tlag)) * gamma;
              } else {
                // After infusion
                // t1=tinf
                // t2 = thisT-tinf
                ret += (rate * C/gamma * ((exp(-gamma * tinf)) * tinf) - rate * C/(gamma * gamma) * (1 - (exp(-gamma * tinf)))) * exp(-gamma * (tT - tinf - tlag)) * gamma -
                  rate * C/gamma * (1 - (exp(-gamma * tinf))) * (exp(-gamma * (tT - tinf - tlag)) * gamma *  (tT - tinf - tlag) - exp(-gamma * (tT - tinf - tlag)));
              }
              break;
            }
            break;
          case 8: //dTlag
            if (thisT < tinf){
              // During infusion
              // t1=thisT
              // t2=0;
              switch(diff2){
              case 0:
                ret -= (rate * C/gamma * (exp(-gamma * (tT - tlag)) * gamma));
                break;
              case 5: // dTlag, dA
                ret -= (rate/gamma * exp(-gamma * (tT - tlag)) * gamma);
                break;
              case 6: // dTlag, dGamma
                ret -= (rate * C/gamma * (exp(-gamma * (tT - tlag)) - exp(-gamma * (tT - tlag)) * (tT - tlag) * gamma) -
                        rate * C/(gamma*gamma) * exp(-gamma * (tT - tlag)) * gamma);
                break;
              case 8: // dTlag dTlag
                ret -= (rate * C/gamma * (exp(-gamma * (tT - tlag)) * gamma * gamma));
                break;
              }
            } else {
              // After infusion
              // t1=tinf
              // t2 = thisT-tinf
              switch(diff2){
              case 0:
                ret += rate * C/gamma * (1 - exp(-gamma * tinf)) * (exp(-gamma * (tT - tlag - tinf)) * gamma);
                break;
              case 5: //dTlag, dA
                ret += rate/gamma * (1 - exp(-gamma * tinf)) *  exp(-gamma * (tT - tlag - tinf)) * gamma;
                break;
              case 6: //dTlag, dGamma
                ret += (rate * C/gamma * (exp(-gamma * tinf) * tinf) - rate * C/(gamma*gamma) *  (1 - exp(-gamma * tinf))) * exp(-gamma * (tT - tlag - tinf)) * gamma + rate * C/gamma * (1 - exp(-gamma * tinf)) * (exp(-gamma * (tT - tlag - tinf)) - exp(-gamma * (tT - tlag - tinf)) * (tT - tlag - tinf) * gamma);
                break;
              case 8: //dTlag, dTlag
                ret += rate * C/gamma * (1 - exp(-gamma * tinf)) * (exp(-gamma * (tT - tlag - tinf)) * gamma * gamma);
                break;
              }
            }
          }
        }
      }
    } else {
      tT = t - rxDosingTime(l);
      thisT = tT - tlag;
      if (thisT < 0) continue;
      res = ((oral == 1) ? exp(-ka * thisT) : 0.0);
      switch(diff1){
      case 0:
        ret += dose * A *(exp(-alpha * thisT) - res);
        break;
      case 1: //dA
	switch(diff2){
	case 0:
	  ret += dose * (exp(-alpha * thisT) - res);
          break;
	case 2: // dA, dAlpha
	  ret -= dose*exp(-alpha * thisT) * thisT;
	  break;
	case 7: // dA, dKa
	  if (oral == 1){
	    ret += dose * (exp(-ka * thisT) * thisT);
	  }
	  break;
	case 8: // dA, dTlag
	  ret += dose* (exp(-alpha * thisT) * alpha - exp(-ka * thisT) * ka);
	  break;
        }
      case 2: //dAlpha
	switch(diff2){
	case 0:
	  ret -= dose * A * (exp(-alpha * thisT) * thisT);
          break;
	case 1: //dAlpha, dA
	  ret -= dose * (exp(-alpha * thisT) * thisT);
          break;
	case 2: //dAlpha, dAlpha
	  ret += dose * A * (exp(-alpha * thisT) * thisT*thisT);
          break;
	case 7: //dAlpha, dKa
	  ret -= 0;
	  break;
	case 8: //dAlpha, dTlag
	  ret -= dose * A * (exp(-alpha * thisT) * alpha * thisT - exp(-alpha * thisT));
	  break;
        }
      case 7: //dKa
        if (oral == 1){
	  switch(diff2){
	  case 0:
	    ret += dose * A * (exp(-ka * thisT) * thisT);
	    break;
	  case 1: //dKa, dA
	    ret += dose * (exp(-ka * thisT) * thisT);
            break;
	  case 7: //dKa, dKa
	    ret -= dose * A * (exp(-ka * thisT) * thisT * thisT);
	    break;
	  case 8: //dKa, dTlag
	    ret += dose * A * (exp(-ka * thisT) * ka * thisT - exp(-ka * thisT));
            break;
          }
        }
        break;
      case 8: //dTlag
	switch(diff2){
	case 0:
	  ret += dose * A * (exp(-alpha * thisT) * alpha - exp(-ka * thisT) * ka);
	  break;
	case 1: // dTlag, dA
	  ret += dose * (exp(-alpha * thisT) * alpha - exp(-ka * thisT) * ka);
	  break;
	case 2: // dTlag, dAlpha
	  ret += dose * A * (exp(-alpha * thisT) - exp(-alpha * thisT) * thisT * alpha);
	  break;
	case 7: // dTlag, dKa
	  ret -=  dose * A * (exp(-ka * thisT) - exp(-ka * thisT) * thisT * ka);
          break;
	case 8: // dTlag, dTlag
	  ret += dose * A * (exp(-alpha * thisT) * alpha * alpha - exp(-ka * thisT) * ka * ka);
          break;
        }
        break;
      }
      if (ncmt >= 2){
	switch(diff1){
        case 0:
          ret += dose * B *(exp(-beta * thisT) - res);
          break;
        case 3: //dA
          switch(diff2){
          case 0:
            ret += dose * (exp(-beta * thisT) - res);
            break;
          case 4: // dA, dBeta
            ret -= dose*exp(-beta * thisT) * thisT;
            break;
          case 7: // dA, dKa
            if (oral == 1){
              ret += dose * (exp(-ka * thisT) * thisT);
            }
            break;
          case 8: // dA, dTlag
            ret += dose* (exp(-beta * thisT) * beta - exp(-ka * thisT) * ka);
            break;
          }
        case 4: //dBeta
          switch(diff2){
          case 0:
            ret -= dose * B * (exp(-beta * thisT) * thisT);
            break;
          case 3: //dBeta, dA
            ret -= dose * (exp(-beta * thisT) * thisT);
            break;
          case 4: //dBeta, dBeta
            ret += dose * B * (exp(-beta * thisT) * thisT*thisT);
            break;
          case 7: //dBeta, dKa
            ret -= 0;
            break;
          case 8: //dBeta, dTlag
            ret -= dose * B * (exp(-beta * thisT) * beta * thisT - exp(-beta * thisT));
            break;
          }
        case 7: //dKa
          if (oral == 1){
            switch(diff2){
            case 0:
              ret += dose * B * (exp(-ka * thisT) * thisT);
              break;
            case 3: //dKa, dA
              ret += dose * (exp(-ka * thisT) * thisT);
              break;
            case 7: //dKa, dKa
              ret -= dose * B * (exp(-ka * thisT) * thisT * thisT);
              break;
            case 8: //dKa, dTlag
              ret += dose * B * (exp(-ka * thisT) * ka * thisT - exp(-ka * thisT));
              break;
            }
          }
          break;
        case 8: //dTlag
          switch(diff2){
          case 0:
            ret += dose * B * (exp(-beta * thisT) * beta - exp(-ka * thisT) * ka);
            break;
          case 3: // dTlag, dA
            ret += dose * (exp(-beta * thisT) * beta - exp(-ka * thisT) * ka);
            break;
          case 4: // dTlag, dBeta
            ret += dose * B * (exp(-beta * thisT) - exp(-beta * thisT) * thisT * beta);
            break;
          case 7: // dTlag, dKa
            ret -=  dose * B * (exp(-ka * thisT) - exp(-ka * thisT) * thisT * ka);
            break;
          case 8: // dTlag, dTlag
            ret += dose * B * (exp(-beta * thisT) * beta * beta - exp(-ka * thisT) * ka * ka);
            break;
          }
          break;
        }
        if (ncmt >= 3){
	  switch(diff1){
          case 0:
            ret += dose * C *(exp(-gamma * thisT) - res);
            break;
          case 5: //dA
            switch(diff2){
            case 0:
              ret += dose * (exp(-gamma * thisT) - res);
              break;
            case 6: // dA, dGamma
              ret -= dose*exp(-gamma * thisT) * thisT;
              break;
            case 7: // dA, dKa
              if (oral == 1){
                ret += dose * (exp(-ka * thisT) * thisT);
              }
              break;
            case 8: // dA, dTlag
              ret += dose* (exp(-gamma * thisT) * gamma - exp(-ka * thisT) * ka);
              break;
            }
          case 6: //dGamma
            switch(diff2){
            case 0:
              ret -= dose * C * (exp(-gamma * thisT) * thisT);
              break;
            case 5: //dGamma, dA
              ret -= dose * (exp(-gamma * thisT) * thisT);
              break;
            case 6: //dGamma, dGamma
              ret += dose * C * (exp(-gamma * thisT) * thisT*thisT);
              break;
            case 7: //dGamma, dKa
              ret -= 0;
              break;
            case 8: //dGamma, dTlag
              ret -= dose * C * (exp(-gamma * thisT) * gamma * thisT - exp(-gamma * thisT));
              break;
            }
          case 7: //dKa
            if (oral == 1){
              switch(diff2){
              case 0:
                ret += dose * C * (exp(-ka * thisT) * thisT);
                break;
              case 5: //dKa, dA
                ret += dose * (exp(-ka * thisT) * thisT);
                break;
              case 7: //dKa, dKa
                ret -= dose * C * (exp(-ka * thisT) * thisT * thisT);
                break;
              case 8: //dKa, dTlag
                ret += dose * C * (exp(-ka * thisT) * ka * thisT - exp(-ka * thisT));
                break;
              }
            }
            break;
          case 8: //dTlag
            switch(diff2){
            case 0:
              ret += dose * C * (exp(-gamma * thisT) * gamma - exp(-ka * thisT) * ka);
              break;
            case 5: // dTlag, dA
              ret += dose * (exp(-gamma * thisT) * gamma - exp(-ka * thisT) * ka);
              break;
            case 6: // dTlag, dGamma
              ret += dose * C * (exp(-gamma * thisT) - exp(-gamma * thisT) * thisT * gamma);
              break;
            case 7: // dTlag, dKa
              ret -=  dose * C * (exp(-ka * thisT) - exp(-ka * thisT) * thisT * ka);
              break;
            case 8: // dTlag, dTlag
              ret += dose * C * (exp(-gamma * thisT) * gamma * gamma - exp(-ka * thisT) * ka * ka);
              break;
            }
            break;
          }          
        }
      }
    }
  } //l
  return ret;
}