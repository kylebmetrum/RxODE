// [[Rcpp::plugins(openmp)]]
#define ARMA_DONT_PRINT_ERRORS
#define ARMA_DONT_USE_OPENMP // Known to cause speed
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Rmath.h>
#include <RcppArmadillo.h>
#define NETAs 20
#define NTHETAs 20
#define NSUBs 100
#define min2( a , b )  ( (a) < (b) ? (a) : (b) )
#define innerOde(id) ind_solve(rx, id, inner_dydt_liblsoda, inner_dydt_lsoda_dum, inner_jdum_lsoda, inner_dydt, inner_update_inis, inner_global_jt, inner_evid_extra)
#define getCholOmegaInv() (as<arma::mat>(rxSymInvCholEnvCalculate(_rxInv, "chol.omegaInv", R_NilValue)))
#define getOmega() (as<NumericMatrix>(rxSymInvCholEnvCalculate(_rxInv, "omega", R_NilValue)))
#define getOmegaMat() (as<arma::mat>(rxSymInvCholEnvCalculate(_rxInv, "omega", R_NilValue)))
#define getOmegaInv() (as<arma::mat>(rxSymInvCholEnvCalculate(_rxInv, "omegaInv", R_NilValue)))
#define getOmegaDet() (as<double>(rxSymInvCholEnvCalculate(_rxInv, "log.det.OMGAinv.5", R_NilValue)))
#define getOmegaN() as<int>(rxSymInvCholEnvCalculate(_rxInv, "ntheta", R_NilValue))
#define getOmegaTheta() as<NumericVector>(rxSymInvCholEnvCalculate(_rxInv, "theta", R_NilValue));
#define setOmegaTheta(x) rxSymInvCholEnvCalculate(_rxInv, "theta", x)
#define tbs(x) powerD(x,    ind->lambda, (int)(ind->yj))
#define tbsL(x) powerL(x,   ind->lambda, (int)(ind->yj))
#define tbsDL(x) powerDL(x, ind->lambda, (int)(ind->yj))
#define tbsD(x) powerDD(x,  ind->lambda, (int)(ind->yj))
#define _safe_log(a) (((a) <= 0) ? log(DOUBLE_EPS) : log(a))
#define _safe_zero(a) ((a) == 0 ? DOUBLE_EPS : (a))
//#define _safe_sqrt(a) ((a) <= 0 ? sqrt(DOUBLE_EPS) : sqrt(a))
#define _safe_sqrt(a) sqrt(a)

using namespace Rcpp;
using namespace arma;
extern "C"{
  int isRstudio();
#include "RxODE.h"
  typedef void (*S2_fp) (int *, int *, double *, double *, double *, int *, float *, double *);
  typedef void (*n1qn1_fp)(S2_fp simul, int n[], double x[], double f[], double g[], double var[], double eps[],
			   int mode[], int niter[], int nsim[], int imp[], int lp[], double zm[], int izs[], 
			   float rzs[], double dzs[]);
  
  n1qn1_fp n1qn1_;
  
  typedef double optimfn(int n, double *par, void *ex);

  typedef void optimgr(int n, double *par, double *gr, void *ex);

  void lbfgsbRX(int n, int lmm, double *x, double *lower,
                double *upper, int *nbd, double *Fmin, optimfn fn,
                optimgr gr, int *fail, void *ex, double factr,
                double pgtol, int *fncount, int *grcount,
                int maxit, char *msg, int trace, int nREPORT);
  

  void ind_solve(rx_solve *rx, unsigned int cid, t_dydt_liblsoda dydt_lls, 
		 t_dydt_lsoda_dum dydt_lsoda, t_jdum_lsoda jdum,
                 t_dydt c_dydt, t_update_inis u_inis, int jt, t_evid_extra u_evid);
  double powerD(double x, double lambda, int yj);
  double powerL(double x, double lambda, int yj);
  double powerDL(double x, double lambda, int yj);
  double powerDD(double x, double lambda, int yj);
  int par_progress(int c, int n, int d, int cores, clock_t t0, int stop);
}

Function getRxFn(std::string name);

SEXP rxSolveC(const RObject &obj,
              const Nullable<CharacterVector> &specParams = R_NilValue,
              const Nullable<List> &extraArgs = R_NilValue,
              const RObject &params = R_NilValue,
              const RObject &events = R_NilValue,
              const RObject &inits = R_NilValue,
              const RObject &scale = R_NilValue,
              const RObject &covs  = R_NilValue,
              const int method = 2, // 0
              const Nullable<LogicalVector> &transit_abs = R_NilValue, //1
              const double atol = 1.0e-6, //2
              const double rtol = 1.0e-4, //3
              const int maxsteps = 5000, //4
              const double hmin = 0, //5
              const Nullable<NumericVector> &hmax = R_NilValue, //6
              const double hini = 0, //7
              const int maxordn = 12, //8
              const int maxords = 5, //9
              const unsigned int cores = 1, //10
              const int covs_interpolation = 0, //11
              bool addCov = false, //12
              int matrix = 0, //13
              const Nullable<NumericMatrix> &sigma= R_NilValue, //14
              const Nullable<NumericVector> &sigmaDf= R_NilValue, //15
              const int &nCoresRV= 1, //16
              const bool &sigmaIsChol= false,
              const int &nDisplayProgress = 10000,
              const CharacterVector &amountUnits = NA_STRING,
              const CharacterVector &timeUnits = "hours",
              const bool addDosing = false,
	      const double stateTrim = R_PosInf,
              const RObject &theta = R_NilValue,
              const RObject &eta = R_NilValue,
              const bool updateObject = false,
              const bool doSolve = true,
              const Nullable<NumericMatrix> &omega = R_NilValue, 
              const Nullable<NumericVector> &omegaDf = R_NilValue, 
              const bool &omegaIsChol = false,
              const unsigned int nSub = 1, 
              const Nullable<NumericMatrix> &thetaMat = R_NilValue, 
              const Nullable<NumericVector> &thetaDf = R_NilValue, 
              const bool &thetaIsChol = false,
              const unsigned int nStud = 1, 
              const double dfSub=0.0,
              const double dfObs=0.0,
              const int setupOnly = 0);

RObject rxSymInvCholEnvCalculate(List obj, std::string what, Nullable<NumericVector> theta = R_NilValue);
bool rxIs(const RObject &obj, std::string cls);

List _rxInv;

// These are focei inner options
typedef struct {
  // 
  // std::string estStr;
  // std::string gradStr;
  // std::string obfStr;
  // 
  List mvi;
  double *geta;
  double *goldEta;
  double *gsaveEta;
  double *gthetaGrad;
  // n1qn1 specific vectors
  double *gZm;
  double *gG;
  double *gVar;
  double *gX;

  double *likSav;
  
  // Integer of ETAs
  unsigned int etaTransN;
  unsigned int gEtaTransN;
  unsigned int gEtaGTransN;
  unsigned int gThetaGTransN;
  unsigned int gZmN;
  // Where likelihood is saved.
  
  int *etaTrans;

  int neta;
  unsigned int ntheta;
  int npars;
  int thetan;
  int omegan;

  int calcGrad;
  int nF;
  int nG;
  int derivMethod;
  int covDerivMethod;
  int covMethod;
  int derivMethodSwitch;
  double derivSwitchTol;
  double lastOfv;
  
  double *fullTheta;
  double *theta;
  double *thetaGrad;
  double *initPar;
  int *xPar;
  NumericVector lowerIn;
  double *lower;
  NumericVector upperIn;
  double *upper;
  int *nbd;

  unsigned int thetaTransN;

  int *fixedTrans;
  int *thetaTrans;

  double scaleTo;
  double epsilon;
  
  int maxOuterIterations;
  int maxInnerIterations;

  int nsim;
  int nzm;

  int imp;
  int printInner;
  int printOuter;
  

  mat omega;
  mat omegaInv;
  mat cholOmegaInv;
  mat etaM;
  mat etaS;
  mat eta1SD;
  double n;
  double logDetOmegaInv5;

  double rEps;
  double aEps;
  double rEpsC;
  double aEpsC;
  //
  double factr;
  double pgtol;
  int lmm;
  int *skipCov;
  int skipCovN;

  int outerOpt;
  int eigen;
  int scaleObjective;
  double scaleObjectiveTo;
  int initObj;
  double initObjective;
  // Confidence Interval
  double ci;
  double sigdig;
  //
  clock_t t0;
  int cur;
  int curTick;
  int totTick;
  int useColor;
  double boundTol;
  int printNcol;
  int noabort;
  int interaction;
  double cholSEtol;
  double hessEps;
  double cholAccept;
  double resetEtaSize;
  int resetHessianAndEta;
  int cholSEOpt;
  int cholSECov;
  int fo;
  int covTryHarder;
} focei_options;

focei_options op_focei;

extern "C" void rxOptionsIniFocei(){
  op_focei.etaTransN = 0;
  op_focei.thetaTransN = 0;
  op_focei.gEtaGTransN=0;
  op_focei.gZmN = 0;
}

void foceiThetaN(unsigned int n){
  if (op_focei.thetaTransN < n){
    unsigned int cur = n;
    Free(op_focei.thetaTrans);
    Free(op_focei.theta);
    Free(op_focei.fullTheta);
    Free(op_focei.initPar);
    Free(op_focei.xPar);
    Free(op_focei.fixedTrans);
    op_focei.fullTheta   = Calloc(cur, double);
    op_focei.thetaTrans  = Calloc(cur, int);
    op_focei.fixedTrans  = Calloc(cur, int);
    op_focei.theta       = Calloc(cur, double);
    op_focei.initPar     = Calloc(cur, double);
    op_focei.xPar        = Calloc(cur, int);
    op_focei.thetaTransN = cur;
  }
}

void foceiEtaN(unsigned int n){
  if (op_focei.etaTransN < n){
    unsigned int cur = n;
    Free(op_focei.etaTrans);
    op_focei.etaTrans = Calloc(cur, int);
    op_focei.etaTransN=cur;
  }
}

void foceiGThetaN(unsigned int n){
  if (op_focei.gThetaGTransN < n){
    unsigned int cur = n;
    Free(op_focei.gthetaGrad);
    op_focei.gthetaGrad = Calloc(cur, double);
    op_focei.gThetaGTransN=cur;
  }
}

void foceiGEtaN(unsigned int n){
  if (op_focei.gEtaGTransN < n){
    unsigned int cur = n;
    Free(op_focei.geta);
    Free(op_focei.goldEta);
    Free(op_focei.gsaveEta);
    Free(op_focei.gG);
    Free(op_focei.gVar);
    Free(op_focei.gX);
    op_focei.geta = Calloc(cur, double);
    op_focei.goldEta = Calloc(cur, double);
    op_focei.gsaveEta = Calloc(cur, double);
    op_focei.gG = Calloc(cur, double);
    op_focei.gVar = Calloc(cur, double);
    op_focei.gX = Calloc(cur, double);
    // Prefill to 0.1 or 10%
    std::fill_n(&op_focei.gVar[0], cur, 0.1);
    std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0); // All etas = -42;  Unlikely if normal
    op_focei.gEtaGTransN = cur;
  }
}

void foceiGgZm(unsigned int n){
  if (op_focei.gZmN < n){
    unsigned int cur = n;
    Free(op_focei.gZm);
    op_focei.gZm = Calloc(cur, double);
    op_focei.gZmN = cur;
  }
}

typedef struct {
  int nInnerF;
  int nInnerG;
  double lik[3]; // lik[0] = liklihood; For central difference: lik[1] = lower lik[2] = upper
  double *eta; // Eta includes the ID number for the patient
  //
  double *thetaGrad; // Theta gradient; Calculated on the individual level for S matrix calculation
  double thVal[2]; // thVal[0] = lower; thVal[2] = upper
  //
  // F and varaibility
  unsigned int nobs;
  unsigned int setup;
  
  double *saveEta; // Saved when lik[0] is saved.
  double *oldEta;
  
  // Likilihood gradient
  double llik;
  mat a;
  mat B;
  mat c;
  mat lp;// = mat(neta,1);

  double *g;

  double tbsLik;
  
  int mode; // 1 = dont use zm, 2 = use zm.
  double *zm;
  double *var;
  double *x;
  unsigned int uzm;
  int doChol;
} focei_ind;

focei_ind *inds_focei = NULL;
int max_inds_focei = 0;

extern "C" void rxOptionsFreeFocei(){
  if (op_focei.thetaTrans != NULL) Free(op_focei.thetaTrans);
  if (op_focei.theta != NULL) Free(op_focei.theta);
  if (op_focei.fullTheta != NULL) Free(op_focei.fullTheta);
  if (op_focei.initPar != NULL) Free(op_focei.initPar);
  if (op_focei.xPar != NULL) Free(op_focei.xPar);
  if (op_focei.fixedTrans != NULL) Free(op_focei.fixedTrans);
  op_focei.thetaTransN=0;
  if (op_focei.etaTrans != NULL) Free(op_focei.etaTrans);
  op_focei.etaTransN=0;
  if (op_focei.geta != NULL) Free(op_focei.geta);
  if (op_focei.goldEta != NULL) Free(op_focei.goldEta);
  if (op_focei.gsaveEta != NULL) Free(op_focei.gsaveEta);
  if (op_focei.gG != NULL) Free(op_focei.gG);
  if (op_focei.gVar != NULL) Free(op_focei.gVar);
  if (op_focei.gX != NULL) Free(op_focei.gX);
  op_focei.gEtaGTransN=0;
  if (op_focei.gthetaGrad != NULL) Free(op_focei.gthetaGrad);
  op_focei.gThetaGTransN=0;
  if (inds_focei != NULL) Free(inds_focei);
  max_inds_focei=0;
  if (op_focei.gZm != NULL) Free(op_focei.gZm);
  op_focei.gZmN = 0;
  if (op_focei.likSav != NULL) Free(op_focei.likSav);
  if (op_focei.skipCovN && op_focei.skipCov != NULL) Free(op_focei.skipCov);
  op_focei.skipCovN = 0;

}

rx_solve *getRxSolve_();

focei_ind *rxFoceiEnsure(int mx){
  if (mx >= max_inds_focei){
    Free(inds_focei);
    inds_focei =Calloc(mx+1024, focei_ind);
    max_inds_focei = mx+1024;
  }
  return inds_focei;
}


t_dydt inner_dydt = NULL;

t_calc_jac inner_calc_jac = NULL;

t_calc_lhs inner_calc_lhs = NULL;

t_update_inis inner_update_inis = NULL;

t_dydt_lsoda_dum inner_dydt_lsoda_dum = NULL;

t_dydt_liblsoda inner_dydt_liblsoda = NULL;

t_jdum_lsoda inner_jdum_lsoda = NULL;

t_set_solve inner_set_solve = NULL;

t_get_solve inner_get_solve = NULL;
t_evid_extra inner_evid_extra=NULL;

int inner_global_jt = 2;
int inner_global_mf = 22;  
int inner_global_debug = 0;

