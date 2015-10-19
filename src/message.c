/*
    SHSQL suite - SQL utility for LINUX/UNIX shell scriptiing
    Copyright (C) 2004  Edward Macnaghten

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Edward Macnaghten
    EDL Systems
    16 Brierley Walk
    Cambridge
    CB4 3NH
    UK

    eddy@edlsystems.com
 */

/*
 * An object to hold non-portable message transmissions
 *
 * This is a means of transfering messages from one process to another.
 * At the moment it uses the UNIX IPC "msg" API, but this may change if
 * neccessary.  If it does the methods/properties of this object will
 * still remain the same.
 *
 * message *new_message(int id) - Creates resource
 *    If id is zero the message queue is created, otherwise it joins
 *    that particular one
 *
 * int message_id(message *)
 *    Returns the ID of the message, can be used in other new_message()'s
 *    to join it
 *
 * int message_send(message *, string *str, int complete, int direction)
 *    Sends string through message object
 *
 *    Messages can be split into a number of sub or part messages, in which
 *    case only the last segment is sentwith the "complete" flag set to
 *    non-zero.  (If it is only one part then that is sent with the complete
 *    flag set).
 *
 *     The direction field is either MES_SERVER_TO_CLIENT or
 *     MES_CLIENT_TO_SERVER.
 *
 *     Returns *    -1 on error or length of transmitted message
 *
 * int message_receive(message *, string *str, int *complete, int direction)
 *    Receives string through message object
 *
 *    *complete is (pre)set to zero if partial messages can be received, or
 *    non-zero  if the message needs to be "completed" before this routine
 *    returns and all "parts" of the messages are concatenated in the string.
 *
 *    Please note though that if the complete flag is not set the "parts" of
 *    the message may not match what has been "sent".  The field is set by
 *    message_receive *    accordingly if the message is complete or not.
 *
 *    The direction field is MES_SERVER_TO_CLIENT or MES_CLIENT_TO_SERVER
 *  
 *    This returns number of bytes received or -1 on error or if 
 *    "message_end" or message_status() has been sent
 *
 * int message_end(message *, int direction)
 *   This transmits the fact the message object is finished with to the
 *   other end. direction is MES_SERVER_TO_CLIENT or MES_CLIENT_TO_SERVER
 *
 * int message_status(message *, int status, int direction)
 *    This transmits a an error to the other process..
 *
 * message_destroy(message *)
 *    This deallocates kernel resources, does not free the message
 *    object itself.
 *
 * message_delete(message *)
 *    This deletes the message object and frees (process) resource
 */

#include "message.h"

/*
 * New message, messid = 0 to create the thing
 * otherwise tries to attach it
 */

#include <errno.h>
 
message *new_message(int messid)
{
	message *mes = NULL;
	FILE *fp;

	if((mes = (message *)malloc(sizeof(message))) == NULL) return NULL;

	if(messid)
	{
		mes->key = messid;
       		mes->mqueue  = msgget(messid, 0600);
		mes->filename = NULL;
		if(mes->mqueue < 0)
		{
			free(mes);
			return NULL;
		}
	}
	else
	{
	
		if((mes->filename = malloc(33)) == NULL) /* Should be enough */
		{
			free(mes);
			return NULL;
		}

		snprintf(mes->filename, 32, "/tmp/.shsql_%d.hdl", getpid());
		mes->filename[32] = 0;		/* Just to make sure */
		if((fp = fopen(mes->filename, "w")) == NULL)
		{
			free(mes->filename);
			free(mes);
			return NULL;
		}

		mes->key = ftok(mes->filename, 's');
		fprintf(fp, "%d\n", mes->key);
		fclose(fp);

       		mes->mqueue  = msgget(mes->key, IPC_CREAT | IPC_EXCL | 0600);
		if(mes->mqueue < 0)
		{
			fprintf(stderr, "Error: %d\n", errno);
			free(mes->filename);
			free(mes);
			return NULL;
		}
	}
	return mes;
}

