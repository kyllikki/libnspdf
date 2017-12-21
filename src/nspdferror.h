typedef enum {
    NSPDFERROR_OK,
    NSPDFERROR_NOMEM,
    NSPDFERROR_SYNTAX, /**< syntax error in parse */
    NSPDFERROR_SIZE, /**< not enough input data */
    NSPDFERROR_RANGE, /**< value outside type range */
    NSPDFERROR_TYPE, /**< wrong type error */
    NSPDFERROR_NOTFOUND, /**< key not found */
} nspdferror;
