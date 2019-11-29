#ifndef MYMFS_LIB_COMANDOS_H
#define MYMFS_LIB_COMANDOS_H

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

  const int sizeFileMax = 512000;				//Define o tamanho dos blocos do MyMFS
  const int sizeFileConfig = 51200;				//Define o tamanho maximo do arquivo de configuração
  const int numThreads = 4;						//Define o numero de threads que o MyMFS deve criar
  const string configFileName = "mymfs.config";	//Nome do arquivo de configuração que fica dentro das unidades

  std::mutex monitor_thread; 					//Semaforo para tratar condição de corrida

  /*Struct utilizado para determinar parametros para as funções multithread*/
  struct Diretrizes {
    long inicio;
    long length;
    string path;
    string compress;
  };

  /*Struct para armazenar dados dos arquivos*/
  struct LinhaConfig {
    string arquivo;
    string extensao;
    int quantidade;
    int tamanho;
  };

  /*Verifica se existe a determinade unidade logica do MyMFS configurada*/
  bool mymfsEstaConfigurado(const string& caminhoComando);

  /*Le o arquivo correspondente a unidade logica do MyMFS, recebida na linha de comando,
   * e retorna um vector com as unidades que foram configuradas*/
  vector<string> obterUnidades(const string& path);

  /*Recebe um path e retorna um tipo LinhaConfig contendo o nome e extensao do arquivo*/
  LinhaConfig separarNomeExtensao(string path);

  /*Verifica se o determinado arquivo existe no sistema de arquivos criado pelo MyMFS*/
  bool arquivoExiste(LinhaConfig &linhaConfig, const string& caminhoComando, const string& nomeArquivo);

  /*Função usada pelas threads para escrever simultaneamente na unidade lógica*/
  void escritaParalela(vector<Diretrizes> *d, string filePath, int i, int th);

  /*Função usada pelas threads para ler simultaneamente da unidade lógica*/
  void leituraParalela(vector<Diretrizes> *d, string filePath, int i, int th);

  /*Função usada pelas threads para alimentar um buffer. Usada na função procuraPalavra()*/
  void alimentarBufferParalelo(vector<Diretrizes> *d, stringstream *buffer_out, int i, int th);

  /*Alimenta um buffer de onde serão retiradas 100 linhas*/
  void primeirasLinhas(vector<Diretrizes> &diretrizes, stringstream &buffer_out, int quant);

  /*Alimenta um buffer de onde serão retiradas 100 linhas*/
  void ultimasLinhas(vector<Diretrizes> &diretrizes, stringstream &buffer_out, int quant);

  /*Converte uma linha do arquivo de configuracao para o tipo LinhaConfig*/
  LinhaConfig converterLinhaConfigParaNomeArquivo(string linha);

  /*Comprime uma string*/
  vector<unsigned char> compress_string(vector<unsigned char> str, int compressionlevel = 9);

  /*Descomprime uma string*/
  vector<unsigned char> decompress_string(vector<unsigned char> str);

  /*Poe os caracteres de uma string em maiusculo*/
  string toUpperCase(string text);

  /*Confere um path e adiciona uma '/' no final caso necessario*/
  string getAbsolutePath(string path);

 public:

  /*Configura as n unidades recebidas com o sistema no MyMFS, criando uma unidade lógica com o nome recebido*/
  string config(const string& caminhoComando, int length, char **unidades);

  /*Importa um arquivo para a unidade lógica do MyMFS determinada*/
  string importarArquivo(string caminhoComando, string caminhoArquivoImport);

  /*Exporta um arquivo da unidade lógica do MyMFS determinada*/
  string exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport);

  /*Lista todos os arquivos presentes na unidade lógica do MyMFS determinada*/
  string listAll(string caminhoComando);

  /*Remove um aquivo da unidade lógica do MyMFS determinada*/
  string remove(string caminhoComando, string nomeArquivo);

  /*Remove todos os arquivos de dados e de configurações, além de eliminar a unidade lógica do MyMFS determinada*/
  string removeAll(const string& caminhoComando);

  /*Procura uma palavra em um determinado arquivo que tenha sido importado para a deteminada unidade lógica do MyMFS*/
  string procuraPalavra(string caminhoComando, string palavra, string arquivoAlvo);

  /*Lê as 100 primeiras linhas de um determinado arquivo que tenha sido importado para a unidade lógica do MyMFS*/
  string primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead);

  /*Lê as 100 ultimas linhas de um determinado arquivo que tenha sido importado para a unidade lógica do MyMFS*/
  string ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead);
};

#endif //MYMFS_LIB_COMANDOS_H