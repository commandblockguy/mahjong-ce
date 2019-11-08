#include <stdint.h>
#include <tice.h>

#include <stdio.h>
#include <string.h>

#include <fileioc.h>

/* Get the number of digits in the base10 representation of a number */
uint8_t num_digits(uint8_t n) {
	return 1 + (n >= 10) + (n >= 100); /* I'm lazy. */
}

/* Shuffle an array */
void shuffle(uint8_t *array, uint24_t size) {
	if (size > 1) {
        uint24_t i;
        for (i = size - 1; i > 0; i--) {
            uint24_t j = random() % (i+1);
            uint8_t t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

/* Count the number of occurrences of delim in str */
uint8_t occurrences(char *str, char delim) {
    int val = 0, i;
    for(i = strlen(str); i >= 0; i--) {
        if(str[i] == delim) val++;
    }
    return val;
}

/* Get the value of Ans */
int get_ans(void) {
    real_t *real_in;
    int in;

    if (ti_RclVar(TI_REAL_TYPE, ti_Ans, (void **) &real_in)) return 0;
    if ((in = os_RealToInt24(real_in)) < 1) return 0;

    return in;
}
