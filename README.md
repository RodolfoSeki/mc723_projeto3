# Projeto 3: Multicore e Aceleração em Hardware

Klaus Rollmann                  146810

Luiz Rodolfo Sekijima           117842

Wendrey Lustosa                 148234

Renan Camargo de Castro         147775


## Roteiro

O benchmark utilizado será a Fast Fourier Transform, que computa a transformada de Fourier de uma sequência de floats.
A implementação será baseada em algoritmos já implementados da transformada de Fourier, adicionando melhoras com o paralelismo e aceleração de hardware.

### Software

* [FFT (Fast Fourier Transform)](https://www.nayuki.io/page/free-small-fft-in-multiple-languages)

### Ganho de Paralelização

O ganho paralelização pode ser usado em várias partes do código, como na geração das tabelas trigonométricas, permutação dos bits e a aproximação dos valores.

### Ganho de Aceleração de Hardware

O ganho de aceleração de hardware será no cálculo de operações envolvendo números de ponto flutuante, e também operações de seno e cosseno.

### Experimentos

Os experimentos consistem em avaliar o ganho de desempenho de cada componente separadamente, e por fim avaliar a melhora total obtida.
Os experimentos serão os mostrados abaixo;
* **Experimento 1:** cálculo da transformada de fourier sem aceleração e sem paralelismo
* **Experimento 2:** cálculo da transformada de fourier sem aceleração e com 2 processadores
* **Experimento 3:** cálculo da transformada de fourier sem aceleração e com 4 processadores
* **Experimento 4:** cálculo da transformada de fourier sem aceleração e com 8 processadores
* **Experimento 5:** cálculo da transformada de fourier com aceleração de ponto flutuante e sem paralelismo
* **Experimento 6:** cálculo da transformada de fourier com aceleração de ponto flutuante e cálculo de operações trigonométricas e sem paralelismo
* **Experimento 7:** cálculo da transformada de fourier com cálculo de operações trigonométricas e sem paralelismo
* **Experimento 8:** cálculo da transformada de fourier com aceleração de ponto flutuante e cálculo de operações trigonométricas e com melhor configuração de paralelismo
