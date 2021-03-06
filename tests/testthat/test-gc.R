library(RxODE)
library(testthat)

rxPermissive({
    context("Garbage collection")
    test_that("Check garbage collection unloads DLLs", {
        ode <- RxODE({
            b       = -1
            d/dt(X) = a*X + Y*Z;
            d/dt(Y) = b*(Y - Z);
            d/dt(Z) = -X*Y + c*Y - Z
        })

        name <- basename(rxDll(ode))
        name <- substr(name, 0, nchar(name) - nchar(.Platform$dynlib.ext))

        expect_false(is.null(getLoadedDLLs()[[name]]))
        Sys.sleep(0.5)
        gc()
        Sys.sleep(0.5)
        expect_false(is.null(getLoadedDLLs()[[name]]))
        rm(ode)
        Sys.sleep(0.5)
        gc()
        Sys.sleep(0.5)
        expect_true(is.null(getLoadedDLLs()[[name]]))

    })

})