void rxUpdateInnerFuns(SEXP trans){
  const char *lib, *s_dydt, *s_calc_jac, *s_calc_lhs, *s_inis, *s_dydt_lsoda_dum, *s_dydt_jdum_lsoda, 
    *s_ode_solver_solvedata, *s_ode_solver_get_solvedata, *s_dydt_liblsoda, *s_evid_extra;
  lib = CHAR(STRING_ELT(trans, 0));
  s_dydt = CHAR(STRING_ELT(trans, 3));
  s_calc_jac = CHAR(STRING_ELT(trans, 4));
  s_calc_lhs = CHAR(STRING_ELT(trans, 5));
  s_inis = CHAR(STRING_ELT(trans, 8));
  s_dydt_lsoda_dum = CHAR(STRING_ELT(trans, 9));
  s_dydt_jdum_lsoda = CHAR(STRING_ELT(trans, 10));
  s_ode_solver_solvedata = CHAR(STRING_ELT(trans, 11));
  s_ode_solver_get_solvedata = CHAR(STRING_ELT(trans, 12));
  s_dydt_liblsoda = CHAR(STRING_ELT(trans, 13));
  s_evid_extra = CHAR(STRING_ELT(trans, 14));
  inner_global_jt = 2;
  inner_global_mf = 22;  
  inner_global_debug = 0;
  if (strcmp(CHAR(STRING_ELT(trans, 1)),"fulluser") == 0){
    inner_global_jt = 1;
    inner_global_mf = 21;
  } else {
    inner_global_jt = 2;
    inner_global_mf = 22;
  }
  inner_calc_lhs =(t_calc_lhs) R_GetCCallable(lib, s_calc_lhs);
  inner_dydt =(t_dydt) R_GetCCallable(lib, s_dydt);
  inner_calc_jac =(t_calc_jac) R_GetCCallable(lib, s_calc_jac);
  inner_update_inis =(t_update_inis) R_GetCCallable(lib, s_inis);
  inner_dydt_lsoda_dum =(t_dydt_lsoda_dum) R_GetCCallable(lib, s_dydt_lsoda_dum);
  inner_jdum_lsoda =(t_jdum_lsoda) R_GetCCallable(lib, s_dydt_jdum_lsoda);
  inner_set_solve = (t_set_solve)R_GetCCallable(lib, s_ode_solver_solvedata);
  inner_get_solve = (t_get_solve)R_GetCCallable(lib, s_ode_solver_get_solvedata);
  inner_dydt_liblsoda = (t_dydt_liblsoda)R_GetCCallable(lib, s_dydt_liblsoda);
  inner_evid_extra = (t_evid_extra)R_GetCCallable(lib, s_evid_extra);
}

void rxClearInnerFuns(){
  inner_calc_lhs              = NULL;
  inner_dydt                  = NULL;
  inner_calc_jac              = NULL;
  inner_update_inis           = NULL;
  inner_dydt_lsoda_dum        = NULL;
  inner_jdum_lsoda            = NULL;
  inner_set_solve             = NULL;
  inner_get_solve             = NULL;
  inner_dydt_liblsoda         = NULL;
}

rx_solve* rx;

////////////////////////////////////////////////////////////////////////////////
// n1qn1 functions
uvec lowerTri(mat H, bool diag = false){
  unsigned int d = H.n_rows;
  mat o(d, d, fill::ones);
  if (!diag){
    return find(trimatl(o,-1));
  } else {
    return find(trimatl(o));
  }
}

