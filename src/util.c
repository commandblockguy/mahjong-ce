#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>
#include <fileioc.h>

/* Get the number of digits in the base10 representation of a number */
uint8_t num_digits(uint8_t n) {
	return 1 + (n >= 10) + (n >= 100); /* I'm lazy. */
}

/* Shuffle an array */
void shuffle(uint8_t * array, uint24_t size) {
	if (size > 1) {
        uint24_t i;
        for (i = size - 1; i > 0; i--) {
            uint24_t j = random() % (i+1);
            char t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

/* Prints a hex representation of the appvar to the console */
void debug_appvar(char *name) {
    int i, j, size;
    ti_var_t appvar;

    ti_CloseAll();

    appvar = ti_Open(name, "r");
    size = ti_GetSize(appvar);

    for(i = 0; i < size;) {
        uint8_t data[8];
        int transferred = ti_Read(&data, sizeof(uint8_t), 8, appvar);
        for(j = 0; j < transferred; j++) {
            dbg_sprintf(dbgout, "%02X ", data[j]);
        }
        dbg_sprintf(dbgout, "\n");
        i += transferred;
    }
}

/* Count the number of occurences of delim in str */
uint8_t occurrences(char *str, char delim) {
    int val = 0, i;
    for(i = strlen(str); i >= 0; i--) {
        if(str[i] == delim) val++;
    }
    return val;
}

/* Get the value of Ans */
int get_ans() {
    real_t *real_in;
    int in;

    if (ti_RclVar(TI_REAL_TYPE, ti_Ans, &real_in)) return 0;    
    if ((in = os_RealToInt24(real_in)) < 1) return 0;

    return in;
}
