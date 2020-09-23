#ifndef HASRETURNVALUESIF_H_
#define HASRETURNVALUESIF_H_

#include <stdint.h>

#define MAKE_RETURN_CODE( interface, number ) ((interface << 8) + (number))
typedef uint16_t ReturnValue_t;


static const ReturnValue_t RETURN_OK = 0;
static const ReturnValue_t RETURN_FAILED = 0xFFFF;


#endif /* HASRETURNVALUESIF_H_ */
