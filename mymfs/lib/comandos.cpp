#include "comandos.h"
#include <zlib.h>
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <experimental/filesystem>
#include <cstring>
#include <mutex>

namespace fsys = std::experimental::filesystem;
using namespace std;

bool Comandos::mymfsEstaConfigurado(string caminhoComando) {
    if (!fsys::exists(caminhoComando))
        return false;
    ifstream arquivoConfig(caminhoComando, ios_base::in);
    string line;
    while (getline(arquivoConfig, line)) {
        if (fsys::exists(line + configFileName)) {
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

Comandos::LinhaConfig Comandos::separarNomeExtensao(string path) {
    LinhaConfig nome_arquivo;
    size_t findChar = path.find_last_of("/");
    if (std::string::npos != findChar) {
        path.erase(0, findChar + 1);
    }

    findChar = path.find_last_of(".");

    if (std::string::npos != findChar) {
        nome_arquivo.extensao = path.substr(findChar + 1);
        path.erase(findChar, path.length());
    }
    nome_arquivo.arquivo = path;
    return nome_arquivo;
}

bool Comandos::arquivoExiste(LinhaConfig *linhaConfig, string caminhoComando, string nomeArquivo) {
    ifstream arqConfig;
    vector<string> unidades = this->obterUnidades(caminhoComando);
    if (fsys::exists(unidades[0] + "/" + configFileName)) {
        arqConfig = ifstream(unidades[0] + "/" + configFileName);
    } else if (fsys::exists(unidades[1] + "/" + configFileName)) {
        arqConfig = ifstream(unidades[1] + "/" + configFileName);
    } else {
        return false;
    }

    LinhaConfig nomeExtensao = this->separarNomeExtensao(nomeArquivo);
    string arquivoProcurado = nomeExtensao.extensao + "-" + nomeExtensao.arquivo;
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
            return true;
        }
    }
    return false;
}

void Comandos::escritaParalela(vector<Diretrizes> *d, string filePath, int i, int th) {
    vector<Diretrizes> diretrizes = *d;
    ifstream infile(filePath, ifstream::in | ifstream::binary);
    for (i; i < diretrizes.size(); i += th) {   //Cria os arquivos de 500KB ou menos
        vector<unsigned char> buffer(diretrizes[i].length);

        infile.seekg(diretrizes[i].inicio);
        infile.read(reinterpret_cast<char *>(&buffer[0]), diretrizes[i].length);

        if (fsys::exists(diretrizes[i].path)) {
            ofstream outfile(diretrizes[i].path + "/" + to_string(i), ofstream::out | ofstream::binary);
            outfile.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
            outfile.flush();
            outfile.close();
        }
        if (fsys::exists(diretrizes[i].compress)) {
            vector<unsigned char> buffer_compresss = compress_string(buffer);

            ofstream compressFile(diretrizes[i].compress + "/" + to_string(i) + ".compress",
                                  ofstream::out | ofstream::binary);
            compressFile.write(reinterpret_cast<const char *>(buffer_compresss.data()), buffer_compresss.size());
            compressFile.flush();
            compressFile.close();
        }
    }
    infile.close();
}

void Comandos::leituraParalela(vector<Diretrizes> *d, string filePath, int i, int th) {
    vector<Diretrizes> diretrizes = *d;
    ofstream outfile(filePath, ofstream::out | ofstream::binary);

    for (i; i < diretrizes.size(); i += th) {   //Cria os arquivos de 500KB ou menos
        if (fsys::exists(diretrizes[i].path)) {
            std::unique_lock<std::mutex> lck(monitor_thread, std::defer_lock);
            ifstream infile(diretrizes[i].path, ios::in | ios::binary | ios::ate);
            int size = infile.tellg();

            vector<unsigned char> buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&buffer[0]), size);

            lck.lock();
            outfile.seekp(diretrizes[i].inicio);
            outfile.write(reinterpret_cast<const char *>(buffer.data()), size);
//            outfile.flush();
            lck.unlock();

            infile.close();
        } else if (fsys::exists(diretrizes[i].compress)) {
            std::unique_lock<std::mutex> lck(monitor_thread, std::defer_lock);
            ifstream infile(diretrizes[i].compress, ios::in | ios::binary | ios::ate);
            int size = infile.tellg();

            vector<unsigned char> compress_buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&compress_buffer[0]), size);

            vector<unsigned char> descompress = decompress_string(compress_buffer);

            lck.lock();
            outfile.seekp(diretrizes[i].inicio);
            outfile.write(reinterpret_cast<const char *>(descompress.data()), descompress.size());
