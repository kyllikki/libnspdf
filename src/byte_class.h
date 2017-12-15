#define BC_RGLR 0 /* regular character */
#define BC_WSPC 1 /* character is whitespace */
#define BC_EOLM (1<<1) /* character signifies end of line */
#define BC_DCML (1<<2) /* character is a decimal */
#define BC_HEXL (1<<3) /* character is a hexadecimal */
#define BC_DELM (1<<4) /* character is a delimiter */

const uint8_t *bclass;
