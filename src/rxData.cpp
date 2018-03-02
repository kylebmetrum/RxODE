// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>
#include <Rmath.h>
#include <thread>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <climits>
extern "C" {
#include "solve.h"
}

using namespace Rcpp;
using namespace arma;

int rxcEvid = -1;
int rxcTime = -1;
int rxcAmt  = -1;
int rxcId   = -1;
int rxcDv   = -1;
int rxcLen  = -1;
bool resetCache = true;
bool rxHasEventNames(CharacterVector &nm){
  int len = nm.size();
  bool reset  = resetCache;
  if (reset || len != rxcLen){
    reset   = resetCache;
    rxcEvid = -1;
    rxcTime = -1;
    rxcAmt  = -1;
    rxcId   = -1;
    rxcDv   = -1;
    rxcLen  = len;
    for (int i = 0; i < len; i++){
      if (as<std::string>(nm[i]) == "evid" || as<std::string>(nm[i]) == "EVID" || as<std::string>(nm[i]) == "Evid"){
        rxcEvid = i;
      } else if (as<std::string>(nm[i]) == "time" || as<std::string>(nm[i]) == "TIME" || as<std::string>(nm[i]) == "Time"){
        rxcTime = i;
      } else if (as<std::string>(nm[i]) == "amt" || as<std::string>(nm[i]) == "AMT" || as<std::string>(nm[i]) == "Amt"){
        rxcAmt = i;
      } else if (as<std::string>(nm[i]) == "id" || as<std::string>(nm[i]) == "ID" || as<std::string>(nm[i]) == "Id"){
        rxcId = i;
      } else if (as<std::string>(nm[i]) == "dv" || as<std::string>(nm[i]) == "DV" || as<std::string>(nm[i]) == "Dv"){
        rxcDv = i;
      }
    }
  }
  resetCache = true;
  if (rxcEvid >= 0 && rxcTime >= 0 && rxcAmt >= 0){
    return true;
  } else {
    return false;
  }
}

//' Check the type of an object using Rcpp
//'
//' @param obj Object to check
//' @param cls Type of class.  Only s3 classes and primitive classes are checked.
//'    For matrix types they are distinguished as \code{numeric.matrix}, \code{integer.matrix},
//'    \code{logical.matrix}, and \code{character.matrix} as well as the traditional \code{matrix}
//'    class. Additionally checks for \code{event.data.frame} which is an \code{data.frame} object
//'    with \code{time},  \code{evid} and \code{amt}. (UPPER, lower or Title cases accepted)
//'
//' @return A boolean indicating if the object is a member of the class.
//' @keywords internal
//' @author Matthew L. Fidler
//' @export
// [[Rcpp::export]]
bool rxIs(const RObject &obj, std::string cls){
  if (cls == "rx.event"){
    return (rxIs(obj, "EventTable") || rxIs(obj, "event.data.frame") || rxIs(obj, "event.matrix"));
  } else if (cls == "event.data.frame"){
    if (rxIs(obj, "data.frame")){
      CharacterVector cv =as<CharacterVector>((as<DataFrame>(obj)).names());
      return rxHasEventNames(cv);
    } else {
      return false;
    }
  } else if (cls == "event.matrix"){
    if (rxIs(obj,"numeric.matrix") && obj.hasAttribute("dimnames")){
      List dn = as<List>(obj.attr("dimnames"));
      if (dn.size() == 2){
	CharacterVector cv = as<CharacterVector>(dn[1]);
        return rxHasEventNames(cv);
      } else {
	return false; // nocov
      }
    } else {
      return false;
    }
  } else if (obj.isObject()){
    CharacterVector classattr = obj.attr("class");
    for (int i = 0; i < classattr.size(); i++){
      if (as<std::string>(classattr[i]) == cls){
	if (cls == "rxSolve"){
	  Environment e = as<Environment>(classattr.attr(".RxODE.env"));
	  List lobj = List(obj);
	  CharacterVector cls2= CharacterVector::create("data.frame");
	  if (as<int>(e["check.ncol"]) != lobj.size()){
	    lobj.attr("class") = cls2;
	    return false;
	  }
	  int nrow = (as<NumericVector>(lobj[0])).size();
	  if (as<int>(e["check.nrow"]) != nrow){
	    lobj.attr("class") = cls2;
            return false;
          }
	  CharacterVector cn = CharacterVector(e["check.names"]);
	  if (cn.size() != lobj.size()){
	    lobj.attr("class") = cls2;
	    return false;
	  }
	  CharacterVector cn2 = CharacterVector(lobj.names());
	  for (int j = 0; j < cn.size();j++){
	    if (cn[j] != cn2[j]){
	      lobj.attr("class") = cls2;
	      return false;
	    }
	  }
	  return true;
        } else {
	  return true;
        }
      }
    }
  } else {
    int type = obj.sexp_type();
    bool hasDim = obj.hasAttribute("dim");
    if (type == REALSXP){
      if (hasDim){
	if (cls == "numeric.matrix" || cls == "matrix"){
	  return true;
	} else {
	  return false;
	}
      } else {
	if (cls == "numeric")
          return true;
        else 
          return false;
      }
    }
    if (type == INTSXP){
      if (hasDim){
	if (cls == "integer.matrix" || cls == "matrix"){
          return true;
        } else {
          return false;
        }
      } else {
	if (cls == "integer")
          return true;
        else
          return false;
      }
    }
    if (type == LGLSXP){
      if (hasDim){
        if (cls == "logical.matrix" || cls == "matrix"){
          return true;
        } else {
          return false;
        }
      } else {
	if (cls == "logical")
          return true;
        else
          return false;
      }
    }
    if (type == STRSXP){
      if (hasDim){
	if (cls == "character.matrix" || cls == "matrix"){
          return true;
        } else {
          return false;
        }
      } else {
	if (cls == "character")
          return true;
        else
          return false;
      }
    }
    if (type == VECSXP){
      if (cls == "list"){
        return true;
      } else {
        return false;
      }
    }
    if (type == ENVSXP){
      if (cls == "environment"){
	return true;
      } else {
	return false;
      }
    }
    if (type == EXTPTRSXP){
      if (cls == "externalptr" || cls == "refObject"){
	return true;
      } else {
	return false;
      }
    }
  }
  return false;
}

extern "C" int rxIsC(SEXP obj, const char *cls){
  std::string str(cls);
  if (rxIs(as<RObject>(obj),cls)){
    return 1;
  } else {
    return 0;
  }
}

RObject rxSimSigma(const RObject &sigma,
		   const RObject &df,
		   int ncores,
		   const bool &isChol,
		   int nObs,
		   const bool checkNames = true){
  if (rxIs(sigma, "numeric.matrix")){
    // FIXME more distributions
    NumericMatrix sigmaM(sigma);
    if (sigmaM.nrow() != sigmaM.ncol()){
      stop("The matrix must be a square matrix.");
    }
    List dimnames;
    StringVector simNames;
    bool addNames = false;
    if (checkNames){
      if (!sigmaM.hasAttribute("dimnames")){
        stop("The matrix must have named dimensions.");
      }
      dimnames = sigmaM.attr("dimnames");
      simNames = as<StringVector>(dimnames[1]);
      addNames = true;
    } else if (sigmaM.hasAttribute("dimnames")){
      dimnames = sigmaM.attr("dimnames");
      simNames = as<StringVector>(dimnames[1]);
      addNames = true;
    }
    Environment base("package:base");
    Function loadNamespace=base["loadNamespace"];
    Environment mvnfast = loadNamespace("mvnfast");
    NumericMatrix simMat(nObs,sigmaM.ncol());
    NumericVector m(sigmaM.ncol());
    // I'm unsure if this for loop is necessary.
    // for (int i = 0; i < m.size(); i++){
    //   m[i] = 0;
    // }
    // Ncores = 1?  Should it be parallelized when it can be...?
    // Note that if so, the number of cores also affects the output.
    if (df.isNULL()){
      Function rmvn = as<Function>(mvnfast["rmvn"]);
      rmvn(_["n"]=nObs, _["mu"]=m, _["sigma"]=sigmaM, _["ncores"]=ncores, _["isChol"]=isChol, _["A"] = simMat); // simMat is updated with the random deviates
    } else {
      double df2 = as<double>(df);
      if (R_FINITE(df2)){
	Function rmvt = as<Function>(mvnfast["rmvt"]);
        rmvt(_["n"]=nObs, _["mu"]=m, _["sigma"]=sigmaM, _["df"] = df, _["ncores"]=ncores, _["isChol"]=isChol, _["A"] = simMat);
      } else {
	Function rmvn = as<Function>(mvnfast["rmvn"]);
        rmvn(_["n"]=nObs, _["mu"]=m, _["sigma"]=sigmaM, _["ncores"]=ncores, _["isChol"]=isChol, _["A"] = simMat);
      }
    }
    if (addNames){
      simMat.attr("dimnames") = List::create(R_NilValue, simNames);
    }
    return wrap(simMat);
  } else {
    return R_NilValue;
  }
}


bool foundEnv = false;
Environment _rxModels;
void getRxModels(){
  if (!foundEnv){ // minimize R call
    Environment RxODE("package:RxODE");
    Function f = as<Function>(RxODE["rxModels_"]);
    _rxModels = f();
    foundEnv = true;
  }
}

// [[Rcpp::export]]
List rxDataSetup(const RObject &ro,
		 const RObject &covNames = R_NilValue,
		 const RObject &sigma = R_NilValue,
		 const RObject &df = R_NilValue,
		 const int &ncoresRV = 1,
		 const bool &isChol = false,
                 const int &nDisplayProgress = 10000,
		 const StringVector &amountUnits = NA_STRING,
		 const StringVector &timeUnits = "hours"){
  // Purpose: get positions of each id and the length of each id's observations
  // Separate out dose vectors and observation vectors
  if (rxIs(ro,"EventTable")){
    List et = List(ro);
    Function f = et["get.EventTable"];
    DataFrame dataf = f();
    f = et["get.units"];
    RObject unitsRO = f();
    CharacterVector units;
    int i, n;
    if (rxIs(unitsRO, "character")){
      units = as<CharacterVector>(unitsRO);
      n=units.size();
      for (i =0; i<n; i++){
	if (units[i] == "NA"){
	  units[i] = NA_STRING;
	}
      }
    } else {
      units = CharacterVector::create(_["dosing"]=NA_STRING,
				      _["time"]=NA_STRING);
    }
    // {
    //   units = StringVector(unitsRO);
    //   if (units[0] == "NA"){
    // 	units[0] = NA_STRING;
    //   }
    //   if (units[1] == "NA"){
    // 	units[1] = NA_STRING;
    //   }
    // } else {
    //   // Otherwise this is likely 2 NAs.
    //   units[0] = NA_STRING;
    //   units[1] = NA_STRING;
    //   StringVector units2(2);
    //   units2[0] = "dosing";
    //   units2[1] = "time";
    //   units.names() = units;
    // }
    CharacterVector amt = (units["dosing"] == NA_STRING) ? StringVector::create(NA_STRING) : as<StringVector>(units["dosing"]);
    CharacterVector time = (units["time"] == NA_STRING) ? StringVector::create(NA_STRING) : as<StringVector>(units["time"]);
    return rxDataSetup(dataf, covNames, sigma, df, ncoresRV, isChol, nDisplayProgress, amt, time);
  } else if (rxIs(ro,"event.data.frame")||
      rxIs(ro,"event.matrix")){
    DataFrame dataf = as<DataFrame>(ro);
    int nSub = 0, nObs = 0, nDoses = 0, i = 0, j = 0, k=0;
    // Since the event data frame can be "wild", these need to be
    // converted to integers.
    IntegerVector evid  = as<IntegerVector>(dataf[rxcEvid]);
    bool missingId = false;
    IntegerVector id(evid.size());
    if (rxcId > -1){
      id    = as<IntegerVector>(dataf[rxcId]);
    } else {
      for (i = 0; i < evid.size(); i++){
	id[i]=1;
      }
      missingId=true;
    }
    bool missingDv = false;
    NumericVector dv(evid.size());
    if (rxcDv > -1){
      dv = as<NumericVector>(dataf[rxcDv]);
    } else {
      for (i = 0; i < evid.size(); i++){
	dv[i] = NA_REAL;
      }
      missingDv = true;
    }
    NumericVector time0 = dataf[rxcTime];
    NumericVector amt   = dataf[rxcAmt];
    int ids = id.size();
    int lastId = id[0]-1;
    // Get the number of subjects
    // Get the number of observations
    // Get the number of doses
    for (i = 0; i < ids; i++){
      if (lastId != id[i]){
        nSub++;
        lastId=id[i];
      }
      if (evid[i]){
        nDoses++;
      } else {
        nObs++;
      }
    }
    // Now create data frames of observations and events
    NumericVector newDv(nObs);
    NumericVector newTimeO(nObs);
    int nCovs = 0;
    StringVector covN, simN;
    bool dataCov = false;
    DataFrame covDf;
    StringVector simNames;
    bool simVals = false;
    if (!sigma.isNULL()){
      NumericMatrix sigma1 = NumericMatrix(sigma);
      List dimnames = sigma1.attr("dimnames");
      simNames = StringVector(dimnames[1]);
      simVals = true;
    }
    int nCovObs = 0;
    if (rxIs(covNames, "character")){
      covN = StringVector(covNames);
      nCovObs = covN.size();
      if (simVals){
	nCovs= nCovObs + simNames.size();
      } else {
        nCovs = nCovObs;
      }
    } else if (rxIs(covNames, "data.frame") || rxIs(covNames,"numeric.matrix")){
      covDf = as<DataFrame>(covNames);
      covN = StringVector(covDf.names());
      nCovObs = covN.size();
      if (simVals){
	nCovs = nCovObs + simNames.size();
      } else {
	nCovs = nCovObs;
      }
      dataCov = true;
    } else if (simVals) {
      nCovs= simNames.size();
    }
    
    // Rprintf("nObs: %d; nCovs: %d\n", nObs, nCovs);
    NumericVector newCov(nObs*nCovs);
    
    IntegerVector newEvid(nDoses);
    IntegerVector idose(nDoses);
    NumericVector newAmt(nDoses);
    NumericVector newTimeA(nDoses);
    lastId = id[0]-1;
  
    IntegerVector newId(nSub);
    IntegerVector posDose(nSub);
    IntegerVector posObs(nSub);
    IntegerVector posCov(nSub);
    IntegerVector nCov(nSub);
    IntegerVector nDose(nSub);
    IntegerVector nObsN(nSub);
    IntegerVector posEt(nSub);
    IntegerVector nEtN(nSub);
    NumericVector Hmax(nSub); // For LSODA default
    IntegerVector rc(nSub);
    
    double minTime = NA_REAL;
    double maxTime = -1e10;
    double minIdTime = NA_REAL;
    double lastTime = 0;
    //hmax <- max(abs(diff(event.table$time)))
    double mdiff = 0;
    double HmaxA = 0;
    double tmp;
    int m = 0, nEt=0;
    for (i = 0; i < ids; i++){
      if (lastId != id[i]){
        lastId     = id[i];
        newId[m]   = id[i];
        posDose[m] = j;
        posObs[m]  = k;
        posEt[m] = i;
        if (m != 0){
          nDose[m-1] = nDoses;
          nObsN[m-1]  = nObs;
          nEtN[m-1] = nEt;
	  Hmax[m-1] = mdiff;
	  rc[m-1] = 0;
        }
        nDoses = 0;
        nObs = 0;
        nEt  = 0;
        m++;
	minIdTime = time0[i];
	lastTime = time0[i];
	mdiff = 0;
      }
      if (minIdTime > time0[i]){
	stop("Data need to be ordered by ID and TIME.");
      } else {
	minIdTime = time0[i];
      }
      tmp = time0[i]-lastTime;
      if (tmp > mdiff){
	mdiff = tmp;
	if (tmp > HmaxA){
          HmaxA = tmp;
	}
      }
      lastTime = time0[i];
      if (evid[i]){
        // Dose
        newEvid[j]  = evid[i];
        newTimeA[j] = time0[i];
        newAmt[j]   = amt[i];
        nDoses++;
        j++;
        nEt++;
      } else {
        // Observation
        newDv[k]    = dv[i];
        newTimeO[k] = time0[i];
        if (nObs == 0){
          minTime = time0[i];
        } else if (time0[i] > maxTime) {
          maxTime = time0[i];
        }
        nObs++;
        k++;
        nEt++;
      }
    }
    nDose[m-1]=nDoses;
    nObsN[m-1]=nObs;
    nEtN[m-1] = nEt;
    Hmax[m-1] = mdiff;
    rc[m-1] = 0;
    k = 0;
    if (dataCov && covDf.nrow() != nObs){
      if (covDf.nrow() == ids){
	List covDf2(covDf.nrow());
	for (j = 0; j < covDf.ncol(); j++){
          covDf2[j] = NumericVector(nObs);
	}
	for (i = 0; i < ids; i++){
	  if (!evid[i]){
	    for (j = 0; j < covDf.ncol(); j++){
              NumericVector cur = covDf2[j];
	      cur[k] = (as<NumericVector>(covDf[j]))[i];
            }
	    k++;
	  }
	}
        covDf2.attr("names") = covDf.attr("names");
        covDf2.attr("class") = "data.frame";
        covDf2.attr("row.names") = IntegerVector::create(NA_INTEGER,-nObs);
	covDf = as<DataFrame>(covDf2);
      } else {
	stop("Covariate data needs to match the number of observations in the overall dataset.");
      }
    }
    // Covariates are stacked by id that is
    // id=cov1,cov1,cov1,cov2,cov2,cov2,...
    lastId = id[0]-1;
    int n0 = 0, n = 0, nc = 0;
    m = 0;
    for (i = 0; i < ids; i++){
      if (lastId != id[i]){
        lastId     = id[i];
        if (m != 0){
          n0 += nObsN[m-1]*nCovs;
        }
        posCov[m] = n0;
        nCov[m]   = nObsN[m]*nCovs;
        nc = 0;
        m++;
      }
      if (!evid[i]){
        // Observation
        for (n = 0; n < nCovs; n++){
	  k = n0 + nc + n*nObsN[m-1];
	  if (n < nCovObs){
	    if (dataCov){
              newCov[k] = (as<NumericVector>(covDf[as<std::string>(covN[n])]))[nc];
            } else {
              newCov[k] = (as<NumericVector>(dataf[as<std::string>(covN[n])]))[i];
            }
          } else {
	    newCov[k] = 0;
          }
        }
        nc++;
      }
    }
    // nCov[m-1] = nObs*nCovs;
    List ret = List::create(_["dose"] = DataFrame::create(_["evid"]         = newEvid,
                                                          _["time"]         = newTimeA,
                                                          _["amt"]          = newAmt),
                            _["obs"]  = DataFrame::create(_["dv"]           = newDv,
                                                          _["time"]         = newTimeO),
                            _["ids"]  = DataFrame::create(_["id"]           = newId,
                                                          _["posDose"]      = posDose,
                                                          _["posObs"]       = posObs,
                                                          _["posCov"]       = posCov,
                                                          _["posEvent"]     = posEt,
                                                          _["nDose"]        = nDose,
                                                          _["nObs"]         = nObsN,
                                                          _["nCov"]         = nCov,
                                                          _["nEvent"]       = nEtN,
							  _["HmaxDefault"]  = Hmax,
							  _["rc"]           = rc),
                            _["et"] = DataFrame::create(_["evid"]=evid,
                                                        _["time"]=time0),
                            _["cov"]=newCov,
                            _["nSub"]=nSub,
                            _["nDoses"]=newEvid.size(),
                            _["nObs"]=newDv.size(),
                            _["min.time"] = minTime,
                            _["max.time"] = maxTime,
                            _["cov.names"]=(covN.size() == 0 ? R_NilValue : wrap(covN)),
			    _["n.observed.covariates"] = nCovObs,
			    _["simulated.vars"] = (simNames.size()== 0 ? R_NilValue : wrap(simNames)),
			    _["sigma"] = wrap(sigma),
                            _["amount.units"]=(as<std::string>(amountUnits) == "NA") ? StringVector::create(NA_STRING) : amountUnits,
                            _["time.units"]=(as<std::string>(timeUnits)  == "NA") ? StringVector::create(NA_STRING) : timeUnits,
			    _["missing.id"]=missingId,
			    _["missing.dv"]=missingDv,
			    _["ncoresRV"] = wrap(ncoresRV),
			    _["isChol"] = wrap(isChol)
                            );
    // Not sure why, but putting this in above gives errors...
    ret["df"]= df;
    ret["idose"] = idose;
    ret["Hmax"] = HmaxA;
    ret["nDisplayProgress"] = nDisplayProgress;
    ret.attr("class") = "RxODE.multi.data";
    return ret;
  } else if (rxIs(ro,"list")){
    Function asDf("as.data.frame", R_BaseNamespace);
    return rxDataSetup(asDf(ro), covNames,sigma, df, ncoresRV, isChol, nDisplayProgress, amountUnits, timeUnits);
  } else {
    stop("Data is not setup appropriately.");
  }
}

