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
//  virtual void SetUp() {
//  }
//
//  virtual void TearDown() {
//  }

  Comandos comandos;

};

TEST_F(ConfigFixture, config_Test) {

    comandos.removeAll(raidX);

    ASSERT_FALSE(fsys::exists(raidX));

    ASSERT_TRUE(fsys::exists(volD));
    ASSERT_TRUE(fsys::exists(volE));
    ASSERT_TRUE(fsys::exists(volF));
    ASSERT_TRUE(fsys::exists(volG));
    ASSERT_TRUE(fsys::exists(volH));

    ASSERT_TRUE(fsys::is_empty(volD));
    ASSERT_TRUE(fsys::is_empty(volE));
    ASSERT_TRUE(fsys::is_empty(volF));
    ASSERT_TRUE(fsys::is_empty(volG));
    ASSERT_TRUE(fsys::is_empty(volH));

    comandos.config(raidX, 5, unidades);

    ASSERT_TRUE(fsys::exists(raidX));

    ASSERT_TRUE(fsys::exists(volD + "mymfs.config"));
    ASSERT_TRUE(fsys::exists(volE + "mymfs.config"));
}
