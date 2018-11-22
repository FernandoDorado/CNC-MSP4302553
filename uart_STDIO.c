#include "uart_STDIO.h"
#include <msp430.h>


/*---------FUNCIONES AUXILIARES--------------*/

// Funciones para el manejo de la consola a través de la UART:
void UARTprintc (char c) {
    while (!(IFG2 & UCA0TXIFG));    //espera a Tx libre
                    UCA0TXBUF = c;
}

void UARTprint (const char * frase) {
    while(*frase)UARTprintc(*frase++);
}

void UARTprintCR (const char *frase) {
    while(*frase)UARTprintc(*frase++);
    UARTprintc(10);
    UARTprintc(13);

}