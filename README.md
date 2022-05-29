## :busts_in_silhouette: Team

![David Machado][david-pic] | ![Miguel Frreitas][mike-pic] | [Tomás Campinho][toms-pic]
:---: | :---: | :---:
[David Machado][david] | [Miguel Freitas][mike] | [Tomás Campinho][toms]

[david]: https://github.com/quantik-git
[david-pic]: https://github.com/quantik-git.png?size=120
[mike]: https://github.com/MrNameless10
[mike-pic]: https://github.com/MrNameless10.png?size=120
[toms]: https://github.com/TomasCampinho
[toms-pic]: https://github.com/TomasCampinho.png?size=120


## 📎 Summary

Pretende-se implementar um serviço que permita aos utilizadores armazenar uma cópia dos seus ficheiros de forma segura e eficiente, poupando espaço de disco. Para tal o serviço disponibilizará funcionalidades de compressão e cifragem dos ficheiros a serem armazenados.

O serviço deverá permitir a submissão de pedidos para processar e armazenar novos ficheiros bem como para recuperar o conteúdo original de ficheiros guardados previamente. Ainda, deverá ser possível consultar as tarefas de processamento de ficheiros a serem efetuadas num dado momento e estatistícas sobre as mesmas

## :rocket: Getting Started
No ficheiro anexo pode encontrar um arquivo Zip contendo a estrutura de pastas e ficheiros que devem procurar usar no desenvolvimento e na submissão do vosso trabalho. Descarregue e expanda esse ficheiro. Logo de seguida renomeie a pasta "grupo-xx" que foi criada, substituindo o "xx" pelo número do seu grupo.

Segue-se uma breve descrição das pastas relativas ao processo de compilação e linkagem do projecto:

    src/: deverá conter o código-fonte do vosso projecto;
    bin/: deverá conter os executáveis resultantes da "linkagem" do código-objecto;
    docs/: deverá conter o relatório do desenvolvimento do projecto.

Nessa estrutura pode encontrar ainda as seguintes pastas:

    etc/: deverá conter o(s) ficheiro(s) de configuração do servidor (o ficheiro "sdstored.conf" apresenta um exemplo de configuração);
    bin/sdstore-transformations/: contém programas executáveis para Linux (e Mac) que funcionam como transformações de ficheiros (recebem o conteúdo de ficheiros de texto(em qualquer formato) pelo seu standard input e produzem o conteúdo de um ficheiro de texto no seu standard output;
    samples/: contém ficheiros de texto que pode usar para testar a execução das transformações disponibilizadas e para testar o correcto funcionamento do serviço a desenvolver;
   
Antes de qualquer envio de processo, deve ser inicializado o servidor executando o comando:
```bash
$ ./sdstored etc/sdstored.conf bin/sdstore-transformations
```

Se quiser testar as transformações com o servidor a correr poderá executar um comando tal como:
```bash
$ ./sdstore proc-file <priority> ../samples/file-a ../samples/file-a-output bcompress nop gcompress encrypt
```

De forma geral:
```bash
$ ./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...
```


**Note que**:

  *  O servidor deverá dar prevalência a pedidos com maior prioridade. Para realizar esta funcionalidade, cada operação deve ser acompanhada da sua prioridade o argumento **priority** (identificadas como inteiros de 0 a 5, em que 5 atribui prioridade máxima)
  * para recuperar o conteúdo original do ficheiro file-a, a operação *proc-file* deve invocar as transformações de compressão e cifra pela ordem inversa.
  * a transformação **nop** é especial, uma vez que é idempotente (i.e., pode ser aplicada em qualquer ordem e entre quaisquer transformações sem modificar o resultado esperado).

O cliente pode consultar (comando status), a qualquer instante, o estado de funcionamento do servidor. Nomeadamente, os pedidos de processamento em execução, bem como, o estado de utilização das transformações:

```bash
$ ./sdstore status
```


A pasta principal comtém também um ficheiro Makefile com um conjunto de regras básicas para a compilação código-fonte, geração de executáveis (linkagem de código-objecto) e limpeza do projecto. Ainda assim poderá ter necessidade de adaptar este ficheiro conforme as especificidades do projecto. Para compilar e gerar os seus programas executáveis apenas deverá ser necessário executar o comando "make". Para limpar os ficheiros de código-objecto e os seus programas executáveis poderá fazer "make clean".

Assuma que é da responsabilidade do cliente aplicar as transformações pela ordem correta ao processar o conteúdo de ficheiros geridos pelo serviço. Por exemplo, se um dado ficheiro, numa operação proc-file, for comprimido e cifrado (por esta ordem), a operação proc-file inversa, com o objetivo de reverter o ficheiro processado para o seu conteúdo original, deverá usar as transformações decifrar e descomprimir (por esta ordem). O mesmo se aplica a usar o mesmo algoritmo para comprimir e descomprimir
