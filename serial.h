#define SERIALN 0

#define BAUD_RATE 9600ull

#define TX_BUFF_LEN 64
#define RX_BUFF_LEN 64

void serial_init(void);

int serial_send(char *, int);
