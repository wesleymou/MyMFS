//
// Created by Wesley-amigdalite on 24/11/2019.
//

#include "gtest/gtest.h"
#include "utils.h"

#include "comandos.h"
#include "variaveis.h"

class ComparacoesFixture : public ::testing::Test {

 protected:
//  virtual void SetUp() {
//  }
//
//  virtual void TearDown() {
//  }

  Comandos comandos;

};

TEST_F(ComparacoesFixture, List_All_Test) {

    string listAll = comandos.listAll(raidX);

    vector<size_t> quant_lines;
    find_all_occurances(quant_lines, listAll, "\n");

    ASSERT_EQ(quant_lines.size(), quant_arquivos);

    ASSERT_NE(listAll.find(arquivo_txt), string::npos);
    ASSERT_NE(listAll.find(arquivo_txtGrande), string::npos);
    ASSERT_NE(listAll.find(arquivo_imagem), string::npos);
    ASSERT_NE(listAll.find(arquivo_video), string::npos);
    ASSERT_NE(listAll.find(arquivo_binario), string::npos);
    ASSERT_NE(listAll.find(arquivo_primeiras100), string::npos);
    ASSERT_NE(listAll.find(arquivo_ultimas100), string::npos);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_txt) {

    comandos.exportarArquivo(raidX, arquivo_txt, caminho_saida);

    int diff = system(("diff " + caminho_txt + " " + caminho_saida + arquivo_txt).c_str());
    ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_txtGrande) {

    comandos.exportarArquivo(raidX, arquivo_txtGrande, caminho_saida);

    int diff = system(("diff " + caminho_txtGrande + " " + caminho_saida + arquivo_txtGrande).c_str());
    ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_imagem) {

    comandos.exportarArquivo(raidX, arquivo_imagem, caminho_saida);

    int diff = system(("diff " + caminho_imagem + " " + caminho_saida + arquivo_imagem).c_str());
    ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_video) {

    comandos.exportarArquivo(raidX, arquivo_video, caminho_saida);

int diff = system(("diff " + caminho_video + " " + caminho_saida + arquivo_video).c_str());
ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_binario) {

    comandos.exportarArquivo(raidX, arquivo_binario, caminho_saida);

    int diff = system(("diff " + caminho_binario + " " + caminho_saida + arquivo_binario).c_str());
    ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_primeiras100) {

    comandos.exportarArquivo(raidX, arquivo_primeiras100, caminho_saida);

    int diff = system(("diff " + caminho_primeiras100 + " " + caminho_saida + arquivo_primeiras100).c_str());
    ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Export_Test_arquivo_ultimas100) {

    comandos.exportarArquivo(raidX, arquivo_ultimas100, caminho_saida);

    int diff = system(("diff " + caminho_ultimas100 + " " + caminho_saida + arquivo_ultimas100).c_str());
    ASSERT_EQ(diff,0);
}

TEST_F(ComparacoesFixture, Head_Test_primeiras_100) {
    string head = comandos.primeiras100Linhas(raidX, arquivo_primeiras100);

    ASSERT_EQ(head, head_primeiras_100);
}

TEST_F(ComparacoesFixture, Head_Test_arquivo_txt) {
    string head = comandos.primeiras100Linhas(raidX, arquivo_txt);

    ASSERT_EQ(head, head_arquivo_txt);
}

TEST_F(ComparacoesFixture, Tail_Test_ultimas_100) {
    string tail = comandos.ultimas100Linhas(raidX, arquivo_ultimas100);

    ASSERT_EQ(tail, tail_ultimas_100);
}
TEST_F(ComparacoesFixture, Tail_Test_aquivo_txt) {
    string tail = comandos.ultimas100Linhas(raidX, arquivo_txt);

    ASSERT_EQ(tail, tail_aquivo_txt);
}
TEST_F(ComparacoesFixture, Tail_Test_arquivo_txt_grande) {
    string tail = comandos.ultimas100Linhas(raidX, arquivo_txtGrande);

    ASSERT_EQ(tail, tail_arquivo_txt_grande);
}

TEST_F(ComparacoesFixture,greb_abstencao_arquivo_txt_grande){
    string linha = comandos.procuraPalavra(raidX,"abstencao", arquivo_txtGrande);

    ASSERT_EQ(linha, greb_arquivo_txt_grande_abstencao);
}
TEST_F(ComparacoesFixture,greb_perseveranca_arquivo_txt_grande){
    string linha = comandos.procuraPalavra(raidX,"perseveranca", arquivo_txtGrande);

    ASSERT_EQ(linha,greb_arquivo_txt_grande_perseveranca);
}