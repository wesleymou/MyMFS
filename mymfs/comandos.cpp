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
#include <assert.h>

namespace fsys = std::experimental::filesystem;
using namespace std;

bool Comandos::mymfsEstaConfigurado(string caminhoComando) {
    if (!fsys::exists(caminhoComando))
        return false;
    ifstream arquivoConfig(caminhoComando, ios_base::in);
    string line;
    while (getline(arquivoConfig, line)) {
        if (fsys::exists(line + "mymfs.config")) {
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

int Comandos::verificarArquivoExisteEmConfig(LinhaConfig *linhaConfig, string caminhoComando, string nomeArquivo) {
    ifstream arqConfig;
//    LinhaConfig linhaConfig = *linha_config;
    vector<string> unidades = this->obterUnidades(caminhoComando);
    if (fsys::exists(unidades[0] + "/mymfs.config")) {
        arqConfig = ifstream(unidades[0] + "/mymfs.config");
    } else if (fsys::exists(unidades[1] + "/mymfs.config")) {
        arqConfig = ifstream(unidades[1] + "/mymfs.config");
    } else {
        return 1;
    }

    string *nomeExtensao = this->nomeExtensao(nomeArquivo);
    string arquivoProcurado = nomeExtensao[1] + "-" + nomeExtensao[0];
    string linha;

    while (getline(arqConfig, linha)) {
        size_t aux = linha.find(arquivoProcurado);
        if (aux != string::npos) {
            size_t x = linha.find_first_of("-");
            size_t y = linha.find_last_of("-");

            linhaConfig->tamanho = stoi(linha.substr(y + 1, string::npos));
            linha.erase(y, string::npos);
            y = linha.find_last_of("-");
            linhaConfig->extensao = linha.substr(0, x);
            linhaConfig->arquivo = linha.substr(x + 1, y - x - 1);
            linhaConfig->quantidade = stoi(linha.substr(y + 1, string::npos));
            return 0;
        }
    }
    return 1;
}

void Comandos::escritaParalela(vector<Diretrizes> *d, string filePath, int i, int th) {
    vector<Diretrizes> diretrizes = *d;
    ifstream infile(filePath, ifstream::binary);
    for (i; i < diretrizes.size(); i += th) {   //Cria os arquivos de 500KB ou menos
        char *buffer = new char[diretrizes[i].length];

        infile.seekg(diretrizes[i].inicio);
        infile.read(buffer, diretrizes[i].length);

        if (fsys::exists(diretrizes[i].path)) {
            ofstream outfile(diretrizes[i].path + "/" + to_string(i), ofstream::binary);
            outfile << buffer;
            outfile.close();
        }
        if (fsys::exists(diretrizes[i].compress)) {
            vector<unsigned char> buffer_compresss = compress_string(buffer);

            ofstream compressFile(diretrizes[i].compress + "/" + to_string(i) + ".zlib", ofstream::binary);
            compressFile.write(reinterpret_cast<const char *>(buffer_compresss.data()), buffer_compresss.size());
            streampos x = compressFile.tellp();
            compressFile.close();
        }
    }
    infile.close();
}

void Comandos::leituraParalela(vector<Diretrizes> *d, string filePath, int i, int th, ofstream *outfile) {
    vector<Diretrizes> diretrizes = *d;
//    ofstream outfile(filePath, ifstream::binary);

    for (i; i < diretrizes.size(); i += th) {   //Cria os arquivos de 500KB ou menos
        if (fsys::exists(diretrizes[i].path)) {
            ifstream infile(diretrizes[i].path, ios::in | ios::binary | ios::ate);
            int size = infile.tellg();
            char *buffer = new char[size];
            infile.seekg(0, ios::beg);
            infile.read(buffer, size);

            outfile->seekp(diretrizes[i].inicio);
            outfile->write(buffer, size);

            infile.close();
        } else if (fsys::exists(diretrizes[i].compress)) {
            ifstream infile = ifstream(diretrizes[i].compress, ios::in | ios::binary | ios::ate);
            int size = infile.tellg();

            vector<unsigned char> compress_buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&compress_buffer[0]), size);

            vector<unsigned char> descompress = decompress_string(compress_buffer);

            outfile->seekp(diretrizes[i].inicio);
            outfile->write(reinterpret_cast<const char *>(descompress.data()), descompress.size());

            infile.close();
        } else {
            cout << "arquivo corrompido" << endl;
            return;
        }
    }
//    outfile.close();
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
        ofstream arquivoConfigCompress((string) unidades[1] + "/mymfs.config");       //Cria o arquivo config, configurando o Mymfs na unidade especificada
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
        if (fsys::exists(unidades[i])&&!fsys::exists(unidades[i] + "files")) {
            fsys::create_directory(unidades[i] + "files");
        }
    }

    if (fsys::exists(caminhoArquivoImport)) {

        //Obtem o nome do diretório a ser criado para o arquivo atraves do seu nome
        string *nomeArquivo = nomeExtensao(caminhoArquivoImport);
        LinhaConfig null;
        if (this->verificarArquivoExisteEmConfig(&null, caminhoComando, nomeArquivo[0] + "." + nomeArquivo[1]) == 1) {
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
                if (fsys::exists(unidades[cont]) && !fsys::exists(unidades[cont] + "files/" + nomeDiretorio)) {
                    fsys::create_directory(unidades[cont] + "files/" + nomeDiretorio);
                }
                diretrizes.push_back(d);
                cont = (cont + 1) % unidades.size();
            }

            vector<std::thread> threads;
            for (int i = 0; i < this->numThreads; ++i) {
                threads.push_back(std::thread(&Comandos::escritaParalela, this,
                                              &diretrizes, caminhoArquivoImport, i, this->numThreads));
            }

            for (std::thread &th : threads) {
                if (th.joinable())
                    th.join();
            }

            string linhaConfig;
            linhaConfig = nomeDiretorio + "-" + to_string(numArquivos) + "-" + to_string(end);
            ofstream arqConfig(unidades[0] + "/mymfs.config", ios_base::app | ios_base::out);
            arqConfig << linhaConfig
                      << endl; //Adiciona o arquivo importado no arquivo de configuração (nomeArquivo;quantidadeArquivos)
            arqConfig.close();

            ofstream arqConfigZip(unidades[1] + "/mymfs.config", ios_base::app | ios_base::out);
            arqConfigZip << linhaConfig
                         << endl; //Adiciona o arquivo importado no arquivo de configuração (nomeArquivo;quantidadeArquivos)
            arqConfigZip.close();

            return "Arquivo importado para o Mymfs com sucesso!";
        } else {
            return "Ja existe um arquivo com esse nome.";
        }
    } else {
        return "Operacao nao realizada! Arquivo não encontrado.";
    }
}

