% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/rxsympy.R
\name{rxSplitLines}
\alias{rxSplitLines}
\title{Split line into multiple lines at + or - breaks}
\usage{
rxSplitLines(lhs, rhs, limit = 1100)
}
\arguments{
\item{lhs}{Left handed side to construct}

\item{rhs}{Right handed side to construct}

\item{limit}{the number of characters for the expression to be
before it is split.  By default this is 1100}
}
\value{
an expression where the lhs is constructed iteratevly by
    splitting the lhs and adding it iteratevly to the lhs.

For example:

lhs = 1 + 2 + 3

Would become

lhs = 1
lhs = lhs + 2
lhs = lhs + 3

When calling rxSplitLines("lhs", "1+2+3", 0)

This is to deal with the unwieldly lines that sometimes come out
of SymPy.
}
\description{
Split line into multiple lines at + or - breaks
}
\author{
Matthew L. Fidler
}
\keyword{internal}
