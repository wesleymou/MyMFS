#include <vector>
#include <string>
#include <mutex>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

using namespace std;

class Comandos {
 private:

  const int sizeFileMax = 512000;          //Define o tamanho maximo dos arquivos como 500KB
  const int sizeFileConfig = 51200;
  const int numThreads = 4;
  const string configFileName = "mymfs.config";

  std::mutex monitor_thread;

  struct Diretrizes {
    long inicio;
    long length;
    string path;
    string compress;
  };

  struct LinhaConfig {
    string arquivo;
    string extensao;
    int quantidade;
    int tamanho;
  };

  bool mymfsEstaConfigurado(string caminhoComando);

  vector<string> obterUnidades(string path);

  LinhaConfig separarNomeExtensao(string path);

  bool arquivoExiste(LinhaConfig *linhaConfig, string caminhoComando, string nomeArquivo);

  void escritaParalela(vector<Diretrizes> *d, string filePath, int i, int th);

  void leituraParalela(vector<Diretrizes> *d, string filePath, int i, int th);

  void alimentarBufferParalelo(vector<Diretrizes> *d, stringstream *buffer_out, int i, int th);

  void primeirasLinhas(vector<Diretrizes> *d, stringstream *buffer_out, int quant);

  void ultimasLinhas(vector<Diretrizes> *d, stringstream *buffer_out, int quant);

  LinhaConfig converterLinhaConfigParaNomeArquivo(string linhaConfig);

  vector<unsigned char> compress_string(vector<unsigned char> str, int compressionlevel = 9);

  vector<unsigned char> decompress_string(vector<unsigned char> str);

  string toUpperCase(string text);

 public:

  string config(string caminhoComando, int length, char **unidades);

  string importarArquivo(string caminhoComando, string caminhoArquivoImport);

  string exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport);

  string listAll(string caminhoComando);

  string remove(string caminhoComando, string nomeArquivo);

  string removeAll(string caminhoComando);

  string procuraPalavra(string caminhoComando, string palavra, string arquivoAlvo);

  string primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead);

  string ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead);
};