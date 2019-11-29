# MyMFS
Projeto da disciplina de Sistemas Operacionais da graduação de Engenharia de Software da PUC Minas.
O MyMFS é um sistema RAID 5, que divide um arquivo entre diversos locais. Este projeto é a continuação do projeto da equipe do semestre anterior, sendo assim, o código deles é o ponto de partida desse.
Código fonte: https://github.com/Haddadson/mymfs

O código é implementado em c/c++, pois visa desempenho ao simular um sistema de arquivos.

### Objetivos

Permitir que os alunos exercitem os conceitos aprendidos em sala, essenciais para os sistemas operacionais, através da implementação em código. O trabalho simula um sistema de gerencia de arquivos - MyMFS, My Micro File System - com a paginação sendo o arquivo dividido em "partições" de 500KB. O trabalho também aborda os conceitos de multithread através da leitura e escrita simultanea de arquivos. Também aborda os conceitos de condição de corrida e exclusão mútua, uma vez que um mesmo arquivo é escrito simultaneamente por n threads. Há implementação de compressão e descompressão de arquivos, mas os conceitos por tras dos códigos não serão abordados em sala.

(O trabalho também aborda vários conceitos da engenharia de software. (Em construção))

### Programa Compilado

Na pasta "executaveis", se encontram as versões compiladas do projeto para linux e windows.
mymfs.exe => Compilado no Windows 10. Compilador "mingw64" através do CLion.
mymfs (sem extensão) => Compilado no Linux Mint 19.2. Compilador "Clang" através do CLion.

### Comandos do MyMFS

Nome: config
Comando de execução: mymfs.exe X config D E F
Pré-condição: Não existe nenhum arquivo nas unidades D, E e F. Não existe um sistema de arquivos raid
X mymfs nas unidades D, E, F. (note que D, E e F é apenas um exemplo, podem ser outros nomes e
quantidade maior que 3)
Pós-condição: Existe um sistema de arquivos mymfs na unidade raid X, que é composta das unidades D,
E, F. No terminal, confirmação da configuração ou erro associado à pré-condição.
Nome: import
Comando de execução: mymfs.exe X import file.txt
Pré-condição: Existe um arquivo file.txt no local indicado fora do mymfs. Existe um sistema de arquivos
mymfs na unidade rai X. Não existe um arquivo file.txt, pelo mymfs, dentro da unidade raid X.Pós-condição: O arquivo file.txt está pelo mymfs dentro da unidade raid X. No terminal, confirmação da
importação ou erro associado à pré-condição.
Nome: listall
Comando de execução: mymfs.exe X listall
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X.
Pós-condição: Os nomes dos arquivos existentes na unidade raid X, pelo mymfs, estão listados no
terminal, um por linha. (Note que são os arquivos percebidos pelo usuário). O estado da unidade raid X
pelo mymfs não foi alterado.
Nome: export
Comando de execução: mymfs.exe X export file.txt C:/file.txt
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X. O arquivo file.txt está, pelo
mymfs, dentro da unidade raid X. Não há um arquivo C:/file.txt.
Pós-condição: O estado da unidade raid X pelo mymfs não foi alterado. O arquivo file.txt está em “C:”
No terminal, confirmação da exportação ou erro associado à pré-condição.
Nome: remove
Comando de execução: mymfs.exe X remove file.txt
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X. O arquivo file.txt está, pelo
mymfs, dentro da unidade raid X.
Pós-condição: Remove o arquivo file.txt da unidade raid X. O estado da unidade raid X pelo mymfs foi
alterado.
Nome: removeall
Comando de execução: mymfs.exe X removeall
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X.
Pós-condição: Remove todos os arquivos que estão, pelo mymfs, na unidade raid X. O estado da unidade
raid X pelo mymfs foi alterado. No terminal, confirmação da remoção ou erro associado à pré-condição.
Nome: head100
Comando de execução: mymfs.exe X read100 file.txt
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X. O arquivo file.txt está, pelo
mymfs, dentro da unidade raid X.
Pós-condição: No terminal, as 100 primeiras linhas do arquivo file.txt estão exibidas ou está exibido erro
associado à pré-condição. O estado da unidade raid X, pelo mymfs, não foi alterado.
Nome: tail100
Comando de execução: mymfs.exe X tail100 file.txt
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X. O arquivo file.txt está, pelo
mymfs, dentro da unidade raid X.
Pós-condição: No terminal, as 100 últimas linhas do arquivo file.txt estão exibidas ou está exibido erro
associado à pré-condição. O estado da unidade raid X, pelo mymfs, não foi alterado.
Nome: grep
Comando de execução: mymfs.exe X grep word file.txt
Pré-condição: Existe um sistema de arquivos mymfs na unidade raid X. O arquivo file.txt está, pelo
mymfs, dentro da unidade raid X.
Pós-condição: Se exite word no arquivo file.txt, está escrito no terminal “Encontrado” seguido do
número da primeira linha do arquivo no qual word foi encontrado. Se não existe word no arquivo, está
escrito no terminal apenas “Não encontrado”. Se alguma pré-condição não foi satisfeita, um erro está
exibido no termina. O estado da unidade raid X, pelo mymfs, não foi alterado.