//            outfile.flush();
            lck.unlock();

            infile.close();
        } else {
            cout << "Arquivo corrompido" << endl;
            return;
//            throw (runtime_error("Arquivo corrompido"));
        }
    }
    outfile.close();
}

void Comandos::alimentarBufferParalelo(vector<Diretrizes> *d, stringstream *buffer_out, int i, int th) {
    vector<Diretrizes> diretrizes = *d;
    for (i; i < diretrizes.size(); i += th) {
        std::unique_lock<std::mutex> lck(monitor_thread, std::defer_lock);
        ifstream infile;
        if (fsys::exists(diretrizes[i].path)) {
            infile = ifstream(diretrizes[i].path, ios_base::in | ios_base::binary | ios_base::ate);
            int size = infile.tellg();

            vector<unsigned char> buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&buffer[0]), size);

            lck.lock();
            buffer_out->seekp(diretrizes[i].inicio);
            buffer_out->write(reinterpret_cast<const char *>(buffer.data()), size);
            buffer_out->flush();
            lck.unlock();

            infile.close();

        } else if (fsys::exists(diretrizes[i].compress)) {
            infile = ifstream(diretrizes[i].compress, ios_base::in | ios_base::binary | ios_base::ate);
            int size = infile.tellg();

            vector<unsigned char> bufferCompress(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&bufferCompress[0]), size);

            vector<unsigned char> bufferUncompress = decompress_string(bufferCompress);

            lck.lock();
            buffer_out->seekp(diretrizes[i].inicio);
            buffer_out->write(reinterpret_cast<const char *>(bufferUncompress.data()), size);
            buffer_out->flush();
            lck.unlock();

            infile.close();

        } else {
            cout << "Arquivo corrompido" << endl;
            return;
//            throw (runtime_error("Arquivo corrompido"));
        }
    }
}

void Comandos::primeirasLinhas(vector<Diretrizes> *d, stringstream *buffer_out, int quant) {
    vector<Diretrizes> diretrizes = *d;
    for (int i = 0; i < quant; i++) {
        if (fsys::exists(diretrizes[i].path)) {
            ifstream infile(diretrizes[i].path,
                            ios_base::in | ios_base::binary | ios_base::ate);
            int size = infile.tellg();

            vector<unsigned char> buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&buffer[0]), size);

            buffer_out->seekp(diretrizes[i].inicio);
            buffer_out->write(reinterpret_cast<const char *>(buffer.data()), size);;

            infile.close();

        } else if (fsys::exists(diretrizes[i].compress)) {
            ifstream infile(diretrizes[i].compress,
                            ios_base::in | ios_base::binary | ios_base::ate);
            int size = infile.tellg();

            vector<unsigned char> buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&buffer[0]), size);

            buffer_out->seekp(diretrizes[i].inicio);
            buffer_out->write(reinterpret_cast<const char *>(buffer.data()), size);;

            infile.close();

        } else {
            cout << "Arquivo corrompido" << endl;
            return;
//            throw (runtime_error("Arquivo corrompido"));
        }
    }
}

