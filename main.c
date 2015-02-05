#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serial.h"
#include "bt.h"

volatile uint16_t timer = 0;

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
	uint8_t count;
};

#define FLAG_INT	(1 << 0)
#define FLAG_BIT_SET	(1 << 1)

#define TIMER_MAX	(0x7fff)

#define NUM_INT 3

#define SERIAL_PC 0

#define TIM_HZ 10000
#define MSR_HZ 10

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
	uint16_t delay, _timer;
	uint32_t temp;

	/* setup timer1 */
	TCCR1B |= 1 << CS10;
	TIMSK1 |= 1 << TOIE1;

	/* setup serial port */
	serial_init(SERIAL_PC, 38400);
	serial_init(BT_SERIAL, 38400);

	/* setup external interrupts */
	for (i = 0; i < NUM_INT; i++) {
		*pcint[i].ddr &= ~(1 << pcint[i].pnum);
		*pcint[i].port |= 1 << pcint[i].pnum;

		PCICR |= (1 << pcint[i].pcie);
		*pcint[i].pcmsk |= (1 << pcint[i].pcint);
		pcint[i].timer = 0;
		pcint[i].flags = 0;
		if (*pcint[i].pin & (1 << pcint[i].pnum))
			pcint[i].flags |= FLAG_BIT_SET;
	}

	sei();

	bt_init();
	bt_connect();

	while (1) {
		cli();
		_timer = timer;
		sei();
		if (_timer) {
			_delay_us(100);
			continue;
		} else {
			cli();
			timer = TIM_HZ / MSR_HZ;
			sei();
		}

		for (i = 0; i < NUM_INT; i++) {
			temp = 0;
			cli();
			if (pcint[i].flags & FLAG_INT) {
				temp = 1;
				pcint[i].flags &= ~FLAG_INT;
				delay = pcint[i].delay / pcint[i].count;
				pcint[i].count = 0;
				pcint[i].delay = 0;
			}
			sei();

			if (temp) {
				temp = 0xaa |
				       ((0xff & i) << 8) |
				       ((uint32_t)delay << 16);
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

	TCNT1 = (1ull << 16) - F_CPU / TIM_HZ;
	for (i = 0; i < NUM_INT; i++) {
		if (pcint[i].timer < TIMER_MAX)
			pcint[i].timer += 1;
	}

	if (timer)
		timer -= 1;
}

inline void pcint_isr()
{
	int i;

	for (i = 0; i < NUM_INT; i++) {
		if (*pcint[i].pin & (1 << pcint[i].pnum)) {
			if (!(pcint[i].flags & FLAG_BIT_SET)) {
				pcint[i].flags |= FLAG_INT;
				pcint[i].flags |= FLAG_BIT_SET;
				pcint[i].count += 1;
				pcint[i].delay += pcint[i].timer;
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