void updateZm(focei_ind *indF){
  std::fill(&indF->zm[0], &indF->zm[0]+op_focei.nzm,0.0);
  if (!indF->uzm){
    // Udate the curvature to Hessian to restart n1qn1
    int n = op_focei.neta;
    mat L = eye(n, n);
    mat D = mat(n, n, fill::zeros);
    mat H = mat(n, n);
    unsigned int l_n = n * (n + 1)/2;
    vec zmV(l_n);
    std::copy(&indF->zm[0], &indF->zm[0]+l_n, zmV.begin());
    H.elem(lowerTri(H, true)) = zmV;
    if (n == 1) H = D;
    else{
      L.elem(lowerTri(H,false)) = H.elem(lowerTri(H,0));
      D.diag() = H.diag();
      H = L*D*L.t();
    }
    // Hessian -> c.hess
    vec hessV = H.elem(lowerTri(H, true));
    std::copy(hessV.begin(),hessV.end(),&indF->zm[0]);
    indF->uzm = 1;
    indF->mode=2;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Likelihood for inner functions

void updateTheta(double *theta){
  // Theta is the acutal theta
  unsigned int j, k;
  if (op_focei.scaleTo > 0){ // Scaling
    for (k = op_focei.npars; k--;){
      j=op_focei.fixedTrans[k];
      op_focei.fullTheta[j] = theta[k] * op_focei.initPar[k] / op_focei.scaleTo; //pars <- pars * inits.vec / con$scale.to
    }
  } else { // No scaling.
    for (k = op_focei.npars; k--;){
      j=op_focei.fixedTrans[k];
      op_focei.fullTheta[j] = theta[k]; //pars <- pars * inits.vec / con$scale.to
    }
  }
  // Update theta parameters in each individual
  rx = getRxSolve_();
  for (int id = rx->nsub; id--;){
    rx_solving_options_ind *ind = &(rx->subjects[id]);
    for (j = op_focei.ntheta; j--;){
      ind->par_ptr[op_focei.thetaTrans[j]] = op_focei.fullTheta[j];
    }
  }
  // Update setOmegaTheta
  NumericVector omegaTheta(op_focei.omegan);
  std::copy(&op_focei.fullTheta[0] + op_focei.thetan, 
	    &op_focei.fullTheta[0] + op_focei.thetan + op_focei.omegan, 
	    omegaTheta.begin());
  setOmegaTheta(omegaTheta);
  if (op_focei.fo){
    op_focei.omega = getOmegaMat();
  } else {
    op_focei.omegaInv = getOmegaInv();
    op_focei.cholOmegaInv = getCholOmegaInv();
    op_focei.logDetOmegaInv5 = getOmegaDet();
  }
  //Now Setup Last theta
  if (!op_focei.calcGrad){
    // op_focei.estStr=sc + un + ex;
    std::copy(&theta[0], &theta[0] + op_focei.npars, &op_focei.theta[0]);
  }  
}

arma::mat cholSE_(arma::mat A, double tol);

double likInner0(double *eta){
  // id = eta[#neta]
  // eta = eta
  rx = getRxSolve_();
  unsigned int id = (unsigned int)(eta[op_focei.neta]);
  rx_solving_options_ind *ind = &(rx->subjects[id]);
  rx_solving_options *op = rx->op;
  focei_ind *fInd = &(inds_focei[id]);
  int i, j;
  bool recalc = false;
  if (!fInd->setup){
    recalc=true;
    fInd->nobs = ind->n_all_times - ind->ndoses;
    fInd->setup = 1;
  } else {
    // Check to see if old ETA matches.
    for (j = op_focei.neta; j--;){
      if (fInd->oldEta[j] != eta[j]){
	recalc=true;
	break;
      }
    }
  }
  if (recalc){
    // Update eta.
    fInd->lp  = arma::mat(op_focei.neta, 1);
    fInd->a   = arma::mat(fInd->nobs, op_focei.neta);
    fInd->B   = arma::mat(fInd->nobs, 1);
    mat Vid;
    if (op_focei.fo){
      Vid = arma::mat(fInd->nobs, fInd->nobs, fill::zeros);
    } else if (op_focei.interaction){
      fInd->c   = arma::mat(fInd->nobs, op_focei.neta);
    }
    for (j = op_focei.neta; j--;){
      ind->par_ptr[op_focei.etaTrans[j]] = eta[j];
    }
    // Solve ODE
    innerOde(id);
    // Rprintf("ID: %d; Solve #2: %f\n", id, ind->solve[2]);
    // Calculate matricies
    unsigned int k = fInd->nobs - 1;
    fInd->lp.fill(0.0);
    fInd->llik=0.0;
    fInd->tbsLik=0.0;
    double f, err, r, fpm, rp,lnr;
    for (j = ind->n_all_times; j--;){
      if (ind->evid[j]){
	ind->tlast = ind->all_times[j];
      } else {
	ind->idx=j;
	inner_calc_lhs((int)id, ind->all_times[j], &ind->solve[j * op->neq], ind->lhs);
        f = ind->lhs[0]; // TBS is performed in the RxODE rx_pred_ statement. This allows derivatives of TBS to be propigated
	if (ISNA(f)) throw std::runtime_error("bad solve");
	// fInd->f(k, 0) = ind->lhs[0];
	err = f - tbs(ind->dv[j]);
	fInd->tbsLik+=tbsL(ind->dv[j]);
	// fInd->err(k, 0) = ind->lhs[0] - ind->dv[k]; // pred-dv
        if (ISNA(ind->lhs[op_focei.neta + 1])) throw std::runtime_error("bad solve");
	r = _safe_zero(ind->lhs[op_focei.neta + 1]);
        if (op_focei.fo){
          // FO
          fInd->B(k, 0) = err; // res
	  Vid(k, k) = r;
          for (i = op_focei.neta; i--; ){
            fInd->a(k, i) = ind->lhs[i+1];
          }
          // Ci = fpm %*% omega %*% t(fpm) + Vi; Vi=diag(r)
        } else {
          lnr =_safe_log(ind->lhs[op_focei.neta + 1]);
          // fInd->r(k, 0) = ind->lhs[op_focei.neta+1];
          // fInd->B(k, 0) = 2.0/ind->lhs[op_focei.neta+1];
          // lhs 0 = F
          // lhs 1-eta = df/deta
          // FIXME faster initiliaitzation via copy or elim	
          fInd->B(k, 0) = 2.0/r;
          if (op_focei.interaction == 1){
            for (i = op_focei.neta; i--; ){
              fpm = fInd->a(k, i) = ind->lhs[i + 1]; // Almquist uses different a (see eq #15)
              rp  = ind->lhs[i + op_focei.neta + 2];
              fInd->c(k, i) = rp/r;
              // lp is eq 12 in Almquist 2015
              // // .5*apply(eps*fp*B + .5*eps^2*B*c - c, 2, sum) - OMGAinv %*% ETA
              fInd->lp(i, 0)  += 0.25 * err * err * fInd->B(k, 0) * fInd->c(k, i) - 0.5 * fInd->c(k, i) - 
                0.5 * err * fpm * fInd->B(k, 0);
            }
            // Eq #10
            //llik <- -0.5 * sum(err ^ 2 / R + log(R));
            fInd->llik += err * err/r + lnr;
          } else if (op_focei.interaction == 0){
            for (i = op_focei.neta; i--; ){
              fpm = fInd->a(k, i) = ind->lhs[i + 1]; // Almquist uses different a (see eq #15)
              fInd->lp(i, 0)  -= 0.5 * err * fpm * fInd->B(k, 0);
            }
            // Eq #10
            //llik <- -0.5 * sum(err ^ 2 / R + log(R));
            fInd->llik += err * err/r + lnr;
          }
        }
        k--;
      }
    }
    if (op_focei.fo){
      mat Ci = fInd->a * op_focei.omega * trans(fInd->a) + Vid;
      mat cholCi = cholSE_(Ci, op_focei.cholSEtol);
      mat CiInv;
      bool success  = inv(CiInv, trimatu(cholCi));
      if (!success){
        CiInv = pinv(trimatu(cholCi));
      }
      CiInv = CiInv * CiInv.t();
      double lik = 0;
      // 2*sum(log(diag(chol(Ci))))
      for (unsigned int j = cholCi.n_rows; j--;){
        lik += 2*_safe_log(cholCi(j,j));
      }
      // + t(.$Ri) %*% solve(Ci) %*% .$Ri
      mat rest =trans(fInd->B) * CiInv * fInd->B;
      lik += rest(0,0);
      // lik = -2*ll
      fInd->llik = -0.5*lik;
    } else {
      fInd->llik = -0.5*fInd->llik;
      // Now finalize lp
      mat etam = arma::mat(op_focei.neta, 1);
      std::copy(&eta[0], &eta[0] + op_focei.neta, etam.begin()); // fill in etam
      // Finalize eq. #12
      fInd->lp = -(fInd->lp - op_focei.omegaInv * etam);
      // Partially finalize #10
      fInd->llik = -trace(fInd->llik - 0.5*(etam.t() * op_focei.omegaInv * etam));
      // print(wrap(fInd->llik));
      std::copy(&eta[0], &eta[0] + op_focei.neta, &fInd->oldEta[0]);
    }
  }
  return fInd->llik;
}

double *lpInner(double *eta, double *g){
  unsigned int id = (unsigned int)(eta[op_focei.neta]);
  focei_ind *fInd = &(inds_focei[id]);
  likInner0(eta);
  std::copy(fInd->lp.begin(), fInd->lp.begin() + op_focei.neta,
	    &g[0]);
  return &g[0];
}

//[[Rcpp::export]]
NumericVector foceiInnerLp(NumericVector eta, int id = 1){
  double *etad = new double[eta.size()+1];
  std::copy(eta.begin(),eta.end(),&etad[0]);
  etad[eta.size()]=(double)(id-1);
  NumericVector lp(eta.size());
  lpInner(etad,&lp[0]);
  delete[] etad;
  return lp;
}

//[[Rcpp::export]]
double likInner(NumericVector eta, int id = 1){
  double *etad = new double[eta.size()+1];
  std::copy(eta.begin(),eta.end(),&etad[0]);
  etad[eta.size()]=(double)(id-1);
  double llik = likInner0(etad);
  delete[] etad;
  return llik;
}

arma::mat gershNested(arma::mat A, int j, int n){
  arma::mat g(n, 1, fill::zeros);
  double sumToI, sumAfterI;
  for (int ii = j; ii < n; ++ii){
    if (ii == 0){
      sumToI=0.0;
    } else if (j == ii){
      sumToI=arma::sum(arma::abs(A(ii, span(ii-1, j))));
    } else {
      sumToI=arma::sum(arma::abs(A(ii, span(j, ii-1))));
    }
    if (ii == n-1){
      sumAfterI = 0;
    } else {
      sumAfterI = arma::sum(arma::abs(A(span(ii+1, n-1), ii)));
    }
    g(ii, 0) = sumToI+sumAfterI-A(ii,ii);
  }
  return g;
}
// Suggested from https://gking.harvard.edu/files/help.pdf
// Translated from 
//http://www.dynare.org/dynare-matlab-m2html/matlab/chol_SE.html
// Use tau1=sqrt(eps) instead of eps^1/3; In my tests eps^1/3 produces NaNs
//[[Rcpp::export]]
bool cholSE0(arma::mat &Ao, arma::mat &E, arma::mat A, double tol){
  int n = A.n_rows;
  double tau1 = tol;//pow(DOUBLE_EPS, 1/3);
  double tau2 = tol;//tau1;
  bool phase1 = true;
  double delta = 0;
  int j;
  arma::mat P(n,1);
  // for (j = n; j--;) p(j,0) = j+1;
  arma::mat g(n,1, fill::zeros);
  // arma::mat E(n,1, fill::zeros);
  E = mat(n, 1, fill::zeros);
  double gamma = A(n-1,n-1);
  if (gamma < 0) phase1 = false;
  for (j = 0; j < n-1; j++){
    if (A(j, j) < 0) phase1 = false;
    if (A(j, j) > gamma) gamma = A(j,j);
  }
  double taugam = tau1*gamma;
  if (!phase1) g = gershNested(A, 0, n);
  // N=1 case
  if (n == 1){
    delta = tau2*std::fabs(A(0,0)) - A(0,0);
    if (delta > 0) E(0,0) = delta;
    if (A(0,0) == 0) E(0,0) = tau2;
    A(0,0)=_safe_sqrt(A(0,0)+E(0,0));
    Ao = A;
    return true;
  }
  int jp1, ii, k;
  double tempjj, temp=1., normj, tmp;
  for (j = 0; j < n-1;  j++){
    // Pivoting not included
    if (phase1){
      jp1 = j+1;
      if (A(j,j)>0){
	arma::mat tmp = (A(span(jp1,n-1),span(jp1,n-1))).diag() - A(span(jp1, n-1),j)%A(span(jp1, n-1),j)/A(j,j);
	double mintmp = tmp[0];
	for (ii = 1; ii < (int)tmp.size(); ii++) mintmp = (mintmp < tmp[ii]) ? mintmp : tmp[ii];
	if (mintmp < taugam) phase1=false;
      } else phase1 = false;

      if (phase1){
	// Do the normal cholesky update if still in phase 1
	A(j,j) = _safe_sqrt(A(j,j));
	tempjj = A(j,j);
	for (ii = jp1; ii < n; ii++){
	  A(ii,j) = A(ii,j)/tempjj;
	}
	for (ii=jp1; ii <n; ii++){
	  temp=A(ii,j);
	  for (k = jp1; k < ii+1; k++){
	    A(ii,k) = A(ii,k)-(temp * A(k,j));
	  }
	}
	if (j == n-2){
	  A(n-1,n-1)=_safe_sqrt(A(n-1,n-1));
	}
      } else {
	// Calculate the negatives of the lower Gershgorin bounds
	g=gershNested(A,j,n);
      }
    }

    if (!phase1){
      if (j != n-2){
        // Calculate delta and add to the diagonal. delta=max{0,-A(j,j) + max{normj,taugam},delta_previous}
	// where normj=sum of |A(i,j)|,for i=1,n, delta_previous is the delta computed at the previous iter and taugam is tau1*gamma.
	normj=arma::sum(arma::abs(A(span(j+1, n-1),j)));
	if (delta < 0) delta = 0;
	tmp  = -A(j,j)+normj;
	if (delta < tmp) delta = tmp;
        tmp  = -A(j,j)+taugam;
        if (delta < tmp) delta = tmp;
	// get adjustment based on formula on bottom of p. 309 of Eskow/Schnabel (1991)
	E(j,0) =  delta;
	A(j,j) = A(j,j) + E(j,0);
	// Update the Gershgorin bound estimates (note: g(i) is the negative of the Gershgorin lower bound.)
	if (A(j,j) != normj){
	  temp = (normj/A(j,j)) - 1;
	  for (ii = j+1; ii < n; ii++){
	    g(ii) = g(ii) + std::fabs(A(ii,j)) * temp;
	  }
	}
	for (int ii = j+1; ii < n; ii++){
	  g(ii,0) = g(ii,0) + std::fabs(A(ii,j)) * temp;
	}
	// Do the cholesky update
	A(j,j) = _safe_sqrt(A(j,j));
	tempjj = A(j,j);
	for (ii = j+1; ii < n; ii++){
	  A(ii,j) = A(ii,j) / tempjj;
	}
	for (ii = j+1; ii < n; ii++){
	  temp = A(ii,j);
	  for (k = j+1; k < ii+1; k++){
	    A(ii,k) = A(ii,k) - (temp * A(k,j));
	  }
	}
      } else {
	// Find eigenvalues of final 2 by 2 submatrix
        // Find delta such that:
	// 1.  the l2 condition number of the final 2X2 submatrix + delta*I <= tau2
	// 2. delta >= previous delta,
	// 3. min(eigvals) + delta >= tau2 * gamma, where min(eigvals) is the smallest eigenvalue of the final 2X2 submatrix
	// A(n-2,n-1)=A(n-1,n-2);
	//set value above diagonal for computation of eigenvalues
        A(n-2,n-1)=A(n-1,n-2); //set value above diagonal for computation of eigenvalues
	vec eigvals  = eig_sym(A(span(n-2, n-1),span(n-2, n-1)));
        // Formula 5.3.2 of Schnabel/Eskow (1990)
	if (delta < 0) delta = 0;
	tmp= (max(eigvals)-min(eigvals))/(1-tau1);
	if (tmp < gamma) tmp = gamma;
	tmp=tau2*tmp;
	tmp =tmp - min(eigvals);
	if (delta < tmp) delta = tmp;
	if (delta > 0){
	  A(n-2, n-2) = A(n-2,n-2) + delta;
	  A(n-1, n-1) = A(n-1,n-1) + delta;
          E(n-2, 0) = delta;
          E(n-1, 0) = delta;
        }
	// Final update
	A(n-2,n-2) = _safe_sqrt(A(n-2,n-2));
        A(n-1,n-2) = A(n-1,n-2)/A(n-2,n-2);
        A(n-1,n-1) = A(n-1,n-1) - A(n-1,n-2)*A(n-1,n-2);
        A(n-1,n-1) = _safe_sqrt(A(n-1,n-1));
      }
    }
  }
  Ao = (trimatl(A)).t();
  return phase1;
}
//[[Rcpp::export]]
arma::mat cholSE_(arma::mat A, double tol){
  arma::mat Ao, E;
  cholSE0(Ao, E, A, tol);
  return Ao;
}

double LikInner2(double *eta, int likId){
  unsigned int id = (unsigned int)(eta[op_focei.neta]);
  focei_ind *fInd = &(inds_focei[id]);
  // print(wrap(-likInner0(eta)));
  // print(wrap(op_focei.logDetOmegaInv5));
  double lik=0;
  if (op_focei.fo){
    // Already almost completely calculated.
    lik = fInd->llik;
  } else {
    lik = -likInner0(eta) + op_focei.logDetOmegaInv5;
    // print(wrap(lik));
    rx = getRxSolve_();
    // rx_solving_options_ind ind = rx->subjects[id];
    // Calclaute lik first to calculate components for Hessian
    // Hessian 
    mat H(op_focei.neta, op_focei.neta, fill::zeros);
    int k, l;
    mat tmp;
    // This is actually -H
    if (op_focei.interaction){
      for (k = op_focei.neta; k--;){
        for (l = k+1; l--;){
          // tmp = fInd->a.col(l) %  fInd->B % fInd->a.col(k);
          H(k, l) = 0.5*sum(fInd->a.col(l) %  fInd->B % fInd->a.col(k) + 
                            fInd->c.col(l) % fInd->c.col(k)) +
            op_focei.omegaInv(k, l);
          H(l, k) = H(k, l);
        }
      }
    } else {
      for (k = op_focei.neta; k--;){
        for (l = k+1; l--;){
          // tmp = fInd->a.col(l) %  fInd->B % fInd->a.col(k);
          H(k, l) = 0.5*sum(fInd->a.col(l) %  fInd->B % fInd->a.col(k)) +
            op_focei.omegaInv(k, l);
          H(l, k) = H(k, l);
        }
      }
    }
    arma::mat H0;
    k=0;
    if (fInd->doChol){
      H0=chol(H);
    } else {
      H0=cholSE_(H, op_focei.cholSEtol);
    }
    // - sum(log(H.diag()));
    for (unsigned int j = H0.n_rows; j--;){
      lik -= _safe_log(H0(j,j));
    }
  }
  lik += fInd->tbsLik;
  if (likId == 0){
    fInd->lik[0] = lik;
    std::copy(&fInd->eta[0], &fInd->eta[0] + op_focei.neta, &fInd->saveEta[0]);
  } else {
    // Use Objective function for Numeric Gradient.
    fInd->lik[likId] = -2*lik;
  }
  return lik;
}

// Scli-lab style cost function for inner
void innerCost(int *ind, int *n, double *x, double *f, double *g, int *ti, float *tr, double *td){
  int id = (int)(x[op_focei.neta]);
  focei_ind *fInd = &(inds_focei[id]);

  if (*ind==2 || *ind==4) {
    // Function
    // Make sure ID remains installed
    *f = likInner0(x);
    fInd->nInnerF++;
    if (op_focei.printInner != 0 && fInd->nInnerF % op_focei.printInner == 0){
      Rprintf(" %d(id:%d):%#14.8g:", fInd->nInnerF, id, *f);
      for (int i = 0; i < *n; i++) Rprintf(" %#10g", x[i]);
      Rprintf(" (nG: %d)\n", fInd->nInnerG);
    }
  }
  if (*ind==3 || *ind==4) {
    // Gradient
    lpInner(x, g);
    g[op_focei.neta] = 0; // Id shouldn't change.
    fInd->nInnerG++;
  }
}

static inline void innerEval(int id){
  focei_ind *fInd = &(inds_focei[id]);
  // Use eta
  likInner0(fInd->eta);
  LikInner2(fInd->eta, 0);
}

static inline void innerOpt1(int id, int likId){
  focei_ind *fInd = &(inds_focei[id]);
  focei_options *fop = &op_focei;
  fInd->nInnerF=0;
  fInd->nInnerG=0;
  // Use eta
  // Convert Zm to Hessian, if applicable.
  mat etaMat(fop->neta, 1);
  if (!op_focei.calcGrad){
    if (op_focei.resetEtaSize <= 0){
      if (op_focei.resetHessianAndEta){
	fInd->mode = 1;
	fInd->uzm = 1;
      }
      std::fill(&fInd->eta[0], &fInd->eta[0] + op_focei.neta, 0.0);
    } else if (R_FINITE(op_focei.resetEtaSize)) {
      std::copy(&fInd->eta[0], &fInd->eta[0] + op_focei.neta, etaMat.begin());
      // Standardized ETAs
      // chol(omega^-1) %*% eta
      mat etaRes = op_focei.cholOmegaInv * etaMat;
      bool doBreak = false;
      for (unsigned int j = etaRes.n_rows; j--;){
	if (std::fabs(etaRes(j, 0)) >= op_focei.resetEtaSize){
	  if (op_focei.resetHessianAndEta){
	    fInd->mode = 1;
	    fInd->uzm = 1;
	  }
	  std::fill(&fInd->eta[0], &fInd->eta[0] + op_focei.neta, 0.0);
	  doBreak=true;
	  break;
	}
      }
      if (!doBreak){
	etaRes = op_focei.eta1SD % etaMat;
	for (unsigned int j = etaRes.n_rows; j--;){
	  if (std::fabs(etaRes(j, 0)) >= op_focei.resetEtaSize){
	    if (op_focei.resetHessianAndEta){
	      fInd->mode = 1;
	      fInd->uzm = 1;
	    }
	    std::fill(&fInd->eta[0], &fInd->eta[0] + op_focei.neta, 0.0);
	    break;
	  }
	}
      }
    }
  }
  updateZm(fInd);
  int lp = 6;
  
  std::fill_n(&fInd->var[0], fop->neta, 0.1);
  fInd->var[fop->neta] = 0; // No change; ID.

  int npar = fop->neta+1;

  std::copy(&fInd->eta[0], &fInd->eta[0]+fop->neta+1,fInd->x);
  double f, epsilon = fop->epsilon;

  // Since these are pointers, without reassignment they are modified.
  int mode = fInd->mode, maxInnerIterations=fop->maxInnerIterations,
    nsim=fop->nsim, imp=fop->imp;
  int izs; float rzs; double dzs;
  
  n1qn1_(innerCost, &npar, fInd->x, &f, fInd->g, 
	 fInd->var, &epsilon,
	 &mode, &maxInnerIterations, &nsim, 
	 &imp, &lp, 
	 fInd->zm, 
	 &izs, &rzs, &dzs);
  std::copy(&fInd->x[0],&fInd->x[0]+fop->neta,&fInd->eta[0]);
  // Update variances
  std::copy(&fInd->eta[0], &fInd->eta[0] + op_focei.neta, etaMat.begin());
  op_focei.n = op_focei.n + 1.0;
  mat oldM = op_focei.etaM;
  op_focei.etaM = op_focei.etaM + (etaMat - op_focei.etaM)/op_focei.n;
  op_focei.etaS = op_focei.etaS + (etaMat - op_focei.etaM) %  (etaMat - oldM);
  fInd->llik = f;
  // Use saved Hessian on next opimization.
  fInd->mode=2;
  fInd->uzm =0;
  LikInner2(fInd->eta, likId);
}

void innerOpt(){
// #ifdef _OPENMP
//   int cores = rx->op->cores;
// #endif
  rx = getRxSolve_();
  op_focei.omegaInv=getOmegaInv();    
  op_focei.logDetOmegaInv5 = getOmegaDet();
  if (op_focei.maxInnerIterations <= 0){
    std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0); // All etas = -42;  Unlikely if normal
// #ifdef _OPENMP
// #pragma omp parallel for num_threads(cores)
// #endif
    // Since we are evaluating the cholesky may be off
    for (int id = 0; id < rx->nsub; id++){
      focei_ind *indF = &(inds_focei[id]);
      indF->doChol = 1;
      try{
        innerEval(id);
      } catch(...) {
	indF->doChol = 0; // Use generalized cholesky decomposition
        innerEval(id);
	warning("Non-positive definite individual Hessian at solution(ID=%d); FOCEi objective functions may not be comparable.",id);
        indF->doChol = 1; // Cholesky again.
      }
    }
  } else {
// #ifdef _OPENMP
// #pragma omp parallel for num_threads(cores)
// #endif    
    for (int id = 0; id < rx->nsub; id++){
      focei_ind *indF = &(inds_focei[id]);
      try {
        innerOpt1(id, 0);
      } catch (...){
      	// First try resetting ETA
	std::fill(&indF->eta[0], &indF->eta[0] + op_focei.neta, 0.0);
      	try {
      	  innerOpt1(id, 0);
        } catch (...) {
      	  // Now try resetting Hessian, and ETA
      	  // Rprintf("Hessian Reset for ID: %d\n", id+1);
          indF->mode = 1;
          indF->uzm = 1;
          std::fill(&indF->eta[0], &indF->eta[0] + op_focei.neta, 0.0);
      	  try {
            // Rprintf("Hessian Reset & ETA reset for ID: %d\n", id+1);
            innerOpt1(id, 0);
          } catch (...){
            indF->mode = 1;
            indF->uzm = 1;
            std::fill(&indF->eta[0], &indF->eta[0] + op_focei.neta, 0.0);
            if(!op_focei.noabort){
              stop("Could not find the best eta even hessian reset and eta reset for ID %d.", id+1);
      	    } else if (indF->doChol == 1){
      	      indF->doChol = 0; // Use generalized cholesky decomposition
              indF->mode = 1;
              indF->uzm = 1;
              std::fill(&indF->eta[0], &indF->eta[0] + op_focei.neta, 0.0);
      	      try {
      		innerOpt1(id, 0);
      		indF->doChol = 1; // Use cholesky again.
      	      } catch (...){
      		// Just use ETA=0
                std::fill(&indF->eta[0], &indF->eta[0] + op_focei.neta, 0.0);
                try{
                  innerEval(id);
                } catch(...){
      		  warning("Bad solve during optimization.");
      		  // ("Cannot correct.");
                }
              }
      	    } else {
              // Just use ETA=0
              std::fill(&indF->eta[0], &indF->eta[0] + op_focei.neta, 0.0);
              try{
                innerEval(id);
              } catch(...){
                warning("Bad solve during optimization.");
                // ("Cannot correct.");
              }
            }
            // 
          }
        }
      }
    }
    // Reset ETA variances for next step
    op_focei.eta1SD = 1/sqrt(op_focei.etaS);
    std::fill(op_focei.etaM.begin(),op_focei.etaM.end(), 0.0);
    std::fill(op_focei.etaS.begin(),op_focei.etaS.end(), 0.0);
    op_focei.n = 0.0;
  }
  Rcpp::checkUserInterrupt();
}

static inline double foceiLik0(double *theta){
  updateTheta(theta);
  innerOpt();
  double lik = 0.0;
  for (int id=rx->nsub; id--;){
    focei_ind *fInd = &(inds_focei[id]);
    lik += fInd->lik[0];
  }
  // Now reset the saved ETAs
  std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0); // All etas = -42;  Unlikely if normal
  return lik;
}


static inline double foceiOfv0(double *theta){
  double ret = -2*foceiLik0(theta);
  if (!op_focei.initObj){
    op_focei.initObj=1;
    op_focei.initObjective=std::fabs(ret);
    if (op_focei.scaleObjective == 1) op_focei.scaleObjective=2;
  }
  if (op_focei.scaleObjective == 2){
    ret = ret / op_focei.initObjective * op_focei.scaleObjectiveTo;
  }
  if (!op_focei.calcGrad){
    if (op_focei.derivMethodSwitch){
      double diff = std::fabs(op_focei.lastOfv-ret);
      if (op_focei.derivMethod==0 && diff <= op_focei.derivSwitchTol){
	op_focei.derivMethod=1;
      } else if (op_focei.derivMethod==1 && diff > op_focei.derivSwitchTol){
	op_focei.derivMethod=0;
      }
    }
    op_focei.lastOfv = ret;
  }
  return ret;
}

//[[Rcpp::export]]
double foceiLik(NumericVector theta){
  return foceiLik0(&theta[0]);
}

//[[Rcpp::export]]
double foceiOfv(NumericVector theta){
  return foceiOfv0(&theta[0]);
}

