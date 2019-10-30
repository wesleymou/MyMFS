#include "comandos.h"
#include <zlib.h>
#include <vector>
#include <thread>
#include <future>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <experimental/filesystem>
#include <cstring>

namespace fsys = std::experimental::filesystem;
using namespace std;

bool Comandos::mymfsEstaConfigurado(string caminhoComando) {
    if (!fsys::exists(caminhoComando))
        return false;
    ifstream arquivoConfig(caminhoComando);
    string line;
    int cont = 0;
    while (getline(arquivoConfig, line)) {
        if (fsys::exists(line + "/mymfs.config")) {
            cont++;
        } else if (fsys::exists(line + "/mymfs.config.zip")) {
            cont++;
        }

        if (cont == 2) {
            return true;
        }
    }
    return false;
}

vector<string> Comandos::obterUnidades(string path) {

    string line;
    vector<string> unidades;
    ifstream unidadeX(path, ifstream::in);

    while (getline(unidadeX, line)) {
        unidades.push_back(line);
    }

    unidadeX.close();

    return unidades;
}

string *Comandos::nomeExtensao(string path) {
    string nomeArquivo = path;
    string *retorno = new string[2];
    const size_t last_slash_idx = nomeArquivo.find_last_of("/");
    if (std::string::npos != last_slash_idx) {
        nomeArquivo.erase(0, last_slash_idx + 1);
    }

    size_t extension_point = nomeArquivo.find_last_of(".");
    string extensaoArquivo;
    if (std::string::npos != extension_point) {
        retorno[1] = nomeArquivo.substr(extension_point + 1);
        nomeArquivo.erase(extension_point, nomeArquivo.length());
    }
    retorno[0] = nomeArquivo;
    return retorno;
}

void Comandos::escritaParalela(vector<Diretrizes> *d, string filePath, int i, int th) {
    vector<Diretrizes> diretrizes = *d;
    cout << this_thread::get_id() << endl;
    ifstream infile(filePath, ifstream::binary);
    for (i; i < diretrizes.size(); i += th) {   //Cria os arquivos de 500KB ou menos
        char *buffer = new char[diretrizes[i].length];
        char *compress = new char[diretrizes[i].length];
        uLongf compress_length = compressBound(diretrizes[i].length);

        infile.seekg(diretrizes[i].inicio);
        infile.read(buffer, diretrizes[i].length);

        compress2((Bytef *) compress, &compress_length,
                  (Bytef *) buffer, diretrizes[i].length,
                  Z_BEST_COMPRESSION);

        ofstream outfile(diretrizes[i].path + "/" + to_string(i), ofstream::binary);
        outfile << buffer << endl;
        outfile.close();

        ofstream compressFile(diretrizes[i].compress + "/" + to_string(i) + ".zip", ofstream::binary);
        compressFile << compress << endl;
        compressFile.close();
    }
}

string Comandos::config(string caminhoComando, int length, char **unidades) {
    for (int i = 0; i < length; ++i) {
        if (!fsys::exists(unidades[i]))
            return "Endereços inválidos.";
        if (!fsys::is_empty(unidades[i])) {
            return "A unidade " + (string) unidades[i] + " não está vazia";
        }
    }
    if (mymfsEstaConfigurado(caminhoComando)) {   //Verifica se o arquivo Config ja existe no caminho especificado
        return "O Mymfs ja esta configurado nesta unidade.";
    } else {
        ofstream arquivoConfig((string) unidades[0] + "/mymfs.config");       //Cria o arquivo config, configurando o Mymfs na unidade especificada
        arquivoConfig.close();
        ofstream arquivoConfigCompress((string) unidades[1] + "/mymfs.config.zip");       //Cria o arquivo config, configurando o Mymfs na unidade especificada
        arquivoConfigCompress.close();

        string unidadesString = "";
        for (int i = 0; i < length; ++i) {
            if (i != length - 1) {
                unidadesString += (string) unidades[i] + "\n";
            } else {
                unidadesString += (string) unidades[i];
            }

        }
        ofstream arquivoConfigUnidade(caminhoComando);       //Cria o arquivo config, configurando o Mymfs na unidade especificada
        arquivoConfigUnidade << unidadesString << endl;
        arquivoConfigUnidade.close();
        return "O Mymfs foi configurado nesta unidade com sucesso.";
    }
}

