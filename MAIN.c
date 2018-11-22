#include <msp430.h>
#include "uart_STDIO.h"

#define STEPy        BIT0   
#define DIRy         BIT6
#define STEPx        BIT7
#define DIRx         BIT3
#define STEPz        BIT4
#define DIRz         BIT5
#define BOTON_VEL    BIT1
#define INTERRUPTORz BIT0
#define BOTONz       BIT3

/*----DECLARACION VARIABLES GLOBALES--------*/
unsigned int ejex, ejey;
volatile int t;
volatile char aux=2;
volatile char estado=0;
volatile char modo=0;
volatile char envia_datos;
long cx=0,cy=0,cz=0;
char dx,dy,dz;
char cadenax[3],cadenay[3],cadenaz[3];

/*--------FUNCIONES AUXILIARES--------------*/
//Función para el manejo de los motores:
void mover_motor(unsigned int ejex, unsigned int ejey) {    // Esta función se encargara de dar los pulsos necesarios a los drivers para mover adecuadamente
                                                            // los motores en funcion de la posición de nuestro Joystick
    if (t>=aux) {
        t=1;

        if (ejex<500 && ejey<500) {                     // X POSITIVO; Y POSITIVO
            P1OUT &=~ DIRx;
            P2OUT &=~DIRy;
            P1OUT ^= STEPy;
            P2OUT ^= STEPx;
            cx++;
            cy++;
        }
        else if (ejex<500 && ejey>500 && ejey<600) {    // X POSITIVO
            P1OUT &=~ DIRx;
            P2OUT ^= STEPx;
            cx++;
        }
        else if (ejex>600 && ejey>500 && ejey<600) {    // X NEGATIVO
            P1OUT |= DIRx;
            P2OUT ^= STEPx;
            cx--;
        }
        else if (ejex<500 && ejey>600) {                // X POSITIVO; Y NEGATIVO
            P1OUT &=~ DIRx;
            P2OUT |= DIRy;
            P1OUT ^= STEPy;
            P2OUT ^= STEPx;
            cx++;
            cy--;
        }
        else if (ejey>600 && ejex>500 && ejex<600) {    // Y NEGATIVO
            P2OUT |= DIRy;
            P1OUT ^= STEPy;
            cy--;
        }
        else if (ejey<500 && ejex>500 && ejex<600) {    // Y POSITIVO
            P2OUT &=~ DIRy;
            P1OUT ^= STEPy;
            cy++;
        }
        else if (ejex>600 && ejey>600) {                // X NEGATIVO; Y NEGATIVO
            P1OUT |= DIRx;
            P2OUT |= DIRy;
            P1OUT ^= STEPy;
            P2OUT ^= STEPx;
            cx--;
            cy--;
        }
        else if (ejex>600 && ejey<500) {                // X NEGATIVO; Y POSITIVO
            P1OUT |= DIRx;
            P2OUT &=~ DIRy;
            P1OUT ^= STEPy;
            P2OUT ^= STEPx;
            cx--;
            cy++;
        }
        else if (t<0) t=0; //Condición desb.

        // Movimiento en el EJE Z
        if (!(P2IN&BOTONz)) {
            if (!(P2IN&INTERRUPTORz)) {  //Z POSITIVO
                P1OUT |= DIRz;
                P1OUT ^= STEPz;
                cz++;
            }
            else {                       //Z NEGATIVO
                P1OUT &=~ DIRz;
                P1OUT ^= STEPz;
                cz--;
            }
        }

        // Calculo coordenadas en cm. 1 cm --> 400 steps
        if (cx<0) dx=(-cx)/400;     // Si los pasos son negativos calculo el valor absoluto
        else dx=cx/400;

        if (cy<0) dy=(-cy)/400;
        else dy=cy/400;

        if (cz<0) dz=(-cz)/400;
        else dz=cz/400;
    }
}