rx_solve *getRxSolve(SEXP ptr){
  if (rxIs(ptr,"RxODE.pointer.multi")){
    List lst = List(ptr);
    rxUpdateFuns(lst["trans"]);
    rx_solve *ret = getRxSolve_();
    // Also assign it.
    return ret;
  } else {
    stop("Cannot get the solving data (getRxSolve).");
  }
  rx_solve *o;
  return o;
}

// 
inline bool fileExists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

// [[Rcpp::export]]
List rxModelVars_(const RObject &obj){
  getRxModels();
  if (rxIs(obj, "rxModelVars")){
    List ret(obj);
    return ret;
  } else if (rxIs(obj,"RxODE")) {
    Environment e = as<Environment>(obj);
    List rxDll = e["rxDll"];
    List ret = rxDll["modVars"];
    return ret;
  } else if (rxIs(obj,"rxSolve")){
    CharacterVector cls = obj.attr("class");
    Environment e = as<Environment>(cls.attr(".RxODE.env"));
    return  rxModelVars_(as<RObject>(e["args.object"]));
  } else if (rxIs(obj,"rxDll")){
    List lobj = (as<List>(obj))["modVars"];
    return lobj;
  } else if (rxIs(obj, "character")){
    CharacterVector modList = as<CharacterVector>(obj);
    if (modList.size() == 1){
      std::string sobj =as<std::string>(obj);
      if ((sobj.find("=") == std::string::npos) &&
	  (sobj.find("<-") == std::string::npos)){
        if (_rxModels.exists(sobj)){
          RObject obj1 = _rxModels.get(sobj);
          if (rxIs(obj1, "rxModelVars")){
            return as<List>(obj1);
          } else if (rxIs(obj1, "RxODE")){
            return rxModelVars_(obj1);
          }
        }
        std::string sobj1 = sobj + "_model_vars";
        if (_rxModels.exists(sobj1)){
          RObject obj1 = _rxModels.get(sobj1);
          if (rxIs(obj1, "rxModelVars")){
            return as<List>(obj1);
          }
        }
        Function get("get",R_BaseNamespace);
        List platform = get(_["x"]=".Platform", _["envir"] = R_BaseEnv);
        sobj1 = sobj + "_" + as<std::string>(platform["r_arch"]) + "_model_vars";
        if (_rxModels.exists(sobj1)){
          RObject obj1 = _rxModels.get(sobj1);
          if (rxIs(obj1, "rxModelVars")){
            return as<List>(obj1);
          }
        }
        Function filePath("file.path", R_BaseNamespace);
        Function getwd("getwd", R_BaseNamespace);
        sobj1 = as<std::string>(getwd());
        std::string sobj2 = sobj + ".d";
        std::string sobj3 = sobj + "_" + as<std::string>(platform["r_arch"]) +
          as<std::string>(platform["dynlib.ext"]);
        sobj1 = as<std::string>(filePath(sobj1,sobj2, sobj3));
        if (fileExists(sobj1)){
          Rcout << "Path: " << sobj1 << "\n";
          Function dynLoad("dyn.load", R_BaseNamespace);
          dynLoad(sobj1);
          sobj1 = sobj + "_" + as<std::string>(platform["r_arch"]) +
            "_model_vars";
          Function call(".Call", R_BaseNamespace);
          List ret = as<List>(call(sobj1));
          return ret;
        }
      }
    } else if (modList.hasAttribute("names")){
      bool containsPrefix = false;
      CharacterVector modListNames = modList.names();
      for (int i = 0; i < modListNames.size(); i++){
	if (modListNames[i] == "prefix"){
	  containsPrefix=true;
	  break;
	}
      }
      if (containsPrefix){
	std::string mvstr = as<std::string>(modList["prefix"]) + "model_vars";
        if(_rxModels.exists(mvstr)){
          RObject obj1 = _rxModels.get(mvstr);
          if (rxIs(obj1, "rxModelVars")){
            return as<List>(obj1);
          }
        }
      }
    }
    // fileExists(const std::string& name)
    Environment RxODE("package:RxODE");
    Function f = as<Function>(RxODE["rxModelVars.character"]);
    return f(obj);
  } else if (rxIs(obj,"list")){
    bool params=false, lhs=false, state=false, trans=false, ini=false, model=false, md5=false, podo=false, dfdy=false;
    List lobj  = as<List>(obj);
    CharacterVector nobj = lobj.names();
    for (int i = 0; i < nobj.size(); i++){
      if (nobj[i] == "modVars"){
	return(rxModelVars_(lobj["modVars"]));
      } else if (!params && nobj[i]== "params"){
	params=true;
      } else if (!lhs && nobj[i] == "lhs"){
	lhs=true;
      } else if (!state && nobj[i] == "state"){
	state=true;
      } else if (!trans && nobj[i] == "trans"){
	trans=true;
      } else if (!ini && nobj[i] == "ini"){
	ini = true;
      } else if (!model && nobj[i] == "model"){
	model = true;
      } else if (!md5 && nobj[i] == "md5"){
	md5 = true;
      } else if (!podo && nobj[i] == "podo"){
	podo=true;
      } else if (!dfdy && nobj[i] == "dfdy"){
	dfdy = true;
      } else {
        return lobj;
      }
    }
    stop("Cannot figure out the model variables.");
  } else {
    CharacterVector cls = obj.attr("class");
    int i = 0;
    Rprintf("Class:\t");
    for (i = 0; i < cls.size(); i++){
      Rprintf("%s\t", (as<std::string>(cls[i])).c_str());
    }
    Rprintf("\n");
    stop("Need an RxODE-type object to extract model variables from.");
  }
}

List rxModelVars(const RObject &obj){
  return rxModelVars_(obj);
}
//' State variables
//'
//' This returns the model's compartments or states.
//'
//' @inheritParams rxModelVars
//'
//' @param state is a string indicating the state or compartment that
//'     you would like to lookup.
//'
//' @return If state is missing, return a character vector of all the states.
//'
//' If state is a string, return the compartment number of the named state.
//'
//' @seealso \code{\link{RxODE}}
//'
//' @author Matthew L.Fidler
//' @export
// [[Rcpp::export]]
RObject rxState(const RObject &obj = R_NilValue, RObject state = R_NilValue){
  List modVar = rxModelVars(obj);
  CharacterVector states = modVar["state"];
  if (state.isNULL()){
    return states;
  }
  else if (rxIs(state,"character")){
    CharacterVector lookup = as<CharacterVector>(state);
    if (lookup.size() > 1){
      // Fixme?
      stop("Can only lookup one state at a time.");
    }
    if (states.size() == 1){
      warning("Only one state variable should be input.");
    }
    IntegerVector ret(1);
    for (int i = 0; i < states.size(); i++){
      if (states[i] == lookup[0]){
	ret[0] = i+1;
	return ret;
      }
    }
    stop("Cannot locate compartment \"%s\".",as<std::string>(lookup[0]).c_str());
  }
  return R_NilValue;
}

//' Parameters specified by the model
//'
//' This return the model's parameters that are required to solve the
//' ODE system.
//'
//' @inheritParams rxModelVars
//'
//' @return a character vector listing the parameters in the model.
//'
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
CharacterVector rxParams(const RObject &obj){
  List modVar = rxModelVars(obj);
  CharacterVector ret = modVar["params"];
  return ret;
}


//' Jacobian and parameter derivatives
//'
//' Return Jacobain and parameter derivatives
//'
//' @inheritParams rxModelVars
//'
//' @return A list of the jacobian parameters defined in this RxODE
//'     object.
//' @author Matthew L. Fidler
//' @export
//[[Rcpp::export]]
CharacterVector rxDfdy(const RObject &obj){
  List modVar = rxModelVars(obj);
  CharacterVector ret = modVar["dfdy"];
  return ret;
}

//' Left handed Variables
//'
//' This returns the model calculated variables
//'
//' @inheritParams rxModelVars
//'
//' @return a character vector listing the calculated parameters
//' @seealso \code{\link{RxODE}}
//'
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
CharacterVector rxLhs(const RObject &obj){
  List modVar = rxModelVars(obj);
  CharacterVector ret = modVar["lhs"];
  return ret;
}
NumericVector rxInits0(const RObject &obj,
		       Nullable<NumericVector> vec = R_NilValue,
		       Nullable<CharacterVector> req = R_NilValue,
		       double defaultValue = 0,
		       bool noerror = false,
		       bool noini=false){
  NumericVector oini;
  CharacterVector cini;
  List modVar = rxModelVars(obj);
  if (!noini){
    oini = (modVar["ini"]);
    cini = oini.names();
  }
  int i, j, k;
  CharacterVector nreq;
  NumericVector miss;
  if (!req.isNull()){
    nreq = CharacterVector(req);
    if ((ISNA(defaultValue) && noerror) || !ISNA(defaultValue)){
      miss = NumericVector(nreq.size());
      for (i = 0; i < nreq.size(); i++) {
	miss[i] = defaultValue;
      }
      miss.attr("names") = CharacterVector(nreq);
    }
  }
  NumericVector nvec;
  CharacterVector nvecNames;
  if (!vec.isNull()){
    nvec = NumericVector(vec);
    if (nvec.size() > 0){
      if (!nvec.hasAttribute("names")){
	if (!req.isNull() && nreq.size() == nvec.size()){
	  nvec.attr("names") = req;
	  nvecNames = req;
	  std::string wstr = "Assumed order of inputs: ";
	  for (i = 0; i < nreq.size(); i++){
	    wstr += (i == 0 ? "" : ", ") + nreq[i];
	  }
	  warning(wstr);
	} else {
	  std::string sstr = "Length mismatch\nreq: c(";
	  for (i = 0; i < nreq.size(); i++){
	    sstr += (i == 0 ? "" : ", ") + nreq[i];
	  }
	  sstr += ")\nvec: c(";
	  for (i = 0; i < nvec.size(); i++){
            sstr += (i == 0 ? "" : ", ") + std::to_string((double)(nvec[i]));
          }
	  sstr += ")";
	  stop(sstr);
	}
      } else {
	nvecNames = nvec.names();
      }
    }
  }
  // Prefer c(vec, ini, miss)
  NumericVector ret;
  CharacterVector nret;
  if (!req.isNull()){
    ret =  NumericVector(nreq.size());
    bool found = false;
    for (i = 0; i < nreq.size(); i++){
      found = false;
      for (j = 0; !found && j < nvec.size(); j++){
	if (nreq[i] == nvecNames[j]){
	  found =  true;
	  ret[i] = nvec[j];
	  break;
	}
      }
      for (j = 0; !found && j < cini.size(); j++){
	if (nreq[i] == cini[j]){
          found =  true;
          ret[i] = oini[j];
          break;
        }
      }
      if (!found)
	ret[i] = miss[i];
    }
    ret.attr("names")= nreq;
  } else {
    // In this case
    // vec <- c(vec, ini);
    // vec <- vec[!duplicated(names(vec))]
    CharacterVector dupnames(nvec.size()+oini.size()+miss.size());
    j = 0;
    for (i = 0; i < nvec.size(); i++){
      dupnames[j] = nvecNames[i];
      j++;
    }
    for (i = 0; i < oini.size(); i++){
      dupnames[j] = cini[i];
      j++;
    }
    LogicalVector dups = duplicated(dupnames);
    j = 0;
    for (i = 0; i < dups.size(); i++){
      if (!dups[i]) j++;
    }
    ret = NumericVector(j);
    CharacterVector retn(j);
    k = 0, j = 0;
    for (i = 0; i < nvec.size(); i++){
      if (!dups[j]){
	ret[k] = nvec[i];
	retn[k] = nvecNames[i];
	k++;
      }
      j++;
    }
    for (i = 0; i < oini.size(); i++){
      if (!dups[j]){
	ret[k] = oini[i];
        retn[k] = cini[i];
        k++;
      }
      j++;
    }
    ret.attr("names") = retn;
  }
  return ret;
}
//' Initial Values and State values for a RxODE object
//'
//' Returns the initial values of the rxDll object
//'
//' @param obj rxDll, RxODE, or named vector representing default
//'     initial arguments
//'
//' @param vec If supplied, named vector for the model.
//'
//' @param req Required names, and the required order for the ODE solver
//'
//' @param defaultValue a number or NA representing the default value for
//'     parameters missing in \code{vec}, but required in \code{req}.
//'
//' @param noerror is a boolean specifying if an error should be thrown
//'     for missing parameter values when \code{default} = \code{NA}
//'
//' @keywords internal
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
NumericVector rxInits(const RObject &obj,
		      RObject vec = R_NilValue,
		      Nullable<CharacterVector> req = R_NilValue,
		      double defaultValue = 0,
		      bool noerror = false,
		      bool noini=false){
  if (vec.isNULL()){
    return rxInits0(obj, R_NilValue, req, defaultValue, noerror,noini);
  } else if (rxIs(vec, "list")){
    List vecL = as<List>(vec);
    Function unlist("unlist", R_BaseNamespace);
    NumericVector vec2 = as<NumericVector>(unlist(vec));
    // if (!vec2.hasAttribute("names")){
    //   stop("When using a list for inits or scales, the list must be named. list(depot=1)");
    // }
    if (vec2.size() != vecL.size()){
      stop("Only one estimate per named list item; i.e. list(x=1) instead of list(x=1:2).");
    }
    return rxInits0(obj, vec2, req, defaultValue, noerror,noini);
  } else if (rxIs(vec, "integer") || rxIs(vec, "numeric")){
    return rxInits0(obj, as<NumericVector>(vec), req, defaultValue, noerror,noini);
  } else {
    stop("Incompatible initial estimate type.");
  }
}

//' Setup the initial conditions.
//'
//' @param obj RxODE object
//' @param inits A numeric vector of initial conditions.
//' @author Matthew L. Fidler
//' @keywords internal
//' @export
//[[Rcpp::export]]
NumericVector rxSetupIni(const RObject &obj,
			   RObject inits = R_NilValue){
  List modVars = rxModelVars(obj);
  CharacterVector state = modVars["state"];
  return rxInits(obj, inits, state, 0.0);
}

