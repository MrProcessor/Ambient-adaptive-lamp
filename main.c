#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#define LED_D1 PB2 //LED pin
#define S1 PD5 //S1 microswitch pin (ON/OFF)
#define S2 PD6 //S2 microswitch pin (LED's brightness depends on the ambient brightness)
#define S3 PD7 //S3 microswitch pin (turns off light after 10 seconds)

bool state_S1;
bool state_S2;

volatile uint8_t countOVF; //count interrupts from timer0
volatile uint8_t tim2Flag; 
uint8_t tim2Delay; //

void timer0_state(char state) //turn on/off timer0
{
    if(state == 1)
    {
        TCCR0|=(1<<CS02)|(1<<CS00); //prescaler = 1024
        TIMSK|=(1<<TOIE0); //permission for interrupts from timer0 overflow event
        TIFR|=(1<<TOV0);
    }
    else TCCR0&=~((1<<CS02)|(1<<CS00)); //turn off timer0
}

void timer1pwm(char state_timer1) //turn on/off timer1
{
    if(state_timer1 == 1)
    {
        TCCR1A|=(1<<WGM10); //Fast PWM Mode, 8-bit
        TCCR1B|=(1<<WGM12);
        TCCR1A|=(1<<COM1A1);
        TCCR1B|=(1<<CS10); //switch on timer1 (prescaler = 1)
        TIMSK|=(1<<TOIE1)|(1<<OCIE1A); //permission for interrupts from timer1 overflow event (and from equalization TCNT1 and OCR1A registers)
    }
    else TCCR1B&=~(1<<CS10); //turn off timer1
}

void adcConf(char state_adc) //ADC configuration
{
    if(state_adc == 1)
    {
        ADCSRA|=(1<<ADEN); //turn on ADC
        ADMUX|=(1<<MUX0); //measurement from PC1
        ADCSRA=1<<ADFR; //enable free runnig mode for ADC
        ADCSRA=1<<ADIE; //activate conversion complete interrupt
        ADCSRA=(1<<ADPS2)|(1>>ADPS1)|(1<<ADPS0); //prescaler = 128
        ADMUX|=(1<<REFS0); //voltage reference (VCC)
        ADMUX|=(1<<ADLAR); //left adjust result in ADCH
        ADCSRA|=1<<ADSC; //start conversion
    }
    else ADCSRA&=~(1<<ADEN); //turn off ADC
}

void timer2ctcInit() //initialize timer2
{
    TCCR2|=(1<<WGM21); //CTC Mode
    TCCR2|=(1<<CS21); //prescaler = 8 --> f = 1 000 000 Hz/8/125 = 1000 Hz
    OCR2=125;
    TIMSK|=(1<<OCIE2); //enable compare match interrupt (TCNT2 == OCR2)
}

int main(void)
{
    sei(); //set global interrupt enable
    DDRB|=(1<<LED_D1); //LED --> output
    PORTD|=(1<<S1)|(1<<S2)|(1<<S3); //connect pull-up resistors for S1, S2, S3 inputs
    timer2ctcInit();

    while (1)
    {
        if(tim2Flag == 1) //check every 1 ms
        {
            if(!(PIND&(1<<S1))) //if S1 is pressed
            {
                tim2Delay++;
                if(tim2Delay == 100) //button has been pressed for 100 ms (condition to eliminate contact vibrations)
                {
                    if(state_S1 == 0) //if lamp was turned on
                    {
                        PORTB|=(1<<LED_D1)
                        timer0_state(0);
                        state_S1 = 1;
                    }
                    else
                    {
                        PORTB&=~(1<<LED_D1);
                        timer1pwm(0);
                        timer0_state(0);
                        state_S1 = 0;
                    }
                    
                }
            }
            if(!(PIND&(1<<S2)) && (state_S1 == 1)) //if S2 is pressed and the lamp was turned on
            {
                tim2Delay++;
                if(tim2Delay == 100)
                {
                    tim2Delay = 0;
                    if(state_S2 == 0)
                    {
                        timer1pwm(1);
                        adcConf(1);
                        state_S2=1;
                    }
                    else
                    {
                        adcConf(0);
                        timer1pwm(0);
                        PORTB|=(1<<LED_D1);
                        state_S2 = 0;
                    }
                    
                }
            }
            if(!(PIND&(1<<S3)) && (state_S1 == 1)) //if S3 is pressed and the lamp was turned on
            {
                tim2Delay++;
                if(tim2Delay == 100)
                {
                    PORTB|=(1<<LED_D1);
                    tim2Delay = 0;
                    timer0_state(1);
                    state_S1 = 0;
                }
            }
            tim2Flag = 0;
        }
    }
}


ISR(TIMER2_COMP_vect)
{
    tim2Flag = 1;
}

ISR(TIMER0_OVF_vect)
{
    TCNT0 = 12;
    countOVF++;
    if(countOVF == 40)
    {
        PORTB&=~(1<<LED_D1)
        liczOVF = 0;
    }
}

ISR(TIMER1_OVF_vect)
{
    PORTB|=(1<<LED_D1);
}

ISR(TIMER1_COMPA_vect)
{
    PORTB&=~(1<<LED_D1);
}

ISR(ADC_vect)
{
    OCR1A = 255-ADCH;
}