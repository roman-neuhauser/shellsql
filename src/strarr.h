/*
 * A string array object
 *
 * This creates an array of strings but optimizes it accordingly,
 * can only append onto the last element
 */

#ifndef __STRARR_H
#define __STRARR_H

struct strarr_t {
	long num;		/* Number of items */
	long pnum;		/* no of items physically allocated */

	long *start;		/* Starting point of each string */
	long blen;		/* The length of the buffer */
	long plen;		/* The allocated length of the buffer */
	char *buf;		/* The buffer */
};

typedef struct strarr_t strarr;

strarr *new_strarr();
int strarr_put(strarr *sa, char *s);
int strarr_put_c(strarr *sa, char c);
int strarr_end(strarr *sa);
char strarr_last(strarr *arr);
void strarr_minus(strarr *sa);
char *strarr_out(strarr *sa, int itm);
int strarr_num(strarr *sa);
void strarr_delete(strarr *sa);
void strarr_clear(strarr *sa);


#endif

