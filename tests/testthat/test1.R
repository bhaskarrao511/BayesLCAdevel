context("String length")
library(BayesLCA)
data("Alzheimer")

test_that("Alzheimer data is loaded Correctly", {
  expect_equal(colnames(Alzheimer)[3],"Aggression")
  expect_equal(nrow(Alzheimer),240)
  expect_equal(ncol(Alzheimer),6)
  
})
