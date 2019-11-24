//
// Created by Wesley-amigdalite on 23/11/2019.
//

#include "gtest/gtest.h"
#include "comandos.h"
#include "variaveis.h"

#include <experimental/filesystem>

namespace fsys = std::experimental::filesystem;

class ConfigFixture : public ::testing::Test {

 protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  Comandos comandos;

};

TEST_F(ConfigFixture, config) {
    ASSERT_FALSE(fsys::exists(raidX));

    ASSERT_TRUE(fsys::exists(volD));
    ASSERT_TRUE(fsys::exists(volE));
    ASSERT_TRUE(fsys::exists(volF));

    ASSERT_TRUE(fsys::is_empty(volD));
    ASSERT_TRUE(fsys::is_empty(volE));
    ASSERT_TRUE(fsys::is_empty(volF));

//    char* unidades[] = {volD.data(),volE.data(),volF.data(),volG.data(),volH.data()};
//    comandos.config(raidX, 5,unidades);
}