//[[Rcpp::export]]
List foceiEtas(){
  List ret(op_focei.neta+2);
  CharacterVector nm(op_focei.neta+2);
  rx = getRxSolve_();
  IntegerVector ids(rx->nsub);
  NumericVector ofv(rx->nsub);
  int j,eta;
  for (j = op_focei.neta; j--;){
    ret[j+1]=NumericVector(rx->nsub);
    nm[j+1] = "ETA[" + std::to_string(j+1) + "]";
  }
  NumericVector tmp;
  for (j=rx->nsub; j--;){
    ids[j] = j+1;
    focei_ind *fInd = &(inds_focei[j]);
    ofv[j] = -2*fInd->lik[0];
    for (eta = op_focei.neta; eta--;){
      tmp = ret[eta+1];
      // Save eta is what the ETAs are saved
      tmp[j] = fInd->saveEta[eta];
    }
  }
  ret[0] = ids;
  nm[0] = "ID";
  ret[op_focei.neta+1]=ofv;
  nm[op_focei.neta+1] = "OBJI";
  ret.attr("names") = nm;
  ret.attr("class") = "data.frame";
  ret.attr("row.names") = IntegerVector::create(NA_INTEGER,-rx->nsub);
  return(ret);
}


// R style optimfn 
extern "C" double outerLikOpim(int n, double *par, void *ex){
  return(foceiOfv0(par));
}

void numericGrad(double *theta, double *g){
  op_focei.calcGrad=1;
  rx = getRxSolve_();
  int npars = op_focei.npars;
  int cpar;
  double cur, delta;
  double f=0;
  // Do Forward difference if the OBJF for *theta has already been calculated.
  bool doForward=false;
  if (op_focei.derivMethod == 0){
    doForward=true;
    // If the first derivative wasn't calculated, then calculate it.
    for (cpar = npars; cpar--;){
      if (theta[cpar] != op_focei.theta[cpar]){
        doForward=false;
        break;
      }
    }
    if (doForward){
      // Fill in lik0
      f=op_focei.lastOfv;
    } else {
      op_focei.calcGrad=0; // Save OBF and theta
      f = foceiOfv0(theta);
      op_focei.calcGrad=1;
      doForward=true;
    }
  }
  for (cpar = npars; cpar--;){
    delta = (std::fabs(theta[cpar])*op_focei.rEps + op_focei.aEps)/sqrt(1+std::fabs(min2(op_focei.initObjective, op_focei.lastOfv)));
    cur = theta[cpar];
    theta[cpar] = cur + delta;
    if (doForward){
      g[cpar] = (foceiOfv0(theta)-f)/delta;
    } else {
      f = foceiOfv0(theta);
      theta[cpar] = cur - delta;
      g[cpar] = (f-foceiOfv0(theta))/(2*delta);
    }
    theta[cpar] = cur;
  }
  op_focei.calcGrad=0;
}

