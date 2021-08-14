
# Sistema Operacional feSO

O sistema operacional feSO (Feso é um Sistema Operacional) foi desenvolvido para servir como uma ferramenta de aprendizado de sistemas operacionais. Para um visão geral do funcionamento do sistema, consultar a [monografia dos autores](https://hllustosa.github.io/feso-operating-system/feso-mono.pdf) ou o [artigo no REIC.](http://seer.ufrgs.br/index.php/reic/article/view/79913).

## Compilando e Executando

Para compilar o feSO no sistema Ubuntu (18.4 ou 20.4), primeiramente é necessário instalar os seguintes pacotes:

```bash
sudo apt-get install git 
sudo apt-get install mkisofs 
sudo apt-get install nasm
```

Após a instalação das dependências, é preciso clonar o repositório:

```bash
git clone https://github.com/hllustosa/feso-operating-system
```

Para executar o build do projeto, execute:

```bash
cd feso-operating-system
chmod +x ./build.sh 
chmod +x ./install_cross.sh 
sudo ./install_cross.sh ./build.sh
```
Após a compilação, verifique se o arquivo feso.iso foi gerado no diretório feso. Este é a imagem iso do sistema feSO, que pode ser usada para dar boot em um hypervisor, ou pode ser gravada em um CD, DVD ou pen-drive para a inicialização de um computador.