string Comandos::importarArquivo(string caminhoComando, string caminhoArquivoImport) {
    streampos end;
    vector<string> unidades;
    if (mymfsEstaConfigurado(caminhoComando)) {

        unidades = obterUnidades(caminhoComando);

        for (int i = 0; i < unidades.size(); ++i) {
            if (fsys::exists(unidades[i] + "/mymfs.config")) {
                ifstream arqConfigExiste(unidades[i] + "/mymfs.config");
                arqConfigExiste.seekg(0, ios::end);
                end = arqConfigExiste.tellg();
                arqConfigExiste.close();
                if (end > this->sizeFileConfig) {                          //Apenas permite a importação se o arquivo de config for menor que 50KB
                    return "Operacao nao realizada! Arquivo mymfs.config esta cheio - Mymfs.";
                }
            }
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada."; //Caso nao exista, nao permite a importacao
    }

    for (int i = 0; i < unidades.size(); ++i) {
        if (!fsys::exists(unidades[i] + "files")) {
            fsys::create_directory(unidades[i] + "files");
        }
    }

    if (fsys::exists(caminhoArquivoImport)) {

        //Obtem o nome do diretório a ser criado para o arquivo atraves do seu nome
        string *nomeArquivo = nomeExtensao(caminhoArquivoImport);

        string nomeDiretorio = nomeArquivo[1] + "-" + nomeArquivo[0];

        ifstream infile(caminhoArquivoImport, ifstream::binary);
        infile.seekg(0, ios::end);
        end = infile.tellg();
        infile.seekg(0, ios::beg);
        infile.close();

        int numArquivos = ceil((float) end / (float) this->sizeFileMax); // Verifica quantos arquivos de 500 KB ou menos serão criados
        vector<Diretrizes> diretrizes;
        int cont = 0;

        for (int i = 0; i < numArquivos; ++i) {
            Diretrizes d;
            d.path = unidades[cont] + "files/" + nomeDiretorio;
            d.compress = unidades[(cont + 1) % unidades.size()] + "files/" + nomeDiretorio;
            d.inicio = i * this->sizeFileMax;
            if (i >= (numArquivos - 1)) {
                d.length = i * this->sizeFileMax + end;
            } else {
                d.length = this->sizeFileMax;
            }
            if (!fsys::exists(unidades[cont] + "files/" + nomeDiretorio)) {
                fsys::create_directory(unidades[cont] + "files/" + nomeDiretorio);
            }
            diretrizes.push_back(d);
            cont = (cont + 1) % unidades.size();
        }

        vector<std::thread> threads;
        for (int i = 0; i < this->numThreads; ++i) {
            threads.push_back(std::thread(&Comandos::escritaParalela, this,
                                          &diretrizes, caminhoArquivoImport, i, 4));
        }

        for (std::thread &th : threads) {
            if (th.joinable())
                th.join();
        }

        string linhaConfig;
        linhaConfig = nomeDiretorio + " " + to_string(numArquivos) + "\n";
        ofstream arqConfig(unidades[0] + "/mymfs.config", ios_base::app | ios_base::out);
        arqConfig << linhaConfig
                  << endl; //Adiciona o arquivo importado no arquivo de configuração (nomeArquivo;quantidadeArquivos)
        arqConfig.close();

        ofstream arqConfigZip(unidades[1] + "/mymfs.config.zip", ios_base::app | ios_base::out);
        arqConfigZip << linhaConfig
                     << endl; //Adiciona o arquivo importado no arquivo de configuração (nomeArquivo;quantidadeArquivos)
        arqConfigZip.close();

        return "Arquivo importado para o Mymfs com sucesso!";
    } else {
        return "Operacao nao realizada! Arquivo não encontrado.";
    }
}

string Comandos::verificarArquivoExisteEmConfig(string caminhoComando, string nomeArquivo) {


    ifstream arqConfig(caminhoComando + "/mymfs.config");

    string extensaoArquivo = nomeArquivo.substr(nomeArquivo.find(".") + 1,
                                                (nomeArquivo.size() - nomeArquivo.find(".")));
    string nomeDiretorioBuscado = extensaoArquivo + "-" + nomeArquivo.substr(0,
                                                                             nomeArquivo.find("."));   //Obtem o nome do diretorio atraves do nome do arquivo
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

void Comandos::exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport) {
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
                    cout << "Operacao nao realizada! O arquivo <" << nomeArquivoExport
                         << "> ja existe na pasta destino" << endl;
                } else {
                    //Caso exista, cria um arquivo no diretorio informado concatenando todos os arquivos de 500KB
                    ofstream combined_file(caminhoDiretorioExport + "/" + nomeArquivoExport);
                    for (int i = 0; i < numArquivos; i++) {
                        auto s = to_string(i);
                        s = s + ".txt";
                        //Percorre os arquivos de 0 a numArquivos concatenando-os no arquivo exportado
                        ifstream srce_file(caminhoComando + "/files/" + nomeDiretorioEncontrado + "/" + s);
                        if (srce_file) {
                            combined_file << srce_file.rdbuf();
                        } else {
                            cerr << "Ocorreu um erro. O arquivo nao pode ser aberto " << s << '\n';
                        }
                        srce_file.close();
                    }
                    combined_file.close();
                    cout << "Arquivo <" << nomeArquivoExport << "> foi exportado para <" << caminhoDiretorioExport
                         << "> com sucesso." << endl;
                }
            } else {
                cout
                    << "Operacao nao realizada! Erro ao ler diretório ou número de arquivos do retorno do mymsf.config"
                    << endl;
            }
        } else {
            cout << "Operacao nao realizada! O arquivo <" << nomeArquivoExport << "> nao existe no Mymfs" << endl;
        }
    } else {
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }
}

