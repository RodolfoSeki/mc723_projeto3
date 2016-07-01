

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

O programa possui dois algoritmos para calcular a transformada rápida de Fourier, o primeiro é o radix-2 Cooley–Tukey, que é aplicado se o número de elementos da entrada é uma potência de 2. Caso contrário é aplicado o algoritmo de Bluestein, que realiza cálculos de convolução circular. Ambos os algoritmos usam tabelas de seno e coseno e diversas operações de ponto flutuante para fazer o cálculo.

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
Para paralelizar um for simples, foi feito da seguinte forma, utilizando as funções anteriores:

~~~c
	// Temporary vectors and preprocessing
	int start, end;
	getVectorParcel(&start,&end, n, procNumber);
	for (i = start; i < end; i++) {
		areal[i] =  real[i] * cos_mod(i,n) + imag[i] * sin_mod(i,n);
		aimag[i] = -real[i] * sin_mod(i,n) + imag[i] * cos_mod(i,n);
	}
	incrementSemaphore(&thirdSemaphore);
	acquireSemaphore(&thirdSemaphore);

~~~

As partes paralelizadas foram:

* Função transform_bluestein - Pré-processamento 
* Função convolve_complex - Cálculo da convolução circular 


### Aceleração de Hardware
Para fazer a aceleração via hardware, foi utilizado dois periféricos conectados ao router: **ac\_tlm\_trigonometric** e **ac\_tlm\_float**.
#### Aceleração de Float
Para essas operações, foram colocados nos seguintes enredeços:

* SUM_ADDRESS 0x6600000
* SUB_ADDRESS 0x6700000
* MUL_ADDRESS 0x6800000
* DIV_ADDRESS 0x6900000
* WRITE_A     0x6700000
* WRITE_B     0x6700001

A leitura dos resultados é feita nos addresses 0x66,0x67,0x68,0x69, de acordo com a operação. e a escrita dos operandos para a operação: a op b, deve ser escrito nos endereços 0x6700000 e 0x6700001.

Os códigos tiveram que ser alterados, para usar o periférico, para isso todas as operações envolvendo ponto flutuante foram substituidas pelas respectivas funções que acessam os endereços. Um exemplo de operação de soma é mostrado abaixo.

	volatile int *sum_op = (volatile int *)  SUM_ADDRESS;
	
	volatile int *write_a = (volatile int *) WRITE_A;
	volatile int *write_b = (volatile int *) WRITE_B;
	
	float sum_acc (float value_a, float value_b){
        (*write_a) = *((uint32_t*)&value_a);
        (*write_b) = *((uint32_t*)&value_b);
        return *((float*)&(*sum_op));
	}

Para receber o valor em float no periférico, foi necessário passar os bits do float para uint32_t. O mesmo teve que ser feito na do periférico, assim como no próprio periférico. Para isso foram usadas as seguintes primitivas 
	
float para uint32_t

	*((uint32_t*)&valorfloat)
	
uint32_t para float

	*((float*)&valorint)
	
No periférico, também é necessário fazer a conversão de endianness, passar de uint32_t para float, e então utilizar a função htonl() e ntohl() para calcular à partir do valor passado.

#### Aceleração de Operações trigonométricas
Para essa operação, foi colocado no endereço 0x6500001 a função seno e em 0x6500000 a função cosseno. O funcionamento é simples, basta escrever o valor que se deseja calcular a função no endereço especificado e ler do mesmo endereço em seguida.

No periférico, é necessário fazer a conversão de endianness, passar de uint32_t para float, e então utilizar a função desejada de acordo com o endereço para calcular à partir dos valores passados, exatamente como no acelerador de floats.

### Resultados

Devido a dificuldades de paralelizar as funções de mencionadas a cima, foi possível realizar processamento com 2 cores, os experimentos com 4 e 8 cores não foram bem sucedidos.
Foi possível observar que os procedimentos que utilizam for no programa, na maioria das vezes acessam posições do vetor inteiro, não sendo restrito ao local. Isso dificultou muito a realização da paralelização.

Colocamos a saída do simulador em uma [tabela](https://docs.google.com/spreadsheets/d/1O20H8Fu9ouf4FdewwVQRJbg8gGWX_GKx7_z5pVvQNdw/edit?usp=sharing), mostrando quantas instruções de cada tipo foram executadas em cada experimento e o total de intruções executadas. As cores nos dados dos experimentos 2 a 8 indicam se os valores são maiores ou menores quando comparados com o experimento 1.

Podemos ver que os experimentos de aceleração de hardware tem um impacto maior no número de instruções, seja para melhor ou para pior, tivemos melhoras em 32% em desempenho (aceleração de float e operações trigonometricas) e quedas de 18% (apenas aceleração de operações de float), enquanto a paralelização em 2 cores reduziu o número de instruções em apenas 1.3%. Mas o experimento 2 conseguiu processar paralelamente partes do código, reduzindo o total de instruções executadas.


No experimento 8 foram incluidas a aceleração de hardware de floats e operações trigonometricas assim como o processamento em 2 cores, com isso obtivemos o nosso melhor resultado, apesar do resultado ser semelhante ao do experimento 7 há melhoras devido ao paralelismo, se o paralelismo fosse feito com mais cores ou em mais partes do programa, este resultado se destacaria ainda mais.

### Conclusão
Com os experimentos 5,6,7, é possível concluir que, para o programa FFT, uma aceleração de FLOAT em conjunto com a aceleração TRIGONOMÉTRICA, apresenta maior desempenho, pois diminui consideravelmente a quantidade de instruções executadas no total.
Também é possível perceber que não é necessariamente melhor apenas acelerar as operações de float em separado, pois o seu desempenho foi o pior dentre os três experimentos. Já a aceleração trigonométrica apresentou melhora significativa quando isolada, o que mostra a alta dependência do programa com cálculos trigonométricos.
A parelelização apresentou resultados pouco expressivos mas no geral a contagem de instruções melhoram, assim foi bem sucedida.

Por fim, a união das duas otimizações atingue o objetivo do nosso projeto, desenvolver uma aplicação com processamento paralelo e aceleração de hardware, apesar do paralelismo ter sofrido alguns problemas podemos ver melhoras no desempenho do programa.

