#include <zlib.h>
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <experimental/filesystem>
#include <cstring>

namespace fsys = std::experimental::filesystem;
using namespace std;

class Comandos {
 private:
  const int sizeFileMax = 512000;          //Define o tamanho maximo dos arquivos como 500KB
  const int sizeFileConfig = 51200;

  struct Diretrizes {
    int inicio;
    int length;
    string path;
    string compress;
  };

  bool mymfsEstaConfigurado(string caminhoComando);

  vector<string> obterUnidades(string path);

  string *nomeExtensao(string path);

  void escritaParalela(Diretrizes diretrizes[], int numArquivos, string filePath, int i, int th);

 public:

  string config(string caminhoComando, int length, char **unidades);

  string importarArquivo(string caminhoComando, string caminhoArquivoImport);

  string verificarArquivoExisteEmConfig(string caminhoComando, string nomeArquivo);

  void exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport);

  void listAll(string caminhoComando);

  string converterLinhaConfigParaNomeArquivo(string linhaConfig);

  void remove(string caminhoComando, string nomeArquivo);

  void removeAll(string caminhoComando);

  void procuraPalavra(string caminhoComando, string palavra, string caminhoArquivoToRead);

  void primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead);

  void ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead);
};