//[[Rcpp::export]]
NumericVector foceiNumericGrad(NumericVector theta){
  NumericVector ret(theta.size());
  numericGrad(&theta[0], &ret[0]);
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Setup FOCEi functions
CharacterVector rxParams_(const RObject &obj);
List rxModelVars_(const RObject &obj);

bool rxDynLoad(RObject obj);

static inline void foceiSetupTrans_(CharacterVector pars){
  unsigned int k, j,  ps = pars.size();
  k=ps;
  std::string thetaS;
  std::string etaS;
  std::string cur;
  foceiEtaN(ps+1);
  foceiThetaN(ps+1);
  op_focei.neta = 0;
  op_focei.ntheta = 0;
  for (;k--;){
    for (j = ps; j--;){
      // Compare ETAS first since they are smaller strings.
      cur = as<std::string>(pars[k]);
      etaS = "ETA[" + std::to_string(j+1) + "]";
      if (cur == etaS){
        op_focei.etaTrans[j] = k;
        op_focei.neta++;
        break;
      } else {
        thetaS = "THETA[" + std::to_string(j+1) + "]";
        if (cur == thetaS){
          op_focei.thetaTrans[j] = k;
          op_focei.ntheta++;
          break;
        }
      }
    }
  }
  op_focei.nzm = (op_focei.neta + 1) * (op_focei.neta + 2) / 2 + (op_focei.neta + 1)*6+1;
}

static inline void foceiSetupTheta_(List mvi,
				    NumericVector theta,
				    Nullable<LogicalVector> thetaFixed, 
				    double scaleTo,
				    bool alloc){
  // Get the fixed thetas
  // fixedTrans gives the theta->full theta translation
  // initPar is the initial parameters used for parameter scaling.
  op_focei.scaleTo=scaleTo;
  int thetan = theta.size();
  int omegan = getOmegaN();
  NumericVector omegaTheta = getOmegaTheta();
  int fixedn = 0;
  int j;
  LogicalVector thetaFixed2;
  if (!thetaFixed.isNull()){
    thetaFixed2 = as<LogicalVector>(thetaFixed);
    for (j = thetaFixed2.size(); j--;){
      if (thetaFixed2[j]) fixedn++;
    }
  } else {
    thetaFixed2 =LogicalVector(0);
  }
  int npars = thetan+omegan-fixedn;
  if (alloc){
    rxUpdateInnerFuns(as<SEXP>(mvi["trans"]));
    foceiSetupTrans_(as<CharacterVector>(mvi["params"]));
    foceiThetaN(thetan);
  }
  std::copy(theta.begin(), theta.end(), &op_focei.fullTheta[0]);  
  std::copy(omegaTheta.begin(), omegaTheta.end(), &op_focei.fullTheta[0]+thetan);
  op_focei.thetan = thetan;
  op_focei.omegan = omegan;
  int k = 0;
  for (j = 0; j < npars+fixedn; j++){
    if (j < thetaFixed2.size() && !thetaFixed2[j]){
      if (j < theta.size()){
        op_focei.initPar[k] = theta[j];
      } else if (j < theta.size() + omegan){
        op_focei.initPar[k] = omegaTheta[j-theta.size()];
      }
      op_focei.fixedTrans[k++] = j;
    } else if (j >= thetaFixed2.size()){
      if (j < theta.size()){
        op_focei.initPar[k] = theta[j];
        op_focei.fixedTrans[k++] = j;
      } else if (j < theta.size() + omegan){
	op_focei.initPar[k] = omegaTheta[j-theta.size()];
	op_focei.fixedTrans[k++] = j;
      } 
    }
  }
  op_focei.npars  = npars;
}

static inline void foceiSetupEta_(NumericMatrix etaMat0){
  rx = getRxSolve_();
  rxFoceiEnsure(rx->nsub);
  etaMat0 = transpose(etaMat0);
  foceiGEtaN((op_focei.neta+1)*rx->nsub);
  foceiGThetaN(op_focei.npars*(rx->nsub + 1));
  foceiGgZm(((op_focei.neta+1)*(op_focei.neta+2)/2+6*(op_focei.neta+1)+1)*rx->nsub);
  op_focei.etaM = mat(op_focei.neta, 1, fill::zeros);
  op_focei.etaS = mat(op_focei.neta, 1, fill::zeros);
  op_focei.eta1SD = mat(op_focei.neta, 1, fill::zeros);
  unsigned int i, j = 0, k = 0, ii=0, jj = 0;
  focei_ind *fInd;
  for (i = rx->nsub; i--;){
    fInd = &(inds_focei[i]);
    fInd->doChol=!(op_focei.cholSEOpt);
    // ETA ini
    fInd->eta = &op_focei.geta[j];
    fInd->oldEta = &op_focei.goldEta[j];
    fInd->saveEta = &op_focei.gsaveEta[j];
    fInd->g = &op_focei.gG[j];
    fInd->x = &op_focei.gX[j];
    fInd->var = &op_focei.gVar[j];

    // Copy in etaMat0 to the inital eta stored (0 if unspecified)
    // std::copy(&etaMat0[i*op_focei.neta], &etaMat0[(i+1)*op_focei.neta], &fInd->saveEta[0]);
    std::copy(&etaMat0[i*op_focei.neta], &etaMat0[(i+1)*op_focei.neta], &fInd->eta[0]);
    
    fInd->eta[op_focei.neta] = i;
    fInd->saveEta[op_focei.neta] = i;
    fInd->oldEta[op_focei.neta] = i;
    j+=op_focei.neta+1;

    k+=op_focei.neta;

    fInd->zm = &op_focei.gZm[ii];
    ii+=(op_focei.neta+1) * (op_focei.neta + 2) / 2 + 6*(op_focei.neta + 1)+1;

    fInd->thetaGrad = &op_focei.gthetaGrad[jj];
    jj+= op_focei.npars;

    fInd->mode = 1;
    fInd->uzm = 1;
  }
  op_focei.thetaGrad = &op_focei.gthetaGrad[jj];
}

// [[Rcpp::export]]
NumericVector foceiSetup_(const RObject &obj,
			  const RObject &data,
			  NumericVector theta,
			  Nullable<LogicalVector> thetaFixed = R_NilValue,
			  Nullable<LogicalVector> skipCov    = R_NilValue,
			  RObject rxInv                      = R_NilValue,
			  Nullable<NumericVector> lower      = R_NilValue,
			  Nullable<NumericVector> upper      = R_NilValue,
			  Nullable<NumericMatrix> etaMat     = R_NilValue,
			  Nullable<List> control             = R_NilValue){
  if (!rxIs(rxInv, "rxSymInvCholEnv")){
    stop("Omega isn't in the proper format.");
  } else {
    _rxInv = as<List>(rxInv);
  }
  if (control.isNull()){
    stop("ODE options must be specified.");
  }
  List odeO = as<List>(control);
  NumericVector cEps=odeO["derivEps"];
  if (cEps.size() != 2){
    stop("derivEps must be 2 elements for determining central or forward difference step size.");
  }
  NumericVector covDerivEps=odeO["covDerivEps"];
  if (covDerivEps.size() != 2){
    stop("covDerivEps must be 2 elements for determining central or forward difference step size.");
  }
  op_focei.derivMethod = as<int>(odeO["derivMethod"]);
  if (op_focei.derivMethod == 3){
    op_focei.derivMethod=0;
    op_focei.derivSwitchTol=as<double>(odeO["derivSwitchTol"]);
    op_focei.derivMethodSwitch=1;
  }  
  if (op_focei.derivMethod){
    op_focei.rEps=std::fabs(cEps[0])/2.0;
    op_focei.aEps=std::fabs(cEps[1])/2.0;
    op_focei.rEpsC=std::fabs(covDerivEps[0])/2.0;
    op_focei.aEpsC=std::fabs(covDerivEps[1])/2.0;
  } else {
    op_focei.rEps=std::fabs(cEps[0]);
    op_focei.aEps=std::fabs(cEps[1]);
    op_focei.rEpsC=std::fabs(covDerivEps[0]);
    op_focei.aEpsC=std::fabs(covDerivEps[1]);
  }
  // This fills in op_focei.neta
  List mvi;
  if (!rxIs(obj, "NULL")){
    if (!rxDynLoad(obj)){
      stop("Cannot load RxODE dlls for this model.");
    }
    mvi = rxModelVars_(obj);
  }
  op_focei.mvi = mvi;
  
  if (op_focei.skipCov != NULL) Free(op_focei.skipCov);
  if (skipCov.isNull()){
    op_focei.skipCovN = 0;
  } else {
    IntegerVector skipCov1 = as<IntegerVector>(skipCov);
    op_focei.skipCovN = skipCov1.size();
    op_focei.skipCov = Calloc(skipCov1.size(), int);
    std::copy(skipCov1.begin(),skipCov1.end(),op_focei.skipCov);
  }
  op_focei.maxOuterIterations = as<int>(odeO["maxOuterIterations"]);
  op_focei.maxInnerIterations = as<int>(odeO["maxInnerIterations"]);
  if (op_focei.maxOuterIterations == 0){
    foceiSetupTheta_(mvi, theta, thetaFixed, 0.0, !rxIs(obj, "NULL"));
    op_focei.scaleObjective=0;
  } else {
    foceiSetupTheta_(mvi, theta, thetaFixed, as<double>(odeO["scaleTo"]), !rxIs(obj, "NULL"));
    op_focei.scaleObjectiveTo=as<double>(odeO["scaleObjective"]);
    if (op_focei.scaleObjectiveTo <= 0){
      op_focei.scaleObjective=0;
    } else {
      op_focei.scaleObjective=1;
    }
  }
  // First see if etaMat is null.
  NumericMatrix etaMat0;
  unsigned int nsub=0;
  // Find the number of IDs to create an etaMat
  List df = as<List>(data);
  CharacterVector dfN = df.names();
  int idn = -1;
  std::string cur;
  for (unsigned int j = dfN.size(); j--;){
    cur = as<std::string>(dfN[j]);
    if (cur == "ID" || cur == "id" || cur == "Id" || cur == "iD"){
      idn=j;
      break;
    }
  }
  if  (idn == -1){
    stop("Can't find ID in dataset.");
  }
  IntegerVector ids = as<IntegerVector>(df[idn]);
  int last = ids[ids.size()-1]-1;
  for (unsigned int j = ids.size(); j--;){
    if (last != ids[j]){
      last = ids[j];
      nsub++;
    }
  }
  etaMat0 = NumericMatrix(nsub, op_focei.neta);
  if (!etaMat.isNull()){
    NumericMatrix etaMat1 = NumericMatrix(etaMat);
    if (etaMat1.nrow() != (int)nsub){
      print(etaMat1);
      stop("The etaMat must have the same number of ETAs (rows) as subjects.");
    }
    if (etaMat1.ncol() != op_focei.neta){
      print(etaMat1);
      stop("The etaMat must have the same number of ETAs (cols) as the model.");
    }
    std::copy(etaMat1.begin(),etaMat1.end(),etaMat0.begin());
  }
  List params(theta.size()+op_focei.neta);
  CharacterVector paramsNames(theta.size()+op_focei.neta);
  unsigned int j;
  for (j = theta.size();j--;){
    params[j] = NumericVector(nsub,theta[j]);
    if (theta.hasAttribute("names")){
      paramsNames[j] = (as<CharacterVector>(theta.names()))[j];
    } else {
      paramsNames[j] = "THETA[" + std::to_string(j + 1) + "]";
    }
  }
  bool hasDimn = etaMat0.hasAttribute("dimnames");
  CharacterVector dims;
  if (hasDimn){
    List diml = etaMat0.attr("dimnames");
    if (!rxIs(as<RObject>(diml[1]),"NULL")){
      dims = as<CharacterVector>(diml[1]);
    } else {
      hasDimn=false;
    }
  }
  for (j=op_focei.neta; j--;){
    params[j+theta.size()]= etaMat0(_, j);
    if (hasDimn){
      paramsNames[j+theta.size()] = dims[j];
    } else {
      paramsNames[j+theta.size()] = "ETA[" + std::to_string(j + 1) + "]";
    }
  }
  params.names() = paramsNames;
  params.attr("class") = "data.frame";
  params.attr("row.names") = IntegerVector::create(NA_INTEGER,-nsub);
  // Now pre-fill parameters.
  if (!rxIs(obj, "NULL")){
    rxSolveC(obj,
	     R_NilValue,//const Nullable<CharacterVector> &specParams = 
	     R_NilValue,//const Nullable<List> &extraArgs = 
	     as<RObject>(params),//const RObject &params = 
	     data,//const RObject &events = 
	     R_NilValue,//const RObject &inits = 
	     R_NilValue,//const RObject &scale = 
	     R_NilValue,//const RObject &covs  = 
	     as<int>(odeO["method"]), // const int method = 
	     odeO["transitAbs"], //1
	     as<double>(odeO["atol"]),//const double atol = 1.0e-6
	     as<double>(odeO["rtol"]),// const double rtol = 1.0e-4
	     as<double>(odeO["maxstepsOde"]),//const int  = 5000, //4
	     as<double>(odeO["hmin"]),
	     odeO["hmax"], //6
	     as<double>(odeO["hini"]), //7
	     as<int>(odeO["maxordn"]), //8
	     as<int>(odeO["maxords"]), //9
	     as<int>(odeO["cores"]), //10
	     as<int>(odeO["covsInterpolation"]), //11
	     false, // bool addCov = false
	     0,//int matrix = 0, //13
	     R_NilValue,//const Nullable<NumericMatrix> &sigma= R_NilValue, //14
	     R_NilValue,//const Nullable<NumericVector> &sigmaDf= R_NilValue, //15
	     1, //const int &nCoresRV= 1, //16
	     false,//const bool &sigmaIsChol= false,
	     10000,//const int &nDisplayProgress = 10000,
	     NA_STRING,//const CharacterVector &amountUnits = NA_STRING,
	     "hours",//const character_vector &timeUnits = "hours",
	     false,//const bool addDosing = false,
	     as<double>(odeO["stateTrim"]),//const double stateTrim = R_PosInf,
	     R_NilValue,//const RObject &theta = R_NilValue,
	     R_NilValue,//const RObject &eta = R_NilValue,
	     false,//const bool updateObject = false,
	     true,//const bool doSolve = true,
	     R_NilValue,//const Nullable<NumericMatrix> &omega = R_NilValue, 
	     R_NilValue,//const Nullable<NumericVector> &omegaDf = R_NilValue, 
	     false,//const bool &omegaIsChol = false,
	     1,//const unsigned int nSub = 1, 
	     R_NilValue,//const Nullable<NumericMatrix> &thetaMat = R_NilValue, 
	     R_NilValue,//const Nullable<NumericVector> &thetaDf = R_NilValue, 
	     false,//const bool &thetaIsChol = false,
	     1,//const unsigned int nStud = 1, 
	     0.0,//const double dfSub=0.0,
	     0.0,//const double dfObs=0.0,
	     1);//const int setupOnly = 0
    rx = getRxSolve_();
    foceiSetupEta_(etaMat0);
  }
  op_focei.epsilon=as<double>(odeO["epsilon"]);
  op_focei.nsim=as<int>(odeO["n1qn1nsim"]);
  op_focei.imp=0;
  op_focei.printInner=as<int>(odeO["printInner"]);
  if (op_focei.printInner < 0) op_focei.printInner = -op_focei.printInner;
  op_focei.printOuter=as<int>(odeO["print"]);
  if (op_focei.printOuter < 0) op_focei.printOuter = -op_focei.printOuter;
  if (op_focei.printInner > 0){
    rx->op->cores=1;
  }
  int totN=op_focei.thetan + op_focei.omegan;
  NumericVector lowerIn(totN);
  NumericVector upperIn(totN);
  if (lower.isNull()){
    std::fill_n(lowerIn.begin(), totN, R_NegInf);
  } else {
    NumericVector lower1=as<NumericVector>(lower);
    if (lower1.size() == 1){
      std::fill_n(lowerIn.begin(), totN, lower1[0]);
    } else if (lower1.size() < totN){
      std::copy(lower1.begin(), lower1.end(), lowerIn.begin());
      std::fill_n(lowerIn.begin()+lower1.size(), totN - lower1.size(), R_NegInf);
    } else if (lower1.size() > totN){
      warning("Lower bound is larger than the number of parameters being estimated.");
      std::copy(lower1.begin(), lower1.begin()+totN, lowerIn.begin());
    } else {
      lowerIn = lower1;
    }
  }
  if (upper.isNull()){
    std::fill_n(upperIn.begin(), totN, R_PosInf);
  } else {
    NumericVector upper1=as<NumericVector>(upper);
    if (upper1.size() == 1){
      std::fill_n(upperIn.begin(), totN, upper1[0]);
    } else if (upper1.size() < totN){
      std::copy(upper1.begin(), upper1.end(), upperIn.begin());
      std::fill_n(upperIn.begin()+upper1.size(), totN - upper1.size(), R_PosInf);
    } else if (upper1.size() > totN){
      warning("Upper bound is larger than the number of parameters being estimated.");
      std::copy(upper1.begin(), upper1.begin()+totN, upperIn.begin());
    } else {
      upperIn = upper1;
    }
  }
  op_focei.lowerIn =lowerIn;
  op_focei.upperIn =upperIn;
  if (op_focei.likSav != NULL) Free(op_focei.likSav);
  if (op_focei.lower != NULL) Free(op_focei.lower);
  if (op_focei.upper != NULL) Free(op_focei.upper);
  op_focei.lower = Calloc(op_focei.npars, double);
  op_focei.upper = Calloc(op_focei.npars, double);
  op_focei.nbd   = Calloc(op_focei.npars, int);
  std::fill_n(&op_focei.nbd[0], op_focei.npars, 0);

  NumericVector ret(op_focei.npars, op_focei.scaleTo);
  if (op_focei.scaleTo <= 0){
    for (unsigned int k = op_focei.npars; k--;){
      j=op_focei.fixedTrans[k];
      ret[k] = op_focei.fullTheta[j];
      if (R_FINITE(lowerIn[j])){
        op_focei.lower[k] = lowerIn[j];
        op_focei.lower[k] += 2*(op_focei.lower[k]*op_focei.rEps + op_focei.aEps);
        // lower bound only = 1
        op_focei.nbd[k]=1;
      } else {
        op_focei.lower[k] = R_NegInf;//std::numeric_limits<double>::lowest();
      }
      if (R_FINITE(upperIn[j])){
        op_focei.upper[k] = upperIn[j];
        op_focei.upper[k] -= 2*(op_focei.upper[k]*op_focei.rEps - op_focei.aEps);
        // Upper bound only = 3
        // Upper and lower bound = 2
        op_focei.nbd[k]= 3 - op_focei.nbd[j];
      } else {
        op_focei.upper[k] = R_PosInf;//std::numeric_limits<double>::max();
      }
    }
  } else {
    for (unsigned int k = op_focei.npars; k--;){
      j=op_focei.fixedTrans[k];
      if (R_FINITE(lowerIn[j])){
        op_focei.lower[k] = lowerIn[j] * op_focei.scaleTo / op_focei.initPar[j];
        op_focei.lower[k] += 2*(op_focei.lower[k]*op_focei.rEps + op_focei.aEps);
        // lower bound only = 1
        op_focei.nbd[k]=1;
      } else {
        op_focei.lower[k] = R_NegInf;//std::numeric_limits<double>::lowest();
      }
      if (R_FINITE(upperIn[j])){
        op_focei.upper[k] = upperIn[j] * op_focei.scaleTo / op_focei.initPar[j];
        op_focei.upper[k] -= 2*(op_focei.upper[k]*op_focei.rEps - op_focei.aEps);
        // Upper bound only = 3
        // Upper and lower bound = 2
        op_focei.nbd[k]= 3 - op_focei.nbd[j];
      } else {
        op_focei.upper[k] = R_PosInf;//std::numeric_limits<double>::max();
      }
    }
  }

  op_focei.calcGrad=0;
  if (op_focei.likSav != NULL) Free(op_focei.likSav);
  if (!rxIs(obj, "NULL")) op_focei.likSav = Calloc(rx->nsub, double);
  n1qn1_ = (n1qn1_fp) R_GetCCallable("n1qn1","n1qn1F");
  
  // Outer options
  op_focei.outerOpt	=as<int>(odeO["outerOpt"]);
  // lbfgsb options
  op_focei.factr	= as<double>(odeO["lbfgsFactr"]);
  op_focei.pgtol	= as<double>(odeO["lbfgsPgtol"]);
  op_focei.lmm		= as<int>(odeO["lbfgsLmm"]);
  op_focei.covDerivMethod = as<int>(odeO["covDerivMethod"]);
  op_focei.covMethod = as<int>(odeO["covMethod"]);
  op_focei.eigen = as<int>(odeO["eigen"]);
  op_focei.ci=0.95;
  op_focei.sigdig=as<double>(odeO["sigdig"]);
  op_focei.useColor=as<int>(odeO["useColor"]);
  op_focei.boundTol=as<double>(odeO["boundTol"]);
  op_focei.printNcol=as<int>(odeO["printNcol"]);
  op_focei.noabort=as<int>(odeO["noAbort"]);
  op_focei.interaction=as<int>(odeO["interaction"]);
  op_focei.cholSEtol=as<double>(odeO["cholSEtol"]);
  op_focei.hessEps=as<double>(odeO["hessEps"]);
  op_focei.cholAccept=as<double>(odeO["cholAccept"]);
  op_focei.resetEtaSize=as<double>(odeO["resetEtaSize"]);
  op_focei.cholSEOpt=as<double>(odeO["cholSEOpt"]);
  op_focei.cholSECov=as<double>(odeO["cholSECov"]);
  op_focei.fo=as<int>(odeO["fo"]);
  if (op_focei.fo) op_focei.maxInnerIterations=0;
  op_focei.covTryHarder=as<int>(odeO["covTryHarder"]);
  op_focei.resetHessianAndEta=as<int>(odeO["resetHessianAndEta"]);
  op_focei.initObj=0;
  op_focei.lastOfv=std::numeric_limits<double>::max();
  return ret;
}

LogicalVector nlmixrEnvSetup(Environment e, double fmin){
  if (e.exists("theta") && rxIs(e["theta"], "data.frame") &&
      e.exists("omega") && rxIs(e["omega"], "matrix") &&
      e.exists("etaObf") && rxIs(e["etaObf"], "data.frame")){
    arma::mat omega = as<arma::mat>(e["omega"]);
    arma::mat D(omega.n_rows,omega.n_rows,fill::zeros);
    arma::mat cor(omega.n_rows,omega.n_rows);
    D.diag() = (sqrt(omega.diag()));
    arma::vec sd=D.diag();
    D = inv_sympd(D);
    cor = D * omega * D;
    cor.diag()= sd;
    e["omegaR"] = wrap(cor); 
    if (op_focei.scaleObjective){
      fmin = fmin * op_focei.initObjective / op_focei.scaleObjectiveTo;
    }
    if (!e.exists("objective")){
      e["objective"] = fmin;
    } else {
      fmin = as<double>(e["objective"]);
    }
    e["OBJF"] = fmin;
    e["objf"] = fmin;
    NumericVector logLik(1);
    logLik[0]=-fmin/2;
    logLik.attr("df") = op_focei.npars;
    if (e.exists("nobs")){
      logLik.attr("nobs") = e["nobs"];
      e["BIC"] = fmin + log(as<double>(e["nobs"]))*op_focei.npars;
    } else {
      logLik.attr("nobs") = rx->nobs;
      e["BIC"] = fmin + log((double)rx->nobs)*op_focei.npars;
      e["nobs"] = rx->nobs;
    }
    logLik.attr("class") = "logLik";
    e["logLik"] = logLik;

    e["AIC"] = fmin+2*op_focei.npars;
    return true;
  } else {
    stop("Not Setup right.........");
    return false;
  }
}

void foceiOuterFinal(double *x, Environment e){
  double fmin = foceiOfv0(x);
  
  NumericVector theta(op_focei.thetan);
  std::copy(&op_focei.fullTheta[0],  &op_focei.fullTheta[0] + op_focei.thetan, 
            theta.begin());

  NumericVector fullTheta(op_focei.thetan+op_focei.omegan);
  std::copy(&op_focei.fullTheta[0],  &op_focei.fullTheta[0] + op_focei.thetan + op_focei.omegan, 
            fullTheta.begin());
  LogicalVector thetaFixed(op_focei.thetan);
  std::fill_n(thetaFixed.begin(),op_focei.thetan, true);
  unsigned int j;
  for (unsigned int k = op_focei.npars; k--;){
    j=op_focei.fixedTrans[k];
    if (j < thetaFixed.size()) thetaFixed[j]=false;
  }
  // std::copy(&op_focei.thetaFixed[0],  &op_focei.thetaFixed[0] + op_focei.thetan, 
  //           thetaFixed.begin());
  NumericVector lowerIn(op_focei.thetan);
  NumericVector upperIn(op_focei.thetan);
  std::copy(&op_focei.lowerIn[0],  &op_focei.lowerIn[0] + op_focei.thetan, 
            lowerIn.begin());
  std::copy(&op_focei.upperIn[0],  &op_focei.upperIn[0] + op_focei.thetan, 
            upperIn.begin());
  e["theta"] = DataFrame::create(_["lower"]=lowerIn, _["theta"]=theta, _["upper"]=upperIn,
				 _["fixed"]=thetaFixed);
  e["fullTheta"] = fullTheta;
  e["omega"] = getOmega();
  e["etaObf"] = foceiEtas();
  nlmixrEnvSetup(e, fmin);
}

static inline void foceiPrintLine(int ncol){
  Rprintf("|-----+---------------+");
  for (int i = 0; i < ncol; i++){
    if (i == ncol-1)
      Rprintf("-----------|");
    else 
      Rprintf("-----------+");
  }
  Rprintf("\n");
}

////////////////////////////////////////////////////////////////////////////////
// Outer l-BFGS-b from R
extern "C" double foceiOfvOptim(int n, double *x, void *ex){
  double ret = foceiOfv0(x);
  op_focei.nF++;
  if (op_focei.printOuter != 0 && op_focei.nF % op_focei.printOuter == 0){
    int finalize = 0, i = 0;
    if (op_focei.useColor && !isRstudio())
      Rprintf("|\033[1m%5d\033[0m|%#14.8g |", op_focei.nF, ret);
    else 
      Rprintf("|%5d|%#14.8g |", op_focei.nF, ret);
    for (i = 0; i < n; i++){
      Rprintf("%#10.4g |", x[i]);
      if ((i + 1) != n && (i + 1) % op_focei.printNcol == 0){
        if (op_focei.useColor && op_focei.printNcol + i  > n){
          Rprintf("\n\033[4m|.....................|");
        } else {
          Rprintf("\n|.....................|");
        }
	finalize=1;
      }
    }
    if (finalize){
      while(true){
        if ((i++) % op_focei.printNcol == 0){
          if (op_focei.useColor) Rprintf("\033[0m");
          Rprintf("\n");
          break;
        } else {
          Rprintf("...........|");
        }
      }
    } else {
      Rprintf("\n");
    }
    if (op_focei.scaleTo > 0){
      if (op_focei.scaleObjective){
        Rprintf("|    U|%14.8g |", op_focei.initObjective * ret / op_focei.scaleObjectiveTo);
      } else {
        Rprintf("|    U|%14.8g |", ret);
      }
      for (i = 0; i < n; i++){
        Rprintf("%#10.4g |", x[i]*op_focei.initPar[i]/op_focei.scaleTo);
        if ((i + 1) != n && (i + 1) % op_focei.printNcol == 0){
          if (op_focei.useColor && op_focei.printNcol + i  > op_focei.npars){
            Rprintf("\n\033[4m|.....................|");
          } else {
            Rprintf("\n|.....................|");
          }
        }
      }
      if (finalize){
        while(true){
          if ((i++) % op_focei.printNcol == 0){
            if (op_focei.useColor) Rprintf("\033[0m");
            Rprintf("\n");
            break;
          } else {
            Rprintf("...........|");
          }
        }
      } else {
        Rprintf("\n");
      }
      if (op_focei.scaleObjective){
        if (op_focei.useColor && !isRstudio())
          Rprintf("|    X|\033[1m%14.8g\033[0m |", op_focei.initObjective * ret / op_focei.scaleObjectiveTo);
        else 
          Rprintf("|    X|%14.8g |", op_focei.initObjective * ret / op_focei.scaleObjectiveTo);
      } else {
        if (op_focei.useColor && !isRstudio())
          Rprintf("|    X|\033[1m%14.8g\033[0m |", ret);
        else 
          Rprintf("|    X|%14.8g |", ret);
      }
      for (i = 0; i < n; i++){
	if (op_focei.xPar[i]){
          Rprintf("%#10.4g |", exp(x[i]*op_focei.initPar[i]/op_focei.scaleTo));
        } else {
          Rprintf("%#10.4g |", x[i]*op_focei.initPar[i]/op_focei.scaleTo);
	}
        if ((i + 1) != n && (i + 1) % op_focei.printNcol == 0){
          if (op_focei.useColor && op_focei.printNcol + i >= op_focei.npars){
            Rprintf("\n\033[4m|.....................|");
          } else {
            Rprintf("\n|.....................|");
          }
        }
      }
    } else {
      if (op_focei.useColor  && !isRstudio()){
	if (op_focei.scaleObjective){
          Rprintf("|    X|\033[1m%14.8g \033[0m|", op_focei.initObjective * ret / op_focei.scaleObjectiveTo);
	} else {
	  Rprintf("|    X|\033[1m%14.8g \033[0m|", ret);
	}
      } else {
        if (op_focei.scaleObjective){
	  Rprintf("|    X|%14.8g |", op_focei.initObjective * ret / op_focei.scaleObjectiveTo);
	} else {
          Rprintf("|    X|\033[1m%14.8g \033[0m|", ret);
        }
      }
      for (i = 0; i < n; i++){
        if (op_focei.xPar[i]){
          Rprintf("%#10.4g |", exp(x[i]));
        } else {
          Rprintf("%#10.4g |", x[i]);
        }
        if ((i + 1) != n && (i + 1) % op_focei.printNcol == 0){
          if (op_focei.useColor && op_focei.printNcol + i  >= op_focei.npars){
            Rprintf("\n\033[4m|.....................|");
          } else {
            Rprintf("\n|.....................|");
          }
        }
      }
    }
    if (finalize){
      while(true){
        if ((i++) % op_focei.printNcol == 0){
          if (op_focei.useColor) Rprintf("\033[0m");
          Rprintf("\n");
          break;
        } else {
          Rprintf("...........|");
        }
      }
    } else {
      Rprintf("\n");
    }
  }
  return ret;
}

//[[Rcpp::export]]
double foceiOuterF(NumericVector &theta){
  int n = theta.size();
  void *ex = NULL;
  return foceiOfvOptim(n, theta.begin(), ex);
}

extern "C" void outerGradNumOptim(int n, double *par, double *gr, void *ex){
  numericGrad(par, gr);
  op_focei.nG++;
  if (op_focei.printOuter != 0 && op_focei.nG % op_focei.printOuter == 0){
    int finalize=0, i = 0;
    if (op_focei.useColor && op_focei.printNcol >= n){
      switch(op_focei.derivMethod){
      case 0:
	Rprintf("|\033[4m    G| Forward Diff. |");
	break;
      case 1:
	Rprintf("|\033[4m    G| Central Diff. |");
	break;
      }
    } else {
      switch(op_focei.derivMethod){
      case 0:
	Rprintf("|    G| Forward Diff. |");
	break;
      case 1:\
	Rprintf("|    G| Central Diff. |");
	break;
      }
    }
    for (i = 0; i < n; i++){
      Rprintf("%#10.4g ", gr[i]);
      if (op_focei.useColor && op_focei.printNcol >= n && i == n-1){
	Rprintf("\033[0m");
      }
      Rprintf("|");
      if ((i + 1) != n && (i + 1) % op_focei.printNcol == 0){
        if (op_focei.useColor && op_focei.printNcol + i  >= op_focei.npars){
          Rprintf("\n\033[4m|.....................|");
        } else {
          Rprintf("\n|.....................|");
        }
        finalize=1;
      }
    }
    if (finalize){
      while(true){
        if ((i++) % op_focei.printNcol == 0){
          if (op_focei.useColor) Rprintf("\033[0m");
          Rprintf("\n");
	  break;
        } else {
          Rprintf("...........|");
	}
      }
    } else {
      Rprintf("\n");
    }
    if (!op_focei.useColor){
      foceiPrintLine(min2(op_focei.npars, op_focei.printNcol));
    }
  }
}

//[[Rcpp::export]]
NumericVector foceiOuterG(NumericVector &theta){
  int n = theta.size();
  void *ex = NULL;
  NumericVector gr(n);
  outerGradNumOptim(n, theta.begin(), gr.begin(), ex);
  return gr;
}

void foceiLbfgsb(Environment e){
  void *ex = NULL;
  double Fmin;
  int fail, fncount=0, grcount=0;
  NumericVector x(op_focei.npars);
  if (op_focei.scaleTo > 0){
    std::fill_n(&x[0], op_focei.npars, op_focei.scaleTo);
  } else {
    std::copy(&op_focei.initPar[0], &op_focei.initPar[0]+op_focei.npars,&x[0]);
  }
  char msg[100];
  lbfgsbRX(op_focei.npars, op_focei.lmm, x.begin(), op_focei.lower,
           op_focei.upper, op_focei.nbd, &Fmin, foceiOfvOptim,
           outerGradNumOptim, &fail, ex, op_focei.factr,
           op_focei.pgtol, &fncount, &grcount,
           op_focei.maxOuterIterations, msg, 0, op_focei.maxOuterIterations+1);
  // Recalculate OFV in case the last calculated OFV isn't at the minimum....
  // Otherwise ETAs may be off
  std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0); // All etas = -42;  Unlikely if normal
  // Finalize environment
  foceiOuterFinal(x.begin(), e);
  e["convergence"] = fail;
  e["message"] = msg;
}

