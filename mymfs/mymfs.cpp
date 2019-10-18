#include "comandos.h"
#include <iostream>
#include <string>
#include <experimental/filesystem>

namespace fsys = std::experimental::filesystem;
using namespace std;

int main(int argc, char **argv) {

    //Deve possuir no minimo 3 argumentos (nome do programa - passado automaticamente,
    //caminho da unidade X, comando a ser executado)
    if (argc >= 3) {
        string caminhoComando = argv[1]; //Caminho de onde o Mymfs deve ser executado
        string comando = argv[2];        //Comando do Mymfs que deve ser executado

        if (comando == "config") {
            config(caminhoComando);
        }

        else if (comando == "import") {
            if (argc == 4) {
                string caminhoArquivoImport = argv[3];    //Caminho do arquivo a ser importado para o diretório especficiado

                importarArquivo(caminhoComando, caminhoArquivoImport);
            }
            else
                cout << "Comando import incorreto." << endl;
        }

        else if (comando == "export") {
            if (argc == 5) {
                string caminhoArquivoExport = argv[3];    //Caminho do arquivo a ser exportado para o diretório especficiado
                string caminhoDiretorioExport = argv[4];  //Caminho do diretório para onde o arquivo deve ser exportado

                exportarArquivo(caminhoComando, caminhoArquivoExport, caminhoDiretorioExport);
            }
            else
                cout << "Comando export incorreto." << endl;
        }

        else if (comando == "listall") {
            listAll(caminhoComando);
        }

        else if (comando == "remove") {
            if (argc == 4) {
                string caminhoArquivoRemove = argv[3];    //Caminho do arquivo a ser removido

                remove(caminhoComando, caminhoArquivoRemove);
            }
            else
                cout << "Comando remove incorreto." << endl;
        }

        else if (comando == "removeall") {
            removeAll(caminhoComando);
        }

        else if (comando == "grep") {
            if (argc == 5) {
                string palavra = argv[3];    //Palavra que será procurado no arquivo
                string caminhoArquivoToRead = argv[4];  //Caminho do arquivo que será pesquisado

                procuraPalavra(caminhoComando, palavra, caminhoArquivoToRead);
            }
            else
                cout << "Comando grep incorreto." << endl;
        }

        else if (comando == "head100") {
            if (argc == 4) {
                string caminhoArquivoToRead = argv[3];  //Caminho do arquivo que será lido

                primeiras100Linhas(caminhoComando, caminhoArquivoToRead);
            }
            else
                cout << "Comando head100 incorreto." << endl;
        }

        else if (comando == "tail100") {
            if (argc == 4) {
                string caminhoArquivoToRead = argv[3];  //Caminho do arquivo que será lido

                ultimas100Linhas(caminhoComando, caminhoArquivoToRead);
            }
            else
                cout << "Comando tail100 incorreto." << endl;
        }
        else
            cout << "Comando invalido." << endl;
    }
    else {
        cout << "Por favor, informe os argumentos necessarios" << endl;
    }

    return 0;
}