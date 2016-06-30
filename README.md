

# Projeto 3: Multicore e Aceleração em Hardware

Luiz Rodolfo Sekijima           117842

Klaus Rollmann                  146810

Wendrey Lustosa                 148234

Renan Camargo de Castro         147775


## Roteiro

O benchmark utilizado será a Fast Fourier Transform, que computa a transformada de Fourier de uma sequência de floats.
A implementação será baseada em algoritmos já implementados da transformada de Fourier, adicionando melhoras com o paralelismo e aceleração de hardware.

### Software

* [FFT (Fast Fourier Transform) em C](https://www.nayuki.io/page/free-small-fft-in-multiple-languages)

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
* **Experimento 6:** cálculo da transformada de fourier com cálculo de operações trigonométricas e sem paralelismo
* **Experimento 7:** cálculo da transformada de fourier com aceleração de ponto flutuante e cálculo de operações trigonométricas e sem paralelismo
* **Experimento 8:** cálculo da transformada de fourier com aceleração de ponto flutuante e cálculo de operações trigonométricas e com melhor configuração de paralelismo


## Relatório

### Introdução

O objetivo do projeto é estudar o ganho de desempenho de um programa quando é realizada aceleração de hardware e aplicado multiprocessamento na sua execução em um simulador de processador MIPS. O programa escolhido realiza o cálculo da transformada de Fourier para dois vetores de mesmo tamanho que representam valores reais e imaginários. Observamos que poderiamos otimizar o programa visto que há trechos de código que podem ser paralelizados e operações com ponto flutuante e cálculos de seno e coseno podem ser aceleradas em um componente de hardware separado.

### Fast Fourier Transform

### Modificações

#### ffttest.c

O programa aplicava o FFT em vetores gerados aleatóriamente, então para sabermos o resultado esperado, passamos a entrada incializada no arquivo input.h. No entanto, mesmo sabendo o resultado esperado, os experimentos apresentam diferenças na saída após aplicar a FFT, isso se deve ao fato de que mesmos os algoritmos "exatos" de FFT tem erros de aritimética de ponto flutuante. Então como nos experimentos a aceleração e otimização alteram a ondem que as operações são realizadas, observamos resultados com diferença da ordem de 10^(-4) entre os experimentos.

### Paralelização

Para permitir o processamento paralelo, as seguintes funções foram implementadas:

~~~c
void AcquireLock(){
	while(*lock);
}
void ReleaseLock(){
	*lock = 0;
}
void incrementSemaphore(volatile int* s){
	AcquireLock();
	*s++;
	ReleaseLock();
}
void acquireSemaphore(volatile int* s){
	while((*s) < NUMBER_OF_CORES);
}
void getVectorParcel(int * start, int * end, int size, int procNumber){
	*start = procNumber*(size/NUMBER_OF_CORES);
	*end = (procNumber+1)*(size/NUMBER_OF_CORES);
	if(procNumber == NUMBER_OF_CORES-1){
		//se tem resto, tem que colocar na ultima parcela
		*end += size%NUMBER_OF_CORES;
	}
}
~~~

O semáforo é utilizado para sincronizar os processadores após todos terminarem de fazer o processamento paralelo, esta espera é necessária para garantir que todos os dados foram calculados corretamente antes de proseguir. E a função getVectorParcel é usada para calcular o intervalo em que o processador deve realizar as operações em paralelo.

As partes paralelizadas foram:

* Função transform_radix2 - Bit-reversed addressing permutation
* Função transform_bluestein - Pré-processamento 
* Função convolve_complex - Cálculo da convolução circular 


### Aceleração de Hardware

### Resultados

#### Instruções

#### Total de instruções



### Conclusão