Function getRxFn(std::string name);
void foceiCustomFun(Environment e){
  NumericVector x(op_focei.npars);
  NumericVector lower(op_focei.npars);
  NumericVector upper(op_focei.npars);
  if (op_focei.scaleTo > 0){
    std::fill_n(&x[0], op_focei.npars, op_focei.scaleTo);
  } else {
    std::copy(&op_focei.initPar[0], &op_focei.initPar[0]+op_focei.npars,&x[0]);
  }
  std::copy(&op_focei.upper[0], &op_focei.upper[0]+op_focei.npars, &upper[0]);
  std::copy(&op_focei.lower[0], &op_focei.lower[0]+op_focei.npars, &lower[0]);
  Function f = getRxFn("foceiOuterF");
  Function g = getRxFn("foceiOuterG");
  List ctl = e["control"];
  Function opt = as<Function>(ctl["outerOptFun"]);
  //.bobyqa <- function(par, fn, gr, lower = -Inf, upper = Inf, control = list(), ...)
  List ret = as<List>(opt(_["par"]=x, _["fn"]=f, _["gr"]=g, _["lower"]=lower,
			  _["upper"]=upper,_["control"]=ctl));
  x = ret["x"];
  // Recalculate OFV in case the last calculated OFV isn't at the minimum....
  // Otherwise ETAs may be off
  std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0); // All etas = -42;  Unlikely if normal
  // Finalize environment
  foceiOuterFinal(x.begin(), e);
  e["convergence"] = ret["convergence"];
  e["message"] = ret["message"];
  e["optReturn"] = ret;
}


////////////////////////////////////////////////////////////////////////////////
// Overall Outer Problem

//[[Rcpp::export]]
Environment foceiOuter(Environment e){
  op_focei.nF=0;
  op_focei.nG=0;
  if (op_focei.maxOuterIterations > 0){
    switch(op_focei.outerOpt){
    case 0:
      foceiLbfgsb(e);
      break;
    case -1:
      foceiCustomFun(e);
    } 
  } else {
    NumericVector x(op_focei.npars);
    if (op_focei.scaleTo > 0){
      std::fill_n(&x[0], op_focei.npars, op_focei.scaleTo);
    } else {
      std::copy(&op_focei.initPar[0], &op_focei.initPar[0]+op_focei.npars,&x[0]);
    }
    foceiOuterFinal(x.begin(), e);
    if (op_focei.maxInnerIterations == 0){
      e["fail"] = NA_INTEGER;
      e["message"] = "Likelihood evaluation with provided ETAs";
    } else {
      e["fail"] = 0;
      e["message"] = "Posthoc prediction with provided THETAs";
    }
  }
  return e;
}


////////////////////////////////////////////////////////////////////////////////
// Covariance functions
void foceiCalcR(Environment e){
  rx = getRxSolve_();
  arma::mat H(op_focei.npars, op_focei.npars);
  arma::vec dpar(op_focei.npars);
  unsigned int i, j, k;
  for (k = op_focei.npars; k--;){
    j=op_focei.fixedTrans[k];
    if (op_focei.scaleTo > 0){
      dpar[k] = op_focei.fullTheta[j] / op_focei.initPar[k] * op_focei.scaleTo;
    } else {
      dpar[k] = op_focei.fullTheta[j];
    }
  }
  arma::vec df1(op_focei.npars);
  arma::vec df2(op_focei.npars);
  double eps;

  double fnscale = 1.0;
  if (op_focei.scaleObjective == 2){
    fnscale = op_focei.initObjective / op_focei.scaleObjectiveTo;
  }
  double parScale1=1.0, parScale2=1.0;
  for (i=op_focei.npars; i--;){
    if (op_focei.scaleTo > 0){
      parScale1=op_focei.initPar[i] / op_focei.scaleTo;
      eps = op_focei.hessEps / parScale1;
    } else {
      eps = op_focei.hessEps;
    }
    dpar[i] = dpar[i]+eps;
    numericGrad(dpar.begin(), df1.begin());
    op_focei.cur++;
    op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
    dpar[i] = dpar[i]-2*eps;
    numericGrad(dpar.begin(), df2.begin());
    op_focei.cur++;
    op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
    dpar[i] = dpar[i] + eps;
    for (j = op_focei.npars; j--;){
      if (op_focei.scaleTo > 0){
        parScale2=op_focei.initPar[j] / op_focei.scaleTo;
      }
      H(i, j) = fnscale*(df1[j]-df2[j])/(2*eps*parScale1*parScale2);
    }
  }
  H = 0.25*H + 0.25*H.t();
  // R matrix = Hessian/2
  // https://github.com/cran/nmw/blob/59478fcc91f368bb3bbc23e55d8d1d5d53726a4b/R/CovStep.R
  // H = 0.25*H + 0.25*H.t();
  if (e.exists("R.1")){
    // This is the 2nd attempt
    arma::mat H2 = 0.5*H + 0.5*as<arma::mat>(e["R.1"]);
    arma::mat cholR;
    arma::mat RE;
    bool rpd = cholSE0(cholR, RE, H2, op_focei.cholSEtol);
    if (rpd){
      e["R.pd"] =  rpd;
      e["R.E"] =  wrap(RE);
      e["cholR"] = wrap(cholR);
    } else {
      e["R.pd2"] = false;
      e["R.2"] = H2;
      e["R.E2"] = wrap(RE);
      e["cholR2"] = wrap(cholR);
      e["R.pd"] = cholSE0(cholR, RE, H, op_focei.cholSEtol);
      e["R.E"] =  wrap(RE);
      e["cholR"] = wrap(cholR);
    }
  } else {
    e["R.0"] = H;
    arma::mat cholR;
    arma::mat RE;
    e["R.pd"] =  cholSE0(cholR, RE, H, op_focei.cholSEtol);
    e["R.E"] =  wrap(RE);
    e["cholR"] = wrap(cholR);
  }
}

// Necessary for S-matrix calculation
void foceiS(double *theta, Environment e){
  rx = getRxSolve_();
  op_focei.calcGrad=1;
  rx = getRxSolve_();
  int npars = op_focei.npars;
  int cpar, gid;
  double cur, delta;
  focei_ind *fInd;
  // Do Forward difference if the OBJF for *theta has already been calculated.
  bool doForward=false;
  if (op_focei.derivMethod == 0){
    doForward=true;
    // If the first derivative wasn't calculated, then calculate it.
    for (cpar = npars; cpar--;){
      if (theta[cpar] != op_focei.theta[cpar]){
        doForward=false;
        break;
      }
    }
    if (doForward){
      // Fill in lik0
      for (gid = rx->nsub; gid--;){
        fInd = &(inds_focei[gid]);
        op_focei.likSav[gid] = -2*fInd->lik[0];
      }
    }
  }
  for (cpar = npars; cpar--;){
    delta = (std::fabs(theta[cpar])*op_focei.rEps + op_focei.aEps);
    std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0); // All etas = -42;  Unlikely if normal
    cur = theta[cpar];
    theta[cpar] = cur + delta;
    updateTheta(theta);
    for (gid = rx->nsub; gid--;){
      innerOpt1(gid,2);
      if (doForward){
        fInd = &(inds_focei[gid]);
        fInd->thetaGrad[cpar] = (fInd->lik[2] - op_focei.likSav[gid])/delta;
      }
    }
    if (!doForward){
      std::fill_n(&op_focei.goldEta[0], op_focei.gEtaGTransN, -42.0);
      theta[cpar] = cur - delta;
      updateTheta(theta);
      for (gid = rx->nsub; gid--;){
        innerOpt1(gid,1);
        fInd = &(inds_focei[gid]);
        fInd->thetaGrad[cpar] = (fInd->lik[2] - fInd->lik[1])/(2*delta);
      }
    }
    theta[cpar] = cur;
    op_focei.cur++;
    op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
  }
  op_focei.calcGrad=0;
  // Now calculate S matrix
  arma::mat m1(1, op_focei.npars), S(op_focei.npars, op_focei.npars, fill::zeros), s1(1, op_focei.npars,fill::ones);
  for (gid = rx->nsub; gid--;){
    fInd = &(inds_focei[gid]);
    std::copy(&fInd->thetaGrad[0],&fInd->thetaGrad[0]+op_focei.npars,&m1[0]);
    S = S + m1.t() * m1;
  }
  // S matrix = S/4
  // According to https://github.com/cran/nmw/blob/59478fcc91f368bb3bbc23e55d8d1d5d53726a4b/R/Objs.R
  S=S*0.25; 
  e["S0"] = wrap(S);
  arma::mat cholS;
  arma::mat SE;
  e["S.pd"] =  cholSE0(cholS, SE, S, op_focei.cholSEtol);
  e["S.E"] =  wrap(SE);
  e["cholS"] = wrap(cholS);
}
//' Return the square root of general square matrix A
//'
//' @param m Matrix to take the square root of.
//' @export
//[[Rcpp::export]]
arma::mat sqrtm(arma::mat m){
  arma::cx_mat ret = sqrtmat(m);
  mat im = arma::imag(ret);
  mat re = arma::real(ret);
  if (arma::any(arma::any(im,0))){
    stop("Some components of sqrtm are imaginary.");
  }
  return re;
}