//' Setup the initial conditions.
//'
//' @param obj RxODE object
//' @param inits A numeric vector of initial conditions.
//' @param extraArgs A list of extra args to parse for initial conditions.
//' @author Matthew L. Fidler
//' @keywords internal
//' @export
//[[Rcpp::export]]
NumericVector rxSetupScale(const RObject &obj,
			   RObject scale = R_NilValue,
			   Nullable<List> extraArgs = R_NilValue){
  List modVars = rxModelVars(obj);
  CharacterVector state = modVars["state"];
  NumericVector ret = rxInits(obj, scale, state, 1.0, false, true);
  unsigned int usedS = 0, foundS = 0;
  if (!extraArgs.isNull()){
    List xtra = as<List>(extraArgs);
    int i, n=state.size();
    std::string cur;
    for (i = 0; i < n; i++){
      cur = "S" + std::to_string(i+1);
      if (xtra.containsElementNamed(cur.c_str())){
	if (ret[i] == 1.0){
	  ret[i] = as<double>(xtra[cur]);
	  usedS++;
	} else {
	  stop("Trying to scale the same compartment by scale=c(%s=%f,...) and S%d=%f;  Cannot do both.",
	       (as<std::string>(state[i])).c_str(), ret[i], i+1,as<double>(xtra[i]));
	}
      } else {
	cur = "s" + std::to_string(i+1);
        if (xtra.containsElementNamed(cur.c_str())){
          if (ret[i] == 1.0){
            ret[i] = as<double>(xtra[cur]);
	    usedS++;
          } else {
            stop("Trying to scale the same compartment by scale=c(%s=%f,...) and s%d=%f;  Cannot do both.",
                 (as<std::string>(state[i])).c_str(), ret[i], i+1,as<double>(xtra[i]));
          }
        }
      }
    }
    Nullable<StringVector> xtraN2 = xtra.names();
    if (!xtraN2.isNull()){
      StringVector xtraN = StringVector(xtraN2);
      n = xtraN.size();
      std::string tmpstr;
      for (i = 0; i < n; i++){
	tmpstr = (as<std::string>(xtraN[i])).substr(0,1);
	if (tmpstr == "S" || tmpstr == "s"){
	  tmpstr = (as<std::string>(xtraN[i])).substr(1,1);
	  if (tmpstr == "0" || tmpstr == "1" || tmpstr == "2" || tmpstr == "3" || tmpstr == "4" ||
	      tmpstr == "5" || tmpstr == "6" || tmpstr == "7" || tmpstr == "8" || tmpstr == "9"){
	    foundS++;
          }
        }
      }
    }
    if (foundS > usedS){
      warning("Scaled a compartment that is not defined by the RxODE model.");
    }
  }
  return ret;
}

NumericMatrix rxSetupParamsThetaEta(const RObject &params = R_NilValue,
				    const RObject &theta = R_NilValue,
				    const RObject &eta = R_NilValue){
  // Now get the parameters as a data.frame
  if (rxIs(params,"list")) {
    Function asDf("as.data.frame", R_BaseNamespace);
    return rxSetupParamsThetaEta(asDf(params), theta, eta);
  }
  NumericMatrix parMat;
  int i;
  if (params.isNULL()){
    if (!theta.isNULL() || !eta.isNULL()){
      // Create the matrix
      NumericVector thetaN;
      if (rxIs(theta,"numeric") || rxIs(theta,"integer")){
        thetaN = as<NumericVector>(theta);
      } else if (rxIs(theta, "matrix")){
        NumericMatrix thetaM = as<NumericMatrix>(theta);
        if (thetaM.nrow() == 1){
          thetaN = NumericVector(thetaM.ncol());
          for (i = 0 ; i < thetaM.ncol(); i++){
            thetaN[i] = thetaM(1,i);
          }
        } else if (thetaM.ncol() == 1){
          thetaN = NumericVector(thetaM.nrow());
          for (i = 0 ; i < thetaM.ncol(); i++){
            thetaN[i] = thetaM(i, i);
          }
        } else {
          stop("'theta' is not compatible with params, check dimensions to make sure they are compatible.");
        }
      } else if (!theta.isNULL()){
        stop("'theta' is not compatible with params, check dimensions to make sure they are compatible.");
      }
      // Now eta
      NumericVector etaN;
      if (rxIs(eta,"numeric") || rxIs(eta,"integer")){
        etaN = as<NumericVector>(eta);
      } else if (rxIs(eta, "matrix")){
        NumericMatrix etaM = as<NumericMatrix>(eta);
        if (etaM.nrow() == 1){
          etaN = NumericVector(etaM.ncol());
          for (i = 0 ; i < etaM.ncol(); i++){
            etaN[i] = etaM(0, i);
          }
        } else if (etaM.ncol() == 1){
          etaN = NumericVector(etaM.nrow());
          for (i = 0 ; i < etaM.ncol(); i++){
            etaN[i] = etaM(i, 0);
          }
        } else {
          stop("'eta' is not compatible with params, check dimensions to make sure they are compatible.");
        }
      } else if (!eta.isNULL()){
        stop("'eta' is not compatible with params, check dimensions to make sure they are compatible.");
      }
      NumericMatrix tmp1(1, thetaN.size()+etaN.size());
      CharacterVector tmpN = CharacterVector(tmp1.size());
      for (i = 0; i < thetaN.size(); i++){
        tmp1(0, i) = thetaN[i];
        tmpN[i] = "THETA[" + std::to_string(i + 1) + "]";
      }
      for (; i < thetaN.size()+ etaN.size(); i++){
        tmp1(0, i) = etaN[i - thetaN.size()];
        tmpN[i] = "ETA[" + std::to_string(i - thetaN.size() + 1) + "]";
      }
      tmp1.attr("dimnames") = List::create(R_NilValue, tmpN);
      parMat = tmp1;
    }
  } else if (rxIs(params, "data.frame") || rxIs(params, "matrix")){
    if (rxIs(params,"data.frame")){
      DataFrame tmp = as<DataFrame>(params);
      int nr = tmp.nrows();
      NumericMatrix tmpM(nr,tmp.size());
      for (i = 0; i < tmp.size(); i++){
        tmpM(_,i) = NumericVector(tmp[i]);
      }
      tmpM.attr("dimnames") = List::create(R_NilValue,tmp.names());
      parMat=tmpM;
    } else {
      parMat = as<NumericMatrix>(params);
    }
  } else if (rxIs(params, "numeric") || rxIs(params, "integer")){
    // Create the matrix
    NumericVector thetaN;
    if (rxIs(theta,"numeric") || rxIs(theta,"integer")){
      thetaN = as<NumericVector>(theta);
    } else if (rxIs(theta, "matrix")){
      NumericMatrix thetaM = as<NumericMatrix>(theta);
      if (thetaM.nrow() == 1){
        thetaN = NumericVector(thetaM.ncol());
        for (i = 0 ; i < thetaM.ncol(); i++){
          thetaN[i] = thetaM(1,i);
        }
      } else if (thetaM.ncol() == 1){
        thetaN = NumericVector(thetaM.nrow());
        for (i = 0 ; i < thetaM.ncol(); i++){
          thetaN[i] = thetaM(i, i);
        }
      } else {
        stop("'theta' is not compatible with params, check dimensions to make sure they are compatible.");
      }
    } else if (!theta.isNULL()){
      stop("'theta' is not compatible with params, check dimensions to make sure they are compatible.");
    }
    // Now eta
    NumericVector etaN;
    if (rxIs(eta,"numeric") || rxIs(eta,"integer")){
      etaN = as<NumericVector>(eta);
    } else if (rxIs(eta, "matrix")){
      NumericMatrix etaM = as<NumericMatrix>(eta);
      if (etaM.nrow() == 1){
        etaN = NumericVector(etaM.ncol());
        for (i = 0 ; i < etaM.ncol(); i++){
          etaN[i] = etaM(0, i);
        }
      } else if (etaM.ncol() == 1){
        etaN = NumericVector(etaM.nrow());
        for (i = 0 ; i < etaM.ncol(); i++){
          etaN[i] = etaM(i, 0);
        }
      } else {
        stop("'eta' is not compatible with params, check dimensions to make sure they are compatible.");
      }
    } else if (!eta.isNULL()){
      stop("'eta' is not compatible with params, check dimensions to make sure they are compatible.");
    }
    NumericVector tmp0 = as<NumericVector>(params);
    NumericMatrix tmp1(1, tmp0.size()+thetaN.size()+etaN.size());
    CharacterVector tmp0N ;
    NumericVector pars = as<NumericVector>(params);
    if (tmp0.hasAttribute("names")){
      tmp0N = tmp0.names();
    } else if (tmp0.size() == pars.size()){
      tmp0N = pars;
    } else if (tmp0.size() > 0){
      // In this case there are no names
      stop("The parameter names must be specified.");
    }
    CharacterVector tmpN = CharacterVector(tmp1.size());
    for (i = 0; i < tmp0.size(); i++){
      tmp1(0, i) = tmp0[i];
      tmpN[i]   = tmp0N[i];
    }
    for (; i < tmp0.size()+thetaN.size(); i++){
      tmp1(0, i) = thetaN[i - tmp0.size()];
      tmpN[i] = "THETA[" + std::to_string(i - tmp0.size() + 1) + "]";
    }
    for (; i < tmp0.size()+thetaN.size()+ etaN.size(); i++){
      tmp1(0, i) = etaN[i - tmp0.size() - thetaN.size()];
      tmpN[i] = "ETA[" + std::to_string(i - tmp0.size() - thetaN.size() + 1) + "]";
    }
    tmp1.attr("dimnames") = List::create(R_NilValue, tmpN);
    parMat = tmp1;
  }
  return parMat;
}

double *gsolve = NULL;
int gsolven = 0;
void gsolveSetup(int n){
  if (gsolven == 0){
    gsolve=Calloc(n, double);
    gsolven=n;
  } else if (n > gsolven){
    gsolve = Realloc(gsolve, n, double);
    gsolven=n;
  }
  for (int i =0;i<n; i++) gsolve[i]=0.0;
}

double *gInfusionRate = NULL;
int gInfusionRaten = 0;
void gInfusionRateSetup(int n){
  if (gInfusionRaten == 0){
    gInfusionRate=Calloc(n, double);
    gInfusionRaten=n;
  } else if (n > gInfusionRaten){
    gInfusionRate = Realloc(gInfusionRate, n, double);
    gInfusionRaten=n;
  }
  for (int i =0;i<n; i++) gInfusionRate[i]=0.0;
}

double *gall_times = NULL;
int gall_timesn = 0;
void gall_timesSetup(int n){
  if (gall_timesn == 0){
    gall_times=Calloc(n, double);
    gall_timesn=n;
  } else if (n > gall_timesn){
    gall_times = Realloc(gall_times, n, double);
    gall_timesn=n;
  }
  // for (int i =0;i<n; i++) gall_times[i]=0.0;
}


double *gamt = NULL;
int gamtn = 0;
void gamtSetup(int n){
  if (gamtn == 0){
    gamt=Calloc(n, double);
    gamtn=n;
  } else if (n > gamtn){
    gamt = Realloc(gamt, n, double);
    gamtn=n;
  }
  // for (int i =0;i<n; i++) gamt[i]=0.0;
}

double *glhs = NULL;
int glhsn = 0;
void glhsSetup(int n){
  if (glhsn == 0){
    glhs=Calloc(n, double);
    glhsn=n;
  } else if (n > glhsn){
    glhs = Realloc(glhs, n, double);
    glhsn=n;
  }
  for (int i =0;i<n; i++) glhs[i]=0.0;
}

double *gcov = NULL;
int gcovn = 0;
void gcovSetup(int n){
  if (gcovn == 0){
    gcov=Calloc(n, double);
    gcovn=n;
  } else if (n > gcovn){
    gcov = Realloc(gcov, n, double);
    gcovn=n;
  }
  for (int i =0;i<n; i++) gcov[i]=0.0;
}

double *ginits = NULL;
int ginitsn = 0;
void ginitsSetup(int n){
  if (ginitsn == 0){
    ginits=Calloc(n, double);
    ginitsn=n;
  } else if (n > ginitsn){
    ginits = Realloc(ginits, n, double);
    ginitsn=n;
  }
  for (int i = 0; i < n; i++) ginits[i]=0.0;
}


double *gscale = NULL;
int gscalen = 0;
void gscaleSetup(int n){
  if (gscalen == 0){
    gscale=Calloc(n, double);
    gscalen=n;
  } else if (n > gscalen){
    gscale = Realloc(gscale, n, double);
    gscalen=n;
  }
  for (int i = 0; i < n; i++) gscale[i]=1.0;
}

double *gatol2 = NULL;
int gatol2n = 0;
void gatol2Setup(int n){
  if (gatol2n == 0){
    gatol2=Calloc(n, double);
    gatol2n=n;
  } else if (n > gatol2n){
    gatol2 = Realloc(gatol2, n, double);
    gatol2n=n;
  }
}

double *grtol2 = NULL;
int grtol2n = 0;
void grtol2Setup(int n){
  if (grtol2n == 0){
    grtol2=Calloc(n, double);
    grtol2n=n;
  } else if (n > grtol2n){
    grtol2 = Realloc(grtol2, n, double);
    grtol2n=n;
  }
}

double *gpars = NULL;
int gparsn = 0;
void gparsSetup(int n){
  if (gparsn == 0){
    gpars=Calloc(n, double);
    gparsn=n;
  } else if (n > gparsn){
    gpars = Realloc(gpars, n, double);
    gparsn=n;
  }
}


int *gevid = NULL;
int gevidn = 0;
void gevidSetup(int n){
  if (gevidn == 0){
    gevid=Calloc(n, int);
    gevidn=n;
  } else if (n > gevidn){
    gevid = Realloc(gevid, n, int);
    gevidn=n;
  }
}

int *gBadDose = NULL;
int gBadDosen = 0;
void gBadDoseSetup(int n){
  if (gBadDosen == 0){
    gBadDose=Calloc(n, int);
    gBadDosen=n;
  } else if (n > gBadDosen){
    gBadDose = Realloc(gBadDose, n, int);
    gBadDosen=n;
  }
}

int *grc = NULL;
int grcn = 0;
void grcSetup(int n){
  if (grcn == 0){
    grc=Calloc(n, int);
    grcn=n;
  } else if (n > grcn){
    grc = Realloc(grc, n, int);
    grcn=n;
  }
}

int *gidose = NULL;
int gidosen = 0;
extern "C" int *gidoseSetup(int n){
  if (gidosen == 0){
    gidose=Calloc(n, int);
    gidosen=n;
  } else if (n > gidosen){
    gidose = Realloc(gidose, n, int);
    gidosen=n;
  }
  return gidose;
}

int *gpar_cov = NULL;
int gpar_covn = 0;
void gpar_covSetup(int n){
  if (gpar_covn == 0){
    gpar_cov=Calloc(n, int);
    gpar_covn=n;
  } else if (n > gpar_covn){
    gpar_cov = Realloc(gpar_cov, n, int);
    gpar_covn=n;
  }
}


int *gsvar = NULL;
int gsvarn = 0;
void gsvarSetup(int n){
  if (gsvarn == 0){
    gsvar=Calloc(n, int);
    gsvarn=n;
  } else if (n > gsvarn){
    gsvar = Realloc(gsvar, n, int);
    gsvarn=n;
  }
}

int *gsiV = NULL;
int gsiVn = 0;
extern "C" int *gsiVSetup(int n){
  if (gsiVn == 0){
    gsiV=Calloc(n, int);
    gsiVn=n;
  } else if (n > gsiVn){
    gsiV = Realloc(gsiV, n, int);
    gsiVn=n;
  }
  return gsiV;
}


void gFree(){
  if (gsiV != NULL) Free(gsiV);
  gsiVn=0;
  if (gsvar != NULL) Free(gsvar);
  gsvarn=0;
  if (gpar_cov != NULL) Free(gpar_cov);
  gpar_covn=0;
  if (gidose != NULL) Free(gidose);
  gidosen=0;
  if (grc != NULL) Free(grc);
  grcn=0;
  if (gBadDose != NULL) Free(gBadDose);
  gBadDosen=0;
  if (gevid != NULL) Free(gevid);
  gevidn=0;
  if (gpars != NULL) Free(gpars);
  gparsn=0;
  if (grtol2 != NULL) Free(grtol2);
  grtol2n=0;
  if (gatol2 != NULL) Free(gatol2);
  gatol2n=0;
  if (gscale != NULL) Free(gscale);
  gscalen=0;
  if (ginits != NULL) Free(ginits);
  ginitsn=0;
  if (gcov != NULL) Free(gcov);
  gcovn=0;
  if (glhs != NULL) Free(glhs);
  glhsn=0;
  if (gamt != NULL) Free(gamt);
  gamtn=0;
  if (gall_times != NULL) Free(gall_times);
  gall_timesn=0;
  if (gInfusionRate != NULL) Free(gInfusionRate);
  gInfusionRaten=0;
  if (gsolve != NULL) Free(gsolve);
  gsolven=0;
}


