#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"

#define CONCAT0(x,y)	x ## y
#define CONCAT(x,y)	CONCAT0(x,y)

#define USARTX	CONCAT(USART, SERIALN)
#define USARTX_TX_vect	CONCAT(USARTX, _TX_vect)
#define USARTX_RX_vect	CONCAT(USARTX, _RX_vect)
#define UDRX	CONCAT(UDR, SERIALN)
#define UDREX	CONCAT(UDRE, SERIALN)
#define UCSRX	CONCAT(UCSR, SERIALN)
#define UCSRXA	CONCAT(UCSRX, A)
#define UCSRXB	CONCAT(UCSRX, B)
#define UCSRXC	CONCAT(UCSRX, C)
#define TXCIEX	CONCAT(TXCIE, SERIALN)
#define RXCIEX	CONCAT(RXCIE, SERIALN)
#define TXENX	CONCAT(TXEN, SERIALN)
#define RXENX	CONCAT(RXEN, SERIALN)
#define UBRRX	CONCAT(UBRR, SERIALN)

static char tx_buff[TX_BUFF_LEN];
static char rx_buff[RX_BUFF_LEN];
static volatile unsigned char tx_running;

static volatile unsigned int txin, txout;
static volatile unsigned int rxin, rxout;

static inline void tx_byte()
{
	while (!(UCSRXA & (1 << UDREX)));

	if (txout != txin) {
		UDRX = tx_buff[txout];
		txout = (txout + 1) % TX_BUFF_LEN;
		tx_running = 1;
	} else
		tx_running = 0;
}

void serial_init(void)
{
	txin = 0;
	txout = 0;
	tx_running = 0;
	rxin = 0;
	rxout = 0;
	UCSRXB |= (1 << RXENX) | (1 << TXENX) | (1 << TXCIEX) | (1 << RXCIEX);
	UBRRX = (F_CPU + 8 * BAUD_RATE) / 16 / BAUD_RATE;
}

int serial_send(const char *data, int count)
{
	int i = 0, newtxin;

	while (count) {
		newtxin = (txin + 1) % TX_BUFF_LEN;
		if (newtxin != txout) {
			tx_buff[txin] = data[i++];
			txin = newtxin;
			count--;
		}
	}
	cli();
	if (!tx_running)
		tx_byte();
	sei();
	return i;
}

int serial_recv(char *buff, int count)
{
	int i = 0;

	while (count) {
		if (rxout != rxin) {
			buff[i++] = rx_buff[rxout];
			rxout = (rxout + 1) % RX_BUFF_LEN;
			count--;
		}
	}
	return i;
}

ISR(USARTX_RX_vect)
{
	int newrxin;

	rx_buff[rxin] = UDRX;
	newrxin = (rxin + 1) % RX_BUFF_LEN;
	if (newrxin != rxout)
		rxin = newrxin;
}

ISR(USARTX_TX_vect)
{
	tx_byte();
}
