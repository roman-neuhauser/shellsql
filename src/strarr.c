/*
 * A string array object
 *
 * This creates an array of strings but optimizes it accordingly,
 * can only append onto the last element
 */

#include <string.h>
#include <stdlib.h>
#include "strarr.h"

/*
 * The C code
 */

strarr *new_strarr()
{
	strarr *s;

	if((s = (strarr *)malloc(sizeof(strarr))) == NULL)
		return NULL;
	
	s->num = 0;
	s->pnum = 16;
	if((s->start = (long *)malloc(sizeof(long) * 16)) == NULL)
	{
		free(s);
		return NULL;
	}
	s->blen = 0;
	s->plen = 16;
	if((s->buf = malloc(16)) == NULL)
	{
		free(s->start);
		free(s);
		return NULL;
	}
	*(s->start) = 0;
	*(s->buf) = 0;

	return s;
}

/*
 * The methods return -1 on error
 */

int strarr_put(strarr *sa, char *s)
{
	long len;
	
	len = strlen(s);

	while((sa->blen + len) >= sa->plen)
	{
		sa->plen += 16;
		if((sa->buf = realloc(sa->buf, sa->plen)) == NULL)
			return -1;
	}

	strcat(sa->buf + sa->blen, s);
	sa->blen += len;
	return len;
}

int strarr_put_c(strarr *sa, char c)
{

	while((sa->blen + 1) >= sa->plen)
	{
		sa->plen += 16;
		if((sa->buf = realloc(sa->buf, sa->plen)) == NULL)
			return -1;
	}

	sa->buf[sa->blen] = c;
	sa->blen++;
	sa->buf[sa->blen] = 0;
	return 1;
}

int strarr_end(strarr *sa)	/* End of string */
{
	while((sa->blen + 1) >= sa->plen)
	{
		sa->plen += 16;
		if((sa->buf = realloc(sa->buf, sa->plen)) == NULL)
			return -1;
	}
	sa->blen++;
	sa->buf[sa->blen] = 0;
	sa->num++;
	while(sa->num >= sa->pnum)
	{
		sa->pnum += 16;
		if((sa->start = (long *)realloc(sa->start, sizeof(long) * sa->pnum)) == NULL)
			return -1;
	}
	sa->start[sa->num] = sa->blen;
	return 0;
}


char strarr_last(strarr *sa)
{
	if(sa->blen)
		return sa->buf[sa->blen - 1];
	else
		return 0;
}

void strarr_minus(strarr *sa)
{
	if(sa->blen)
	{
		if(sa->buf[sa->blen - 1]);
		{
			sa->blen--;
			sa->buf[sa->blen] = 0;
		}
	}
}


char emptystring = 0;

char *strarr_out(strarr *sa, int itm)	/* Return the string item */
{
	if(itm < 0 || itm >= sa->num)
		return &emptystring;
	return sa->buf + sa->start[itm];
}

int strarr_num(strarr *sa)
{
	return sa->num;
}

void strarr_delete(strarr *sa)
{
	free(sa->start);
	free(sa->buf);
	free(sa);
}

void strarr_clear(strarr *sa)
{
	sa->num = 0;
	*(sa->buf) = 0;
	sa->blen = 0;
	*(sa->start) = 0;
}

	
	

