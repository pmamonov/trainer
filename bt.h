#define BT_PIN	"0000"

#define BT_SERIAL 1

#define BT_STATUS_DDR DDRD
#define BT_STATUS_PIN PIND
#define BT_STATUS_BIT 0

#define BT_INT_NUM 0

#define BT_VECT INT0_vect

void bt_init();

void bt_connect();

int bt_status();

int bt_send(char*, int);
