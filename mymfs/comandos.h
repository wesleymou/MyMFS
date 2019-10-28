
#include <cstring>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <experimental/filesystem>

namespace fsys = std::experimental::filesystem;
using namespace std;

class Comandos {
private:
    const int sizeFileMax = 512000;          //Define o tamanho maximo dos arquivos como 500KB
    const int sizeFileConfig = 51200;

public:
    bool mymfsEstaConfigurado(string caminhoComando) {
        if (caminhoComando.empty())
            return false;
        return fsys::exists(caminhoComando + "/mymfs.config");
    }

    bool arquivoEstaVazio(std::ifstream& pFile) {
        return pFile.peek() == std::ifstream::traits_type::eof();
    }

    string config(string caminhoComando) {
        if (mymfsEstaConfigurado(caminhoComando)) {   //Verifica se o arquivo Config ja existe no caminho especificado
            return "O Mymfs ja esta configurado nesta unidade.";
        }
        else {
            ofstream arquivoConfig(caminhoComando + "/mymfs.config");       //Cria o arquivo config, configurando o Mymfs na unidade especificada
            arquivoConfig.close();
            return "O Mymfs foi configurado nesta unidade com sucesso.";
        }
    }

    string importarArquivo(string caminhoComando, string caminhoArquivoImport) {
        streampos end, pos;
        if (mymfsEstaConfigurado(caminhoComando)) {
            ifstream arqConfigExiste(caminhoComando + "/mymfs.config"); // Verifica se o arquivo Config existe
            arqConfigExiste.seekg(0, ios::end);
            end = arqConfigExiste.tellg();
            arqConfigExiste.close();

        }
        else {
            return "O Mymfs nao esta configurado na unidade informada."; //Caso nao exista, nao permite a importacao
        }

        if (end < this->sizeFileConfig) {                          //Apenas permite a importação se o arquivo de config for menor que 50KB

            if (!fsys::exists(caminhoComando + "/files")) {
                fsys::create_directory(caminhoComando + "/files");
            }

            if (fsys::exists(caminhoArquivoImport)) {
                ifstream infile(caminhoArquivoImport, ifstream::binary);
                infile.seekg(0, ios::end);
                int numArquivos = ceil((infile.tellg()) / this->sizeFileMax); // Verifica quantos arquivos de 500 KB ou menos serão criados
                infile.seekg(0, ios::beg);

                //Obtem o nome do diretório a ser criado para o arquivo atraves do seu nome
                string nomeArquivo = caminhoArquivoImport;

                const size_t last_slash_idx = nomeArquivo.find_last_of("/");
                if (std::string::npos != last_slash_idx)
                {
                    nomeArquivo.erase(0, last_slash_idx + 1);
                }

                string nomeDiretorio = nomeArquivo.substr(nomeArquivo.find_last_of(".")) + "-" + nomeDiretorio.substr(0, nomeDiretorio.find("."));


                string caminhoDiretorio = caminhoComando + "/files/" + nomeDiretorio;

                if (!fsys::exists(caminhoDiretorio)){ //Caso o diretorio já exista, retorna um erro
                    fsys::create_directory(caminhoDiretorio);
                    char* buffer;

                    for (int i = 0; i < numArquivos; i++) {//Cria os arquivos de 500KB ou menos
                        auto s = to_string(i);
                        ofstream outfile(caminhoDiretorio + "/" + s + ".txt", ofstream::binary);

                        if (i != numArquivos - 1) {  //Efetua a divisao do arquivo de importação em arquivos de 500KB ou menos
                            infile.seekg(pos);
                            buffer = new char[this->sizeFileMax];
                            infile.read(buffer, this->sizeFileMax);
                            pos = infile.tellg();
                            outfile.write(buffer, this->sizeFileMax);
                            delete[] buffer;
                        }
                        else {
                            infile.seekg(pos);
                            int finalSize = (end - pos);
                            buffer = new char[finalSize];
                            infile.read(buffer, finalSize);
                            pos = infile.tellg();
                            outfile.write(buffer, finalSize);
                            delete[] buffer;
                        }
                        outfile.close();
                    }

                    ofstream arqConfig(caminhoComando + "/mymfs.config", ios_base::app | ios_base::out);
                    ifstream arqConfigVazio(caminhoComando + "/mymfs.config");
                    string linhaConfig;
                    linhaConfig = nomeDiretorio + " " + to_string(numArquivos) + "\n";
                    arqConfigVazio.close();
                    arqConfig << linhaConfig; //Adiciona o arquivo importado no arquivo de configuração (nomeArquivo;quantidadeArquivos)
                    arqConfig.close();
                    return "Arquivo importado para o Mymfs com sucesso!";
                }
                else {
                    return "Operacao nao realizada! Um arquivo com esse nome ja existe no Mymfs.";
                }

                infile.close();
            }
            else {
                return "Operacao nao realizada! Arquivo não encontrado.";
            }
        }
        else {
            return "Operacao nao realizada! Arquivo mymfs.config esta cheio - Mymfs.";
        }
    }