string Comandos::exportarArquivo(string caminhoComando, string nomeArquivoExport, string caminhoDiretorioExport) {
    //Verifica se o arquivo de configuração e se o arquivo a ser exportado existem
    if (mymfsEstaConfigurado(caminhoComando)) {
        LinhaConfig linhaConfig;
        if (verificarArquivoExisteEmConfig(&linhaConfig,
                                           caminhoComando,
                                           nomeArquivoExport) == 0) {//Verifica se encontrou o diretorio do arquivo a ser exportado
            if (!fsys::exists(caminhoDiretorioExport + "/" + nomeArquivoExport)) { //Verifica se o arquivo a ser exportado existe no destino
                //Caso não exista, cria um arquivo no diretorio informado concatenando todos os arquivos de 500KB
                vector<string> unidades = obterUnidades(caminhoComando);
                vector<Diretrizes> diretrizes;
                int cont = 0;

                for (int i = 0; i < linhaConfig.quantidade; ++i) {
                    Diretrizes d;
                    d.inicio = i * this->sizeFileMax;
                    d.length = this->sizeFileMax;
                    d.path = unidades[cont] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                        i);
                    d.compress = unidades[(cont + 1) % unidades.size()] +
                        "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(i) + ".zlib";
                    cont = (cont + 1) % unidades.size();
                    diretrizes.push_back(d);
                }


                ofstream arquivoPrototipo;
                arquivoPrototipo.rdbuf()->pubsetbuf(0, 0);
//                arquivoPrototipo.sync_with_stdio(true);
                arquivoPrototipo.open(caminhoDiretorioExport + "/" + linhaConfig.arquivo + "." + linhaConfig.extensao,
                                      ios::ate);

                vector<std::thread> threads;
                for (int i = 0; i < this->numThreads; ++i) {
                    threads.push_back(std::thread(&Comandos::leituraParalela, this,
                                                  &diretrizes,
                                                  caminhoDiretorioExport + "/" + linhaConfig.arquivo + "." + linhaConfig.extensao,
                                                  i,
                                                  this->numThreads,
                                                  &arquivoPrototipo));
                }

                for (std::thread &th : threads) {
                    if (th.joinable())
                        th.join();
                }

                arquivoPrototipo.close();

                return "Arquivo <" + nomeArquivoExport + "> foi exportado para <" + caminhoDiretorioExport + "> com sucesso.";
            } else {
                return "Operacao nao realizada! O arquivo <" + nomeArquivoExport + "> ja existe na pasta destino";
            }
        } else {
            return "Operacao nao realizada! O arquivo <" + nomeArquivoExport + "> nao existe no Mymfs";
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

string Comandos::listAll(string caminhoComando) {
    //Valida se o arquivo config existe no diretorio especificado
    if (mymfsEstaConfigurado(caminhoComando)) {
        //Caso exista, percorre o arquivo buscando os nomes dos diretorios/arquivos e listando-os
        vector<string> unidades = obterUnidades(caminhoComando);
        ifstream arqConfig(unidades[0] + "/mymfs.config");
        string linha;
        string lista;
        while (getline(arqConfig, linha)) {
            lista += linha + "\n";
        }
        if (lista.length() > 0) {
            return lista;
        } else {
            return "Nao ha arquivos salvos pelo Mymfs!";
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

string Comandos::converterLinhaConfigParaNomeArquivo(string linhaConfig) {
    return "";
}

void Comandos::remove(string caminhoComando, string nomeArquivo) {
    ifstream arqConfig(caminhoComando + "/mymfs.config");

    if (nomeArquivo.empty()) {
        cout << "Deve-se informar o nome do arquivo";
        return;
    }

    if (mymfsEstaConfigurado(caminhoComando)) {
//        string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, nomeArquivo);
        string linhaConfig;
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
//        string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, caminhoArquivoToRead);
        string linhaConfig;

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
//        string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, caminhoArquivoToRead);
        string linhaConfig;

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
//        string linhaConfig = verificarArquivoExisteEmConfig(caminhoComando, caminhoArquivoToRead);
        string linhaConfig;

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

vector<unsigned char> Comandos::compress_string(const char *str, int compressionlevel) {
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw (std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = (Bytef *) str;
    zs.avail_in = strlen(reinterpret_cast<const char *>(str));           // set the z_stream's input

    int ret;
    unsigned char outbuffer[32768];
    vector<unsigned char> outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            // append the block to the output string
            for (unsigned char c : outbuffer) {
                outstring.push_back(c);
            }
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw (std::runtime_error(oss.str()));
    }

    return outstring;
}

vector<unsigned char> Comandos::decompress_string(vector<unsigned char> str) {
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw (std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = (Bytef *) str.data();
    zs.avail_in = str.size();

    int ret;
    unsigned char outbuffer[32768];
    vector<unsigned char> outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out) {
            // append the block to the output string
            for (unsigned char c : outbuffer) {
                outstring.push_back(c);
            }
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ") "
            << zs.msg;
        throw (std::runtime_error(oss.str()));
    }

    return outstring;
}