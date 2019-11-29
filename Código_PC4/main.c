#include <msp430.h>


int k;
int time_ms;
int distance;
long sensor;

void delay(volatile unsigned long i)
{
    do (i--);
    while (i != 0);
}

int main(void)
{
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;                     // Clock de 1Mhz
  WDTCTL = WDTPW + WDTHOLD;                 // Desliga o WatchdogTimer

  CCTL0 = CCIE;                             // Habilita a interrupção pelo CCR0
  CCR0 = 1000;                              // 1ms em 1mhz
  TACTL = TASSEL_2 + MC_1;                  // SMCLK - UpMode

  P1IFG  = 0x00;                            // Limpa todas as flags de interrupçao

  P1DIR |= 0x01;                            // P1.0 como saida do LED vermelho
  P1OUT &= ~0x01;                           // Desliga o LED

  P2DIR |= 0x01;                            // P2.0 como saida do LED verde
  P2OUT &= ~0x01;                           // Desliga o LED

  _BIS_SR(GIE);                             // Habilita a interrupcao global

  while (1)
  {
        P1IE &= ~0x01;                  // Desabilita a interrupcao
        P1DIR |= 0x02;                  // Pino do TRIGGER (P1.1) como saida
        P1OUT |= 0x02;                  // Gera o pulso
        __delay_cycles(10);             // Delay de 10us
        P1OUT &= ~0x02;                 // Para o pulso
        P1DIR &= ~0x04;                 // Pino do ECHO (P1.2) como entrada
        P1IFG = 0x00;                   // Limpa a flag, por precaução
        P1IE |= 0x04;                   // Habilita a interrupção pelo pino do ECHO
        P1IES &= ~0x04;                 // Define a borda de subida no pino ECHO

        __delay_cycles(30000);          // Delay de 30ms (depois desse tempo, o ECHO acaba se nenhum objeto for detectado)
        distance = sensor / 58;         // Converte o valor de ECHO para cm

        P1DIR |= BIT6;                  // P1.6/TA0.1 É USADO PARA O PWM, QUE FUNCIONA COMO A SAÍDA
        P1OUT = 0;                      // LIMPA AS SAIDAS P1
        P1SEL |= BIT6;                  // P1.6 SELECIONA TA0.1

        P1DIR &= ~BIT3;                 // Botão
        P1REN |= BIT3;
        P1OUT |= BIT3;

        // O clock é de 1MHZ, sabendo que 1000ms = 1Hz => CCRO = 20000
        // Temos que: 20000(1000000 / 1000 * 20) é igual a um período de 20ms

        TA0CCR0 = 20000-1;                         // PERIODO DO PWM TA0.1
        TA1CCR0 = 20000-1;                         // PERIODO DO PWM TA1.1

        // SETANDO 2000 -> 0deg (Pós SERVO)
        TA0CCR1 = 2000;                            // CCR1 PWM duty cycle
        TA1CCR1 = 2000;                            // CCR1 PWM duty cycle

        TA0CCTL1 = OUTMOD_7;                       // CCR1 reset/set
        TA0CTL   = TASSEL_2 + MC_1;                // SMCLK, up mode
        TA1CCTL1 = OUTMOD_7;                       // CCR1 reset/set
        TA1CTL   = TASSEL_2 + MC_1;                // SMCLK, up mode

    if (distance > 30 && distance != 0 ) {
      P1OUT |= 0x01;                                // Se a distância calculada for menor que 30 cm e diferente de 0, o LED vermelho acende
      P2OUT &= ~0x01;                               // led verde é apagado
    }
    else {
      P1OUT &= ~0x01;                               // Caso contrario, o led vermelho permanece desligado
      P2OUT |= 0x01;                                // LED verde acende
    }

    if((P1IN&BIT3)==0)
        {
            for(k=0;k<4;k++){                       // Repete 4 vezes (apresentação)
                aciona_motor();
                __delay_cycles(10000000);           // Abre a escotilha a cada 10s
            }
        }
  }
}

void aciona_motor(){
        delay(30000);
            TA0CCR1 = 1000;
            TA1CCR1 = 2000;

        delay(30000);
            TA0CCR1 = 2000;
            TA1CCR1 = 1000;
     }

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  if (P1IFG & 0x04)             // Verifica se tem alguma interrupçãoo pendente
  {
    if (!(P1IES & 0x04))        // Verifica se tem uma borda de subida
    {
      TACTL |= TACLR;           // Se tem, limpa o timer A
      time_ms = 0;
      P1IES |= 0x04;            // Borda de descida
    }
    else
    {
      sensor = (long)time_ms * 1000 + (long)TAR;        // Calcula o comprimento de ECHO
    }
    P1IFG &= ~0x04;                                     // Limpa a flag~0
  }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  time_ms++;
}