//' Setup Data and Parameters
//'
//' @inheritParams rxSolve
//' @param sigma Named sigma matrix.
//' @param sigmaDf The degrees of freedom of a t-distribution for
//'     simulation.  By default this is \code{NULL} which is
//'     equivalent to \code{Inf} degrees, or to simulate from a normal
//'     distribution instead of a t-distribution.
//' @param nCoresRV Number of cores for residual simulation.  This,
//'     along with the seed, affects both the outcome and speed of
//'     simulation. By default it is one.
//' @param sigmaIsChol Indicates if the \code{sigma} supplied is a
//'     Cholesky decomposed matrix instead of the traditional
//'     symmetric matrix.
//' @return Data setup for running C-based RxODE runs.
//' @author Matthew L. Fidler
//' @keywords internal
//' @export
//[[Rcpp::export]]
List rxDataParSetup(const RObject &object,
			   const RObject &params = R_NilValue,
			   const RObject &events = R_NilValue,
			   const RObject &inits = R_NilValue,
			   const RObject &covs  = R_NilValue,
			   const RObject &sigma= R_NilValue,
			   const RObject &sigmaDf= R_NilValue,
			   const int &nCoresRV= 1,
			   const bool &sigmaIsChol= false,
			   const int &nDisplayProgress = 10000,
			   const StringVector &amountUnits = NA_STRING,
			   const StringVector &timeUnits = "hours",
			   const RObject &theta = R_NilValue,
			   const RObject &eta = R_NilValue,
			   const RObject &scale = R_NilValue,
			   const Nullable<List> &extraArgs = R_NilValue){
  List modVars = rxModelVars(object);
  CharacterVector state = modVars["state"];
  // The initial conditions cannot be changed for each individual; If
  // they do they need to be a parameter.
  NumericVector initsC = rxInits(object, inits, state, 0.0);
  NumericVector scaleC = rxSetupScale(object, scale, extraArgs);
  // The parameter vector/matrix/data frame contains the parameters
  // that will be used.
  RObject par0 = params;
  RObject ev0  = events;
  RObject ev1;
  RObject par1;
  if (rxIs(par0, "rx.event")){
    // Swapped events and parameters
    ev1 = par0;
    par1 = ev0;
  } else if (rxIs(ev0, "rx.event")) {
    ev1 = ev0;
    par1 = par0;
  } else {
    stop("Need some event information (observation/dosing times) to solve.\nYou can use either 'eventTable' or an RxODE compatible data frame/matrix.");
  }
  // Now get the parameters (and covariates)
  //
  // Unspecified parameters can be found in the modVars["ini"]
  NumericVector modVarsIni = modVars["ini"];
  // The event table can contain covariate information, if it is acutally a data frame or matrix.
  Nullable<CharacterVector> covnames0, simnames0;
  CharacterVector covnames, simnames;
  CharacterVector pars = modVars["params"];
  int i, j, k = 0;
  CharacterVector tmpCv;
  List ret;
  if (!rxIs(ev1,"EventTable") &&  covs.isNULL()){
    // Now covnames is setup correctly, import into a setup data table.
    // In this case the events are a data frame or matrix
    CharacterVector tmpCv =as<CharacterVector>((as<DataFrame>(ev1)).names());
    for (i = 0; i < pars.size(); i++){
      for (j = 0; j < tmpCv.size(); j++){
	if (pars[i] == tmpCv[j]){
	  k++;
	  break;
	}
      }
    }
    covnames = CharacterVector(k);
    k = 0;
    for (i = 0; i < pars.size(); i++){
      for (j = 0; j < tmpCv.size(); j++){
	if (pars[i] == tmpCv[j]){
	  covnames[k] = pars[i];
	  k++;
	  break;
	}
      }
    }
    ret = rxDataSetup(ev1, (covnames.size() == 0 ? R_NilValue : wrap(covnames)),
		      sigma, sigmaDf, nCoresRV, sigmaIsChol, nDisplayProgress, 
		      amountUnits, timeUnits);
  } else {
    ret = rxDataSetup(ev1, covs, sigma, sigmaDf,
		      nCoresRV, sigmaIsChol, nDisplayProgress, amountUnits, timeUnits);
    covnames0 = as<Nullable<CharacterVector>>(ret["cov.names"]);
    if (!covnames0.isNull()){
      covnames = CharacterVector(covnames0);
    }
  }
  simnames0 = as<Nullable<CharacterVector>>(ret["simulated.vars"]);
  if (!simnames0.isNull()){
    simnames = CharacterVector(simnames0);
  }
  NumericMatrix parMat = rxSetupParamsThetaEta(par1, theta, eta);
  int nSub = as<int>(ret["nSub"]);
  if (parMat.nrow() % nSub != 0){
    stop("The Number of parameters must be a multiple of the number of subjects.");
  }
  k = 0;
  IntegerVector pcov(covnames.size());
  IntegerVector svar(simnames.size());
  for (i = 0; i < covnames.size(); i++){
    for (j = 0; j < pars.size(); j++){
      if (covnames[i] == pars[j]){
	pcov[i] = j + 1;
	break;
      }
    }
  }
  for (i = 0; i < simnames.size(); i++){
    for (j = 0; j < pars.size(); j++){
      if (simnames[i] == pars[j]){
        svar[i] = j;
        break;
      }
    }
  }
  // Now pcov gives the which for the covariate parameters.
  // Now check if we have all the parameters that are needed.
  std::string errStr = "";
  bool allPars = true;
  bool curPar = false;
  IntegerVector posPar(pars.size());
  CharacterVector nms = modVarsIni.names();
  Nullable<CharacterVector> nmP2 = (as<List>(parMat.attr("dimnames")))[1];
  CharacterVector nmP;
  if (!nmP2.isNull()){
    nmP = CharacterVector(nmP2);
  }
  for (i = 0; i < pars.size(); i++){
    curPar = false;
    // integers are faster to compare than strings.
    for (j = 0; j < pcov.size(); j++){
      if (pcov[j] == i + 1){
	posPar[i] = 0; // Covariates are zeroed out.
	curPar = true;
	break;
      }
    }
    // First Check to see if the user specified the parameter.
    if (!curPar){
      for (j = 0; j < nmP.size(); j++){
        if (nmP[j] == pars[i]){
          curPar = true;
          posPar[i] = j + 1;
          break;
        }
      }
    }
    // Now check $ini
    if (!curPar){
      for(j = 0; j < modVarsIni.size(); j++){
        if (nms[j] == pars[i]){
          curPar = true;
          posPar[i] = -j - 1;
          break;
        }
      }
    }
    if (!curPar){
      if (errStr == ""){
	errStr = "The following parameter(s) are required for solving: " + pars[i];
      } else {
	errStr = errStr + ", " + pars[i];
      }
      allPars = false;
    }
  }
  if (!allPars){
    CharacterVector modSyntax = modVars["model"];
    Rcout << "Model:\n\n" + modSyntax[0] + "\n";
    stop(errStr);
  }
  // Now  the parameter names are setup.
  // The parameters are setup in a numeric vector in order of pars
  int nr = parMat.nrow();
  if (nr == 0) nr = 1;
  NumericVector parsVec(pars.size()*nr);
  j = 0;
  for (i = 0; i < parsVec.size(); i++){
    j = floor(i / pars.size());
    k = i % pars.size();
    if (posPar[k] == 0){
      parsVec[i] = 0;
    } else if (posPar[k] > 0){
      // posPar[i] = j + 1;
      parsVec[i] = parMat(j, posPar[k]-1);
    } else {
      // posPar[i] = -j - 1;
      parsVec[i] = modVarsIni[-(posPar[k]+1)];
    }
  }
  ret["pars"] = parsVec;
  nr = parMat.nrow() / nSub;
  if (nr == 0) nr = 1;

  ret["nsim"] = nr;
  // NumericVector initsS = NumericVector(initsC.size()*nSub*nr);
  // for (i = 0; i < initsS.size(); i++){
  //   j = i % initsS.size();
  //   initsS[i] =  initsC[j];
  // }
  ret["inits"] = initsC;
  ret["scale"] = scaleC;
  // ret["inits.full"] = initsS;
  ret["n.pars"] = (int)(pars.size());
  ret["pcov"] = pcov;
  ret["svar"] = svar;
  ret["neq"] = state.size();
  DataFrame et      = as<DataFrame>(ret["et"]);
  // et.nrow includes ALL subjects doses/observations
  // nr is the number of simulations
  // state.size is the number of states solved for....
  gsolveSetup(state.size()*et.nrow()*nr);
  CharacterVector lhs = as<CharacterVector>(modVars["lhs"]);
  // ret["lhs"] = NumericVector(lhs.size()*nSub*nr);
  glhsSetup(lhs.size()*nSub*nr);
  ret["lhsSize"] = lhs.size();
  gInfusionRateSetup(state.size()*nSub*nr);
  ret["BadDose"] =IntegerVector(state.size()*nSub*nr);
  ret["state.ignore"] = modVars["state.ignore"];
  ret["trans"] = modVars["trans"];
  NumericVector atol(state.size(), 1e-08);
  NumericVector rtol(state.size(), 1e-06);
  ret["atol"] = atol;
  ret["rtol"] = rtol;
  CharacterVector cls(2);
  cls(1) = "RxODE.par.data";
  cls(0) = "RxODE.multi.data";
  ret.attr("class") = cls;
  return ret;
}

void rxSolvingOptions(const RObject &object,
                      const std::string &method = "liblsoda",
                      const Nullable<LogicalVector> &transit_abs = R_NilValue,
                      const double atol = 1.0e-8,
                      const double rtol = 1.0e-6,
                      const int maxsteps = 5000,
                      const double hmin = 0,
		      const double hini = 0,
		      const int maxordn = 12,
                      const int maxords = 5,
		      const int cores = 1,
		      const int ncov = 0,
		      int *par_cov = NULL,
		      int do_par_cov = 0,
		      double *inits = NULL,
		      double *scale = NULL,
		      std::string covs_interpolation = "linear",
		      double hmax2 = 0,
                      double *atol2 = NULL,
                      double *rtol2 = NULL,
                      int nDisplayProgress = 10000,
                      int ncoresRV = 1,
                      int isChol = 1,
                      int *svar =NULL){
  if (maxordn < 1 || maxordn > 12){
    stop("'maxordn' must be >1 and <= 12.");
  }
  if (maxords < 1 || maxords > 5){
    stop("'maxords' must be >1 and <= 5.");
  }
  if (hmin < 0){
    stop("'hmin' must be a non-negative value.");
  }
  // HMAX is determined by the problem since it can be thought of as the maximum difference of the event table's time
  if (hini < 0){
    stop("'hini' must be a non-negative value.");
  }
  List modVars = rxModelVars(object);
  int transit = 0;
  if (transit_abs.isNull()){
    transit = modVars["podo"];
    if (transit){
      warning("Assumed transit compartment model since 'podo' is in the model.");
    }
  } else {
    LogicalVector tr = LogicalVector(transit_abs);
    if (tr[0]){
      transit=  1;
    }
  }
  int is_locf = 0;
  double f1 = 1.0, f2 = 0.0;
  int kind=1;
  if (covs_interpolation == "linear"){
  } else if (covs_interpolation == "constant" || covs_interpolation == "locf" || covs_interpolation == "LOCF"){
    f2 = 0.0;
    f1 = 1.0;
    kind = 0;
    is_locf=1;
  } else if (covs_interpolation == "nocb" || covs_interpolation == "NOCB"){
    f2 = 1.0;
    f1 = 0.0;
    kind = 0;
    is_locf=2;
  }  else if (covs_interpolation == "midpoint"){
    f1 = f2 = 0.5;
    kind = 0;
    is_locf=3;
  } else {
    stop("Unknown covariate interpolation specified.");
  }
  int st=0;
  if (method == "liblsoda"){
    st = 2;
  } else if (method == "lsoda"){
    st = 1;
  } else if (method == "dop853"){
    st = 0;
  } else {
    stop("Unknown ODE solving method specified.");
  }
  CharacterVector lhs = as<CharacterVector>(modVars["lhs"]);
  CharacterVector state = as<CharacterVector>(modVars["state"]);
  CharacterVector params = as<CharacterVector>(modVars["params"]);
  CharacterVector trans = modVars["trans"];
  // Make sure the model variables are assigned...
  // This fixes random issues on windows where the solves are done and the data set cannot be solved.
  std::string ptrS = (as<std::string>(trans["model_vars"]));
  _rxModels[ptrS] = modVars;
  getSolvingOptionsPtr(atol,rtol,hini, hmin,
		       maxsteps, maxordn, maxords, transit,
		       lhs.size(), state.size(),
		       st, f1, f2, kind, is_locf, cores,
		       ncov,par_cov, do_par_cov, &inits[0], &scale[0],
		       ptrS.c_str(), hmax2, atol2, rtol2, 
		       nDisplayProgress, ncoresRV, isChol,svar);
}

inline void rxSolvingData(const RObject &model,
			  const RObject &parData,
			  const std::string &method = "liblsoda",
			  const Nullable<LogicalVector> &transit_abs = R_NilValue,
			  const double atol = 1.0e-8,
			  const double rtol = 1.0e-6,
			  const int maxsteps = 5000,
			  const double hmin = 0,
			  const Nullable<NumericVector> &hmax = R_NilValue,
			  const double hini = 0,
			  const int maxordn = 12,
			  const int maxords = 5,
			  const int cores = 1,
			  std::string covs_interpolation = "linear",
			  bool addCov = false,
			  bool matrix = false) {
  if (rxIs(parData, "RxODE.par.data")){
    int i = 0;
    List opt = List(parData);
    DataFrame ids = as<DataFrame>(opt["ids"]);
    IntegerVector BadDose = as<IntegerVector>(opt["BadDose"]);
    gBadDoseSetup(BadDose.size());
    for (i = 0; i < BadDose.size(); i++){
      gBadDose[i] = BadDose[i];
    }
    NumericVector par = as<NumericVector>(opt["pars"]);
    gparsSetup(par.size());
    for (i = 0; i < par.size();i++){
      gpars[i] = par[i];
    }
    double hm;
    int nPar = as<int>(opt["n.pars"]);
    int nSub = as<int>(opt["nSub"]);
    NumericVector inits = as<NumericVector>(opt["inits"]);
    ginitsSetup(inits.size());
    for (i = 0; i < inits.size(); i++){
      ginits[i] = inits[i];
    }
    NumericVector scale = as<NumericVector>(opt["scale"]);
    gscaleSetup(scale.size());
    for (i = 0; i < scale.size(); i++){
      gscale[i] = scale[i];
    }
    DataFrame doseDf = as<DataFrame>(opt["dose"]);
    NumericVector amt = as<NumericVector>(doseDf["amt"]);
    gamtSetup(amt.size());
    for (i = 0; i < amt.size(); i++){
      gamt[i]= amt[i];
    }
    IntegerVector idose = as<IntegerVector>(opt["idose"]);
    gidoseSetup(idose.size());
    for (i = 0; i < idose.size(); i++){
      gidose[i]= idose[i];
    }
    IntegerVector posDose = as<IntegerVector>(ids["posDose"]);
    IntegerVector posEvent = as<IntegerVector>(ids["posEvent"]);
    IntegerVector posCov = as<IntegerVector>(ids["posCov"]);
    IntegerVector nEvent = as<IntegerVector>(ids["nEvent"]);
    DataFrame et         = as<DataFrame>(opt["et"]);
    IntegerVector evid   = as<IntegerVector>(et["evid"]);
    gevidSetup(evid.size());
    for (i = 0; i < evid.size(); i++){
      gevid[i] = evid[i];
    }
    NumericVector all_times = as<NumericVector>(et["time"]);
    gall_timesSetup(all_times.size());
    for (i = 0; i < all_times.size(); i++){
      gall_times[i] = all_times[i];
    }
    // NumericVector lhs = as<NumericVector>(opt["lhs"]);
    int lhsSize = as<int>(opt["lhsSize"]);
    IntegerVector par_cov = as<IntegerVector>(opt["pcov"]);
    gpar_covSetup(par_cov.size());
    for (i = 0; i < par_cov.size(); i++){
      gpar_cov[i] = par_cov[i];
    }
    NumericVector cov = as<NumericVector>(opt["cov"]);
    gcovSetup(cov.size());
    for (i = 0; i < cov.size(); i++){
      gcov[i] = cov[i];
    }
    IntegerVector rc=as<IntegerVector>(ids["rc"]);
    grcSetup(rc.size());
    for (i = 0; i < rc.size(); i++){
      grc[i] = rc[i];
    }
    IntegerVector siV = as<IntegerVector>(opt["state.ignore"]);
    gsiVSetup(siV.size());
    for (i = 0; i < siV.size(); i++){
      gsiV[i] = siV[i];
    }
    int do_par_cov = 0;
    if (par_cov.size() > 0){
      do_par_cov = 1;
    }
    rx_solving_options_ind *inds;
    int nsim = as<int>(opt["nsim"]);
    inds = rxOptionsIniEnsure(nSub*nsim);
    int neq = as<int>(opt["neq"]);
    int ncov =-1;
    int cid;
    for (int simNum = 0; simNum < nsim; simNum++){
      for (int id = 0; id < nSub; id++){
	cid = id+simNum*nSub;
	if (hmax.isNull()){
          // Get from data.
          NumericVector hmn = as<NumericVector>(ids["HmaxDefault"]);
          hm = hmn[id];
        } else {
          NumericVector hmn = NumericVector(hmax);
          if (R_FINITE(hmn[0])){
	    stop("'hmax' must be a non-negative value.");
            hm = hmn[0];
          } else {
            hm = 0.0;
          }
        }
	ncov = par_cov.size();
        getSolvingOptionsIndPtr(&gInfusionRate[cid*neq],&gBadDose[cid*neq], hm,
				&gpars[cid*nPar], &gamt[posDose[id]],
				&gidose[posDose[id]],
                                // Solve and lhs are written to in the solve...
                                &gsolve[cid*nEvent[id]*neq],
                                &glhs[cid*lhsSize],
                                // Doesn't change with the solve.
				&gevid[posEvent[id]], &grc[id], &gcov[posCov[id]],
                                nEvent[id], &gall_times[posEvent[id]], id, simNum,
                                &inds[cid]);
      }
    }
    NumericVector atol2 = as<NumericVector>(opt["atol"]);
    NumericVector rtol2 = as<NumericVector>(opt["rtol"]);
    gatol2Setup(atol2.size());
    grtol2Setup(rtol2.size());
    for (i = 0; i < atol2.size(); i++){
      gatol2[i]=atol;
      grtol2[i]=rtol;
    }
    double hmax2 = as<double>(opt["Hmax"]);
    IntegerVector svar = as<IntegerVector>(opt["svar"]);
    gsvarSetup(svar.size());
    for (i = 0; i < svar.size(); i++){
      gsvar[i] = svar[i];
    }
    bool isCholB =  as<bool>(opt["isChol"]);
    int isChol = 0;
    if (isCholB) isChol = 1;
    rxSolvingOptions(model,method, transit_abs, atol, rtol, maxsteps, hmin, hini, maxordn,
		     maxords, cores, ncov, &gpar_cov[0], do_par_cov, &ginits[0], &gscale[0], covs_interpolation,
		     hmax2,&gatol2[0],&grtol2[0], as<int>(opt["nDisplayProgress"]),
		     as<int>(opt["ncoresRV"]),isChol, &gsvar[0]);
    int add_cov = 0;
    if (addCov) add_cov = 1;
    int nobs = as<int>(opt["nObs"]);
    int mat = 0;
    if (matrix) mat = 1;
    rxSolveData(inds, nSub, nsim, &siV[0], nobs, add_cov, mat);
  } else {
    stop("This requires something setup by 'rxDataParSetup'.");
  }
}