//[[Rcpp::export]]
NumericMatrix foceiCalcCov(Environment e){
  if (op_focei.covMethod){
    op_focei.derivMethodSwitch=0;
    // Check boundaries
    unsigned int j, k;
    double cur;
    bool boundary=false;
    rx = getRxSolve_();
    if (op_focei.boundTol > 0){
      for (k = op_focei.npars; k--;){
        if (op_focei.nbd[k] != 0){
          // bounds
          j=op_focei.fixedTrans[k];
          cur = (op_focei.scaleTo > 0) ? (op_focei.fullTheta[j] / op_focei.initPar[k]  *op_focei.scaleTo) : op_focei.fullTheta[j];
          if (op_focei.nbd[k] == 1){
            // Lower only
            if ((cur-op_focei.lower[k])/cur < op_focei.boundTol){
              boundary = true;
              break;
            }
          } else if (op_focei.nbd[k] == 2){
            // Upper and lower
            if ((cur-op_focei.lower[k])/cur < op_focei.boundTol){
              boundary = true;
              break;
            }
            if ((op_focei.upper[k]-cur)/cur < op_focei.boundTol){
              boundary = true;
              break;
            }
          } else {
            // Upper only
            if ((op_focei.upper[k]-cur)/cur < op_focei.boundTol){
              boundary = true;
              break;
            }
          }
        }
      }
    }
    for (unsigned int j = rx->nsub; j--;){
      focei_ind *fInd = &(inds_focei[j]);
      fInd->doChol=!(op_focei.cholSECov);
    }
    op_focei.resetEtaSize = R_PosInf; // Dont reset ETAs
    NumericVector fullT = e["fullTheta"];
    NumericVector fullT2(op_focei.thetan);
    std::copy(fullT.begin(), fullT.begin()+fullT2.size(), fullT2.begin());
    LogicalVector skipCov(op_focei.thetan+op_focei.omegan);//skipCovN
    if (op_focei.skipCovN == 0){
      std::fill_n(skipCov.begin(), op_focei.thetan, false);
      std::fill_n(skipCov.begin()+op_focei.thetan, skipCov.size() - op_focei.thetan, true);
    } else {
      std::copy(&op_focei.skipCov[0],&op_focei.skipCov[0]+op_focei.skipCovN,skipCov.begin());
      std::fill_n(skipCov.begin()+op_focei.skipCovN,skipCov.size()-op_focei.skipCovN,true);
    }
    e["skipCov"] = skipCov;
  
    foceiSetupTheta_(op_focei.mvi, fullT2, skipCov, op_focei.scaleTo, false);
    op_focei.rEps=op_focei.rEpsC;
    op_focei.aEps=op_focei.aEpsC;
  
    if (op_focei.covMethod && !boundary){
      rx = getRxSolve_();
      op_focei.t0 = clock();
      op_focei.totTick=0;
      op_focei.cur=0;
      op_focei.curTick=0;
      Rprintf("Calculating covariance matrix\n");
    
      // Change options to covariance options
      op_focei.scaleObjective = 0;
      op_focei.derivMethod = op_focei.covDerivMethod;

      arma::mat Rinv;
      if (op_focei.covMethod == 1){
        // Rinv * S *Rinv
        op_focei.totTick = 1 + 5*op_focei.npars;
      } else if (op_focei.covMethod == 2){
        // R matrix
        op_focei.totTick = 1 + 4*op_focei.npars;
      } else if (op_focei.covMethod == 3){
        // S matrix
        op_focei.totTick = 1 + op_focei.npars;
      }
      bool isPd;
      std::string rstr = "r";
      bool checkSandwich = false;
      if (op_focei.covMethod == 1 || op_focei.covMethod == 2){
        // R matrix based covariance
        arma::mat cholR;
        try{
          if (!e.exists("cholR")){
            foceiCalcR(e);
          } else {
            op_focei.cur += op_focei.npars*2;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
          }
          isPd = as<bool>(e["R.pd"]);
          if (!isPd){
            isPd = true;
            arma::vec E = as<arma::vec>(e["R.E"]);
            for (int j = E.size(); j--;){
              if (E[j] > op_focei.cholAccept){
                isPd=false;
                break;
              }
            }
            if (isPd){
	      rstr = "r+";
	      checkSandwich = true;
            }
          }
	  if (!isPd){
	    // Suggted by https://www.tandfonline.com/doi/pdf/10.1198/106186005X78800
	    mat H0 = as<arma::mat>(e["R.0"]);
	    H0 = H0*H0;
	    cx_mat H1;
	    bool success = sqrtmat(H1,H0);
	    if (success){
	      mat im = arma::imag(H1);
	      mat re = arma::real(H1);
	      if (!arma::any(arma::any(im,0))){
		success= chol(H0,re);
		if (success){
		  e["cholR"] = wrap(H0);
		  rstr = "|r|";
		  checkSandwich = true;
		  isPd = true;
		}
	      }
	    }
	  }
          if (!isPd && op_focei.covTryHarder){
            e["R.1"] = wrap(e["R.0"]);
            e["R.E1"] = wrap(e["R.E1"]);
            e["cholR1"] = wrap(e["cholR1"]);
            // Now Try unscaled.
            op_focei.scaleObjective=0;
            foceiSetupTheta_(op_focei.mvi, fullT2, skipCov, 0, false);
            foceiCalcR(e);
            isPd = as<bool>(e["R.pd"]);
            if (!isPd){
              isPd = true;
              arma::vec E = as<arma::vec>(e["R.E"]);
              for (int j = E.size(); j--;){
                if (E[j] > op_focei.cholAccept){
                  isPd=false;
                  break;
                }
              }
              if (isPd){
		rstr = "r+";
		checkSandwich = true;
              }
            }
	    if (!isPd){
	      // Suggted by https://www.tandfonline.com/doi/pdf/10.1198/106186005X78800
	      mat H0 = as<arma::mat>(e["R.2"]); // Second attempt Hessian
	      H0 = H0*H0;
	      cx_mat H1;
	      bool success = sqrtmat(H1,H0);
	      if (success){
		mat im = arma::imag(H1);
		mat re = arma::real(H1);
		if (!arma::any(arma::any(im,0))){
		  success= chol(H0,re);
		  if (success){
		    e["cholR"] = wrap(H0);
		    rstr = "|r|";
		    checkSandwich = true;
		    isPd = true;
		  }
		}
	      }
	    }
          } else {
            op_focei.cur += op_focei.npars*2;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
          }
          if (!isPd){
            warning("R matrix non-positive definite");
            e["R"] = wrap(e["R.0"]);
            op_focei.covMethod = 3;
            op_focei.cur += op_focei.npars*2;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
          } else {
            cholR = as<arma::mat>(e["cholR"]);
            e["R"] = wrap(trans(cholR) * cholR);
            if (!e.exists("Rinv")){
              bool success  = inv(Rinv, trimatu(cholR));
              if (!success){
                warning("Hessian (R) matrix seems singular; Using pseudo-inverse");
                Rinv = pinv(trimatu(cholR));
		checkSandwich = true;
              }
              Rinv = Rinv * Rinv.t();
              e["Rinv"] = wrap(Rinv);
            } else {
              Rinv = as<arma::mat>(e["Rinv"]);
            }
            op_focei.cur++;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
            if (!e.exists("covR")){
              e["covR"] = wrap(2*Rinv);
            }
            if (op_focei.covMethod == 2){
              e["cov"] = as<NumericMatrix>(e["covR"]);
            }
          }
        } catch (...){
          Rprintf("\rR matrix calculation failed; Switch to S-matrix covariance.\n");
          op_focei.covMethod = 3;
          op_focei.cur += op_focei.npars*2;
          op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
        }
      }
      arma::mat cholS;
      int origCov = op_focei.covMethod;
      std::string sstr="s";
      if (op_focei.covMethod == 1 || op_focei.covMethod == 3){
        try{
          arma::vec dpar(op_focei.npars);
          unsigned int j, k;
          for (k = op_focei.npars; k--;){
            j=op_focei.fixedTrans[k];
            dpar[k] = op_focei.fullTheta[j];
          }      
          if (!e.exists("cholS")){
            foceiSetupTheta_(op_focei.mvi, fullT2, skipCov, 0, false);
            foceiS(&dpar[0], e);
          } else {
            op_focei.cur += op_focei.npars;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
          }
          isPd = as<bool>(e["S.pd"]);
          if (!isPd){
            isPd=true;
            arma::vec E = as<arma::vec>(e["S.E"]);
            for (int j = E.size(); j--;){
              if (E[j] > op_focei.cholAccept){
                isPd=false;
                break;
              }
            }
            if (isPd){
	      sstr="s+";
	      checkSandwich = true;
            }
          }
	  if (!isPd){
	    // Suggted by https://www.tandfonline.com/doi/pdf/10.1198/106186005X78800
	    mat H0 = as<arma::mat>(e["S0"]);
	    H0 = H0*H0;
	    cx_mat H1;
	    bool success = sqrtmat(H1,H0);
	    if (success){
	      mat im = arma::imag(H1);
	      mat re = arma::real(H1);
	      if (!arma::any(arma::any(im,0))){
		success= chol(H0,re);
		if (success){
		  e["cholS"] = wrap(H0);
		  sstr = "|s|";
		  checkSandwich = true;
		  isPd = true;
		}
	      }
	    }
	  }
          if (!isPd){
            warning("S matrix non-positive definite");
            if (op_focei.covMethod == 1){
	      e["cov"] = as<NumericMatrix>(e["covR"]);
              op_focei.covMethod = 2;
            } else {
              Rprintf("Cannot calculate covariance.\n");
            }
            op_focei.cur += op_focei.npars*2;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
          } else {
            cholS = as<arma::mat>(e["cholS"]);
            arma::mat S;
            if (e.exists("S")){
              S = as<arma::mat>(e["S"]);
            } else {
              S = trans(cholS) * cholS;
              e["S"] = wrap(S);
            }
            if (op_focei.covMethod == 1){
              e["covRS"] = Rinv * S *Rinv;
	      arma::mat covRS = as<arma::mat>(e["covRS"]);
	      if (checkSandwich){
		mat Sinv;
		bool success;
		success = inv(Sinv, trimatu(cholS));
		if (!success){
		  warning("Hessian (S) matrix seems singular; Using pseudo-inverse");
		  Sinv = pinv(trimatu(cholS));
		}
		Sinv = Sinv * Sinv.t();
		e["covS"]= 4 * Sinv;
		// Now check sandwich matrix against R and S methods
		double covRSd= sum(covRS.diag());
		arma::mat covR = as<arma::mat>(e["covR"]);
		double covRd= sum(covR.diag());
		arma::mat covS = as<arma::mat>(e["covS"]);
		double  covSd= sum(covS.diag());
		if (covRSd > covRd){
		  // SE(RS) > SE(R)
		  if (covRd > covSd){
		    // SE(R) > SE(S)
		    e["cov"] = covS;
		    op_focei.covMethod=3;
		  } else {
		    e["cov"] = covR;
		    op_focei.covMethod=2;
		  }
		} else if (covRSd > covSd){
		  e["cov"] = covS;
		  op_focei.covMethod=3;
		} else {
		  e["cov"] = covRS;
		}
	      } else {
		e["cov"] = covRS;
	      }
            } else {
              mat Sinv;
              bool success;
              success = inv(Sinv, trimatu(cholS));
              if (!success){
                warning("Hessian (S) matrix seems singular; Using pseudo-inverse.");
                Sinv = pinv(trimatu(cholS));
              }
              Sinv = Sinv * Sinv.t();
              op_focei.cur++;
              op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
              e["cov"]= 4 * Sinv;
            }
          }
        } catch (...){
          if (op_focei.covMethod == 1){
            Rprintf("\rS matrix calculation failed; Switch to R-matrix covariance.\n");
            e["cov"] = wrap(e["covR"]);
            op_focei.covMethod = 2;
          } else {
            op_focei.covMethod=0;
            Rprintf("\rCould not calculate covariance matrix.\n");
            op_focei.cur++;
            op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
          }
        }
      }
      op_focei.cur=op_focei.totTick;
      op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
      if (op_focei.covMethod==0){
        warning("Covariance step failed");
	e["covMethod"] = CharacterVector::create("failed");
        NumericMatrix ret;
        return ret;
      } else {
        if (op_focei.covMethod == 1){
	  bool doWarn=false;
	  if (rstr == "|r|"){
	    warning("R matrix non-positive definite but corrected by R = sqrtm(R%%*%%R)");
	    doWarn=true;
	  } else if (rstr == "r+"){
	    warning("R matrix non-positive definite but corrected (because of cholAccept)");
	    doWarn=true;
	  }
	  if (sstr == "|s|"){
	    warning("S matrix non-positive definite but corrected by S = sqrtm(S%%*%%S)");
	    doWarn=true;
	  } else if (sstr == "s+"){
	    warning("S matrix non-positive definite but corrected (because of cholAccept)");
	    doWarn=true;
	  }
	  if (doWarn){
	    warning("Since sandwich matrix is corrected, you may compare to $covR or $covS if you wish.");
	  }
	  rstr =  rstr + "," + sstr;
          e["covMethod"] = wrap(rstr);
        } else if (op_focei.covMethod == 2){
	  if (rstr == "|r|"){
	    warning("R matrix non-positive definite but corrected by R = sqrtm(R%%*%%R)");
	  } else if (rstr == "r+"){
	    warning("R matrix non-positive definite but corrected (because of cholAccept)");
	  }
          e["covMethod"] = wrap(rstr);
          if (origCov != 2){
	    if (checkSandwich){
	      warning("Using R matrix to calculate covariance, can check sandwich or S matrix with $covRS and $covS");
	    } else {
	      warning("Using R matrix to calculate covariance");
	    }
          }
        } else if (op_focei.covMethod == 3){
          e["covMethod"] = wrap(sstr);
          if (origCov != 2){
	    if (checkSandwich){
	      warning("Using S matrix to calculate covariance, can check sandwich or R matrix with $covRS and $covR");
	    } else {
	      warning("Using S matrix to calculate covariance");
	    }
          }
        }
        return as<NumericMatrix>(e["cov"]);
      }
    } else {
      if (boundary){
        warning("Parameter estimate near boundary; covariance not caculated. Use getVarCov to calculate anyway.");
      }
      op_focei.cur=op_focei.totTick;
      op_focei.curTick = par_progress(op_focei.cur, op_focei.totTick, op_focei.curTick, rx->op->cores, op_focei.t0, 0);
      NumericMatrix ret;
      return ret;
    }
  }
  NumericMatrix ret;
  return ret;
}

LogicalVector rxSolveFree();

