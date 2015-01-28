#define TX_BUFF_LEN 64
#define RX_BUFF_LEN 64

int serial_init(unsigned int, uint32_t);

int serial_send(unsigned int, const char *, int);

int serial_recv(unsigned int, char *, int);