    string verificarArquivoExisteEmConfig(string caminhoComando, string nomeArquivo) {

        ifstream arqConfig(caminhoComando + "/mymfs.config");

        string extensaoArquivo = nomeArquivo.substr(nomeArquivo.find(".") + 1, (nomeArquivo.size() - nomeArquivo.find(".")));
        string nomeDiretorioBuscado = extensaoArquivo + "-" + nomeArquivo.substr(0, nomeArquivo.find("."));   //Obtem o nome do diretorio atraves do nome do arquivo
        string nomeDiretorioEncontrado;
        string qtdArquivosEncontrado;
        string linhaConfig;

        do {
            //Percorre o arquivo config até o final ou até encontrar o arquivo a ser exportado
            getline(arqConfig, linhaConfig);
            nomeDiretorioEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
            qtdArquivosEncontrado = linhaConfig.substr(linhaConfig.find(" ") + 1,
                                                       (linhaConfig.size() - linhaConfig.find(" ")));
        } while (strcmp(nomeDiretorioEncontrado.c_str(), nomeDiretorioBuscado.c_str()) != 0 &&
                 !arqConfig.eof() && !nomeDiretorioEncontrado.empty());

        if (strcmp(nomeDiretorioEncontrado.c_str(), nomeDiretorioBuscado.c_str()) == 0 &&
            !nomeDiretorioEncontrado.empty() && !qtdArquivosEncontrado.empty()) {
            return linhaConfig;
        }

        return "";
    }