void foceiFinalizeTables(Environment e){
  CharacterVector thetaNames=as<CharacterVector>(e["thetaNames"]);
  arma::mat cov;
  bool covExists = e.exists("cov");
  if (covExists){
    cov= as<arma::mat>(e["cov"]);
  }
  LogicalVector skipCov = e["skipCov"];

  if (covExists && op_focei.eigen){
    arma::vec eigval;
    arma::mat eigvec;

    
    eig_sym(eigval, eigvec, cov);
    e["eigen"] = eigval;
    e["eigenVec"] = eigvec;
    unsigned int k=0;
    if (eigval.size() > 0){
      double mx=std::fabs(eigval[0]), mn, cur;
      mn=mx;
      for (k = eigval.size(); k--;){
        cur = std::fabs(eigval[k]);
        if (cur > mx){
          mx=cur;
        }
        if (cur < mn){
          mn=cur;
        }
      }
      e["conditionNumber"] = mx/mn;
    } else {
      e["conditionNumber"] = NA_REAL;
    }
  }
  arma::vec se1;
  if (covExists){
    se1 = sqrt(cov.diag());
  }
  DataFrame thetaDf = as<DataFrame>(e["theta"]);
  arma::vec theta = as<arma::vec>(thetaDf["theta"]);
  NumericVector se(theta.size());
  NumericVector cv(theta.size());
  std::fill_n(&se[0], theta.size(), NA_REAL);
  std::fill_n(&cv[0], theta.size(), NA_REAL);
  int j=0;
  if (covExists){
    for (unsigned int k = 0; k < se.size(); k++){
      if (k >= skipCov.size()) break;
      if (!skipCov[k]){
        se[k] = se1[j++];
        cv[k] = std::fabs(se[k]/theta[k])*100;
      }
    }
  }
  e["se"] = se;
  List popDf = List::create(_["Estimate"]=thetaDf["theta"], _["SE"]=se, 
			      _["%RSE"]=cv);
  popDf.attr("class") = "data.frame";
  popDf.attr("row.names") = IntegerVector::create(NA_INTEGER,-theta.size());
  e["popDf"] = popDf;
  
  e["fixef"]=thetaDf["theta"];
  List etas = e["etaObf"];
  IntegerVector idx = seq_len(etas.length())-1;
  etas = etas[idx != etas.length()-1];
  e["ranef"]=etas;

  // Now put names on the objects
  ////////////////////////////////////////////////////////////////////////////////
  // Eta Names
  NumericMatrix tmpNM;
  CharacterVector etaNames=as<CharacterVector>(e["etaNames"]);
  tmpNM = getOmega();
  tmpNM.attr("dimnames") = List::create(etaNames, etaNames);
  e["omega"] = tmpNM;

  tmpNM = as<NumericMatrix>(e["omegaR"]);
  tmpNM.attr("dimnames") = List::create(etaNames, etaNames);
  e["omegaR"] = tmpNM;

  List tmpL  = as<List>(e["ranef"]);
  List tmpL2 = as<List>(e["etaObf"]);
  CharacterVector tmpN  = tmpL.attr("names");
  CharacterVector tmpN2 = tmpL2.attr("names");
  int i;
  for (i = 0; i < etaNames.size(); i++){
    if (i + 1 <  tmpN.size())  tmpN[i+1] = etaNames[i];
    if (i + 1 < tmpN2.size()) tmpN2[i+1] = etaNames[i];
  }
  ////////////////////////////////////////////////////////////////////////////////
  tmpL.attr("names") = tmpN;
  tmpL2.attr("names") = tmpN2;
  e["ranef"] = tmpL;
  e["etaObf"] = tmpL2;


  ////////////////////////////////////////////////////////////////////////////////
  // Theta names
  //
  // omegaR
  arma::mat omega = as<arma::mat>(e["omega"]);
  arma::mat D(omega.n_rows,omega.n_rows,fill::zeros);
  arma::mat cor(omega.n_rows,omega.n_rows);
  D.diag() = (sqrt(omega.diag()));
  arma::vec sd=D.diag();
  D = inv_sympd(D);
  cor = D * omega * D;
  cor.diag()= sd;
  
  tmpL = as<List>(e["theta"]);
  tmpL.attr("row.names") = thetaNames;
  e["theta"] = tmpL;

  tmpL=e["popDf"];
  // Add a few columns
  
  NumericVector Estimate = tmpL["Estimate"];
  NumericVector SE = tmpL["SE"];
  NumericVector RSE = tmpL["%RSE"];
  NumericVector EstBT(Estimate.size());
  NumericVector EstLower(Estimate.size());
  NumericVector EstUpper(Estimate.size());

  CharacterVector EstS(Estimate.size());
  CharacterVector SeS(Estimate.size());
  CharacterVector rseS(Estimate.size());
  CharacterVector btCi(Estimate.size());
  // LogicalVector EstBT(Estimate.size());
  // Rf_pt(stat[7],(double)n1,1,0)
  // FIXME figure out log thetas outside of foceisetup.
  IntegerVector logTheta;
  if (e.exists("logThetas")){
    logTheta =  as<IntegerVector>(e["logThetas"]);
  } else if (e.exists("model")){
    List model = e["model"];
    logTheta =  as<IntegerVector>(model["log.thetas"]);
  } 
  j = logTheta.size()-1;
  double qn= Rf_qnorm5(1.0-(1-op_focei.ci)/2, 0.0, 1.0, 1, 0);
  std::string cur;
  char buff[100];
  LogicalVector thetaFixed =thetaDf["fixed"];
  
  for (i = Estimate.size(); i--;){
    snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, Estimate[i]);
    EstS[i]=buff;
    if (logTheta.size() > 0 && j >= 0 && logTheta[j]-1==i){
      EstBT[i] = exp(Estimate[i]);
      snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, EstBT[i]);
      cur = buff;
      if (ISNA(SE[i])){
        EstLower[i] = NA_REAL;
        EstUpper[i] = NA_REAL;
	if (thetaFixed[i]){
          SeS[i]  = "FIXED";
          rseS[i] = "FIXED";
	} else {
          SeS[i] = "";
          rseS[i]="";
        }
      } else {
        EstLower[i] = exp(Estimate[i]-SE[i]*qn);
        EstUpper[i] = exp(Estimate[i]+SE[i]*qn);
	snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, SE[i]);
        SeS[i]=buff;
	snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, RSE[i]);
        rseS[i]=buff;
        snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, EstLower[i]);
	cur = cur + " (" + buff + ", ";
        snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, EstUpper[i]);  
	cur = cur + buff + ")";
      }
      btCi[i] = cur;
      j--;
    } else {
      EstBT[i]= Estimate[i];
      snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, Estimate[i]);
      EstS[i]=buff;
      snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, EstBT[i]);
      cur = buff;
      if (ISNA(SE[i])){
        EstLower[i] = NA_REAL;
        EstUpper[i] = NA_REAL;
        if (thetaFixed[i]){
          SeS[i]  = "FIXED";
          rseS[i] = "FIXED";
        } else {
          SeS[i] = "";
          rseS[i]="";
        }
      } else {
        EstLower[i] = Estimate[i]-SE[i]*qn;
        EstUpper[i] = Estimate[i]+SE[i]*qn;
	snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, SE[i]);
        SeS[i]=buff;
	snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, RSE[i]);
        rseS[i]=buff;
	snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, EstLower[i]);
	cur = cur + " (" + buff + ", ";
        snprintf(buff, sizeof(buff), "%.*g", (int)op_focei.sigdig, EstUpper[i]);
	cur = cur + buff + ")";
      }
      btCi[i] = cur;
    }
  }
  tmpL["Back-transformed"] = EstBT;
  tmpL["CI Lower"] = EstLower;
  tmpL["CI Upper"] = EstUpper;
  tmpL.attr("row.names") = thetaNames;
  tmpL.attr("class") = "data.frame";
  e["popDf"]=tmpL;
  std::string bt = "Back-transformed(" + std::to_string((int)(op_focei.ci*100)) + "%CI)";
  
  List popDfSig;
  if (e.exists("cov")){
    popDfSig = List::create(_["Est."]=EstS, 
                   _["SE"]=SeS, 
                   _["%RSE"]=rseS,
                   _[bt]=btCi);
  } else {
    popDfSig = List::create(_["Est."]=EstS, 
			    _["Back-transformed"] = btCi);
  }
  
  popDfSig.attr("row.names") = thetaNames;
  popDfSig.attr("class") = "data.frame";
  e["popDfSig"]=popDfSig;
  
  NumericVector tmpNV = e["fixef"];
  tmpNV.names() = thetaNames;
  e["fixef"] = tmpNV;

  
  tmpNV = e["se"];
  tmpNV.names() = thetaNames;
  e["se"] = tmpNV;

  // Now get covariance names
  if (e.exists("cov")){
    tmpNM = as<NumericMatrix>(e["cov"]);
    CharacterVector thetaCovN(tmpNM.nrow());
    LogicalVector skipCov = e["skipCov"];
    unsigned int j=0;
    for (unsigned int k = 0; k < thetaNames.size(); k++){
      if (k >= skipCov.size()) break;
      if (j >= thetaCovN.size()) break;
      if (!skipCov[k]){
        thetaCovN[j++] = thetaNames[k];
      }
    }
    List thetaDim = List::create(thetaCovN,thetaCovN);
    tmpNM.attr("dimnames") = thetaDim;
    e["cov"]=tmpNM;
    if (e.exists("Rinv")){
      tmpNM = as<NumericMatrix>(e["Rinv"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["Rinv"]=tmpNM;
    }
    if (e.exists("Sinv")){
      tmpNM = as<NumericMatrix>(e["Sinv"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["Sinv"]=tmpNM;
    }
    if (e.exists("S")){
      tmpNM = as<NumericMatrix>(e["S"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["S"]=tmpNM;
    }
    if (e.exists("R")){
      tmpNM = as<NumericMatrix>(e["R"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["R"]=tmpNM;
    }
    if (e.exists("covR")){
      tmpNM = as<NumericMatrix>(e["covR"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["covR"]=tmpNM;
    }
    if (e.exists("covRS")){
      tmpNM = as<NumericMatrix>(e["covRS"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["covRS"]=tmpNM;
    }
    if (e.exists("covS")){
      tmpNM = as<NumericMatrix>(e["covS"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["covS"]=tmpNM;
    }
    if (e.exists("R.1")){
      tmpNM = as<NumericMatrix>(e["R.1"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["R.1"]=tmpNM;
    }
    if (e.exists("R.2")){
      tmpNM = as<NumericMatrix>(e["R.2"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["R.2"]=tmpNM;
    }
    if (e.exists("cholR")){
      tmpNM = as<NumericMatrix>(e["cholR"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["cholR"]=tmpNM;
    }
    if (e.exists("cholR2")){
      tmpNM = as<NumericMatrix>(e["cholR2"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["cholR2"]=tmpNM;
    }
    if (e.exists("cholS")){
      tmpNM = as<NumericMatrix>(e["cholS"]);
      tmpNM.attr("dimnames") = thetaDim;
      e["cholS"]=tmpNM;
    }
  }
  List objDf;
  if (e.exists("conditionNumber")){
    objDf = List::create(_["OBJF"] = as<double>(e["objective"]), _["AIC"]=as<double>(e["AIC"]), 
                         _["BIC"] = as<double>(e["BIC"]), _["Log-likelihood"]=-as<double>(e["objective"])/2, 
                         _["Condition Number"]=as<double>(e["conditionNumber"]));
  } else {
    objDf = List::create(_["OBJF"] = as<double>(e["objective"]), _["AIC"]=as<double>(e["AIC"]), 
                         _["BIC"] = as<double>(e["BIC"]), _["Log-likelihood"]=-as<double>(e["objective"])/2);
  }
  if (op_focei.fo){
    objDf.attr("row.names") = CharacterVector::create("FO");
  } else if (op_focei.interaction){
    objDf.attr("row.names") = CharacterVector::create("FOCEi");
  } else {
    objDf.attr("row.names") = CharacterVector::create("FOCE");
  }
  objDf.attr("class") = "data.frame";
  e["objDf"]=objDf;
  if (!e.exists("method")){
    if (op_focei.fo){
      e["method"] = "FO";
    } else {
      e["method"] = "FOCE";
    }
  }
  if (!e.exists("extra")){
    if (op_focei.fo){
      e["extra"] = "";
      e["skipTable"] = LogicalVector::create(true);
    } else if (op_focei.interaction){
      if(op_focei.useColor){
        e["extra"] = "\033[31;1mi\033[0m";
      } else {
        e["extra"] = "i";
      }
    } else {
      e["extra"] = "";
    }
  }
  rxSolveFree();
  e.attr("class") = "nlmixrFitCore";
}


////////////////////////////////////////////////////////////////////////////////
// FOCEi fit

//' Fit/Evaulate FOCEi 
//'
//' This shouldn't be called directly.
//'
//' @param e Enviornment 
//'
//' @keywords internal
//' @export
//[[Rcpp::export]]
Environment foceiFitCpp_(Environment e){
  clock_t t0 = clock();
  List model = e["model"];
  bool doPredOnly = false;
  if (model.containsElementNamed("inner")){
    RObject inner = model["inner"];
    if (rxIs(inner, "RxODE")){
      foceiSetup_(inner, as<RObject>(e["dataSav"]), 
		  as<NumericVector>(e["thetaIni"]), e["thetaFixed"], e["skipCov"],
		  as<RObject>(e["rxInv"]), e["lower"], e["upper"], e["etaMat"],
		  e["control"]);
    } else if (model.containsElementNamed("pred.only")){
      inner = model["pred.only"];
      if (rxIs(inner, "RxODE")){
	doPredOnly = true;
	foceiSetup_(inner, as<RObject>(e["dataSav"]), 
		    as<NumericVector>(e["thetaIni"]), e["thetaFixed"], e["skipCov"],
		    as<RObject>(e["rxInv"]), e["lower"], e["upper"], e["etaMat"],
		    e["control"]);
      } else {
	stop("Cannot run this function.");
      }
    } else {
      doPredOnly=true;
      foceiSetupTrans_(as<CharacterVector>(e[".params"]));
      foceiThetaN(as<unsigned int>(e[".thetan"]));
      foceiSetup_(R_NilValue, as<RObject>(e["dataSav"]), 
                  as<NumericVector>(e["thetaIni"]), e["thetaFixed"], e["skipCov"],
                  as<RObject>(e["rxInv"]), e["lower"], e["upper"], e["etaMat"],
                  e["control"]);
    }
  } else {
    doPredOnly=true;
    foceiSetupTrans_(as<CharacterVector>(e[".params"]));
    foceiThetaN(as<unsigned int>(e[".thetan"]));
    foceiSetup_(R_NilValue, as<RObject>(e["dataSav"]), 
                as<NumericVector>(e["thetaIni"]), e["thetaFixed"], e["skipCov"],
                as<RObject>(e["rxInv"]), e["lower"], e["upper"], e["etaMat"],
                e["control"]);
  }
  if (e.exists("setupTime")){
    e["setupTime"] = as<double>(e["setupTime"])+(((double)(clock() - t0))/CLOCKS_PER_SEC);
  } else {
    e["setupTime"] = (((double)(clock() - t0))/CLOCKS_PER_SEC);
  }
  t0 = clock();
  CharacterVector thetaNames=as<CharacterVector>(e["thetaNames"]);
  IntegerVector logTheta;
  if (e.exists("logThetas")){
    logTheta =  as<IntegerVector>(e["logThetas"]);
  } else if (e.exists("model")){
    List model = e["model"];
    logTheta =  as<IntegerVector>(model["log.thetas"]);
  } 
  int j;
  // Setup which paramteres are transformed
  for (unsigned int k = op_focei.npars; k--;){
    j=op_focei.fixedTrans[k];
    op_focei.xPar[k] = 0;
    for (unsigned int m=logTheta.size(); m--;){
      if (logTheta[m]-1 == j){
	op_focei.xPar[k] = 1;
	break;
      }
    }
  }
  std::string tmpS;
  if (op_focei.maxOuterIterations > 0){
    if (op_focei.useColor)
      Rprintf("\033[1mKey:\033[0m ");
    else 
      Rprintf("Key: ");
    if (op_focei.scaleTo > 0){
      Rprintf("U: Unscaled Parameters; ");
    }
    Rprintf("X: Back-transformed parameters; ");
    Rprintf("G: Gradient\n");
    foceiPrintLine(min2(op_focei.npars, op_focei.printNcol));
    Rprintf("|    #| Objective Fun |");
    int j,  i=0, finalize=0;
    for (i = 0; i < op_focei.npars; i++){
      j=op_focei.fixedTrans[i];
      if (j < thetaNames.size()){
	tmpS = thetaNames[j];
	Rprintf("%#10s |", tmpS.c_str());
      } else {
	Rprintf("           |");
      } 
      if ((i + 1) != op_focei.npars && (i + 1) % op_focei.printNcol == 0){
	if (op_focei.useColor && op_focei.printNcol + i  >= op_focei.npars){
	  Rprintf("\n\033[4m|.....................|");
	} else {
	  Rprintf("\n|.....................|");
	}
	finalize=1;
      }
    }
    if (finalize){
      while(true){
	if ((i++) % op_focei.printNcol == 0){
	  if (op_focei.useColor) Rprintf("\033[0m");
	  Rprintf("\n");
	  break;
	} else {
	  Rprintf("...........|");
	}
      }
    } else {
      Rprintf("\n");
    }
    if (!op_focei.useColor){
      foceiPrintLine(min2(op_focei.npars, op_focei.printNcol));
    }
  }
  if (doPredOnly){
    if (e.exists("objective")){
      nlmixrEnvSetup(e, as<double>(e["objective"]));
    } else {
      stop("Not setup right.");
    }
  } else {
    foceiOuter(e);
  }
  e["optimTime"] = (((double)(clock() - t0))/CLOCKS_PER_SEC);
  t0 = clock();
  foceiCalcCov(e);
    
  e["covTime"] = (((double)(clock() - t0))/CLOCKS_PER_SEC);
  List timeDf = List::create(_["setup"]=as<double>(e["setupTime"]),
			     _["optimize"]=as<double>(e["optimTime"]),
			     _["covariance"]=as<double>(e["covTime"]));
  timeDf.attr("class") = "data.frame";
  timeDf.attr("row.names") = "";
  e["time"] = timeDf;
  foceiFinalizeTables(e);
  if (op_focei.maxOuterIterations){
    Rprintf("done\n");
  }
  return e;
}

//[[Rcpp::export]]
NumericVector coxBox_(NumericVector x = 1, double lambda=1, int yj = 0){
  NumericVector ret(x.size());
  for (unsigned int i = x.size(); i--;){
    ret[i] = powerD(x[i], lambda, yj);
  }
  return ret;
}