List rxData(const RObject &object,
            const RObject &params = R_NilValue,
            const RObject &events = R_NilValue,
            const RObject &inits = R_NilValue,
            const RObject &covs  = R_NilValue,
            const std::string &method = "liblsoda",
            const Nullable<LogicalVector> &transit_abs = R_NilValue,
            const double atol = 1.0e-8,
            const double rtol = 1.0e-6,
            const int maxsteps = 5000,
            const double hmin = 0,
            const Nullable<NumericVector> &hmax = R_NilValue,
            const double hini = 0,
            const int maxordn = 12,
            const int maxords = 5,
            const int cores = 1,
            std::string covs_interpolation = "linear",
	    bool addCov = false,
            bool matrix = false,
            const RObject &sigma= R_NilValue,
            const RObject &sigmaDf= R_NilValue,
            const int &nCoresRV= 1,
            const bool &sigmaIsChol= false,
            const int &nDisplayProgress = 10000,
            const StringVector &amountUnits = NA_STRING,
            const StringVector &timeUnits = "hours",
	    const RObject &theta = R_NilValue,
            const RObject &eta = R_NilValue,
	    const RObject &scale = R_NilValue,
	    const Nullable<List> &extraArgs = R_NilValue){
  List parData = rxDataParSetup(object,params, events, inits, covs, sigma, sigmaDf,
				nCoresRV, sigmaIsChol, nDisplayProgress, amountUnits, timeUnits,
				theta,eta, scale, extraArgs);
  rxSolvingData(object, parData, method, transit_abs, atol,  rtol, maxsteps,
		hmin, hmax,  hini, maxordn, maxords, cores, covs_interpolation,
		addCov, matrix);
  List modVars = rxModelVars(object);
  StringVector cls(3);
  cls(2) = "RxODE.par.data";
  cls(1) = "RxODE.multi.data";
  cls(0) = "RxODE.pointer.multi";
  parData.attr("class") = cls;
  return parData;
}

#define defrx_params R_NilValue
#define defrx_events R_NilValue
#define defrx_inits R_NilValue
#define defrx_covs R_NilValue
#define defrx_method "liblsoda"
#define defrx_transit_abs R_NilValue
#define defrx_atol 1.0e-8
#define defrx_rtol 1.0e-8
#define defrx_maxsteps 5000
#define defrx_hmin 0
#define defrx_hmax R_NilValue
#define defrx_hini 0
#define defrx_maxordn 12
#define defrx_maxords 5
#define defrx_cores 1
#define defrx_covs_interpolation "linear"
#define defrx_addCov false
#define defrx_matrix false
#define defrx_sigma  R_NilValue
#define defrx_sigmaDf R_NilValue
#define defrx_nCoresRV 1
#define defrx_sigmaIsChol false
#define defrx_nDisplayProgress 10000
#define defrx_amountUnits NA_STRING
#define defrx_timeUnits "hours"
#define defrx_addDosing false


RObject rxCurObj;

Nullable<Environment> rxRxODEenv(RObject obj);

std::string rxDll(RObject obj);

bool rxDynLoad(RObject obj);

SEXP rxSolveC(const RObject &object,
              const Nullable<CharacterVector> &specParams = R_NilValue,
	      const Nullable<List> &extraArgs = R_NilValue,
	      const RObject &params = R_NilValue,
	      const RObject &events = R_NilValue,
	      const RObject &inits = R_NilValue,
	      const RObject &scale = R_NilValue,
	      const RObject &covs  = R_NilValue,
	      const CharacterVector &method = "liblsoda",
	      const Nullable<LogicalVector> &transit_abs = R_NilValue,
	      const double atol = 1.0e-8,
	      const double rtol = 1.0e-6,
	      const int maxsteps = 5000,
	      const double hmin = 0,
	      const Nullable<NumericVector> &hmax = R_NilValue,
	      const double hini = 0,
	      const int maxordn = 12,
	      const int maxords = 5,
	      const int cores = 1,
	      const CharacterVector &covs_interpolation = "linear",
	      bool addCov = false,
	      bool matrix = false,
	      const RObject &sigma= R_NilValue,
	      const RObject &sigmaDf= R_NilValue,
	      const int &nCoresRV= 1,
	      const bool &sigmaIsChol= false,
	      const int &nDisplayProgress = 10000,
	      const CharacterVector &amountUnits = NA_STRING,
	      const CharacterVector &timeUnits = "hours",
              const bool addDosing = false,
	      const RObject &theta = R_NilValue,
	      const RObject &eta = R_NilValue,
	      const bool updateObject = false,
	      const bool doSolve = true
              ){
  if (updateObject && !rxIs(object, "rxSolve")){
    return rxSolveC(rxCurObj, specParams, extraArgs, params, events, inits,
                    scale, covs, method, transit_abs, atol, rtol, maxsteps,
                    hmin, hmax, hini, maxordn, maxords, cores,covs_interpolation,
                    addCov, matrix, sigma, sigmaDf, nCoresRV, sigmaIsChol, nDisplayProgress,
                    amountUnits,timeUnits, addDosing, R_NilValue, R_NilValue, updateObject, false);
  } else if (rxIs(object, "rxSolve") || rxIs(object, "environment")){
    // Check to see what parameters were updated by specParams
    bool update_params = false,
      update_events = false,
      update_inits = false,
      update_covs = false,
      update_method = false,
      update_transit_abs = false,
      update_atol = false,
      update_rtol = false,
      update_maxsteps = false,
      update_hini = false,
      update_hmin = false,
      update_hmax = false,
      update_maxordn = false,
      update_maxords = false,
      update_cores = false,
      update_covs_interpolation = false,
      update_addCov = false,
      update_matrix = false,
      update_sigma  = false,
      update_sigmaDf = false,
      update_nCoresRV = false,
      update_sigmaIsChol = false,
      update_amountUnits = false,
      update_timeUnits = false,
      update_scale = false,
      update_dosing = false;
    if (specParams.isNull()){
      warning("No additional parameters were specified; Returning fit.");
      return object;
    }
    CharacterVector specs = CharacterVector(specParams);
    int n = specs.size(), i;
    for (i = 0; i < n; i++){
      if (as<std::string>(specs[i]) == "params")
	update_params = true;
      else if (as<std::string>(specs[i]) == "events")
	update_events = true;
      else if (as<std::string>(specs[i]) == "inits")
	update_inits = true;
      else if (as<std::string>(specs[i]) == "covs")
	update_covs = true;
      else if (as<std::string>(specs[i]) == "method")
	update_method = true;
      else if (as<std::string>(specs[i]) == "transit_abs")
	update_transit_abs = true;
      else if (as<std::string>(specs[i]) == "atol")
	update_atol = true;
      else if (as<std::string>(specs[i]) == "rtol")
	update_rtol = true;
      else if (as<std::string>(specs[i]) == "maxsteps")
	update_maxsteps = true;
      else if (as<std::string>(specs[i]) == "hmin")
	update_hmin = true;
      else if (as<std::string>(specs[i]) == "hmax")
	update_hmax = true;
      else if (as<std::string>(specs[i]) == "maxordn")
	update_maxordn = true;
      else if (as<std::string>(specs[i]) == "maxords")
	update_maxords = true;
      else if (as<std::string>(specs[i]) == "cores")
	update_cores = true;
      else if (as<std::string>(specs[i]) == "covs_interpolation")
	update_covs_interpolation = true;
      else if (as<std::string>(specs[i]) == "addCov")
	update_addCov = true;
      else if (as<std::string>(specs[i]) == "matrix")
	update_matrix = true;
      else if (as<std::string>(specs[i]) == "sigma")
	update_sigma  = true;
      else if (as<std::string>(specs[i]) == "sigmaDf")
	update_sigmaDf = true;
      else if (as<std::string>(specs[i]) == "nCoresRV")
	update_nCoresRV = true;
      else if (as<std::string>(specs[i]) == "sigmaIsChol")
	update_sigmaIsChol = true;
      else if (as<std::string>(specs[i]) == "amountUnits")
	update_amountUnits = true;
      else if (as<std::string>(specs[i]) == "timeUnits")
	update_timeUnits = true;
      else if (as<std::string>(specs[i]) == "hini")
	update_hini = true;
      else if (as<std::string>(specs[i]) == "scale")
	update_scale = true;
      else if (as<std::string>(specs[i]) == "addDosing")
	update_dosing = true;
    }
    // Now update
    Environment e;
    List obj;
    if (rxIs(object, "rxSolve")){
      obj = as<List>(obj);
      CharacterVector classattr = object.attr("class");
      e = as<Environment>(classattr.attr(".RxODE.env"));
    } else if (rxIs(object, "environment")) {
      e = as<Environment>(object);
      obj = as<List>(e["obj"]);
    }
    getRxModels();
    if(e.exists(".sigma")){
      _rxModels[".sigma"]=as<NumericMatrix>(e[".sigma"]);
    }
    if(e.exists(".sigmaL")){
      _rxModels[".sigmaL"]=as<List>(e[".sigmaL"]);
    }
    if(e.exists(".omegaL")){
      _rxModels[".omegaL"] = as<List>(e[".omegaL"]);
    }
    if(e.exists(".theta")){
      _rxModels[".theta"] = as<NumericMatrix>(e[".theta"]);
    }
    RObject new_params = update_params ? params : e["args.params"];
    RObject new_events = update_events ? events : e["args.events"];
    RObject new_inits = update_inits ? inits : e["args.inits"];
    RObject new_covs  = update_covs  ? covs  : e["args.covs"];
    CharacterVector new_method = update_method ? method : e["args.method"];
    Nullable<LogicalVector> new_transit_abs = update_transit_abs ? transit_abs : e["args.transit_abs"];
    double new_atol = update_atol ? atol : e["args.atol"];
    double new_rtol = update_rtol ? rtol : e["args.rtol"];
    int new_maxsteps = update_maxsteps ? maxsteps : e["args.maxsteps"];
    int new_hmin = update_hmin ? hmin : e["args.hmin"];
    Nullable<NumericVector> new_hmax = update_hmax ? hmax : e["args.hmax"];
    int new_hini = update_hini ? hini : e["args.hini"];
    int new_maxordn = update_maxordn ? maxordn : e["args.maxordn"];
    int new_maxords = update_maxords ? maxords : e["args.maxords"];
    int new_cores = update_cores ? cores : e["args.cores"];
    CharacterVector new_covs_interpolation = update_covs_interpolation ? covs_interpolation : e["args.covs_interpolation"];
    bool new_addCov = update_addCov ? addCov : e["args.addCov"];
    bool new_matrix = update_matrix ? matrix : e["args.matrix"];
    RObject new_sigma = update_sigma ? sigma : e["args.sigma"];
    RObject new_sigmaDf = update_sigmaDf ? sigmaDf : e["args.sigmaDf"];
    int new_nCoresRV = update_nCoresRV ? nCoresRV : e["args.nCoresRV"];
    bool new_sigmaIsChol = update_sigmaIsChol ? sigmaIsChol : e["args.sigmaIsChol"];
    int new_nDisplayProgress = e["args.nDisplayProgress"];
    CharacterVector new_amountUnits = update_amountUnits ? amountUnits : e["args.amountUnits"];
    CharacterVector new_timeUnits = update_timeUnits ? timeUnits : e["args.timeUnits"];
    RObject new_scale = update_scale ? scale : e["args.scale"];
    bool new_addDosing = update_dosing ? addDosing : e["args.addDosing"];

    RObject new_object = as<RObject>(e["args.object"]);
    CharacterVector new_specParams(0);
    List dat = as<List>(rxSolveC(new_object, new_specParams, extraArgs, new_params, new_events, new_inits, new_scale, new_covs,
				 new_method, new_transit_abs, new_atol, new_rtol, new_maxsteps, new_hmin,
				 new_hmax, new_hini,new_maxordn, new_maxords, new_cores, new_covs_interpolation,
				 new_addCov, new_matrix, new_sigma, new_sigmaDf, new_nCoresRV, new_sigmaIsChol,
                                 new_nDisplayProgress, new_amountUnits, new_timeUnits, new_addDosing));
    if (updateObject && as<bool>(e[".real.update"])){
      List old = as<List>(rxCurObj);
      //Should I zero out the List...?
      CharacterVector oldNms = old.names();
      CharacterVector nms = dat.names();
      if (oldNms.size() == nms.size()){
        int i;
        for (i = 0; i < nms.size(); i++){
          old[as<std::string>(nms[i])] = as<SEXP>(dat[as<std::string>(nms[i])]);
        }
        old.attr("class") = dat.attr("class");
        old.attr("row.names") = dat.attr("row.names");
        return old;
      } else {
        warning("Cannot update object...");
        return dat;
      }
    }
    e[".real.update"] = true;
    return dat;
  } else {
    if (!rxDynLoad(object)){
      stop("Cannot load RxODE dlls for this model.");
    }
    List parData = rxData(object, params, events, inits, covs, as<std::string>(method[0]), transit_abs, atol,
                          rtol, maxsteps, hmin,hmax, hini, maxordn, maxords, cores,
                          as<std::string>(covs_interpolation[0]), addCov, matrix, sigma, sigmaDf, nCoresRV, sigmaIsChol,
                          nDisplayProgress, amountUnits, timeUnits, theta, eta, scale, extraArgs);
    if (!doSolve){
      // Backwards Compatible; Create solving environment
      if (as<int>(parData["nsim"]) == 1 && as<int>(parData["nSub"]) == 1){
	int stiff = 0;
        if (as<std::string>(method[0]) == "liblsoda"){
	  stiff = 2;
	} else if (as<std::string>(method[0]) == "lsoda"){
	  stiff = 1;
	} else if (as<std::string>(method[0]) != "dop853") {
	  stop("Only lsoda or dop853 can be used with do.solve=FALSE");
	}
	List mv = rxModelVars(object);
	List et = parData["et"];
	List dose = parData["dose"];
        List ret;
	if (!extraArgs.isNull()){
	  ret = as<List>(extraArgs);
	}
        RObject par0 = params;
        RObject ev0  = events;
        RObject par1;
        if (rxIs(par0, "rx.event")){
          // Swapped events and parameters
          par1 = ev0;
        } else if (rxIs(ev0, "rx.event")) {
          par1 = par0;
        } else {
          stop("Need some event information (observation/dosing times) to solve.\nYou can use either 'eventTable' or an RxODE compatible data frame/matrix.");
        }
	NumericVector p = parData["pars"];
	p.attr("names") = mv["params"];
        ret["params"] = p;
	ret["inits"] = parData["inits"]; // named
	ret["scale"] = parData["scale"];
	// add.cov
	ret["state.ignore"] = parData["state.ignore"];
	ret["object"] = rxRxODEenv(mv);
	// event.table
	// events
	// extra.args
	ret["lhs_vars"] = mv["lhs"];
	ret["time"] = et["time"];
	ret["evid"] = et["evid"];
	ret["amt"] = dose["amt"];
	ret["pcov"] = parData["pcov"];
	ret["covs"] = parData["cov"];
	// FIXME: isLocf
        int is_locf = 0;
        if (as<std::string>(covs_interpolation[0]) == "linear"){
        } else if (as<std::string>(covs_interpolation[0]) == "constant" || as<std::string>(covs_interpolation[0]) == "locf" || as<std::string>(covs_interpolation[0]) == "LOCF"){
          is_locf=1;
        } else if (as<std::string>(covs_interpolation[0]) == "nocb" || as<std::string>(covs_interpolation[0]) == "NOCB"){
          is_locf=2;
        }  else if (as<std::string>(covs_interpolation[0]) == "midpoint"){
          is_locf=3;
        } else {
          stop("Unknown covariate interpolation specified.");
        }
	ret["isLocf"] = is_locf;
        ret["atol"] = atol;
        ret["rtol"] = rtol;
	ret["hmin"] = hmin;
        if (hmax.isNull()){
          // Get from data.
	  List ids = as<List>(parData["ids"]);
          NumericVector hmn = as<NumericVector>(ids["HmaxDefault"]);
          ret["hmax"] = hmn[0];
	} else {
	  NumericVector hmn = as<NumericVector>(hmax);
          ret["hmax"] = hmn[0];
	}
	ret["hini"] = hini;
	ret["maxordn"] = maxordn;
	ret["maxords"] = maxords;
	ret["maxsteps"] = maxsteps;
	// FIXME stiff
	ret["stiff"] = stiff;
        int transit = 0;
        if (transit_abs.isNull()){
          transit = mv["podo"];
          if (transit){
            warning("Assumed transit compartment model since 'podo' is in the model.");
          }
        } else {
          LogicalVector tr = LogicalVector(transit_abs);
          if (tr[0]){
            transit=  1;
          }
        }
	ret["transit_abs"] = transit;
	IntegerVector rc(1);
	ret["rc"] = rc;
        return as<SEXP>(ret);
      } else {
	stop("do.solve = TRUE only works with single subject data (currently).");
      }
    }
    DataFrame ret;
    
    rx_solve *rx;
    rx = getRxSolve(parData);
    rxModelVars(object);
    par_solve(rx);
    rx_solving_options *op = getRxOp(rx);
    if (op->abort){
      stop("Aborted solve.");
    }
    int doDose = 0;
    if (addDosing){
      doDose = 1;
    } else {
      doDose = 0;
    }
    List dat = RxODE_df(parData, doDose);
    List xtra;
    if (!rx->matrix) xtra = RxODE_par_df(parData);
    int nr = as<NumericVector>(dat[0]).size();
    int nc = dat.size();
    if (rx->matrix){
      getRxModels();
      if(_rxModels.exists(".sigma")){
        _rxModels.remove(".sigma");
      }
      if(_rxModels.exists(".sigmaL")){
        _rxModels.remove(".sigmaL");
      }
      if(_rxModels.exists(".omegaL")){
        _rxModels.remove(".omegaL");
      }
      if(_rxModels.exists(".theta")){
        _rxModels.remove(".theta");
      }
      dat.attr("class") = "data.frame";
      NumericMatrix tmpM(nr,nc);
      for (int i = 0; i < dat.size(); i++){
        tmpM(_,i) = as<NumericVector>(dat[i]);
      }
      tmpM.attr("dimnames") = List::create(R_NilValue,dat.names());
      return tmpM;
    } else {
      Function newEnv("new.env", R_BaseNamespace);
      Environment RxODE("package:RxODE");
      Environment e = newEnv(_["size"] = 29, _["parent"] = RxODE);
      getRxModels();
      if(_rxModels.exists(".theta")){
        e[".theta"] = as<NumericMatrix>(_rxModels[".theta"]);
        _rxModels.remove(".theta");
      }
      if(_rxModels.exists(".sigma")){
        e[".sigma"] = as<NumericMatrix>(_rxModels[".sigma"]);
        _rxModels.remove(".sigma");
      }
      if(_rxModels.exists(".omegaL")){
        e[".omegaL"] = as<List>(_rxModels[".omegaL"]);
        _rxModels.remove(".omegaL");
      }
      if(_rxModels.exists(".sigmaL")){
        e[".sigmaL"] = as<List>(_rxModels[".sigmaL"]);
        _rxModels.remove(".sigmaL");
      }
      e["check.nrow"] = nr;
      e["check.ncol"] = nc;
      e["check.names"] = dat.names();
      // Save information
      // Remove one final; Just for debug.
      // e["parData"] = parData;
      List pd = as<List>(xtra[0]);
      if (pd.size() == 0){
	e["params.dat"] = R_NilValue;
      } else {
	e["params.dat"] = pd;
      }
      if (as<int>(parData["nSub"]) == 1 && as<int>(parData["nsim"]) == 1){
        int n = pd.size();
        NumericVector par2(n);
        for (int i = 0; i <n; i++){
          par2[i] = (as<NumericVector>(pd[i]))[0];
        }
        par2.names() = pd.names();
	if (par2.size() == 0){
	  e["params.single"] = R_NilValue;
	} else {
	  e["params.single"] = par2;
        }
      } else {
        e["params.single"] = R_NilValue;
      }
      e["EventTable"] = xtra[1];
      e["dosing"] = xtra[3];
      e["sampling"] = xtra[2];
      e["obs.rec"] = xtra[4];
      e["covs"] = xtra[5];
      e["counts"] = xtra[6];
      e["inits.dat"] = parData["inits"];
      CharacterVector units(2);
      units[0] = as<std::string>(parData["amount.units"]);
      units[1] = as<std::string>(parData["time.units"]);
      CharacterVector unitsN(2);
      unitsN[0] = "dosing";
      unitsN[1] = "time";
      units.names() = unitsN;
      e["units"] = units;
      e["nobs"] = parData["nObs"];
    
      Function eventTable("eventTable",RxODE);
      List et = eventTable(_["amount.units"] = as<std::string>(parData["amount.units"]), _["time.units"] =as<std::string>(parData["time.units"]));
      Function importEt = as<Function>(et["import.EventTable"]);
      importEt(e["EventTable"]);
      e["events.EventTable"] = et;
      Function parse2("parse", R_BaseNamespace);
      Function eval2("eval", R_BaseNamespace);
      // eventTable style methods
      e["get.EventTable"] = eval2(_["expr"]   = parse2(_["text"]="function() EventTable"),
                                  _["envir"]  = e);
      e["get.obs.rec"] = eval2(_["expr"]   = parse2(_["text"]="function() obs.rec"),
                               _["envir"]  = e);
      e["get.nobs"] = eval2(_["expr"]   = parse2(_["text"]="function() nobs"),
                            _["envir"]  = e);
      e["add.dosing"] = eval2(_["expr"]   = parse2(_["text"]="function(...) {et <- create.eventTable(); et$add.dosing(...); invisible(rxSolve(args.object,events=et,update.object=TRUE))}"),
                              _["envir"]  = e);
      e["clear.dosing"] = eval2(_["expr"]   = parse2(_["text"]="function(...) {et <- create.eventTable(); et$clear.dosing(...); invisible(rxSolve(args.object,events=et,update.object=TRUE))}"),
                                _["envir"]  = e);
      e["get.dosing"] = eval2(_["expr"]   = parse2(_["text"]="function() dosing"),
                              _["envir"]  = e);

      e["add.sampling"] = eval2(_["expr"]   = parse2(_["text"]="function(...) {et <- create.eventTable(); et$add.sampling(...); invisible(rxSolve(args.object,events=et,update.object=TRUE))}"),
                                _["envir"]  = e);
      
      e["clear.sampling"] = eval2(_["expr"]   = parse2(_["text"]="function(...) {et <- create.eventTable(); et$clear.sampling(...); invisible(rxSolve(args.object,events=et,update.object=TRUE))}"),
                                  _["envir"]  = e);

      e["replace.sampling"] = eval2(_["expr"]   = parse2(_["text"]="function(...) {et <- create.eventTable(); et$clear.sampling(); et$add.sampling(...); invisible(rxSolve(args.object,events=et,update.object=TRUE))}"),
                                _["envir"]  = e);

      e["get.sampling"] = eval2(_["expr"]   = parse2(_["text"]="function() sampling"),
				_["envir"]  = e);
      
      e["get.units"] = eval2(_["expr"]   = parse2(_["text"]="function() units"),
                             _["envir"]  = e);

      e["import.EventTable"] = eval2(_["expr"]   = parse2(_["text"]="function(imp) {et <- create.eventTable(imp); invisible(rxSolve(args.object,events=et,update.object=TRUE))}"),
				     _["envir"]  = e);
      
      e["create.eventTable"] = eval2(_["expr"]   = parse2(_["text"]="function(new.event) {et <- eventTable(amount.units=units[1],time.units=units[2]);if (missing(new.event)) {nev <- EventTable; } else {nev <- new.event;}; et$import.EventTable(nev); return(et);}"),
                                     _["envir"]  = e);
      // Note event.copy doesn't really make sense...?  The create.eventTable does basically the same thing.
      e["args.object"] = object;
      e["dll"] = rxDll(object);
      if (rxIs(events, "rx.event")){
	e["args.params"] = params;    
        e["args.events"] = events;
      } else {
	e["args.params"] = events;    
        e["args.events"] = params;
      }
      e["args.inits"] = inits;
      e["args.covs"] = covs;
      e["args.method"] = method;
      e["args.transit_abs"] = transit_abs;
      e["args.atol"] = atol;
      e["args.rtol"] = rtol;
      e["args.maxsteps"] = maxsteps;
      e["args.hmin"] = hmin;
      e["args.hmax"] = hmax;
      e["args.hini"] = hini;
      e["args.maxordn"] = maxordn;
      e["args.maxords"] = maxords;
      e["args.cores"] = cores;
      e["args.covs_interpolation"] = covs_interpolation;
      e["args.addCov"] = addCov;
      e["args.matrix"] = matrix;
      e["args.sigma"] = sigma;
      e["args.sigmaDf"] = sigmaDf;
      e["args.nCoresRV"] = nCoresRV;
      e["args.sigmaIsChol"] = sigmaIsChol;
      e["args.nDisplayProgress"] = nDisplayProgress;
      e["args.amountUnits"] = amountUnits;
      e["args.timeUnits"] = timeUnits;
      e["args.addDosing"] = addDosing;
      e[".real.update"] = true;
      CharacterVector cls(2);
      cls(0) = "rxSolve";
      cls(1) = "data.frame";
      cls.attr(".RxODE.env") = e;    
      dat.attr("class") = cls;
      return(dat);
    }
  }
  return R_NilValue;
}

