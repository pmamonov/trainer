#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "serial.h"

struct serial_t {
	volatile uint8_t 	*udr;
	volatile uint8_t 	*ucsra;
	volatile uint8_t 	*ucsrb;
	volatile uint8_t 	*ucsrc;
	volatile uint16_t 	*ubrr;
	volatile uint8_t 	*tx_ddr;
	volatile uint8_t 	*rx_ddr;
	uint8_t 		tx_bit;
	uint8_t 		rx_bit;
	uint8_t 		udre;
	uint8_t 		txcie;
	uint8_t 		rxcie;
	uint8_t 		txen;
	uint8_t 		rxen;
	char 			*tx_buff;
	char 			*rx_buff;
	volatile unsigned char 	tx_running;
	volatile unsigned int 	txin, txout;
	volatile unsigned int 	rxin, rxout;
};

#ifdef _AVR_IOM2560_H_

#define NUM_SERIAL 4

static struct serial_t serial[NUM_SERIAL] = {
	{
		.udr = &UDR0,
		.ucsra = &UCSR0A,
		.ucsrb = &UCSR0B,
		.ucsrc = &UCSR0C,
		.ubrr = &UBRR0,
		.tx_ddr = &DDRE,
		.rx_ddr = &DDRE,
		.tx_bit = 1,
		.rx_bit = 0,
		.udre = UDRE0,
		.txcie = TXCIE0,
		.rxcie = RXCIE0,
		.txen = TXEN0,
		.rxen = RXEN0,
		.tx_buff = 0,
		.rx_buff = 0,
	},
	{
		.udr = &UDR1,
		.ucsra = &UCSR1A,
		.ucsrb = &UCSR1B,
		.ucsrc = &UCSR1C,
		.ubrr = &UBRR1,
		.tx_ddr = &DDRD,
		.rx_ddr = &DDRD,
		.tx_bit = 3,
		.rx_bit = 2,
		.udre = UDRE1,
		.txcie = TXCIE1,
		.rxcie = RXCIE1,
		.txen = TXEN1,
		.rxen = RXEN1,
		.tx_buff = 0,
		.rx_buff = 0,
	},
	{
		.udr = &UDR2,
		.ucsra = &UCSR2A,
		.ucsrb = &UCSR2B,
		.ucsrc = &UCSR2C,
		.ubrr = &UBRR2,
		.tx_ddr = &DDRH,
		.rx_ddr = &DDRH,
		.tx_bit = 1,
		.rx_bit = 0,
		.udre = UDRE2,
		.txcie = TXCIE2,
		.rxcie = RXCIE2,
		.txen = TXEN2,
		.rxen = RXEN2,
		.tx_buff = 0,
		.rx_buff = 0,
	},
	{
		.udr = &UDR3,
		.ucsra = &UCSR3A,
		.ucsrb = &UCSR3B,
		.ucsrc = &UCSR3C,
		.ubrr = &UBRR3,
		.tx_ddr = &DDRJ,
		.rx_ddr = &DDRJ,
		.tx_bit = 1,
		.rx_bit = 0,
		.udre = UDRE3,
		.txcie = TXCIE3,
		.rxcie = RXCIE3,
		.txen = TXEN3,
		.rxen = RXEN3,
		.tx_buff = 0,
		.rx_buff = 0,
	},
};
#endif


static inline void rx_byte(struct serial_t *sr)
{
	unsigned int newrxin;

	sr->rx_buff[sr->rxin] = *sr->udr;
	newrxin = (sr->rxin + 1) % RX_BUFF_LEN;
	if (newrxin != sr->rxout)
		sr->rxin = newrxin;
}

static inline void tx_byte(struct serial_t *sr)
{
	while (!(*sr->ucsra & (1 << sr->udre)));

	if (sr->txout != sr->txin) {
		*sr->udr = sr->tx_buff[sr->txout];
		sr->txout = (sr->txout + 1) % TX_BUFF_LEN;
		sr->tx_running = 1;
	} else
		sr->tx_running = 0;
}

int serial_init(unsigned int s, uint32_t baudrate)
{
	struct serial_t *sr;

	if (s >= NUM_SERIAL)
		return -1;

	sr = &serial[s];

	if (sr->tx_buff == 0) {
		sr->tx_buff = malloc(TX_BUFF_LEN);
		sr->rx_buff = malloc(RX_BUFF_LEN);
	}
	
	sr->txin = 0;
	sr->txout = 0;
	sr->tx_running = 0;
	sr->rxin = 0;
	sr->rxout = 0;
	*sr->ucsrb |= (1 << sr->rxen) |
		      (1 << sr->txen) |
		      (1 << sr->txcie) |
		      (1 << sr->rxcie);
	*sr->ubrr = (F_CPU + 8 * baudrate) / 16 / baudrate;
	*sr->tx_ddr |= 1 << sr->tx_bit;
	*sr->rx_ddr &= ~(1 << sr->rx_bit);

	return 0;
}

int serial_send(unsigned int s, const char *data, int count)
{
	struct serial_t *sr = &serial[s];
	int i = 0, newtxin;

	while (count) {
		newtxin = (sr->txin + 1) % TX_BUFF_LEN;
		if (newtxin != sr->txout) {
			sr->tx_buff[sr->txin] = data[i++];
			sr->txin = newtxin;
			count--;
		}
	}
	cli();
	if (!sr->tx_running)
		tx_byte(sr);
	sei();
	return i;
}

int serial_recv(unsigned int s, char *buff, int count)
{
	struct serial_t *sr = &serial[s];
	int i = 0;

	while (count) {
		if (sr->rxout != sr->rxin) {
			buff[i++] = sr->rx_buff[sr->rxout];
			sr->rxout = (sr->rxout + 1) % RX_BUFF_LEN;
			count--;
		}
	}
	return i;
}


#ifdef _AVR_IOM2560_H_

ISR(USART0_RX_vect)
{
	rx_byte(&serial[0]);
}

ISR(USART0_TX_vect)
{
	tx_byte(&serial[0]);
}

ISR(USART1_RX_vect)
{
	rx_byte(&serial[1]);
}

ISR(USART1_TX_vect)
{
	tx_byte(&serial[1]);
}

ISR(USART2_RX_vect)
{
	rx_byte(&serial[2]);
}

ISR(USART2_TX_vect)
{
	tx_byte(&serial[2]);
}

ISR(USART3_RX_vect)
{
	rx_byte(&serial[3]);
}

ISR(USART3_TX_vect)
{
	tx_byte(&serial[3]);
}

#endif
