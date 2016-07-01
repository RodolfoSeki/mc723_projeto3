#!/bin/bash

ESC_SEQ="\x1b["
COL_RESET=$ESC_SEQ"39;49;00m"
COL_RED=$ESC_SEQ"31;01m"
COL_GREEN=$ESC_SEQ"32;01m"
COL_YELLOW=$ESC_SEQ"33;01m"
COL_BLUE=$ESC_SEQ"34;01m"
COL_MAGENTA=$ESC_SEQ"35;01m"
COL_CYAN=$ESC_SEQ"36;01m"
COL_BOLD=$ESC_SEQ";1m"

self="${0##*/}"

if [ $# -lt 1 ]; then
  printf "${COL_BOLD}%-8s${COL_RESET}: ./%s <path_pasta_do_fft>\n" "Uso" "$self"
  exit 1
fi


export LD_LIBRARY_PATH=/home/staff/lucas/mc723/hw_tools/systemc-2.3.1/lib-linux64/:$LD_LIBRARY_PATH
export PATH=$PATH:/home/staff/lucas/mc723/hw_tools/ArchC-2.2.0/installed/bin:/home/staff/lucas/mc723/hw_tools/mips-newlib-elf/bin/


printf "${COL_MAGENTA}%-12s${COL_RESET}\n" "make clean"
make -f Makefile.archc clean
printf "${COL_YELLOW}%-12s${COL_RESET}\n" "make"
make -f Makefile.archc

printf "${COL_CYAN}%-12s${COL_RESET}\n" "Complilando"
/home/staff/lucas/mips-newlib-elf/bin/mips-newlib-elf-gcc -specs=archc $1/*.c -o fft.mips -Os -lm


printf "${COL_RED}%-12s${COL_RESET}\n" "Executando"
./mips.x --load=fft.mips > $1/saida.out 2> $1/resultado.out


printf "${COL_CYAN}%-12s${COL_RESET}\n" "Comparando saída com codigo_original/saida_gcc.out"
diff $1/saida.out ../codigo_original/saida_gcc.out
SAIDA_GCC=$?
diff $1/saida.out ../codigo_original/saida.out
SAIDA_MIPS=$?
if [ $SAIDA_GCC -eq 0 ]; then
        printf "${COL_GREEN}%-12s${COL_RESET}\n" "Saída igual SAIDA GCC e diferente da SAIDA MIPS"
elif [ $SAIDA_MIPS -eq 0 ]; then
	printf "${COL_GREEN}%-12s${COL_RESET}\n" "Saída igual SAIDA MIPS e diferente da SAIDA GCC"
else
        printf "${COL_RED}%-12s${COL_RESET}\n" "Saída incorreta, verificar problema de aproximação"
fi

printf "${COL_CYAN}%-12s${COL_RESET}\n" "Parseando resultado"
#cat $1/resultado.out | awk 'BEGIN { ORS=" " }; /from instruction/{ getline; print $3 ; printf "\n" }' > $1/resultado.out

#printf "${COL_RED}%-12s:${COL_RESET} " "Numero de instruções"

#cat $1/resultado.out | awk '/instructions executed/{ print $5 }'

#cat $1/resultado.out | awk 'BEGIN { ORS=" " }; /from instruction/{ getline; print $3 ; printf "\n" }' > new_res

#mv new_res  $1/resultado.out

printf "${COL_BLUE}%-12s${COL_RESET}\n" "saida em resultado.out"