void imprime_pantalla (unsigned int ejex, unsigned int ejey) {  // Imprimo en pantalla el valor de las coordenadas y la velocidad actual

    if (ejex>600 || ejex<500 || ejey>600 || ejey<500 || !(P2IN&BOTONz)) envia_datos=1;
    else {
        if (envia_datos) {
            //Imprimimos las coordenadas:
            numero_cadena(dx,cadenax,10);
            numero_cadena(dy,cadenay,10);
            numero_cadena(dz,cadenaz,10);
            UARTprint("X:");
            if (cx<0) UARTprint("-");
            UARTprint(cadenax);
            UARTprint(" Y:");
            if (cy<0) UARTprint("-");
            UARTprint(cadenay);
            UARTprint(" Z:");
            if (cz<0) UARTprint("-");
            UARTprint(cadenaz);
            //Imprimimos la velocidad:
            UARTprint(" Velocidad:");
            if (aux==2) UARTprintCR("1");
            else if (aux==5) UARTprintCR("2");
            else if (aux==10) UARTprintCR("3");
            else if (aux==30) UARTprintCR("4");
            envia_datos=0;
        }
    }

}

void numero_cadena(char value, char* result, int base) {    // Esta función convierte un valor numerico en caracter

      if (base < 2 || base > 36) { *result = '\0';}

      char* ptr = result, *ptr1 = result, tmp_char;
      int tmp_value;

      do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
      } while ( value );

      if (tmp_value < 0) *ptr++ = '-';
      *ptr-- = '\0';
      while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
      }

    }


//Funciones para el manejo del ADC:
int lee_ch(char canal){
    ADC10CTL0 &= ~ENC;                  //Deshabilita el ADC
    ADC10CTL1&=(0x0fff);                //Borra canal anterior
    ADC10CTL1|=canal<<12;               //selecciona nuevo canal
    ADC10CTL0|= ENC;                    //Habilita el ADC
    ADC10CTL0|=ADC10SC;                 //Empieza la conversión
    LPM0;                               //Espera fin en modo LPM0
    return(ADC10MEM);                   //Devuelve valor leido
}

void inicia_ADC(char canales) {
    ADC10CTL0 &= ~ENC;                  //Deshabilita ADC
    ADC10CTL0 = ADC10ON | ADC10SHT_3 | SREF_0| ADC10IE;     //Enciende ADC, S/H lento, REF:VCC, con INT
    ADC10CTL1 = CONSEQ_0 | ADC10SSEL_0 | ADC10DIV_0 | SHS_0 | INCH_0;       //Modo simple, reloj ADC, sin subdivision, Disparo soft, Canal 0
    ADC10AE0 = canales;                 //Habilita los canales indicados
    ADC10CTL0 |= ENC;                   //Habilita el ADC
}



