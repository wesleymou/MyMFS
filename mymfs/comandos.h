#include <vector>
#include <string>

using namespace std;

class Comandos {
 private:
  const int sizeFileMax = 512000;          //Define o tamanho maximo dos arquivos como 500KB
  const int sizeFileConfig = 51200;
  const int numThreads = 4;

  struct Diretrizes {
    int inicio;
    int length;
    string path;
    string compress;
  };

  struct LinhaConfig {
    string arquivo;
    string extensao;
    int quantidade;
  };

  bool mymfsEstaConfigurado(string caminhoComando);

  vector<string> obterUnidades(string path);

  string *nomeExtensao(string path);

  int verificarArquivoExisteEmConfig(LinhaConfig *linhaConfig, string caminhoComando, string nomeArquivo);

  void escritaParalela(vector<Diretrizes> *d, string filePath, int i, int th);

  void leituraParalela(vector<Diretrizes> *d, string filePath, int i, int th);

 public:

  string config(string caminhoComando, int length, char **unidades);

  string importarArquivo(string caminhoComando, string caminhoArquivoImport);

  string exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport);

  void listAll(string caminhoComando);

  string converterLinhaConfigParaNomeArquivo(string linhaConfig);

  void remove(string caminhoComando, string nomeArquivo);

  void removeAll(string caminhoComando);

  void procuraPalavra(string caminhoComando, string palavra, string caminhoArquivoToRead);

  void primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead);

  void ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead);
};