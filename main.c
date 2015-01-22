#include <avr/io.h>
#include <avr/interrupt.h>

#include "Descriptors.h"
#include <LUFA/Version.h>
#include <LUFA/Drivers/USB/USB.h>

inline void process_ovf(uint32_t *d, uint16_t *s){
	if (*d<=0x7ffeffffUL){
  	*d += 0x10000UL - *s;
		*s = 0;
	}
}

volatile uint32_t d1, D1;
volatile uint16_t s1;
volatile uint8_t r1;

ISR(TIMER3_OVF_vect){
  process_ovf(&d1, &s1);
}

ISR(TIMER3_CAPT_vect){
	uint16_t t;
	t=ICR3;

  if (TIFR3 & 1<<TOV3 && t==0){ // process pending OVF interrupt
    process_ovf(&d1, &s1);
    TIFR3 |= 1<<TOV3;
  }

	d1 += t - s1;
	if (!r1) {D1=d1; r1=1;}
	s1=t;	d1=0;
}

volatile uint32_t d0, D0;
volatile uint16_t s0;
volatile uint8_t r0;



ISR(TIMER1_OVF_vect){
  process_ovf(&d0, &s0);
}

ISR(TIMER1_CAPT_vect){
	uint16_t t;
	t=ICR1;

  if (TIFR1 & 1<<TOV1 && t==0){ // process pending OVF interrupt
    process_ovf(&d0, &s0);
    TIFR1 |= 1<<TOV1;
  }

	d0 += t - s0;
	if (!r0) {D0=d0; r0=1;}
	s0=t;	d0=0;
}

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = 0,

				.DataINEndpointNumber           = CDC_TX_EPNUM,
				.DataINEndpointSize             = CDC_TXRX_EPSIZE,
				.DataINEndpointDoubleBank       = false,

				.DataOUTEndpointNumber          = CDC_RX_EPNUM,
				.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
				.DataOUTEndpointDoubleBank      = false,

				.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
				.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
				.NotificationEndpointDoubleBank = false,
			},
	};


void main(){
#if defined(__AVR_AT90USB1287__)
  CLKPR=0x80;
  CLKPR=0x00;
#warning this is for AT90USB1287
#endif

	DDRC = ~(1<<7); PORTC |= 1<<7;
	DDRD = ~(1<<4); PORTD |= 1<<4;

	// Timer setup
	TCCR1B |= 2<<CS10 | 1<<ICES1;
	TIMSK1 |= 1<<TOIE1 | 1<<ICIE1;

	TCCR3B |= 2<<CS30 | 1<<ICES3;
	TIMSK3 |= 1<<TOIE3 | 1<<ICIE3;

	// USB init	
	USBCON &= ~(1 << OTGPADE); 
  USB_Init();

	sei();

	while (1) {
		if (r1){
			CDC_Device_SendData(&VirtualSerial_CDC_Interface, "\r\n", 2);
			D1 |= (uint32_t)1<<31;
			CDC_Device_SendData(&VirtualSerial_CDC_Interface, &D1, sizeof(D1));
		  CDC_Device_Flush(&VirtualSerial_CDC_Interface);
			r1=0;
		}
		if (r0){
			CDC_Device_SendData(&VirtualSerial_CDC_Interface, "\r\n", 2);
			CDC_Device_SendData(&VirtualSerial_CDC_Interface, &D0, sizeof(D0));
		  CDC_Device_Flush(&VirtualSerial_CDC_Interface);
			r0=0;
		}
	}
}



/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