    void exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport) {
        ifstream arqConfigExiste(caminhoComando + "/mymfs.config");

        //Verifica se o arquivo de configuração e se o arquivo a ser exportado existem
        if (!nomeArquivoExport.empty() && !caminhoDiretorioExport.empty() && mymfsEstaConfigurado(caminhoComando)) {
            string nomeDiretorioEncontrado;
            string qtdArquivosEncontrado;

            //Percorre o arquivoConfig para obter a linha de configuração do arquivo a ser exportado
            string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, nomeArquivoExport);


            //Verifica se encontrou o diretorio do arquivo a ser exportado
            if (!linhaConfig.empty()) {

                nomeDiretorioEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
                qtdArquivosEncontrado = linhaConfig.substr(linhaConfig.find(" ") + 1,
                                                           (linhaConfig.size() - linhaConfig.find(" ")));

                if (!nomeDiretorioEncontrado.empty() && !qtdArquivosEncontrado.empty()) {

                    int numArquivos = stoi(qtdArquivosEncontrado);
                    if (fsys::exists(caminhoDiretorioExport + "/" + nomeArquivoExport)) { //Verifica se o arquivo a ser exportado existe
                        cout << "Operacao nao realizada! O arquivo <" << nomeArquivoExport << "> ja existe na pasta destino" << endl;
                    }
                    else {
                        //Caso exista, cria um arquivo no diretorio informado concatenando todos os arquivos de 500KB
                        ofstream combined_file(caminhoDiretorioExport + "/" + nomeArquivoExport);
                        for (int i = 0; i < numArquivos; i++) {
                            auto s = to_string(i);
                            s = s + ".txt";
                            //Percorre os arquivos de 0 a numArquivos concatenando-os no arquivo exportado
                            ifstream srce_file(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + s);
                            if (srce_file) {
                                combined_file << srce_file.rdbuf();
                            }
                            else {
                                cerr << "Ocorreu um erro. O arquivo nao pode ser aberto " << s << '\n';
                            }
                            srce_file.close();
                        }
                        combined_file.close();
                        cout << "Arquivo <" << nomeArquivoExport << "> foi exportado para <" << caminhoDiretorioExport << "> com sucesso." << endl;
                    }
                }
                else {
                    cout << "Operacao nao realizada! Erro ao ler diretório ou número de arquivos do retorno do mymsf.config" << endl;
                }
            }
            else {
                cout << "Operacao nao realizada! O arquivo <" << nomeArquivoExport << "> nao existe no Mymfs" << endl;
            }
        }
        else {
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
        }
    }

    void listAll(string caminhoComando) {
        //Valida se o arquivo config existe no diretorio especificado
        ifstream arqConfig(caminhoComando + "/mymfs.config");
        if (mymfsEstaConfigurado(caminhoComando)) {
            //Caso exista, percorre o arquivo buscando os nomes dos diretorios/arquivos e listando-os
            string nomeDiretorioEncontrado = "x";
            string qtdArquivosEncontrado = "";
            string linhaConfig = "";
            getline(arqConfig, linhaConfig);
            arqConfig.seekg(0, ios::beg);
            if (linhaConfig.length() > 0) {
                do {
                    getline(arqConfig, linhaConfig);
                    if (linhaConfig.length() > 0) {
                        //Caso existam registros no arquivo config, eles serão buscados e exibidos
                        nomeDiretorioEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
                        string extensaoDiretorio = linhaConfig.substr(0, linhaConfig.find("-"));
                        nomeDiretorioEncontrado = nomeDiretorioEncontrado.substr(nomeDiretorioEncontrado.find("-") + 1, (nomeDiretorioEncontrado.size() - nomeDiretorioEncontrado.find("-")));
                        //Exibe nome do diretório/arquivo
                        cout << nomeDiretorioEncontrado + "." << extensaoDiretorio << endl;
                    }
                } while (!arqConfig.eof());
            }
            else {
                cout << "Nao ha arquivos salvos pelo Mymfs!" << endl;
            }
        }
        else {
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
        }
    }

    string converterLinhaConfigParaNomeArquivo(string linhaConfig) {
        string nomeArquivoEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
        string extensaoArquivoEncontrado = nomeArquivoEncontrado.substr(0, nomeArquivoEncontrado.find("-"));
        nomeArquivoEncontrado = nomeArquivoEncontrado.substr(nomeArquivoEncontrado.find("-") + 1, (nomeArquivoEncontrado.size() - nomeArquivoEncontrado.find("-")));
        return nomeArquivoEncontrado + "." + extensaoArquivoEncontrado;
    }

    void remove(string caminhoComando, string nomeArquivo) {
        ifstream arqConfig(caminhoComando + "/mymfs.config");

        if (nomeArquivo.empty()) {
            cout << "Deve-se informar o nome do arquivo";
            return;
        }

        if (mymfsEstaConfigurado(caminhoComando)) {
            string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, nomeArquivo);
            string linhaConfigNovo = "";
            string configNovo = "";
            string nomeArquivoEncontrado = "";

            if (!linhaConfig.empty()) {
                do {
                    getline(arqConfig, linhaConfigNovo);
                    if (linhaConfigNovo.length() > 0) {
                        nomeArquivoEncontrado = converterLinhaConfigParaNomeArquivo(linhaConfigNovo);
                        if ((strcmp(nomeArquivoEncontrado.c_str(), nomeArquivo.c_str())) != 0) {
                            configNovo += (linhaConfigNovo + "\n");
                        }
                    }
                } while (!arqConfig.eof());

                arqConfig.close();

                ofstream arquivoConfig(caminhoComando + "/mymfs.config", std::ofstream::out | std::ofstream::trunc);
                arquivoConfig << configNovo;
                arquivoConfig.close();

                string diretorioArquivoRemover = linhaConfig.substr(0, linhaConfig.find(" "));
                fsys::remove_all(caminhoComando + "/files/" + diretorioArquivoRemover);

                cout << "O arquivo <" + nomeArquivo + "> foi removido com sucesso." << endl;
            }
            else {
                arqConfig.close();
                cout << "Operacao nao realizada! O arquivo <" << nomeArquivo << "> nao existe no Mymfs" << endl;
            }
        }
        else {
            arqConfig.close();
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
        }
    }

    void removeAll(string caminhoComando) {
        ofstream arquivoConfig;
        if (mymfsEstaConfigurado(caminhoComando)) {
            if (fsys::exists(caminhoComando + "/files")) {
                fsys::remove_all(caminhoComando + "/files");
                arquivoConfig.open(caminhoComando + "/mymfs.config", std::ofstream::out | std::ofstream::trunc);
                arquivoConfig.close();
                cout << "Os arquivos do Mymfs foram removidos com sucesso." << endl;
                return;
            }
            else {
                cout << "Operacao nao realizada! O Mymfs nao possui arquivos gravados, portanto nao foram removidos.";
            }
        }
        else {
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
        }
    }

    void procuraPalavra(string caminhoComando, string palavra, string caminhoArquivoToRead) {
        ifstream arqConfigExiste(caminhoComando + "/mymfs.config");

        //Verifica se o arquivo de configuração e se o arquivo a ser exportado existem
        if (!caminhoArquivoToRead.empty() && mymfsEstaConfigurado(caminhoComando)) {
            string nomeDiretorioEncontrado;
            string qtdArquivosEncontrado;

            //Percorre o arquivoConfig para obter a linha de configuração do arquivo a ser exportado
            string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, caminhoArquivoToRead);


            //Verifica se encontrou o diretorio do arquivo a ser exportado
            if (!linhaConfig.empty()) {

                nomeDiretorioEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
                qtdArquivosEncontrado = linhaConfig.substr(linhaConfig.find(" ") + 1,
                                                           (linhaConfig.size() - linhaConfig.find(" ")));

                if (!nomeDiretorioEncontrado.empty() && !qtdArquivosEncontrado.empty()) {

                    int numArquivos = stoi(qtdArquivosEncontrado);
                    int contaLinha = 0;
                    string linha = "";
                    string ultimaLinha = "";
                    int i;
                    for (i = 0; i < numArquivos; i++) {
                        auto s = to_string(i);
                        s = s + ".txt";
                        ifstream arqPesquisa(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + s);
                        if (arqPesquisa) {
                            getline(arqPesquisa, linha);
                            do {
                                contaLinha++;
                                if (ultimaLinha.length() > 0) {
                                    linha = ultimaLinha + linha;
                                    ultimaLinha = "";
                                }
                                if (linha.find(palavra) != -1) {
                                    cout << "Encontrado " << contaLinha << '\n' << endl;
                                    i = numArquivos + 1;
                                    return;
                                }
                                getline(arqPesquisa, linha);
                            } while (!arqPesquisa.eof());
                            if (arqPesquisa.eof()) {
                                if (linha.length() > 0) {
                                    if (i == numArquivos - 1) {
                                        if (linha.find(palavra) != -1) {
                                            contaLinha++;
                                            cout << "Encontrado " << contaLinha << '\n' << endl;
                                            i = numArquivos + 1;
                                            return;
                                        }
                                    }
                                    else
                                        ultimaLinha = linha;
                                }
                            }
                        }
                        else {
                            cerr << "Ocorreu um erro. O arquivo nao pode ser aberto " << s << '\n';
                        }
                        arqPesquisa.close();
                    }
                    if (i == numArquivos) {
                        cout << "Nao Encontrado" << endl;
                    }
                }
                else {
                    cout << "Operacao nao realizada! Erro ao ler diretório ou numero de arquivos do retorno do mymsf.config" << endl;
                }
            }
            else {
                cout << "Operacao nao realizada! O arquivo <" << caminhoArquivoToRead << "> nao existe no Mymfs" << endl;
            }
        }
        else
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }

    void primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead) {

        ifstream arqConfigExiste(caminhoComando + "/mymfs.config");

        //Verifica se o arquivo de configuração e se o arquivo a ser exportado existem
        if (!caminhoArquivoToRead.empty() && mymfsEstaConfigurado(caminhoComando)) {
            string nomeDiretorioEncontrado;
            string qtdArquivosEncontrado;

            //Percorre o arquivoConfig para obter a linha de configuração do arquivo a ser exportado
            string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, caminhoArquivoToRead);


            //Verifica se encontrou o diretorio do arquivo a ser exportado
            if (!linhaConfig.empty()) {

                nomeDiretorioEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
                qtdArquivosEncontrado = linhaConfig.substr(linhaConfig.find(" ") + 1,
                                                           (linhaConfig.size() - linhaConfig.find(" ")));

                if (!nomeDiretorioEncontrado.empty() && !qtdArquivosEncontrado.empty()) {

                    int numArquivos = stoi(qtdArquivosEncontrado);
                    int contaLinha = 0;
                    string linha = "";
                    string ultimaLinha = "";
                    for (int i = 0; i < numArquivos; i++) {
                        auto s = to_string(i);
                        s = s + ".txt";
                        //Percorre os arquivos de 0 a numArquivos concatenando-os no arquivo exportado
                        ifstream arqPesquisa(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + s);
                        if (arqPesquisa) {
                            int loop = 0;
                            int condicao = 100 - contaLinha;
                            getline(arqPesquisa, linha);
                            do {
                                contaLinha++;
                                loop++;
                                if (ultimaLinha.length() > 0) {
                                    linha = ultimaLinha + linha;
                                    ultimaLinha = "";
                                    cout << linha << endl;
                                }
                                else
                                    cout << linha << endl;
                                getline(arqPesquisa, linha);
                            } while (loop < condicao && !arqPesquisa.eof());
                            if (arqPesquisa.eof()) {
                                if (i == numArquivos - 1) {
                                    cout << linha << endl;
                                }
                                else if ((linha.length() > 0 && i < numArquivos - 1)) {
                                    ultimaLinha = linha;
                                }
                            }
                            if (contaLinha == 100)
                                i = numArquivos;
                        }
                        else {
                            cerr << "Ocorreu um erro. O arquivo nao pode ser aberto " << s << '\n';
                        }
                        arqPesquisa.close();
                    }
                }
                else {
                    cout << "Operacao nao realizada! Erro ao ler diretório ou número de arquivos do retorno do mymsf.config" << endl;
                }
            }
            else {
                cout << "Operacao nao realizada! O arquivo <" << caminhoArquivoToRead << "> nao existe no Mymfs" << endl;
            }
        }
        else {
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
        }

    }

    void ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead) {
        ifstream arqConfigExiste(caminhoComando + "/mymfs.config");

        //Verifica se o arquivo de configuração e se o arquivo existem
        if (!caminhoArquivoToRead.empty() && mymfsEstaConfigurado(caminhoComando)) {
            string nomeDiretorioEncontrado;
            string qtdArquivosEncontrado;

            //Percorre o arquivoConfig para obter a linha de configuração do arquivo
            string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, caminhoArquivoToRead);


            //Verifica se encontrou o diretorio do arquivo
            if (!linhaConfig.empty()) {

                nomeDiretorioEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
                qtdArquivosEncontrado = linhaConfig.substr(linhaConfig.find(" ") + 1,
                                                           (linhaConfig.size() - linhaConfig.find(" ")));

                if (!nomeDiretorioEncontrado.empty() && !qtdArquivosEncontrado.empty()) {

                    int numArquivos = stoi(qtdArquivosEncontrado);
                    int contaLinha = 0;
                    int linhasUltimoArquivo = 0;
                    string linha = "";
                    string ultimaLinha = "";
                    for (int i = numArquivos - 1; i >= 0; i--) {
                        linhasUltimoArquivo = 0;
                        auto s = to_string(i);
                        s = s + ".txt";
                        ifstream arqPesquisa(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + s);
                        if (arqPesquisa) {
                            getline(arqPesquisa, linha);
                            do {
                                linhasUltimoArquivo++;
                                getline(arqPesquisa, linha);
                            } while (!arqPesquisa.eof());
                            if (i == numArquivos - 1) {
                                if (linha.length() > 0) {
                                    linhasUltimoArquivo++;
                                }
                            }
                        }
                        arqPesquisa.close();
                        if (linhasUltimoArquivo >= 100 && contaLinha == 0) {
                            int comecaLeitura = linhasUltimoArquivo - 100;
                            ifstream arqPesquisa(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + s);
                            if (arqPesquisa) {
                                while (!arqPesquisa.eof()) {
                                    getline(arqPesquisa, linha);
                                    if (comecaLeitura == 0)
                                        cout << linha << endl;
                                    else
                                        comecaLeitura--;
                                }
                                i = -2;
                            }
                            arqPesquisa.close();
                        }
                        else if ((linhasUltimoArquivo + contaLinha) >= 100) {
                            int faltaCompletarContaLinha = 100 - contaLinha;
                            int comecaLeitura = linhasUltimoArquivo - faltaCompletarContaLinha;
                            for (int j = i; j < numArquivos; j++) {
                                auto k = to_string(j);
                                k = k + ".txt";
                                ifstream arqPesquisa(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + k);
                                if (arqPesquisa) {
                                    getline(arqPesquisa, linha);
                                    do {
                                        if (comecaLeitura == 0) {
                                            if (ultimaLinha.length() > 0) {
                                                linha = ultimaLinha + linha;
                                                ultimaLinha = "";
                                                cout << linha << endl;
                                            }
                                            else
                                                cout << linha << endl;
                                        }
                                        else
                                            comecaLeitura--;
                                        getline(arqPesquisa, linha);
                                    } while (!arqPesquisa.eof());
                                    if (arqPesquisa.eof()) {
                                        if (linha.length() > 0 && j < numArquivos - 1) {
                                            ultimaLinha = linha;
                                        }
                                        if (j == numArquivos - 1) {
                                            if (linha.length() > 0) {
                                                cout << linha << endl;
                                            }
                                        }
                                    }
                                }
                                arqPesquisa.close();
                            }
                            i = -2;
                        }
                        else if ((linhasUltimoArquivo + contaLinha) < 100 && i == 0) {
                            for (int j = i; j < numArquivos; j++) {
                                auto k = to_string(j);
                                k = k + ".txt";
                                ifstream arqPesquisa(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + k);
                                if (arqPesquisa) {
                                    getline(arqPesquisa, linha);
                                    do {
                                        if (ultimaLinha.length() > 0) {
                                            linha = ultimaLinha + linha;
                                            ultimaLinha = "";
                                            cout << linha << endl;
                                        }
                                        else
                                            cout << linha << endl;
                                        getline(arqPesquisa, linha);
                                    } while (!arqPesquisa.eof());
                                    if (arqPesquisa.eof()) {
                                        if (linha.length() > 0 && j < numArquivos - 1) {
                                            ultimaLinha = linha;
                                        }
                                        if (j == numArquivos - 1) {
                                            if (linha.length() > 0) {
                                                cout << linha << endl;
                                            }
                                        }
                                    }
                                }
                                arqPesquisa.close();
                            }
                            i = -2;
                        }
                        else if ((linhasUltimoArquivo + contaLinha) < 100) {
                            contaLinha += linhasUltimoArquivo;
                        }
                    }
                }
                else {
                    cout << "Operacao nao realizada! Erro ao ler diretório ou número de arquivos do retorno do mymsf.config" << endl;
                }

            }
            else {
                cout << "Operacao nao realizada! O arquivo <" << caminhoArquivoToRead << "> nao existe no Mymfs" << endl;
            }
        }
        else {
            cout << "O Mymfs nao esta configurado na unidade informada." << endl;
        }

    }
};