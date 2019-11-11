#include "comandos.h"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char **argv) {

    const string help = "Comando incorreto.\n        mymfs [UNIDADE LOGICA] [COMANDO] [...]\n[UL] config [DIRETORIOS...]: Cria um arquivo com os DIRETORIOS do MyMFS\n[UL] import [ARQUIVO]: Importa ARQUIVO para o MyMFS\n[UL] export [ARQUIVO] [DESTINO]: Exporta ARQUIVO do MyMFS para DESTINO\n[UL] listall: Lista os arquivos presentes no MyMFS\n[UL] removeall: Remove todos os arquivos referentes ao MyMFS\n[UL] remove [ARQUIVO]: Remove ARQUIVO do MyMFS\n[UL] head100 [ARQUIVO]: Imprime as primeiras 100 linhas de ARQUIVO\n                            (nao funciona com arquivos binarios)\n[UL] tail100 [ARQUIVO]: Imprime as ultimas 100 linhas de ARQUIVO\n                            (nao funciona com arquivos binarios)\n[UL] grep [TERMO] [ARQUIVO]: Busca e retorna a primeira aparicao de TERMO em ARQUIVO\n\n";

    Comandos comandos;

    //Deve possuir no minimo 3 argumentos (nome do programa - passado automaticamente,
    //caminho da unidade X, comando a ser executado)
    if ((argc >= 3) || (argv[2] == "help")) {
        string caminhoComando = argv[1]; //Caminho de onde o Mymfs deve ser executado
        string comando = argv[2];        //Comando do Mymfs que deve ser executado

        if (comando == "config") {
            if (argc >= 4)
                cout << comandos.config(caminhoComando, argc - 3, argv + 3) << endl;
            else
                cout << help << endl;
        } else if (comando == "import") {
            if (argc == 4) {
                string caminhoArquivoImport = argv[3];    //Caminho do arquivo a ser importado para o diretório especficiado

                cout << comandos.importarArquivo(caminhoComando, caminhoArquivoImport) << endl;
            } else
                cout << help << endl;
        } else if (comando == "export") {
            if (argc == 5) {
                string caminhoArquivoExport = argv[3];    //Caminho do arquivo a ser exportado para o diretório especficiado
                string caminhoDiretorioExport = argv[4];  //Caminho do diretório para onde o arquivo deve ser exportado

                cout << comandos.exportarArquivo(caminhoComando, caminhoArquivoExport, caminhoDiretorioExport) << endl;
            } else
                cout << help << endl;
        } else if (comando == "listall") {
            cout << comandos.listAll(caminhoComando) << endl;
        } else if (comando == "removeall") {
            cout << comandos.removeAll(caminhoComando) << endl;
        } else if (comando == "remove") {
            if (argc == 4) {
                string caminhoArquivoRemove = argv[3];    //Arquivo a ser removido

                cout << comandos.remove(caminhoComando, caminhoArquivoRemove) << endl;
            } else
                cout << help << endl;
        } else if (comando == "grep") {
            if (argc == 5) {
                string palavra = argv[3];    //Palavra que será procurado no arquivo
                string caminhoArquivoToRead = argv[4];  //Caminho do arquivo que será pesquisado

                comandos.procuraPalavra(caminhoComando, palavra, caminhoArquivoToRead);
            } else
                cout << "Comando grep incorreto." << endl;
        } else if (comando == "head100") {
            if (argc == 4) {
                string caminhoArquivoToRead = argv[3];  //Caminho do arquivo que será lido

                comandos.primeiras100Linhas(caminhoComando, caminhoArquivoToRead);
            } else
                cout << "Comando head100 incorreto." << endl;
        } else if (comando == "tail100") {
            if (argc == 4) {
                string caminhoArquivoToRead = argv[3];  //Caminho do arquivo que será lido

                comandos.ultimas100Linhas(caminhoComando, caminhoArquivoToRead);
            } else
                cout << "Comando tail100 incorreto." << endl;
        } else
            cout << "Comando invalido." << endl;
    } else {
        cout << help << endl;
    }

    return 0;
}