//[[Rcpp::export]]
SEXP rxSolveCsmall(const RObject &object,
                   const Nullable<CharacterVector> &specParams = R_NilValue,
                   const Nullable<List> &extraArgs = R_NilValue,
                   const RObject &params = R_NilValue,
                   const RObject &events = R_NilValue,
                   const RObject &inits = R_NilValue,
		   const RObject &scale = R_NilValue,
                   const RObject &covs  = R_NilValue,
                   const Nullable<List> &optsL = R_NilValue){
  if (optsL.isNull()){
    stop("Not meant to be called directly.  Needs options setup.");
  }
  List opts = List(optsL);
  return rxSolveC(object, specParams, extraArgs, params, events, inits, scale, covs,
                  opts[0], // const CharacterVector &method = "lsoda",
                  opts[1], // const Nullable<LogicalVector> &transit_abs = R_NilValue,
                  opts[2], //const double atol = 1.0e-8,
                  opts[3], // const double rtol = 1.0e-6,
                  opts[4], //const int maxsteps = 5000,
                  opts[5], //const double hmin = 0,
                  opts[6], //const Nullable<NumericVector> &hmax = R_NilValue,
                  opts[7], //const double hini = 0,
                  opts[8], //const int maxordn = 12,
                  opts[9], //const int maxords = 5,
                  opts[10], //const int cores = 1,
                  opts[11], //const CharacterVector &covs_interpolation = "linear",
                  opts[12], //bool addCov = false,
                  opts[13], //bool matrix = false,
                  opts[14], //const RObject &sigma= R_NilValue,
                  opts[15], //const RObject &sigmaDf= R_NilValue,
                  opts[16], //const int &nCoresRV= 1,
                  opts[17], //const bool &sigmaIsChol= false,
                  opts[18], // nDisplayProgress
                  opts[19], //const CharacterVector &amountUnits = NA_STRING,
                  opts[20], //const CharacterVector &timeUnits = "hours",
                  opts[21], //const RObject &theta = R_NilValue,
                  opts[22], //const RObject &eta = R_NilValue,
                  opts[23], //const bool addDosing = false
		  opts[24],
		  opts[25]);//const bool updateObject = false)
}

//[[Rcpp::export]]
RObject rxSolveGet(RObject obj, RObject arg, LogicalVector exact = true){
  std::string sarg;
  int i, j, n;
  if (rxIs(obj, "data.frame")){
    List lst = as<List>(obj);
    if (rxIs(arg, "character")){
      sarg = as<std::string>(arg);
      CharacterVector nm = lst.names();
      n = nm.size();
      unsigned int slen = strlen(sarg.c_str());
      int dexact = -1;
      if (exact[0] == TRUE){
	dexact = 1;
      } else if (exact[0] == FALSE){
	dexact = 0;
      }
      unsigned int slen2;
      for (i = 0; i < n; i++){
	slen2 = strlen((as<std::string>(nm[i])).c_str());
	if (slen <= slen2 &&
	    (strncmp((as<std::string>(nm[i])).c_str(), sarg.c_str(), slen)  == 0 ) &&
	    (dexact != 1 || (dexact == 1 && slen == slen2))){
	  if (dexact == -1){
	    warning("partial match of '%s' to '%s'",sarg.c_str(), (as<std::string>(nm[i])).c_str());
	  }
	  return lst[i];
	}
      }
      if (rxIs(obj, "rxSolve")){
	rxCurObj = obj;
	CharacterVector cls = lst.attr("class");
	Environment e = as<Environment>(cls.attr(".RxODE.env"));
	if (sarg == "env"){
	  return as<RObject>(e);
	}
	if (sarg == "model"){
	  List mv = rxModelVars(obj);
	  CharacterVector mods = mv["model"];
	  CharacterVector retS = CharacterVector::create(mods["model"]);
	  retS.attr("class") = "RxODE.modeltext";
	  return(retS);
	}
	if (e.exists(sarg)){
	  return e[sarg];
	}
	if (sarg == "params" || sarg == "par" || sarg == "pars" || sarg == "param"){
	  return e["params.dat"];
	} else if (sarg == "inits" || sarg == "init"){
	  return e["inits.dat"];
	} else if (sarg == "t"){
	  return lst["time"];
	} else if (sarg == "sigma.list" && e.exists(".sigmaL")){
	  return e[".sigmaL"];
	} else if (sarg == "omega.list" && e.exists(".omegaL")){
          return e[".omegaL"];
	}
	// Now parameters
	List pars = List(e["params.dat"]);
	CharacterVector nmp = pars.names();
	n = pars.size();
	for (i = 0; i < n; i++){
	  if (nmp[i] == sarg){
	    return pars[sarg];
	  }
	}
	// // Now inis.
	// Function sub("sub", R_BaseNamespace);
	NumericVector ini = NumericVector(e["inits.dat"]);
	CharacterVector nmi = ini.names();
	n = ini.size();
        std::string cur;
        NumericVector retN(1);
        for (i = 0; i < n; i++){
	  cur = as<std::string>(nmi[i]) + "0";
	  if (cur == sarg){
	    retN = ini[i];
	    return as<RObject>(retN);
	  }
	  cur = as<std::string>(nmi[i]) + "_0";
          if (cur == sarg){
	    retN = ini[i];
            return as<RObject>(retN);
          }
          cur = as<std::string>(nmi[i]) + ".0";
          if (cur == sarg){
            retN = ini[i];
            return as<RObject>(retN);
          }
          cur = as<std::string>(nmi[i]) + "[0]";
          if (cur == sarg){
	    retN = ini[i];
            return as<RObject>(retN);
          }
          cur = as<std::string>(nmi[i]) + "(0)";
          if (cur == sarg){
	    retN = ini[i];
	    return as<RObject>(retN);
          }
          cur = as<std::string>(nmi[i]) + "{0}";
          if (cur == sarg){
	    retN = ini[i];
            return as<RObject>(retN);
          }
	}
	List mv = rxModelVars(obj);
	CharacterVector normState = mv["normal.state"];
	CharacterVector parsC = mv["params"];
        CharacterVector lhsC = mv["lhs"];
	for (i = 0; i < normState.size(); i++){
	  for (j = 0; j < parsC.size(); j++){
	    std::string test = "_sens_" + as<std::string>(normState[i]) + "_" + as<std::string>(parsC[j]);
	    if (test == sarg){
	      test = "rx__sens_" + as<std::string>(normState[i]) + "_BY_" + as<std::string>(parsC[j]) + "__";
	      return lst[test];
	    }
            test = as<std::string>(normState[i]) + "_" + as<std::string>(parsC[j]);
            if (test == sarg){
              test = "rx__sens_" + as<std::string>(normState[i]) + "_BY_" + as<std::string>(parsC[j]) + "__";
	      return lst[test];
	    }
            test = as<std::string>(normState[i]) + "." + as<std::string>(parsC[j]);
            if (test == sarg){
              test = "rx__sens_" + as<std::string>(normState[i]) + "_BY_" + as<std::string>(parsC[j]) + "__";
              return lst[test];
            }
	  }
          for (j = 0; j < lhsC.size(); j++){
            std::string test = "_sens_" + as<std::string>(normState[i]) + "_" + as<std::string>(lhsC[j]);
            if (test == sarg){
              test = "rx__sens_" + as<std::string>(normState[i]) + "_BY_" + as<std::string>(lhsC[j]) + "__";
              return lst[test];
            }
            test = as<std::string>(normState[i]) + "_" + as<std::string>(lhsC[j]);
            if (test == sarg){
              test = "rx__sens_" + as<std::string>(normState[i]) + "_BY_" + as<std::string>(lhsC[j]) + "__";
              return lst[test];
            }
            test = as<std::string>(normState[i]) + "." + as<std::string>(lhsC[j]);
            if (test == sarg){
              test = "rx__sens_" + as<std::string>(normState[i]) + "_BY_" + as<std::string>(lhsC[j]) + "__";
              return lst[test];
            }
          }
	}
      }
    } else {
      if (rxIs(arg, "integer") || rxIs(arg, "numeric")){
	int iarg = as<int>(arg);
	if (iarg < lst.size()){
	  return lst[iarg-1];
	}
      }
    }
  }
  return R_NilValue;
}

