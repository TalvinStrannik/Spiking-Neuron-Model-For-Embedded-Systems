#include <msp430g2553.h>

#define LED1  BIT0
#define LED2 BIT6
#define BOT BIT3

/*  ���������� ����������  */
unsigned int count=0;
unsigned int sw_start=0;
unsigned int sw_print=0;
/*  ���������� �������  */
void delay(void);
void transfer(unsigned int);
void init_UART(void);
void delay_min(void);

void init_pins()
{
    P1OUT = 0;
    P1DIR = LED1+LED2;  // P1.0 ����� �� ���������, P1.3 ���� ��� ������
  // ��������� ��� ������� ��� LaunchPad ������ 1.5
    P1REN |= BIT3+BIT4; //��������� ��������
    P1OUT |= BIT3+BIT4; //�������� ������ P1.3 �����
    P1IES |= BIT3+BIT4; // ���������� �� �������� 1->0 ���������� ����� IESx = 1.
    P1IFG &= ~(BIT3+BIT4); // ��� �������������� ������������ ������ ����������,
                    // �������� ��� ���� ��� P1.3 �� ���������� ����������
    P1IE |= BIT3+BIT4;   // ��������� ���������� ��� P1.3
}

void main(void) {

        WDTCTL = WDTPW + WDTHOLD;    // ��������� ���������� ������

        DCOCTL=0;
        BCSCTL1 = CALBC1_12MHZ; // ������������� ������� DCO �� ������������� 1 MHz.
        DCOCTL = CALDCO_12MHZ;

        init_pins();

        __enable_interrupt();

        for(;;){};
} // main

void delay(void)
{
    unsigned int n;
    for (n=0; n<60000; n++);
    for (n=0; n<60000; n++);
    for (n=0; n<60000; n++);
//    for (n=0; n<60000; n++);
//    for (n=0; n<60000; n++);
//    for (n=0; n<60000; n++);
//    for (n=0; n<60000; n++);
} // ��������

void delay_min()
{
    unsigned int n;
    for (n=0; n<60000; n++);
}

void init_UART()
{
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1SEL |= BIT1 | BIT2 ;
    P1SEL2 |= BIT1 | BIT2;

    /* Place UCA0 in Reset to be configured */
    UCA0CTL1 = UCSWRST;

    /* Configure */
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104; // 1MHz 9600
    UCA0BR1 = 0; // 1MHz 9600
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1

    /* Take UCA0 out of reset */
    UCA0CTL1 &= ~UCSWRST;

    /* Enable USCI_A0 RX interrupt */
    IE2 |= UCA0RXIE;
}

void init_T0()
{
    BCSCTL1 = CALBC1_12MHZ; // ������������� ������� DCO �� ������������� 1 MHz.
    DCOCTL = CALDCO_12MHZ;
//    TACCR0 = 62000;
    TACCR0 = 120;    // ������ � 62,500 �����, �� 0 �� 62,499.
    TACCTL0 = CCIE;        // ��������� ���������� ������� �� ���������� �������� CCR0.
    TACTL = TASSEL_2 + ID_0 + MC_1 + TACLR; // ��������� ������ ������ ������� Timer_A:
                                    // TASSEL_2 - �������� ������ SMCLK (SubMainCLocK),
                                    // �� ��������� ����������� �� ������ �� DCO
                                    // ID_3 - �������� ������� �� 8, �� 1MHz ��� ����� 125kHz
                                    // MC_1 - ����� ������� ����� (�� TACCR0)
                                    // TACLR - ��������� ��������� �������
}

void transfer(unsigned int i)

{
    init_UART();
    //i=192;
    while(i)
    {

          while (!(IFG2&UCA0TXIFG));
          UCA0TXBUF='0'+i%10;
          i/=10;
//          delay();
    }
//    delay();
    while (!(IFG2&UCA0TXIFG));
    UCA0TXBUF='\n';
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void CCR0_ISR(void)
{
  if(sw_print==1)
  {
         transfer(count);
         sw_start=0;
         sw_print=0;
         count=0;
         P1OUT ^= LED2;
  }
  else if(sw_start==1)
  {
      P1OUT = LED2|BOT|BIT4;
  }
} // CCR0_ISR

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
        switch(P1IFG & BOT)
        {
                case BOT:
                  {
                        delay();
                        P1IFG &= ~BIT3;  // �������� ���� ����������
                        if(sw_start == 0)
                        {
                           sw_start=1;
                           init_T0();
                           TAR=0;
                        }
                        else
                        {
                            sw_print=1;
                        }
                        return;
                  }
                default:
                  {
                    if(sw_start == 1)
                    {
                        count++;
                        P1IFG = 0;
                        return;
                    }
                  }
        }
} // P1_ISR