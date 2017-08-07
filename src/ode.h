#define __HD_ODE__ "#include <R.h>\n#include <Rinternals.h>\n#include <Rmath.h>\n#include <R_ext/Rdynload.h>\n#define JAC_Rprintf Rprintf\n#define JAC0_Rprintf if (_jac_counter_val() == 0) Rprintf\n#define ODE_Rprintf Rprintf\n#define ODE0_Rprintf if (_dadt_counter_val() == 0) Rprintf\n#define LHS_Rprintf Rprintf\n#define max(a,b) (((a)>(b))?(a):(b))\n#define min(a,b) (((a)<(b))?(a):(b))\n\n// Types for par pointers.r\ntypedef void (*RxODE_update_par_ptr)(double t);\ntypedef double (*RxODE_transit3)(double t, double n, double mtt);\ntypedef double (*RxODE_fn) (double x);\ntypedef double (*RxODE_fn2) (double x, double y);\ntypedef double (*RxODE_transit4)(double t, double n, double mtt, double bio);\ntypedef double (*RxODE_vec) (int val);\ntypedef long (*RxODE_cnt) ();\ntypedef void (*RxODE_inc) ();\ntypedef double (*RxODE_val) ();\ntypedef SEXP (*RxODE_ode_solver) (SEXP sexp_theta, SEXP sexp_inits, SEXP sexp_lhs, SEXP sexp_time, SEXP sexp_evid,SEXP sexp_dose, SEXP sexp_pcov, SEXP sexp_cov, SEXP sexp_locf, SEXP sexp_atol, SEXP sexp_rtol, SEXP sexp_hmin, SEXP sexp_hmax, SEXP sexp_h0, SEXP sexp_mxordn, SEXP sexp_mxords, SEXP sexp_mx,SEXP sexp_stiff, SEXP sexp_transit_abs, SEXP sexp_object, SEXP sexp_extra_args, SEXP sexp_matrix, SEXP sexp_add_cov);\ntypedef void (*RxODE_assign_fn_pointers)(void (*fun_dydt)(unsigned int, double, double *, double *),void (*fun_calc_lhs)(double, double *, double *),void (*fun_calc_jac)(unsigned int, double, double *, double *, unsigned int),void (*fun_update_inis)(SEXP _ini_sexp),int fun_jt,int fun_mf, int fun_debug);\n\ntypedef void (*RxODE_ode_solver_old_c)(int *neq,double *theta,double *time,int *evid,int *ntime,double *inits,double *dose,double *ret,double *atol,double *rtol,int *stiff,int *transit_abs,int *nlhs,double *lhs,int *rc);\ntypedef void (*RxODE_ode_solver_0_6_c)(int *neq,double *theta,double *time,int *evid,int *ntime,double *inits,double *dose,double *ret,double *atol,double *rtol,int *stiff,int *transit_abs,int *nlhs,double *lhs,int *rc,double hmin, double hmax,double h0,int mxordn,int mxords,int mxstep);\n\ntypedef double (*RxODE_solveLinB)(double t, int linCmt, int diff1, int diff2, double A, double alpha, double B, double beta, double C, double gamma, double ka, double tlag);\n// Give par pointers\nRxODE_vec _par_ptr, _InfusionRate;\nRxODE_update_par_ptr _update_par_ptr;\nRxODE_cnt _dadt_counter_val, _jac_counter_val;\nRxODE_inc _dadt_counter_inc, _jac_counter_inc;\nRxODE_val podo, tlast;\nRxODE_transit4 _transit4;\nRxODE_transit3 _transit3;\nRxODE_fn _safe_log, safe_zero, factorial, _as_zero, abs_log;\nRxODE_fn2 sign_exp;\nRxODE_assign_fn_pointers _assign_fn_pointers;\nRxODE_ode_solver_old_c _old_c;\nRxODE_ode_solver_0_6_c _c_0_6;\nRxODE_solveLinB solveLinB;\n\nextern void __ODE_SOLVER_PTR__();\n\n"
#define __HD_SOLVE__ "extern void __ODE_SOLVER__(\n                    int *neq,\n                    double *theta,      //order:\n                    double *time,\n                    int *evid,\n                    int *ntime,\n                    double *inits,\n                    double *dose,\n                    double *ret,\n                    double *atol,\n                    double *rtol,\n                    int *stiff,\n                    int *transit_abs,\n                    int *nlhs,\n                    double *lhs,\n                    int *rc\n                    ){\n  // Backward compatible ode solver for 0.5* C interface\n  __ODE_SOLVER_PTR__();\n  _old_c(neq, theta, time, evid, ntime, inits, dose, ret, atol, rtol, stiff, transit_abs, nlhs, lhs, rc);\n}\n\nvoid __ODE_SOLVER_0_6__(int *neq,\n                        double *theta,  //order:\n                        double *time,\n                        int *evid,\n                        int *ntime,\n                        double *inits,\n                        double *dose,\n                        double *ret,\n                        double *atol,\n                        double *rtol,\n                        int *stiff,\n                        int *transit_abs,\n                        int *nlhs,\n                        double *lhs,\n                        int *rc,\n                        double hmin,\n                        double hmax,\n                        double h0,\n                        int mxordn,\n                        int mxords,\n                        int mxstep) {\n  // Backward compatible ode solver for 0.5* C interface\n  __ODE_SOLVER_PTR__();\n  _c_0_6(neq, theta, time, evid, ntime, inits, dose, ret, atol, rtol, stiff, transit_abs, nlhs, lhs, rc,\n	hmin, hmax, h0, mxordn, mxords, mxstep);\n}\n\nextern void __ODE_SOLVER_PTR__  (){\n  _assign_fn_pointers(__DYDT__ , __CALC_LHS__ , __CALC_JAC__, __INIS__, __JT__ , __MF__,\n#ifdef __DEBUG__\n                      1\n#else\n                      0\n#endif\n                      );\n}\n\nextern SEXP __ODE_SOLVER_SEXP__ (// Parameters\n                                 SEXP sexp_theta,\n                                 SEXP sexp_inits,\n                                 SEXP sexp_lhs,\n				 // Events\n				 SEXP sexp_time,\n				 SEXP sexp_evid,\n				 SEXP sexp_dose,\n				 // Covariates\n				 SEXP sexp_pcov,\n				 SEXP sexp_cov,\n				 SEXP sexp_locf,\n				 // Solver Options\n				 SEXP sexp_atol,\n				 SEXP sexp_rtol,\n				 SEXP sexp_hmin,\n				 SEXP sexp_hmax,\n				 SEXP sexp_h0,\n				 SEXP sexp_mxordn,\n				 SEXP sexp_mxords,\n				 SEXP sexp_mx,\n				 SEXP sexp_stiff,\n				 SEXP sexp_transit_abs,\n				 // Object Creation\n				 SEXP sexp_object,\n				 SEXP sexp_extra_args,\n				 SEXP sexp_matrix,\n				 SEXP sexp_add_cov){\n  __ODE_SOLVER_PTR__();\n  RxODE_ode_solver ode_solver = (RxODE_ode_solver) R_GetCCallable(\"RxODE\",\"RxODE_ode_solver\");\n  ode_solver(sexp_theta,sexp_inits,sexp_lhs,sexp_time,sexp_evid,sexp_dose,sexp_pcov,sexp_cov,sexp_locf,sexp_atol,\n	      sexp_rtol,sexp_hmin,sexp_hmax,sexp_h0,sexp_mxordn,sexp_mxords,sexp_mx,sexp_stiff,sexp_transit_abs,\n	     sexp_object,sexp_extra_args,sexp_matrix,sexp_add_cov);\n}\n\n//Initilize the dll to match RxODE's calls\nvoid __R_INIT__ (DllInfo *info){\n  // Get the RxODE calling interfaces\n  _InfusionRate   = (RxODE_vec) R_GetCCallable(\"RxODE\",\"RxODE_InfusionRate\");\n  _update_par_ptr = (RxODE_update_par_ptr) R_GetCCallable(\"RxODE\",\"RxODE_update_par_ptr\");\n  _par_ptr = (RxODE_vec) R_GetCCallable(\"RxODE\",\"RxODE_par_ptr\");\n  _dadt_counter_val = (RxODE_cnt) R_GetCCallable(\"RxODE\",\"RxODE_dadt_counter_val\");\n  _jac_counter_val  = (RxODE_cnt) R_GetCCallable(\"RxODE\",\"RxODE_jac_counter_val\");\n  _dadt_counter_inc = (RxODE_inc) R_GetCCallable(\"RxODE\",\"RxODE_dadt_counter_inc\");\n  _jac_counter_inc  = (RxODE_inc) R_GetCCallable(\"RxODE\",\"RxODE_jac_counter_inc\");\n  podo  = (RxODE_val) R_GetCCallable(\"RxODE\",\"RxODE_podo\");\n  tlast = (RxODE_val) R_GetCCallable(\"RxODE\",\"RxODE_tlast\");\n  factorial=(RxODE_fn) R_GetCCallable(\"RxODE\",\"RxODE_factorial\");\n  _transit3 = (RxODE_transit3) R_GetCCallable(\"RxODE\",\"RxODE_transit3\");\n  _transit4 = (RxODE_transit4) R_GetCCallable(\"RxODE\",\"RxODE_transit4\");\n  _safe_log =(RxODE_fn) R_GetCCallable(\"RxODE\",\"RxODE_safe_log\");\n  safe_zero =(RxODE_fn) R_GetCCallable(\"RxODE\",\"RxODE_safe_zero\");\n  _as_zero = (RxODE_fn) R_GetCCallable(\"RxODE\",\"RxODE_as_zero\");\n  _assign_fn_pointers=(RxODE_assign_fn_pointers) R_GetCCallable(\"RxODE\",\"RxODE_assign_fn_pointers\");\n  _old_c = (RxODE_ode_solver_old_c) R_GetCCallable(\"RxODE\",\"RxODE_ode_solver_old_c\");\n  _c_0_6 = (RxODE_ode_solver_0_6_c)R_GetCCallable(\"RxODE\",\"RxODE_ode_solver_0_6_c\");\n  sign_exp = (RxODE_fn2) R_GetCCallable(\"RxODE\",\"RxODE_sign_exp\");\n  abs_log = (RxODE_fn) R_GetCCallable(\"RxODE\",\"RxODE_abs_log\");\n  solveLinB = (RxODE_solveLinB) R_GetCCallable(\"RxODE\",\"RxODE_solveLinB\");\n  // Register the outside functions\n  R_RegisterCCallable(__LIB_STR__,__ODE_SOLVER_STR__,       (DL_FUNC) __ODE_SOLVER__);\n  R_RegisterCCallable(__LIB_STR__,__ODE_SOLVER_SEXP_STR__,  (DL_FUNC) __ODE_SOLVER_SEXP__);\n  R_RegisterCCallable(__LIB_STR__,__ODE_SOLVER_0_6_STR__,   (DL_FUNC) __ODE_SOLVER_0_6__);\n  R_RegisterCCallable(__LIB_STR__,__ODE_SOLVER_PTR_STR__,   (DL_FUNC) __ODE_SOLVER_PTR__);\n\n  /* R_CallMethodDef callMethods[]  = { */\n  /*   {__ODE_SOLVER_PTR_STR__, (DL_FUNC) &__ODE_SOLVER_PTR__, 0}, */\n  /*   {__ODE_SOLVER_SEXP_STR__, (DL_FUNC) &__ODE_SOLVER_SEXP__, 21}, */\n  /*   {NULL, NULL, 0} */\n  /* }; */\n  R_registerRoutines(info, NULL, NULL, NULL, NULL);\n  R_useDynamicSymbols(info,TRUE);\n  // Register the function pointers so if someone directly calls the\n  // ode solvers directly, they use the last loaded RxODE model.\n  __ODE_SOLVER_PTR__();\n}\n"