//[[Rcpp::export]]
RObject rxSolveUpdate(RObject obj,
		      RObject arg = R_NilValue,
		      RObject value = R_NilValue){
  if (rxIs(obj,"rxSolve")){
    rxCurObj = obj;
    if (rxIs(arg,"character")){
      CharacterVector what = CharacterVector(arg);
      if (what.size() == 1){
	std::string sarg = as<std::string>(what[0]);
	// Now check to see if this is something that can be updated...
	if (sarg == "params"){
	  return rxSolveC(obj,
                          CharacterVector::create("params"),
			  R_NilValue,
                          value, //defrx_params,
                          defrx_events,
                          defrx_inits,
			  R_NilValue, // scale (cannot be updated currently.)
                          defrx_covs,
                          defrx_method,
                          defrx_transit_abs,
                          defrx_atol,
                          defrx_rtol,
                          defrx_maxsteps,
                          defrx_hmin,
                          defrx_hmax,
                          defrx_hini,
                          defrx_maxordn,
                          defrx_maxords,
                          defrx_cores,
                          defrx_covs_interpolation,
                          defrx_addCov,
                          defrx_matrix,
                          defrx_sigma,
                          defrx_sigmaDf,
                          defrx_nCoresRV,
                          defrx_sigmaIsChol,
                          defrx_nDisplayProgress,
                          defrx_amountUnits,
                          defrx_timeUnits, defrx_addDosing);
	} else if (sarg == "events"){
	  return rxSolveC(obj,
			  CharacterVector::create("events"),
			  R_NilValue,
			  defrx_params,
			  value, // defrx_events,
			  defrx_inits,
			  R_NilValue, // scale
			  defrx_covs,
			  defrx_method,
			  defrx_transit_abs,
			  defrx_atol,
			  defrx_rtol,
			  defrx_maxsteps,
			  defrx_hmin,
			  defrx_hmax,
			  defrx_hini,
			  defrx_maxordn,
			  defrx_maxords,
			  defrx_cores,
			  defrx_covs_interpolation,
			  defrx_addCov,
			  defrx_matrix,
			  defrx_sigma,
			  defrx_sigmaDf,
			  defrx_nCoresRV,
			  defrx_sigmaIsChol,
                          defrx_nDisplayProgress,
			  defrx_amountUnits,
			  defrx_timeUnits, 
			  defrx_addDosing);
	} else if (sarg == "inits"){
	  return rxSolveC(obj,
                          CharacterVector::create("inits"),
			  R_NilValue,
                          defrx_params,
                          defrx_events,
                          as<RObject>(value), //defrx_inits,
			  R_NilValue, // scale
			  defrx_covs,
                          defrx_method,
                          defrx_transit_abs,
                          defrx_atol,
                          defrx_rtol,
                          defrx_maxsteps,
                          defrx_hmin,
                          defrx_hmax,
                          defrx_hini,
                          defrx_maxordn,
                          defrx_maxords,
                          defrx_cores,
                          defrx_covs_interpolation,
                          defrx_addCov,
                          defrx_matrix,
                          defrx_sigma,
                          defrx_sigmaDf,
                          defrx_nCoresRV,
                          defrx_sigmaIsChol,
                          defrx_nDisplayProgress,
                          defrx_amountUnits,
                          defrx_timeUnits, 
			  defrx_addDosing);
	} else if (sarg == "covs"){
	  return rxSolveC(obj,
                          CharacterVector::create("covs"),
			  R_NilValue,
                          defrx_params,
                          defrx_events,
                          defrx_inits,
			  R_NilValue,
                          value,// defrx_covs,
                          defrx_method,
                          defrx_transit_abs,
                          defrx_atol,
                          defrx_rtol,
                          defrx_maxsteps,
                          defrx_hmin,
                          defrx_hmax,
                          defrx_hini,
                          defrx_maxordn,
                          defrx_maxords,
                          defrx_cores,
                          defrx_covs_interpolation,
                          defrx_addCov,
                          defrx_matrix,
                          defrx_sigma,
                          defrx_sigmaDf,
                          defrx_nCoresRV,
                          defrx_sigmaIsChol,
                          defrx_nDisplayProgress,
                          defrx_amountUnits,
                          defrx_timeUnits, 
			  defrx_addDosing);
	} else if (sarg == "t" || sarg == "time"){
	  CharacterVector classattr = obj.attr("class");
          Environment e = as<Environment>(classattr.attr(".RxODE.env"));
	  Function f = as<Function>(e["replace.sampling"]);
	  return f(value);
        } else {
	  CharacterVector classattr = obj.attr("class");
	  Environment e = as<Environment>(classattr.attr(".RxODE.env"));
	  List pars = List(e["params.dat"]);
	  CharacterVector nmp = pars.names();
	  int i, n, np, nc, j;
	  np = (as<NumericVector>(pars[0])).size();
	  RObject covsR = e["covs"];
	  List covs;
	  if (!covsR.isNULL()){
	    covs = List(covsR);
	  }
	  CharacterVector nmc;
	  if (covs.hasAttribute("names")){
	    nmc = covs.names();
	    nc = (as<NumericVector>(covs[0])).size();
	  } else {
	    nc = as<int>(e["nobs"]);
	  }
	  //////////////////////////////////////////////////////////////////////////////
	  // Update Parameters by name
	  n = pars.size();
	  for (i = 0; i < n; i++){
	    if (nmp[i] == sarg){
	      // Update solve.
	      NumericVector val = NumericVector(value);
	      if (val.size() == np){
		// Update Parameter
		pars[i] = val;
		return rxSolveC(obj,
				CharacterVector::create("params"),
				R_NilValue,
				pars, //defrx_params,
				defrx_events,
				defrx_inits,
				R_NilValue,
				defrx_covs,
				defrx_method,
				defrx_transit_abs,
				defrx_atol,
				defrx_rtol,
				defrx_maxsteps,
				defrx_hmin,
				defrx_hmax,
				defrx_hini,
				defrx_maxordn,
				defrx_maxords,
				defrx_cores,
				defrx_covs_interpolation,
				defrx_addCov,
				defrx_matrix,
				defrx_sigma,
				defrx_sigmaDf,
				defrx_nCoresRV,
				defrx_sigmaIsChol,
                                defrx_nDisplayProgress,
				defrx_amountUnits,
				defrx_timeUnits, 
				defrx_addDosing);
	      } else if (val.size() == nc){
		// Change parameter -> Covariate
		List newPars(pars.size()-1);
		CharacterVector newParNames(pars.size()-1);
		for (j = 0; j < i; j++){
		  newPars[j]     = pars[j];
		  newParNames[j] = nmp[j];
		}
		for (j=i+1; j < pars.size(); j++){
		  newPars[j-1]     = pars[j];
		  newParNames[j-1] = nmp[j];
		}
		newPars.attr("names") = newParNames;
		newPars.attr("class") = "data.frame";
		newPars.attr("row.names") = IntegerVector::create(NA_INTEGER,-np);
		List newCovs(covs.size()+1);
		CharacterVector newCovsNames(covs.size()+1);
		for (j = 0; j < covs.size(); j++){
		  newCovs[j]      = covs[j];
		  newCovsNames[j] = nmc[j];
		}
		newCovs[j]      = val;
		newCovsNames[j] = nmp[i];
		newCovs.attr("names") = newCovsNames;
		newCovs.attr("class") = "data.frame";
		newCovs.attr("row.names") = IntegerVector::create(NA_INTEGER,-nc);
		return rxSolveC(obj,
				CharacterVector::create("params","covs"),
				R_NilValue,
				newPars, //defrx_params,
				defrx_events,
				defrx_inits,
				R_NilValue,
				newCovs, //defrx_covs
				defrx_method,
				defrx_transit_abs,
				defrx_atol,
				defrx_rtol,
				defrx_maxsteps,
				defrx_hmin,
				defrx_hmax,
				defrx_hini,
				defrx_maxordn,
				defrx_maxords,
				defrx_cores,
				defrx_covs_interpolation,
				defrx_addCov,
				defrx_matrix,
				defrx_sigma,
				defrx_sigmaDf,
				defrx_nCoresRV,
				defrx_sigmaIsChol,
                                defrx_nDisplayProgress,
				defrx_amountUnits,
				defrx_timeUnits, 
				defrx_addDosing);
	      }
	      return R_NilValue;
	    }
	  }
	  ///////////////////////////////////////////////////////////////////////////////
	  // Update Covariates by covariate name
	  n = covs.size();
	  for (i = 0; i < n; i++){
	    if (nmc[i] == sarg){
	      // Update solve.
	      NumericVector val = NumericVector(value);
	      if (val.size() == nc){
		// Update Covariate
		covs[i]=val;
		return rxSolveC(obj,
				CharacterVector::create("covs"),
				R_NilValue,
				defrx_params,
				defrx_events,
				defrx_inits,
				R_NilValue,
				covs, // defrx_covs,
				defrx_method,
				defrx_transit_abs,
				defrx_atol,
				defrx_rtol,
				defrx_maxsteps,
				defrx_hmin,
				defrx_hmax,
				defrx_hini,
				defrx_maxordn,
				defrx_maxords,
				defrx_cores,
				defrx_covs_interpolation,
				defrx_addCov,
				defrx_matrix,
				defrx_sigma,
				defrx_sigmaDf,
				defrx_nCoresRV,
				defrx_sigmaIsChol,
                                defrx_nDisplayProgress,
				defrx_amountUnits,
				defrx_timeUnits, 
				defrx_addDosing);
	      } else if (val.size() == np){
		// Change Covariate -> Parameter
		List newPars(pars.size()+1);
		CharacterVector newParNames(pars.size()+1);
		for (j = 0; j < pars.size(); j++){
		  newPars[j]     = pars[j];
		  newParNames[j] = nmp[j];
		}
		newPars[j]     = val;
		newParNames[j] = nmc[i];
		newPars.attr("names") = newParNames;
		newPars.attr("class") = "data.frame";
		newPars.attr("row.names") = IntegerVector::create(NA_INTEGER,-np);
		// if ()
		List newCovs(covs.size()-1);
		CharacterVector newCovsNames(covs.size()-1);
		for (j = 0; j < i; j++){
		  newCovs[j]      = covs[j];
		  newCovsNames[j] = nmc[j];
		}
		for (j=i+1; j < covs.size(); j++){
		  newCovs[j-1]      = covs[j];
		  newCovsNames[j-1] = nmc[j];
		}
		newCovs.attr("names") = newCovsNames;
		newCovs.attr("class") = "data.frame";
		newCovs.attr("row.names") = IntegerVector::create(NA_INTEGER,-nc);
		return rxSolveC(obj,
				CharacterVector::create("covs", "params"),
				R_NilValue,
				newPars,//defrx_params,
				defrx_events,
				defrx_inits,
				R_NilValue,
				newCovs, // defrx_covs,
				defrx_method,
				defrx_transit_abs,
				defrx_atol,
				defrx_rtol,
				defrx_maxsteps,
				defrx_hmin,
				defrx_hmax,
				defrx_hini,
				defrx_maxordn,
				defrx_maxords,
				defrx_cores,
				defrx_covs_interpolation,
				defrx_addCov,
				defrx_matrix,
				defrx_sigma,
				defrx_sigmaDf,
				defrx_nCoresRV,
				defrx_sigmaIsChol,
                                defrx_nDisplayProgress,
				defrx_amountUnits,
				defrx_timeUnits, 
				defrx_addDosing);
	      }
	    }
	  }
	  ////////////////////////////////////////////////////////////////////////////////
          // Update Initial Conditions
	  NumericVector ini = NumericVector(e["inits.dat"]);
          CharacterVector nmi = ini.names();
          n = ini.size();
          std::string cur;
	  bool doIt = false;
          for (i = 0; i < n; i++){
            cur = as<std::string>(nmi[i]) + "0";
	    if (cur == sarg){
	      doIt = true;
	    } else {
              cur = as<std::string>(nmi[i]) + ".0";
              if (cur == sarg){
                doIt = true;
	      } else {
		cur = as<std::string>(nmi[i]) + "_0";
                if (cur == sarg){
		  doIt = true;
		} else {
		  cur = as<std::string>(nmi[i]) + "(0)";
                  if (cur == sarg){
                    doIt = true;
                  } else {
		    cur = as<std::string>(nmi[i]) + "[0]";
                    if (cur == sarg){
		      doIt = true;
		    } 
		  }
		}
	      }
	    }
	    if (doIt){
	      cur=as<std::string>(nmi[i]);
              NumericVector ini = NumericVector(e["inits.dat"]);
	      double v = as<double>(value);
	      for (j = 0; j < n; j++){
		if (cur == as<std::string>(nmi[j])){
		  ini[j] = v;
		}
	      }
              return rxSolveC(obj,
			      CharacterVector::create("inits"),
			      R_NilValue,
			      defrx_params,
			      defrx_events,
			      ini,
			      R_NilValue,
			      defrx_covs,
			      defrx_method,
			      defrx_transit_abs,
			      defrx_atol,
			      defrx_rtol,
			      defrx_maxsteps,
			      defrx_hmin,
			      defrx_hmax,
			      defrx_hini,
			      defrx_maxordn,
			      defrx_maxords,
			      defrx_cores,
			      defrx_covs_interpolation,
			      defrx_addCov,
			      defrx_matrix,
			      defrx_sigma,
			      defrx_sigmaDf,
			      defrx_nCoresRV,
			      defrx_sigmaIsChol,
                              defrx_nDisplayProgress,
			      defrx_amountUnits,
			      defrx_timeUnits, 
			      defrx_addDosing);
	    }
	  }
	  return R_NilValue;
	}
      }
    }
  }
  return R_NilValue;
}

extern "C" SEXP rxGetModelLib(const char *s){
  std::string str(s);
  getRxModels();
  if (_rxModels.exists(str)){
    return wrap(_rxModels.get(str));
  } else {
    return R_NilValue;
  }
}

//[[Rcpp::export]]
void rxRmModelLib_(std::string str){
  getRxModels();
  if (_rxModels.exists(str)){
    List trans =(as<List>(as<List>(_rxModels[str]))["trans"]);
    std::string rxlib = as<std::string>(trans["prefix"]);
    _rxModels.remove(str);
    if (_rxModels.exists(rxlib)){
      _rxModels.remove(rxlib);
    }
  }  
}
extern "C" void rxClearFuns();
extern "C" void rxRmModelLib(const char* s){
  std::string str(s);
  rxClearFuns();
  rxRmModelLib_(str);
}

Nullable<Environment> rxRxODEenv(RObject obj){
  if (rxIs(obj, "RxODE")){
    return(as<Environment>(obj));
  } else if (rxIs(obj, "rxSolve")){
    CharacterVector cls = obj.attr("class");
    Environment e = as<Environment>(cls.attr(".RxODE.env"));
    return rxRxODEenv(as<RObject>(e["args.object"]));
  } else if (rxIs(obj, "rxModelVars")){
    List mv = as<List>(obj);
    CharacterVector trans = mv["trans"];
    getRxModels();
    std::string prefix = as<std::string>(trans["prefix"]);
    if (_rxModels.exists(prefix)){
      return as<Environment>(_rxModels[prefix]);
    } else {
      return R_NilValue;
    }
  } else {
    return rxRxODEenv(as<RObject>(rxModelVars(obj)));
  }
}

//' Get RxODE model from object
//' @param obj RxODE family of objects
//' @export
//[[Rcpp::export]]
RObject rxGetRxODE(RObject obj){
  Nullable<Environment> rxode1 = rxRxODEenv(obj);
  if (rxode1.isNull()){
    // FIXME compile if needed.
    stop("Can't figure out the RxODE object");
  } else {
    Environment e = as<Environment>(rxode1);
    e.attr("class") = "RxODE";
    return as<RObject>(e);
  }
}

bool rxVersion_b = false;
CharacterVector rxVersion_;

extern "C" const char *rxVersion(const char *what){
  std::string str(what);
  if (!rxVersion_b){
    Environment RxODE("package:RxODE");
    Function f = as<Function>(RxODE["rxVersion"]);
    rxVersion_ = f();
    rxVersion_b=true;
  }
  return (as<std::string>(rxVersion_[str])).c_str();
}

