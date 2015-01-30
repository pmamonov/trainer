#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"

struct pcint_t {
	uint8_t pcint;
	uint8_t pcie;
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin;
	volatile uint8_t *pcmsk;
	uint8_t pnum;
	uint16_t timer;
	uint16_t delay;
	uint8_t flags;
};

#define FLAG_INT	(1 << 0)
#define FLAG_BIT_SET	(1 << 1)

#define TIMER_MAX	((1ull << 16) - 1)

#define NUM_INT 3

struct pcint_t pcint[NUM_INT] = {
	{
		.pcint = PCINT4,
		.pcie = PCIE0,
		.pcmsk = &PCMSK0,
		.ddr = &DDRB,
		.port = &PORTB,
		.pin = &PINB,
		.pnum = 4,
	},
	{
		.pcint = PCINT5,
		.pcie = PCIE0,
		.pcmsk = &PCMSK0,
		.ddr = &DDRB,
		.port = &PORTB,
		.pin = &PINB,
		.pnum = 5,
	},
	{
		.pcint = PCINT6,
		.pcie = PCIE0,
		.pcmsk = &PCMSK0,
		.ddr = &DDRB,
		.port = &PORTB,
		.pin = &PINB,
		.pnum = 6,
	},
};

int main()
{
	char c;

	/* setup serial port */
	serial_init();

	sei();

	while (1) {
		serial_recv(&c, 1);
		serial_send(&c, 1);
	}
}

ISR(TIMER1_OVF_vect)
{
	int i;

	TCNT1 = (1ull << 16) - F_CPU / 1000;
	for (i = 0; i < NUM_INT; i++) {
		if (pcint[i].timer < TIMER_MAX)
			pcint[i].timer += 1;
	}
}

inline void pcint_isr()
{
	int i;

	for (i = 0; i < NUM_INT; i++) {
		if (*pcint[i].pin & (1 << pcint[i].pnum)) {
			if (!(pcint[i].flags & FLAG_BIT_SET)) {
				pcint[i].flags |= FLAG_INT;
				pcint[i].flags |= FLAG_BIT_SET;
				pcint[i].delay = pcint[i].timer;
				pcint[i].timer = 0;
			}
		} else
			pcint[i].flags &= ~FLAG_BIT_SET;
	}
}

ISR(PCINT0_vect)
{
	pcint_isr();
}

ISR(PCINT1_vect)
{
	pcint_isr();
}

ISR(PCINT2_vect)
{
	pcint_isr();
}