/*----------FUNCION PRINCIPAL---------------*/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

   /*-----CONFIGURACION DEL RELOJ------*/
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

   /*-----CONFIGURACION PINES E/S------*/
  //PUERTO 1:
    P1DIR |= (STEPy | STEPz | DIRx | DIRz);
    P1OUT |= (STEPy | STEPz | DIRx | DIRz);
  //PUERTO 2:
    P2SEL &=~ (STEPx | DIRy);
    P2DIR |= (DIRy | STEPx);
    P2OUT |= (STEPx | DIRy);

    P2DIR|=(BIT2+BIT4);     //P2.2 y P2.4 salida PWM
    P2SEL|=(BIT2+BIT4);
    P2SEL2&=~(BIT2+BIT4);

    P2OUT = BOTONz+INTERRUPTORz+BOTON_VEL;
    P2REN = BOTONz+INTERRUPTORz+BOTON_VEL;

    P2IE|=BOTON_VEL;        //Habilito int. botón velocidad
    P2IES|=BOTON_VEL;
    P1IFG &= ~BOTON_VEL;    //Quito flag, por si acaso

  //CONFIGURACION UART:
    P1SEL       |= (BIT1 + BIT2);   // P1.1=RXD, P1.2=TXD (Configuran Los Puertos)
    P1SEL2      |= (BIT1 + BIT2);
    P1DIR       |= BIT2;

    UCA0CTL1 |= UCSWRST;  // Reset
    UCA0CTL1 = UCSSEL_2 | UCSWRST;  /* UCSSEL_2 : SMCLK (1MHz)
                                       Reset sigue activo     */
    UCA0BR0 = 104;        // 1MHz/104=9615,38...
    UCA0CTL1 &= ~UCSWRST; // Quita reset
    IFG2 &= ~(UCA0RXIFG); // Quita flag
    IE2 |= UCA0RXIE;      // y habilita int. recepcion

   /*-----CONFIGURACION TIMERS---------*/
  //Timer A0:
    TA0CCTL0=CCIE;  //CCIE=1
    TA0CTL=TASSEL_1|ID_0| MC_1; //ACLK, ID=1, UP
    TA0CCR0=4;      //Periodo 5
  //Timer A1:
    TA1CCTL1=OUTMOD_7;      //OUTMOD=7 (Activo a nivel bajo)
    TA1CCTL2=OUTMOD_7;      //OUTMOD=7
    TA1CTL=TASSEL_2| MC_1;  //SMCLK, DIV=1, UP
    TA1CCR0=9999;           //periodo=10000
    TA1CCR1=0;              //duty cycle GRN
    TA1CCR2=9999;           //duty cycle RED

    __bis_SR_register(GIE);

    while(1) {

            while (modo==0) {   /* STAND-BY */
                TA1CCR1=0;                      // Enciendo LED rojo
                TA1CCR2=9999;
                UARTprintCR("Start (S): ");     // Pongo el micro en bajo consumo y espero a recibir una 'S'
                LPM0;
            }

            while (modo==1) {   /* EN FUNCIONAMIENTO */
                __delay_cycles(1);
                ejey=lee_ch(6);                 // Leo valores del eje x y del eje y
                ejex=lee_ch(7);
                mover_motor(ejex,ejey);         // Muevo motores en funcion a dichos valores
                imprime_pantalla(ejex,ejey);    // Muestro los datos de posición y velocidad por pantalla

                if (ejex>600 || ejex<500 || ejey>600 || ejey<500 || !(P2IN&BOTONz)) TA1CCR2=9999; // LED de color amarillo cuando movemos joystick
                else TA1CCR2=0;
            }
        }
    return 0;
}


/*------RUTINAS DE INTERRUPCION---------*/
#pragma vector=ADC10_VECTOR
__interrupt void ConvertidorAD(void)
{
    LPM0_EXIT;  //Despierta al micro al final de la conversión
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR_HOOK(void)
{
    t=t+2;      //Incrementa una variable global
}

#pragma vector=PORT2_VECTOR
__interrupt void Interrupcion_P21(void)
{
    if(P2IFG & BOTON_VEL) {       //Interrupcion ocasionada por el botón de velocidad

        while(!(P2IN&BOTON_VEL)); //Espero a que soltemos el boton

        estado++;
        switch(estado) {          //Maquina de estados para la velocidad
            case 1:
                aux=2;
                break;
            case 2:
                aux=5;
                break;
            case 3:
                aux=10;
                break;
            case 4:
                aux=30;
                break;
            default:
                estado=0;
        }
        envia_datos=1;      // Envio datos por pantalla
        P2IFG&=~BOTON_VEL;  // Borra flag
    }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK(void)
{
    int caracter;
    caracter=UCA0RXBUF;         // Leo dato recibido

    if (caracter=='S') {        // START
        UARTprintCR("S");
        modo=1;
        inicia_ADC(BIT6|BIT7);  // Inicio ADC en canales 6 y 7
        TA1CCR1=9999;           // Enciendo LED verde
        TA1CCR2=0;
        LPM0_EXIT;
    }
    else if (caracter=='P') {   // PARADA
        UARTprintCR("P");
        ADC10CTL0 &= ~ENC;      // Deshabilito ADC
        modo=0;
    }
    else if (caracter=='O') {   // ORIGEN COORDENADAS
        UARTprintCR("Origen establecido");
        cx=0;   // Reseteo cx, cy, cz
        cy=0;
        cz=0;
        }
    IFG2 &= ~(UCA0RXIFG); /* Quita flag */
}
