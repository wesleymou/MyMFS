//
// Created by Wesley-amigdalite on 24/11/2019.
//

#include "gtest/gtest.h"
#include "comandos.h"
#include "variaveis.h"

#include <experimental/filesystem>

namespace fsys = std::experimental::filesystem;

class MymfsFixture : public ::testing::Test {

 protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  Comandos comandos;

};