void Comandos::ultimasLinhas(vector<Diretrizes> *d, stringstream *buffer_out, int quant) {
    vector<Diretrizes> diretrizes = *d;
    for (int i = 0; i < quant; i++) {
        if (fsys::exists(diretrizes[(diretrizes.size() - quant) + i].path)) {
            ifstream infile(diretrizes[(diretrizes.size() - quant) + i].path,
                            ios_base::in | ios_base::binary | ios_base::ate);
            int size = infile.tellg();

            vector<unsigned char> buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&buffer[0]), size);

            buffer_out->seekp(diretrizes[i].inicio);
            buffer_out->write(reinterpret_cast<const char *>(buffer.data()), size);;

            infile.close();

        } else if (fsys::exists(diretrizes[(diretrizes.size() - quant) + i].compress)) {
            ifstream infile(diretrizes[(diretrizes.size() - quant) + i].compress,
                            ios_base::in | ios_base::binary | ios_base::ate);
            int size = infile.tellg();

            vector<unsigned char> buffer(size);
            infile.seekg(0, ios::beg);
            infile.read(reinterpret_cast<char *>(&buffer[0]), size);

            buffer_out->seekp(diretrizes[i].inicio);
            buffer_out->write(reinterpret_cast<const char *>(buffer.data()), size);;

            infile.close();

        } else {
            cout << "Arquivo corrompido" << endl;
            return;
//            throw (runtime_error("Arquivo corrompido"));
        }
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
        ofstream arquivoConfig((string) unidades[0] + "/" + configFileName);       //Cria o arquivo config, configurando o Mymfs na unidade especificada
        arquivoConfig.close();
        ofstream arquivoConfigCompress((string) unidades[1] + "/" + configFileName);       //Cria o arquivo config, configurando o Mymfs na unidade especificada
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
            if (fsys::exists(unidades[i] + "/" + configFileName)) {
                ifstream arqConfigExiste(unidades[i] + "/" + configFileName);
                arqConfigExiste.seekg(0, ios::end);
                end = arqConfigExiste.tellg();
                arqConfigExiste.close();
                if (end > this->sizeFileConfig) {                          //Apenas permite a importação se o arquivo de config for menor que 50KB
                    return "Operacao nao realizada! Arquivo " + configFileName + " esta cheio - Mymfs.";
                }
            }
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada."; //Caso nao exista, nao permite a importacao
    }

    for (int i = 0; i < unidades.size(); ++i) {
        if (fsys::exists(unidades[i]) && !fsys::exists(unidades[i] + "files")) {
            fsys::create_directory(unidades[i] + "files");
        }
    }

    if (fsys::exists(caminhoArquivoImport)) {

        //Obtem o nome do diretório a ser criado para o arquivo atraves do seu nome
        LinhaConfig nomeArquivo = separarNomeExtensao(caminhoArquivoImport);
        LinhaConfig null;
        if (!this->arquivoExiste(&null,
                                 caminhoComando,
                                 nomeArquivo.arquivo + "." + nomeArquivo.extensao)) {
            string nomeDiretorio = nomeArquivo.extensao + "-" + nomeArquivo.arquivo;

            ifstream infile(caminhoArquivoImport, ifstream::binary | ios_base::ate);
            end = infile.tellg();
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
                    d.length = (int) end - (i * this->sizeFileMax);
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
            ofstream arqConfig(unidades[0] + "/" + configFileName, ios_base::app | ios_base::out);
            arqConfig << linhaConfig
                      << endl; //Adiciona o arquivo importado no arquivo de configuração (nomeArquivo;quantidadeArquivos)
            arqConfig.close();

            ofstream arqConfigZip(unidades[1] + "/" + configFileName, ios_base::app | ios_base::out);
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
        if (arquivoExiste(&linhaConfig,
                          caminhoComando,
                          nomeArquivoExport)) {//Verifica se encontrou o diretorio do arquivo a ser exportado
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
                        "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(i) + ".compress";
                    cont = (cont + 1) % unidades.size();
                    diretrizes.push_back(d);
                }

                vector<std::thread> threads;
                for (int i = 0; i < this->numThreads; ++i) {
                    threads.push_back(std::thread(&Comandos::leituraParalela, this,
                                                  &diretrizes,
                                                  caminhoDiretorioExport + "/" + linhaConfig.arquivo + "." + linhaConfig.extensao,
                                                  i,
                                                  this->numThreads));
                }

                for (std::thread &th : threads) {
                    if (th.joinable())
                        th.join();
                }

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

        ifstream arqConfig;
        for (auto unidade : unidades) {
            if (fsys::exists(unidade + configFileName)) {
                arqConfig = ifstream(unidade + configFileName);
                break;
            }
        }

        string linha;
        string lista = "";
        while (getline(arqConfig, linha)) {
            LinhaConfig linhaConvertida = converterLinhaConfigParaNomeArquivo(linha);
            lista += linhaConvertida.arquivo + "." + linhaConvertida.extensao + "\n";
        }
        if (lista.length()) {
            return lista;
        } else {
            return "Nao ha arquivos salvos pelo Mymfs!";
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

Comandos::LinhaConfig Comandos::converterLinhaConfigParaNomeArquivo(string linha) {
    LinhaConfig nomeArquivo;
    size_t x = linha.find_first_of("-");
    size_t y = linha.find_last_of("-");

    nomeArquivo.tamanho = stoi(linha.substr(y + 1, string::npos));
    linha.erase(y, string::npos);
    y = linha.find_last_of("-");
    nomeArquivo.extensao = linha.substr(0, x);
    nomeArquivo.arquivo = linha.substr(x + 1, y - x - 1);
    nomeArquivo.quantidade = stoi(linha.substr(y + 1, string::npos));
    return nomeArquivo;
}

string Comandos::remove(string caminhoComando, string nomeArquivo) {
    if (mymfsEstaConfigurado(caminhoComando)) {
        vector<string> unidades = obterUnidades(caminhoComando);

        ifstream arquivoConfig;

        for (auto unidade : unidades) {
            if (fsys::exists(unidade + configFileName)) {
                arquivoConfig = ifstream(unidade + configFileName);
                break;
            }
        }

        LinhaConfig arquivoExtensao = separarNomeExtensao(nomeArquivo);
        bool existe = false;
        string line;
        LinhaConfig comparacao;
        string configNovo = "";
        while (getline(arquivoConfig, line)) {
            comparacao = converterLinhaConfigParaNomeArquivo(line);
            if (comparacao.arquivo == arquivoExtensao.arquivo && comparacao.extensao == arquivoExtensao.extensao)
                existe = true;
            else
                configNovo += line + "\n";
        }
        if (!existe) {
            arquivoConfig.close();
            return "O arquivo nao existe no MyMFS.";
        }
        arquivoConfig.close();

        for (auto unidade : unidades) {
            if (fsys::exists(unidade + "files/" + arquivoExtensao.extensao + "-" + arquivoExtensao.arquivo))
                fsys::remove_all(unidade + "files/" + arquivoExtensao.extensao + "-" + arquivoExtensao.arquivo);
            ofstream arquivoConfigNovo(unidade + configFileName, ifstream::out | ifstream::trunc);
            arquivoConfigNovo << configNovo;
            arquivoConfigNovo.close();
        }

        return "O arquivo <" + nomeArquivo + "> foi removido com sucesso.";
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

string Comandos::removeAll(string caminhoComando) {
    if (mymfsEstaConfigurado(caminhoComando)) {
        vector<string> unidades = obterUnidades(caminhoComando);

        for (auto unidade : unidades) {
            if (fsys::exists(unidade + "files"))
                fsys::remove_all(unidade + "files");
            if (fsys::exists(unidade + configFileName))
                fsys::remove(unidade + configFileName);
        }
        fsys::remove(caminhoComando);
        return "Os arquivos do Mymfs foram removidos com sucesso.";
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

string Comandos::procuraPalavra(string caminhoComando, string palavra, string arquivoAlvo) {

    //Verifica se o arquivo de configuração e se o arquivo a ser exportado existem
    if (mymfsEstaConfigurado(caminhoComando)) {
        LinhaConfig linhaConfig;

        //Verifica se encontrou o diretorio do arquivo a ser exportado
        if (arquivoExiste(&linhaConfig, caminhoComando, arquivoAlvo)) {

            vector<string> unidades = obterUnidades(caminhoComando);

            vector<Diretrizes> diretrizes;
            for (int i = 0; i < linhaConfig.quantidade; ++i) {
                Diretrizes d;
                d.inicio = i * this->sizeFileMax;
                d.length = this->sizeFileMax;
                d.path = unidades[i % unidades.size()] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                    i);
                d.compress = unidades[(i + 1) % unidades.size()] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                    i) + ".compress";
                diretrizes.push_back(d);
            }
            diretrizes[diretrizes.size() - 1].length = linhaConfig.tamanho - ((linhaConfig.quantidade - 1) * this->sizeFileMax);

            string line;
            std::stringstream buffer;
            string oldLine = "";
            int numLine = 1;
            palavra = toUpperCase(palavra);
            vector<std::thread> threads;
            int numThreads = 1;
            for (int i = 0; i < linhaConfig.quantidade; i += numThreads * 3) {
                for (int j = 0; j < numThreads; j++) {
                    threads.push_back(std::thread(&Comandos::alimentarBufferParalelo,
                                                  this,
                                                  &diretrizes,
                                                  &buffer,
                                                  j,
                                                  numThreads));
                }

                for (std::thread &thread :threads) {
                    if (thread.joinable())
                        thread.join();
                }

                while (getline(buffer, line)) {
                    if (toUpperCase(line).find(palavra) != string::npos) {
                        return "Encontrado na linha " + to_string(numLine) + ": " + line;
                    }
                    numLine++;
                }
                if (line[line.length() - 1] != '\n') {
                    oldLine = line;
                }
            }
            return "Nao encontrado";
        } else {
            return "Operacao nao realizada! O arquivo <" + arquivoAlvo + "> nao existe no Mymfs";
        }
    } else
        return "O Mymfs nao esta configurado na unidade informada.";
}

string Comandos::primeiras100Linhas(string caminhoComando, string caminhoArquivoToRead) {
    int quantLines = 100;

    ifstream arqConfigExiste(caminhoComando + "/" + configFileName);

    //Verifica se o arquivo de configuração existe
    if (mymfsEstaConfigurado(caminhoComando)) {
        LinhaConfig linhaConfig;

        //Verifica se encontrou o diretorio do arquivo a ser exportado
        if (arquivoExiste(&linhaConfig, caminhoComando, caminhoArquivoToRead)) {

            vector<string> unidades = obterUnidades(caminhoComando);
            vector<Diretrizes> diretrizes;
            for (int i = 0; i < linhaConfig.quantidade; ++i) {
                Diretrizes d;
                d.inicio = i * this->sizeFileMax;
                d.length = this->sizeFileMax;
                d.path = unidades[i % unidades.size()] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                    i);
                d.compress = unidades[(i + 1) % unidades.size()] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                    i) + ".compress";
                diretrizes.push_back(d);
            }
            diretrizes[diretrizes.size() - 1].length = linhaConfig.tamanho - ((linhaConfig.quantidade - 1) * this->sizeFileMax);

            stringstream buffer;
            string line;
            vector<string> linhas;
            int cont = 0;

            while ((linhas.size() < (quantLines + 1)) && (cont < diretrizes.size())) {

                primeirasLinhas(&diretrizes, &buffer, cont + 1);
                while (getline(buffer, line)) {
                    linhas.push_back(line);
                }
                if (linhas.size() >= (quantLines + 1)) {
                    string linhasRetorno = "";
                    for (int i = 0; i < quantLines; ++i) {
                        linhasRetorno += linhas[i] + "\n";
                    }
                    return linhasRetorno;
                }
                cont++;
                if (cont < diretrizes.size()) {
                    linhas.clear();
                    buffer.clear();
                }
            }

            string linhasRetorno = "";
            for (int i = 0; i < linhas.size(); ++i) {
                linhasRetorno += linhas[i] + "\n";
            }
            return linhasRetorno;

        } else {
            return "Operacao nao realizada! O arquivo <" + caminhoArquivoToRead + "> nao existe no Mymfs";
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

string Comandos::ultimas100Linhas(string caminhoComando, string caminhoArquivoToRead) {

    int quantLines = 100;

    ifstream arqConfigExiste(caminhoComando + "/" + configFileName);

    //Verifica se o arquivo de configuração existe
    if (mymfsEstaConfigurado(caminhoComando)) {
        LinhaConfig linhaConfig;

        //Verifica se encontrou o diretorio do arquivo a ser exportado
        if (arquivoExiste(&linhaConfig, caminhoComando, caminhoArquivoToRead)) {

            vector<string> unidades = obterUnidades(caminhoComando);
            vector<Diretrizes> diretrizes;
            for (int i = 0; i < linhaConfig.quantidade; ++i) {
                Diretrizes d;
                d.inicio = i * this->sizeFileMax;
                d.length = this->sizeFileMax;
                d.path = unidades[i % unidades.size()] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                    i);
                d.compress = unidades[(i + 1) % unidades.size()] + "files/" + linhaConfig.extensao + "-" + linhaConfig.arquivo + "/" + to_string(
                    i) + ".compress";
                diretrizes.push_back(d);
            }
            diretrizes[diretrizes.size() - 1].length = linhaConfig.tamanho - ((linhaConfig.quantidade - 1) * this->sizeFileMax);

            stringstream buffer;
            string line;
            vector<string> linhas;
            int cont = 0;

            while ((linhas.size() < (quantLines + 1)) && (cont < diretrizes.size())) {

                ultimasLinhas(&diretrizes, &buffer, cont + 1);
                while (getline(buffer, line)) {
                    linhas.push_back(line);
                }
                if (linhas.size() >= (quantLines + 1)) {
                    string linhasRetorno = "";
                    for (int i = 0; i < quantLines; ++i) {
                        linhasRetorno += linhas[(linhas.size() - quantLines) + i] + "\n";
                    }
                    return linhasRetorno;
                }
                cont++;
                if (cont < diretrizes.size()) {
                    linhas.clear();
                    buffer.clear();
                }
            }

            string linhasRetorno = "";
            for (int i = 0; i < linhas.size(); ++i) {
                linhasRetorno += linhas[i] + "\n";
            }
            return linhasRetorno;

        } else {
            return "Operacao nao realizada! O arquivo <" + caminhoArquivoToRead + "> nao existe no Mymfs";
        }
    } else {
        return "O Mymfs nao esta configurado na unidade informada.";
    }
}

vector<unsigned char> Comandos::compress_string(vector<unsigned char> str, int compressionlevel) {
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw (std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = (Bytef *) str.data();
    zs.avail_in = str.size();           // set the z_stream's input

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

string Comandos::toUpperCase(string text) {
    for (int i = 0; i < text.size(); ++i) {
        text[i] = toupper(text[i]);
    }
    return text;
}