#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "bt.h"

struct pcint_t {
	uint8_t pcint;
	uint8_t pcie;
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin;
	volatile uint8_t *pcmsk;
	uint8_t pnum;
	uint16_t count;
	uint16_t count_save;
	uint8_t flags;
};

#define FLAG_INT	(1 << 0)
#define FLAG_BIT_SET	(1 << 1)

#define TIMER_MAX	((1ull << 16) - 1)

#define NUM_INT 3

#define SERIAL_PC 0

#define TCNT1_CS 4
#define TCNT1_HZ (F_CPU / 256)
#define TIM_HZ 1


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
	int i;
	uint16_t count;
	uint32_t temp;

	/* setup timer1 */
	TCCR1B |= TCNT1_CS << CS10;
	TIMSK1 |= 1 << TOIE1;

	/* setup serial port */
	serial_init(SERIAL_PC, 38400);

	/* setup external interrupts */
	for (i = 0; i < NUM_INT; i++) {
		*pcint[i].ddr &= ~(1 << pcint[i].pnum);
		*pcint[i].port |= 1 << pcint[i].pnum;

		PCICR |= (1 << pcint[i].pcie);
		*pcint[i].pcmsk |= (1 << pcint[i].pcint);
		pcint[i].count = 0;
		pcint[i].flags = 0;
		if (*pcint[i].pin & (1 << pcint[i].pnum))
			pcint[i].flags |= FLAG_BIT_SET;
	}

	sei();

	bt_init();
	bt_connect();

	while (1) {
		for (i = 0; i < NUM_INT; i++) {
			temp = 0;
			cli();
			if (pcint[i].flags & FLAG_INT) {
				temp = 1;
				pcint[i].flags &= ~FLAG_INT;
				count = pcint[i].count_save;
			}
			sei();

			if (temp) {
				temp = 0xaa |
				       ((0xff & i) << 8) |
				       ((uint32_t)count << 16);
				serial_send(SERIAL_PC, (char *)&temp, 4);
				bt_send((char *)&temp, 4);
			}
		}
		bt_status(1);
	}
}

ISR(TIMER1_OVF_vect)
{
	int i;

	TCNT1 = (1ull << 16) - TCNT1_HZ / TIM_HZ;
	for (i = 0; i < NUM_INT; i++) {
		pcint[i].count_save = pcint[i].count;
		pcint[i].count = 0;
		pcint[i].flags |= FLAG_INT;
	}
}

inline void pcint_isr()
{
	int i;

	for (i = 0; i < NUM_INT; i++) {
		if (*pcint[i].pin & (1 << pcint[i].pnum)) {
			if (!(pcint[i].flags & FLAG_BIT_SET)) {
				pcint[i].flags |= FLAG_BIT_SET;
				pcint[i].count += 1;
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