/*
 * Sending string, int is -1 for error, else bytes sent
 * complete is -1 for a completed string sent
 */
int message_send(message *mes, string *str, int complete, int direction)
{
	int ilen = 0;
	for(;;)
	{
		mes->mbuf.mtype = direction;
		*(mes->mbuf.mtext) = ilen ? MES_MIDDLE : MES_BEGINNING;
		if((string_len(str) - ilen) < MES_SIZE - 1)
		{
			
			if(complete) *(mes->mbuf.mtext) |= MES_END;
			strcpy(mes->mbuf.mtext + 1, string_s(str) + ilen);
			if(msgsnd(mes->mqueue, &(mes->mbuf), string_len(str) - ilen + 2, 0) < 0) return -1;
			break;
		}
		else
		{
			memcpy(mes->mbuf.mtext + 1, string_s(str) + ilen, MES_SIZE - 2);
			mes->mbuf.mtext[MES_SIZE - 1] = 0;
			if(msgsnd(mes->mqueue, &(mes->mbuf), MES_SIZE, 0) < 0) return -1;
			ilen += MES_SIZE - 2;
		}
	}
	return str->len;
}


/*
 * message_eof denotes end of all, send an MES_EOF
 */
int message_end(message *mes, int direction)
{
	mes->mbuf.mtype = direction;
	mes->mbuf.mtext[0] = MES_EOF;
	mes->mbuf.mtext[1] = 0;
	if(msgsnd(mes->mqueue, &(mes->mbuf), 2, 0) < 0) return -1;
	return 0;
}

int message_status(message *mes, int status, char *s, int direction)
{
	long l;

	l = strlen(s);
	if(l > MES_SIZE - 1) l = MES_SIZE - 1;
	mes->mbuf.mtype = direction;
	if(status)
		mes->mbuf.mtext[0] = 127;
	else
		mes->mbuf.mtext[0] = 0;
	strncpy(mes->mbuf.mtext + 1, s, MES_SIZE - 1);
	mes->mbuf.mtext[l] = 0;
	if(msgsnd(mes->mqueue, &(mes->mbuf), l + 1, 0) < 0) return -1;
	return 0;
}
	
	

/*
 * Receiveing, returns -1 for error or status of 127
 *
 * Complete is both in and out, if -1 then do not leave this until
 * all message segments are in, else you can
 */
		

int message_receive(message *mes, string *str, int *complete, int direction)
{
	size_t sz;
	char c;
	int incomp;

	incomp = *complete;

	string_clear(str);

	*complete = 0;

	for(;;)
	{
		sz = msgrcv(mes->mqueue, &(mes->mbuf), MES_SIZE, direction, 0);

		if(!sz) continue;

		/*
		 * The first character is 1 for new string, 2 for middlw of string 4 for end of mes 8 for end of trans (ored)
	 	 *		16 is continue and should not be here
		 * I suppose I should do some validation, but I won't
 		 */

		c = *(mes->mbuf.mtext);


		if((c & 127) == 127)
		{
			string_set(str, mes->mbuf.mtext + 1);
			return -1;	/* Error */
		}

		if(c & MES_EOF) return -1;	/* End of file */
		if(!c) return 0;

		if(c & MES_BEGINNING) string_set(str, mes->mbuf.mtext + 1);
		if(c & MES_MIDDLE   ) string_cat(str, mes->mbuf.mtext + 1);
		if(c & MES_END) *complete = -1;
		if((!incomp) || *complete) break;

	}
	return str->len;
}


/*
 * Do not confuse this with message_delete
 * this destroys the message queue
 */
void message_destroy(message *mes)
{

	msgctl(mes->mqueue, IPC_RMID, NULL);
	if(mes->filename != NULL)
	{
		unlink(mes->filename);
	}
}

void message_delete(message *mes)
{
	if(mes->filename != NULL) free(mes->filename);
	free(mes);
}

int message_id(message *mes)
{
	return mes->key;
}
