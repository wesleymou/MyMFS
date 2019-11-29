//
// Created by Wesley-amigdalite on 24/11/2019.
//

#include "gtest/gtest.h"
#include "comandos.h"
#include "variaveis.h"

#include <experimental/filesystem>
#include <fstream>
#include <cmath>

namespace fsys = std::experimental::filesystem;

class ImportFixture : public ::testing::Test {

 protected:
//  virtual void SetUp() {
//  }
//
//  virtual void TearDown() {
//  }

  Comandos comandos;

};


TEST_F(ImportFixture, Import_Test_Txt) {
    string arquivo = caminho_txt;
    string file_folder = "files/txt-texto";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}

TEST_F(ImportFixture, Import_Test_Txt_Grande) {
    string arquivo = caminho_txtGrande;
    string file_folder = "files/txt-texto-grande";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}

TEST_F(ImportFixture, Import_Test_Imagem) {
    string arquivo = caminho_imagem;
    string file_folder = "files/jpg-imagem";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}

TEST_F(ImportFixture, Import_Test_Video) {
    string arquivo = caminho_video;
    string file_folder = "files/mp4-video";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}

TEST_F(ImportFixture, Import_Test_Binario) {
    string arquivo = caminho_binario;
    string file_folder = "files/bin-binario";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}

TEST_F(ImportFixture, Import_Test_Primeiras_100) {
    string arquivo = caminho_primeiras100;
    string file_folder = "files/txt-primeiras-100";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}

TEST_F(ImportFixture, Import_Test_Ultimas_100) {
    string arquivo = caminho_ultimas100;
    string file_folder = "files/txt-ultimas-100";

    ifstream file(arquivo, ios_base::ate);
    streamoff size_file = file.tellg();
    file.close();

    int quant_files = ceil((float) size_file / (float) 512000);

    for (auto unidade : unidades) {
        ASSERT_FALSE(fsys::exists(unidade + file_folder));
    }

    comandos.importarArquivo(raidX, arquivo);

    int cont;
    if (quant_files>=quant_unidades){
        cont = quant_unidades;
    } else{
        cont = quant_files;
    }

    for (int i = 0; i<cont; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i] + file_folder));
    }

    for (int i = 0; i < quant_files; ++i) {
        ASSERT_TRUE(fsys::exists(unidades[i % quant_unidades] + file_folder + "/" + to_string(i)));
        ASSERT_TRUE(fsys::exists(unidades[(i + 1) % quant_unidades] + file_folder + "/" + to_string(i) + ".compress"));
    }
}