//' Checks if the RxODE object was built with the current build
//'
//' @inheritParams rxModelVars
//'
//' @return boolean indicating if this was built with current RxODE
//'
//' @export
//[[Rcpp::export]]
bool rxIsCurrent(RObject obj){
  List mv = rxModelVars(obj);
  if (mv.containsElementNamed("version")){
    CharacterVector version = mv["version"];
    const char* cVerC = rxVersion("md5");
    std::string str(cVerC);
    std::string str2 = as<std::string>(version["md5"]);
    if (str2 == str){
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

extern "C" void RxODE_assign_fn_pointers_(const char *mv);

//' Assign pointer based on model variables
//' @param object RxODE family of objects
//' @export
//[[Rcpp::export]]
void rxAssignPtr(SEXP object = R_NilValue){
  List mv=rxModelVars(as<RObject>(object));
  CharacterVector trans = mv["trans"];
    RxODE_assign_fn_pointers_((as<std::string>(trans["model_vars"])).c_str());
    rxUpdateFuns(as<SEXP>(trans));
    getRxSolve_();
    // Update rxModels environment.
    getRxModels();
  
    std::string ptr = as<std::string>(trans["model_vars"]); 
    if (!_rxModels.exists(ptr)){
      _rxModels[ptr] = mv;
    } else if (!rxIsCurrent(as<RObject>(_rxModels[ptr]))) {
      _rxModels[ptr] = mv;
    }
    Nullable<Environment> e1 = rxRxODEenv(object);
    if (!e1.isNull()){
      std::string prefix = as<std::string>(trans["prefix"]);
      if (!_rxModels.exists(prefix)){
        Environment e = as<Environment>(e1);
        _rxModels[prefix] = e;
      }
    }
}

extern "C" void rxAssignPtrC(SEXP obj){
  rxAssignPtr(obj);
}

//' Get the number of cores in a system
//' @export
//[[Rcpp::export]]
IntegerVector rxCores(){
  unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
  return IntegerVector::create((int)(concurentThreadsSupported));
}

//' Return the DLL associated with the RxODE object
//'
//' This will return the dynamic load library or shared object used to
//' run the C code for RxODE.
//'
//' @param obj A RxODE family of objects or a character string of the
//'     model specification or location of a file with a model
//'     specification.
//'
//' @return a path of the library
//'
//' @keywords internal
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
std::string rxDll(RObject obj){
  if (rxIs(obj,"RxODE")){
    Environment e = as<Environment>(obj);
    return as<std::string>((as<List>(e["rxDll"]))["dll"]);
  } else if (rxIs(obj,"rxSolve")) {
    CharacterVector cls = obj.attr("class");
    Environment e = as<Environment>(cls.attr(".RxODE.env"));
    return(as<std::string>(e["dll"]));
  } else if (rxIs(obj, "rxDll")){
    return as<std::string>(as<List>(obj)["dll"]);
  } else if (rxIs(obj, "character")){
    Environment RxODE("package:RxODE");
    Function f = as<Function>(RxODE["rxCompile.character"]);
    RObject newO = f(as<std::string>(obj));
    return(rxDll(newO));
  } else {
    List mv = rxModelVars(obj);
    Nullable<Environment> en = rxRxODEenv(mv);
    if (en.isNull()){
      stop("Can't figure out the DLL for this object");
    } else {
      Environment e = as<Environment>(en);
      return as<std::string>((as<List>(e["rxDll"]))["dll"]);
    }
  }
}

//' Return the C file associated with the RxODE object
//'
//' This will return C code for generating the RxODE DLL.
//'
//' @param obj A RxODE family of objects or a character string of the
//'     model specification or location of a file with a model
//'     specification.
//'
//' @return a path of the library
//'
//' @keywords internal
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
CharacterVector rxC(RObject obj){
  std::string rets;
  CharacterVector ret(1);
  if (rxIs(obj,"RxODE")){
    Environment e = as<Environment>(obj);
    rets = as<std::string>((as<List>(e["rxDll"]))["c"]);
  } else if (rxIs(obj,"rxSolve")) {
    CharacterVector cls = obj.attr("class");
    Environment e = as<Environment>(cls.attr(".RxODE.env"));
    rets = as<std::string>(e["c"]);
  } else if (rxIs(obj, "rxDll")){
    rets = as<std::string>(as<List>(obj)["c"]);
  } else if (rxIs(obj, "character")){
    Environment RxODE("package:RxODE");
    Function f = as<Function>(RxODE["rxCompile.character"]);
    RObject newO = f(as<std::string>(obj));
    rets = rxDll(newO);
  } else {
    List mv = rxModelVars(obj);
    Nullable<Environment> en = rxRxODEenv(mv);
    if (en.isNull()){
      stop("Can't figure out the DLL for this object");
    } else {
      Environment e = as<Environment>(en);
      rets = as<std::string>((as<List>(e["rxDll"]))["dll"]);
    }
  }
  ret[0] = rets;
  ret.attr("class") = "rxC";
  return ret;
}

//' Determine if the DLL associated with the RxODE object is loaded
//'
//' @param obj A RxODE family of objects 
//'
//' @return Boolean returning if the RxODE library is loaded.
//'
//' @keywords internal
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
bool rxIsLoaded(RObject obj){
  if (obj.isNULL()) return false;
  Function isLoaded("is.loaded", R_BaseNamespace);
  List mv = rxModelVars(obj);
  CharacterVector trans = mv["trans"];
  std::string dydt = as<std::string>(trans["ode_solver"]);
  return as<bool>(isLoaded(dydt));
}

//' Load RxODE object
//'
//' @param obj A RxODE family of objects 
//'
//' @return Boolean returning if the RxODE library is loaded.
//'
//' @keywords internal
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
bool rxDynLoad(RObject obj){
  if (!rxIsLoaded(obj)){
    std::string file = rxDll(obj);
    if (fileExists(file)){
      Function dynLoad("dyn.load", R_BaseNamespace);
      dynLoad(file);
    } else {
      Nullable<Environment> e1 = rxRxODEenv(obj);
      if (!e1.isNull()){
	Environment e = as<Environment>(e1);
	Function compile = as<Function>(e["compile"]);
	compile();
      }
    }
  }
  if (rxIsLoaded(obj)){
    rxAssignPtr(obj);
    return true;
  } else {
    return false;
  }
}

//' Unload RxODE object
//'
//' @param obj A RxODE family of objects 
//'
//' @return Boolean returning if the RxODE library is loaded.
//'
//' @keywords internal
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
bool rxDynUnload(RObject obj){
  List mv = rxModelVars(obj);
  CharacterVector trans = mv["trans"];
  std::string ptr = as<std::string>(trans["model_vars"]);
  if (rxIsLoaded(obj)){
    Function dynUnload("dyn.unload", R_BaseNamespace);
    std::string file = rxDll(obj);
    dynUnload(file);
  } 
  rxRmModelLib_(ptr);
  return !(rxIsLoaded(obj));
}

//' Delete the DLL for the model
//'
//' This function deletes the DLL, but doesn't delete the model
//' information in the object.
//'
//' @param obj RxODE family of objects
//'
//' @return A boolean stating if the operation was successful.
//'
//' @author Matthew L.Fidler
//' @export
//[[Rcpp::export]]
bool rxDelete(RObject obj){
  std::string file = rxDll(obj);
  if (rxDynUnload(obj)){
    CharacterVector cfileV = rxC(obj);
    std::string cfile = as<std::string>(cfileV[0]);
    if (fileExists(cfile)) remove(cfile.c_str());
    if (!fileExists(file)) return true;
    if (remove(file.c_str()) == 0) return true;
  }
  return false;
}

arma::mat rwish5(double nu, int p){
  GetRNGstate();
  arma::mat Z(p,p, fill::zeros);
  double curp = nu;
  double tmp =sqrt(Rf_rchisq(curp--));
  Z(0,0) = (tmp < 1e-100) ? 1e-100 : tmp;
  int i, j;
  if (p > 1){
    for (i = 1; i < (int)p; i++){
      tmp = sqrt(Rf_rchisq(curp--));
      Z(i,i) = (tmp < 1e-100) ? 1e-100 : tmp;
      for (j = 0; j < i; j++){
        // row,col
        Z(j,i) = norm_rand();
      }
    }
  }
  PutRNGstate();
  return Z;
}

NumericMatrix cvPost0(double nu, NumericMatrix omega, bool omegaIsChol = false,
		      bool returnChol = false){
  arma::mat S =as<arma::mat>(omega);
  int p = S.n_rows;
  if (p == 1){
    GetRNGstate();
    NumericMatrix ret(1,1);
    if (omegaIsChol){
      ret[0] = nu*omega[0]*omega[0]/(Rf_rgamma(nu/2.0,2.0));
    } else {
      ret[0] = nu*omega[0]/(Rf_rgamma(nu/2.0,2.0));
    }
    if (returnChol) ret[0] = sqrt(ret[0]);
    PutRNGstate();
    return ret;
  } else {
    arma::mat Z = rwish5(nu, p);
    // Backsolve isn't available in armadillo
    arma::mat Z2 = arma::trans(arma::inv(trimatu(Z)));
    arma::mat cv5;
    if (omegaIsChol){
      cv5 = S;
    } else {
      cv5 = arma::chol(S);
    }
    arma::mat mat1 = Z2 * cv5;
    mat1 = mat1.t() * mat1;
    mat1 = mat1 * nu;
    if (returnChol) mat1 = arma::chol(mat1);
    return wrap(mat1);
  }
}

//' Sample a covariance Matrix from the Posteior Inverse Wishart distribution.
//'
//' Note this Inverse wishart rescaled to match the original scale of the covariance matrix.
//'
//' If your covariance matrix is a 1x1 matrix, this uses an scaled inverse chi-squared which 
//' is equivalent to the Inverse Wishart distribution in the uni-directional case.
//'
//' @param nu Degrees of Freedom (Number of Observations) for 
//'        covariance matrix simulation.
//' @param omega Estimate of Covariance matrix.
//' @param n Number of Matricies to sample.  By default this is 1.
//' @param omegaIsChol is an indicator of if the omega matrix is in the cholesky decomposition. 
//' @param returnChol Return the cholesky decomposition of the covariance matrix sample.
//'
//' @return a matrix (n=1) or a list of matricies (n > 1)
//'
//' @author Matthew L.Fidler & Wenping Wang
//' 
//' @export
//[[Rcpp::export]]
RObject cvPost(double nu, RObject omega, int n = 1, bool omegaIsChol = false, bool returnChol = false){
  if (n == 1){
    if (rxIs(omega,"numeric.matrix") || rxIs(omega,"integer.matrix")){
      return as<RObject>(cvPost0(nu, as<NumericMatrix>(omega), omegaIsChol));
    } else if (rxIs(omega, "numeric") || rxIs(omega, "integer")){
      NumericVector om1 = as<NumericVector>(omega);
      if (om1.size() % 2 == 0){
        int n1 = om1.size()/2;
        NumericMatrix om2(n1,n1);
        for (int i = 0; i < om1.size();i++){
          om2[i] = om1[i];
        }
        return as<RObject>(cvPost0(nu, om2, omegaIsChol, returnChol));
      }
    }
  } else {
    List ret(n);
    for (int i = 0; i < n; i++){
      ret[i] = cvPost(nu, omega, 1, omegaIsChol, returnChol);
    }
    return(as<RObject>(ret));
  }
  stop("omega needs to be a matrix or a numberic vector that can be converted to a matrix.");
  return R_NilValue;
}

//' Scaled Inverse Chi Squared distribution
//'
//' @param n Number of random samples
//' @param nu degrees of freedom of inverse chi square
//' @param scale  Scale of inverse chi squared distribution 
//'         (default is 1).
//' @return a vector of inverse chi squared deviates .
//' @export
//[[Rcpp::export]]
NumericVector rinvchisq(const int n = 1, const double &nu = 1.0, const double &scale = 1){
  NumericVector ret(n);
  GetRNGstate();
  for (int i = 0; i < n; i++){
    ret[i] = nu*scale/(Rf_rgamma(nu/2.0,2.0));
  }
  PutRNGstate();
  return ret;
}

//' Simulate Parameters from a Theta/Omega specification
//'
//' @param params Named Vector of RxODE model parameters
//'
//' @param thetaMat Named theta matrix.
//'
//' @param thetaDf The degrees of freedom of a t-distribution for
//'     simulation.  By default this is \code{NULL} which is
//'     equivalent to \code{Inf} degrees, or to simulate from a normal
//'     distribution instead of a t-distribution.
//'
//' @param thetaIsChol Indicates if the \code{theta} supplied is a
//'     Cholesky decomposed matrix instead of the traditional
//'     symmetric matrix.
//'
//' @param nSub Number between subject variabilities (ETAs) simulated for every 
//'        realization of the parameters.
//'
//' @param omega Named omega matrix.
//'
//' @param omegaDf The degrees of freedom of a t-distribution for
//'     simulation.  By default this is \code{NULL} which is
//'     equivalent to \code{Inf} degrees, or to simulate from a normal
//'     distribution instead of a t-distribution.
//'
//' @param omegaIsChol Indicates if the \code{omega} supplied is a
//'     Cholesky decomposed matrix instead of the traditional
//'     symmetric matrix.
//'
//' @param nStud Number virtual studies to characterize uncertainty in estimated 
//'        parameters.
//'
//' @param sigma Matrix for residual variation.  Adds a "NA" value for each of the 
//'     indivdual parameters, residuals are updated after solve is completed. 
//'
//' @inheritParams rxSolve
//'
//' @param simVariability For each study simulate the uncertanty in the Omega and 
//'       Sigma item
//'
//' @param nObs Number of observations to simulate for sigma.
//'
//' @author Matthew L.Fidler
//'
//' @export
//[[Rcpp::export]]
List rxSimThetaOmega(const Nullable<NumericVector> &params    = R_NilValue,
		     const Nullable<NumericMatrix> &omega= R_NilValue,
		     const Nullable<NumericMatrix> &omegaDf= R_NilValue,
		     const bool &omegaIsChol = false,
		     int nSub = 1,
		     const Nullable<NumericMatrix> &thetaMat = R_NilValue,
		     const Nullable<NumericMatrix> &thetaDf  = R_NilValue,
		     const bool &thetaIsChol = false,
		     int nStud = 1,
                     const Nullable<NumericMatrix> sigma = R_NilValue,
		     const Nullable<NumericMatrix> &sigmaDf= R_NilValue,
                     const bool &sigmaIsChol = false,
		     int nCoresRV = 1,
		     int nObs = 1,
                     bool simVariability = true){
  NumericVector par;
  if (params.isNull()){
    stop("This function requires overall parameters.");
  } else {
    par = NumericVector(params);
    if (!par.hasAttribute("names")){
      stop("'params' must be a named vector.");
    }
  }
  bool simSigma = false;
  NumericMatrix sigmaM;
  CharacterVector sigmaN;
  NumericMatrix sigmaMC;
  if (!sigma.isNull() && nObs > 1){
    simSigma = true;
    sigmaM = as<NumericMatrix>(sigma);
    if (!sigmaM.hasAttribute("dimnames")){
      stop("'sigma' must be a named Matrix.");
    }
    if (sigmaIsChol){
      sigmaMC = sigmaM;
    } else {
      sigmaMC = wrap(arma::chol(as<arma::mat>(sigmaM)));
    }
    sigmaN = as<CharacterVector>((as<List>(sigmaM.attr("dimnames")))[1]);
  }  
  int scol = 0;
  if (simSigma){
    scol = sigmaMC.ncol();
    if (nObs*nStud*nSub*scol < 0){
      // nStud = INT_MAX/(nObs*nSub*scol)*0.25;
      stop("Simulation Overflow; Reduce the number of observations, number of subjects or number of studies.");
    }
  }
  NumericMatrix thetaM;
  CharacterVector thetaN;
  bool simTheta = false;
  CharacterVector parN = CharacterVector(par.attr("names"));
  IntegerVector thetaPar(parN.size());
  int i, j, k;
  if (!thetaMat.isNull() && nStud > 1){
    thetaM = as<NumericMatrix>(thetaMat);
    if (!thetaM.hasAttribute("dimnames")){
      stop("'thetaMat' must be a named Matrix.");
    }
    thetaM = as<NumericMatrix>(rxSimSigma(as<RObject>(thetaMat), as<RObject>(thetaDf), nCoresRV, thetaIsChol, nStud));
    thetaN = as<CharacterVector>((as<List>(thetaM.attr("dimnames")))[1]);
    for (i = 0; i < parN.size(); i++){
      thetaPar[i] = -1;
      for (j = 0; j < thetaN.size(); j++){
	if (parN[i] == thetaN[j]){
	  thetaPar[i] = j;
	  break;
	}
      }
    }
    simTheta = true;
  } else if (!thetaMat.isNull() && nStud <= 1){
    warning("'thetaMat' is ignored since nStud <= 1.");
  }
  bool simOmega = false;
  NumericMatrix omegaM;
  CharacterVector omegaN;
  NumericMatrix omegaMC;
  if (!omega.isNull() && nSub > 1){
    simOmega = true;
    omegaM = as<NumericMatrix>(omega);
    if (!omegaM.hasAttribute("dimnames")){
      stop("'omega' must be a named Matrix.");
    }
    if (omegaIsChol){
      omegaMC = omegaM;
    } else {
      omegaMC = wrap(arma::chol(as<arma::mat>(omegaM)));
    }
    omegaN = as<CharacterVector>((as<List>(omegaM.attr("dimnames")))[1]);
  } else if (nSub > 1){
    stop("'omega' is required for multi-subject simulations.");
  }
  // Now create data frame of parameter values
  List omegaList;
  List sigmaList;  
  if (simVariability && nStud > 1){
    if (simOmega) {
      omegaList = cvPost((double)nSub, as<RObject>(omegaMC), nStud,  true, true);
    }
    if (simSigma){
      sigmaList = cvPost((double)nObs, as<RObject>(sigmaMC), nStud,  true, true);
    }
  }
  int pcol = par.size();
  int ocol = 0;
  int ncol = pcol;
  if (simOmega){
    ocol = omegaMC.ncol();
    ncol += ocol;
  }
  NumericMatrix ret1;
  if (simSigma){
    ncol += scol;
    ret1 = NumericMatrix(nObs*nStud*nSub, scol);
  }
  List ret0(ncol);
  NumericVector nm;
  NumericMatrix nm1;
  for (i = 0; i < ncol; i++){
    nm = NumericVector(nSub*nStud);
    ret0[i] = nm;
  }
  for (i = 0; i < nStud; i++){
    for (j = 0; j < pcol; j++){
      nm = ret0[j];
      for (k = 0; k < nSub; k++){
	nm[nSub*i + k] = par[j];
      }
      if (simTheta){
	if(thetaPar[j] != -1){
          for (k = 0; k < nSub; k++){
            nm[nSub*i + k] += thetaM(i, thetaPar[j]);
          }
        }
      }
      ret0[j] = nm;
    }
    // Now Omega Covariates
    if (ocol > 0){
      if (simVariability && nStud > 1){
        // nm = ret0[j]; // parameter column
        nm1 = as<NumericMatrix>(rxSimSigma(as<RObject>(omegaList[i]), as<RObject>(omegaDf), nCoresRV, true, nSub,false));
      } else {
        nm1 = as<NumericMatrix>(rxSimSigma(as<RObject>(omegaMC), as<RObject>(omegaDf), nCoresRV, true, nSub,false));
      }
      for (j=pcol; j < pcol+ocol; j++){
	nm = ret0[j];
	for (k = 0; k < nSub; k++){
	  nm[nSub*i + k] = nm1(k, j-pcol);
	}
	ret0[j] = nm;
      }
    }
    if (scol > 0){
      if (simVariability  && nStud > 1){
	nm1 = as<NumericMatrix>(rxSimSigma(as<RObject>(sigmaList[i]), as<RObject>(sigmaDf), nCoresRV, true, nObs*nSub, false));
      } else {
	nm1 = as<NumericMatrix>(rxSimSigma(as<RObject>(sigmaMC), as<RObject>(sigmaDf), nCoresRV, true, nObs*nSub, false));
      }
      for (j = 0; j < scol; j++){
	for (k = 0; k < nObs*nSub; k++){
	  // ret1 = NumericMatrix(nObs*nStud, scol);
	  ret1(nObs*nSub*i+k, j) = nm1(k, j);
	}
      }
    }
  }
  CharacterVector dfName(ncol);
  for (i = 0; i < pcol; i++){
    dfName[i] = parN[i];
  }
  for (i = pcol; i < pcol+ocol; i++){
    dfName[i] = omegaN[i-pcol];
  }
  for (i = pcol+ocol; i < ncol; i++){
    dfName[i] = sigmaN[i-pcol-ocol];
  }
  ret0.attr("names") = dfName;
  ret0.attr("class") = "data.frame";
  ret0.attr("row.names") = IntegerVector::create(NA_INTEGER,-nSub*nStud);
  ret1.attr("dimnames") = List::create(R_NilValue, sigmaN);
  getRxModels();
  _rxModels[".sigma"] = ret1;
  if (simTheta){
    _rxModels[".theta"] = thetaM;
  }
  if (simVariability && nStud > 1){
    _rxModels[".omegaL"] = omegaList;
    _rxModels[".sigmaL"] =sigmaList;
  }
  return ret0;
}

extern "C" double *rxGetErrs(){
  getRxModels();
  if (_rxModels.exists(".sigma")){
    NumericMatrix sigma = _rxModels[".sigma"];
    return &sigma[0];
  } 
  return NULL;
}

extern "C" int rxGetErrsNcol(){
  getRxModels();
  if (_rxModels.exists(".sigma")){
    NumericMatrix sigma = _rxModels[".sigma"];
    int ret = sigma.ncol();
    return ret;
  } 
  return 0;
}
  
SEXP rxGetFromChar(char *ptr, std::string var){
  std::string str(ptr);
  // Rcout << str << "\n";
  CharacterVector cv(1);
  cv[0] = str;
  List mv = rxModelVars(as<RObject>(cv));
  if (var == ""){
    return wrap(mv);
  } else {
    return wrap(mv[var]);
  }
}

extern "C" SEXP rxModelVarsC(char *ptr){
  // Rcout << "rxModelVars C: ";
  return rxGetFromChar(ptr, "");
}

extern "C" SEXP rxStateNames(char *ptr){
  // Rcout << "State: ";
  return rxGetFromChar(ptr, "state");
}

extern "C" SEXP rxLhsNames(char *ptr){
  // Rcout << "Lhs: ";
  return rxGetFromChar(ptr, "lhs");
}

extern "C" SEXP rxParamNames(char *ptr){
  // Rcout << "Param: ";
  return rxGetFromChar(ptr, "params");
}

extern "C" int rxIsCurrentC(SEXP obj){
  RObject robj = as<RObject>(obj);
  if (robj.isNULL()) return 0;
  bool ret = rxIsCurrent(robj);
  if (ret) return 1;
  return 0;
}


