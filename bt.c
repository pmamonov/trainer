#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <serial.h>
#include <bt.h>

#define CONCAT0(x,y)   x ## y
#define CONCAT(x,y)    CONCAT0(x,y)

#define BT_INT CONCAT(INT, BT_INT_NUM)
//#define _BT_VECT CONCAT(BT_INT, _vect)
//#define BT_VECT _BT_VECT

#define _BT_ISC CONCAT(ISC, BT_INT_NUM)
#define BT_ISC CONCAT(_BT_ISC, 0)

#if (BT_INT_NUM < 4)
	#define BT_ICR EICRA
#else
	#define BT_ICR EICRB
#endif

static volatile char connected = 0;

void bt_init()
{
	serial_init(BT_SERIAL, 38400);
	BT_STATUS_DDR &= ~(1 << BT_STATUS_BIT);
	EIMSK |= 1 << BT_INT;
	/* interrupt on rising edge */
	BT_ICR |= 3 << BT_ISC;
}

int bt_status(int reconnect)
{
	int status = BT_STATUS_PIN & (1 << BT_STATUS_BIT);
	/* reconnect on connection loss */
	if (!status && reconnect && connected) {
		connected = 0;
		bt_connect();
	}
	return status;
}

void bt_connect()
{
	serial_send(BT_SERIAL, "\r\n+STWMOD=0\r\n", 13);
	serial_send(BT_SERIAL, "\r\n+STPIN=" BT_PIN "\r\n", 15);
	serial_send(BT_SERIAL, "\r\n+STNA=TRAINER\r\n", 17);
	serial_send(BT_SERIAL, "\r\n+STOAUT=1\r\n", 13);
	serial_send(BT_SERIAL, "\r\n+STAUTO=0\r\n", 13);
	_delay_ms(2000);
	serial_send(BT_SERIAL, "\r\n+INQ=1\r\n", 10);
}

int bt_send(char *data, int len)
{
	if (bt_status(0))
		return serial_send(BT_SERIAL, data, len);
	return 0;
}

ISR(BT_VECT)
{
	connected = 1;
	_delay_ms(2000);
}
