#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 12000000ull
#define GEN_HZ 400

uint8_t x = 8;

int main(void)
{
	DDRC |= 0xf;
  
	TCCR1B |= 1 << CS10;
	TIMSK |= 1 << TOIE1;

	sei();

  while (1);
}

ISR(TIMER1_OVF_vect) {
	TCNT1 = 0x10000ull - F_CPU / GEN_HZ;
	PORTC = x;
	x >>= 1;
	if (!x)
		x = 8;
}