void Comandos::listAll(string caminhoComando) {
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
                    nomeDiretorioEncontrado = nomeDiretorioEncontrado.substr(nomeDiretorioEncontrado.find("-") + 1,
                                                                             (nomeDiretorioEncontrado.size() - nomeDiretorioEncontrado.find(
                                                                                 "-")));
                    //Exibe nome do diretório/arquivo
                    cout << nomeDiretorioEncontrado + "." << extensaoDiretorio << endl;
                }
            } while (!arqConfig.eof());
        } else {
            cout << "Nao ha arquivos salvos pelo Mymfs!" << endl;
        }
    } else {
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }
}

string Comandos::converterLinhaConfigParaNomeArquivo(string linhaConfig) {
    string nomeArquivoEncontrado = linhaConfig.substr(0, linhaConfig.find(" "));
    string extensaoArquivoEncontrado = nomeArquivoEncontrado.substr(0, nomeArquivoEncontrado.find("-"));
    nomeArquivoEncontrado = nomeArquivoEncontrado.substr(nomeArquivoEncontrado.find("-") + 1,
                                                         (nomeArquivoEncontrado.size() - nomeArquivoEncontrado.find(
                                                             "-")));
    return nomeArquivoEncontrado + "." + extensaoArquivoEncontrado;
}

void Comandos::remove(string caminhoComando, string nomeArquivo) {
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
        } else {
            arqConfig.close();
            cout << "Operacao nao realizada! O arquivo <" << nomeArquivo << "> nao existe no Mymfs" << endl;
        }
    } else {
        arqConfig.close();
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }
}

void Comandos::removeAll(string caminhoComando) {
    ofstream arquivoConfig;
    if (mymfsEstaConfigurado(caminhoComando)) {
        if (fsys::exists(caminhoComando + "/files")) {
            fsys::remove_all(caminhoComando + "/files");
            arquivoConfig.open(caminhoComando + "/mymfs.config", std::ofstream::out | std::ofstream::trunc);
            arquivoConfig.close();
            cout << "Os arquivos do Mymfs foram removidos com sucesso." << endl;
            return;
        } else {
            cout << "Operacao nao realizada! O Mymfs nao possui arquivos gravados, portanto nao foram removidos.";
        }
    } else {
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }
}

void Comandos::procuraPalavra(string caminhoComando, string palavra, string caminhoArquivoToRead) {
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
                                } else
                                    ultimaLinha = linha;
                            }
                        }
                    } else {
                        cerr << "Ocorreu um erro. O arquivo nao pode ser aberto " << s << '\n';
                    }
                    arqPesquisa.close();
                }
                if (i == numArquivos) {
                    cout << "Nao Encontrado" << endl;
                }
            } else {
                cout
                    << "Operacao nao realizada! Erro ao ler diretório ou numero de arquivos do retorno do mymsf.config"
                    << endl;
            }
        } else {
            cout << "Operacao nao realizada! O arquivo <" << caminhoArquivoToRead << "> nao existe no Mymfs" << endl;
        }
    } else
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
}

void Comandos::primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead) {

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
                            } else
                                cout << linha << endl;
                            getline(arqPesquisa, linha);
                        } while (loop < condicao && !arqPesquisa.eof());
                        if (arqPesquisa.eof()) {
                            if (i == numArquivos - 1) {
                                cout << linha << endl;
                            } else if ((linha.length() > 0 && i < numArquivos - 1)) {
                                ultimaLinha = linha;
                            }
                        }
                        if (contaLinha == 100)
                            i = numArquivos;
                    } else {
                        cerr << "Ocorreu um erro. O arquivo nao pode ser aberto " << s << '\n';
                    }
                    arqPesquisa.close();
                }
            } else {
                cout
                    << "Operacao nao realizada! Erro ao ler diretório ou número de arquivos do retorno do mymsf.config"
                    << endl;
            }
        } else {
            cout << "Operacao nao realizada! O arquivo <" << caminhoArquivoToRead << "> nao existe no Mymfs" << endl;
        }
    } else {
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }

}

void Comandos::ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead) {
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
                    } else if ((linhasUltimoArquivo + contaLinha) >= 100) {
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
                                        } else
                                            cout << linha << endl;
                                    } else
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
                    } else if ((linhasUltimoArquivo + contaLinha) < 100 && i == 0) {
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
                                    } else
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
                    } else if ((linhasUltimoArquivo + contaLinha) < 100) {
                        contaLinha += linhasUltimoArquivo;
                    }
                }
            } else {
                cout
                    << "Operacao nao realizada! Erro ao ler diretório ou número de arquivos do retorno do mymsf.config"
                    << endl;
            }

        } else {
            cout << "Operacao nao realizada! O arquivo <" << caminhoArquivoToRead << "> nao existe no Mymfs" << endl;
        }
    } else {
        cout << "O Mymfs nao esta configurado na unidade informada." << endl;
    }

}