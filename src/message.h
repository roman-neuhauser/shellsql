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
 */

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/*
 * LINUX being NON-POSIX - Naughty!!
 * MSGMAX is not declared without following :-(
#include <linux/msg.h>
 */

#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

#include "string.h"


/*
 * The folloing must not be greater than MSGMAX
 * and more than 33
 */
#define MES_SIZE 4096

struct message_t {
	int mqueue;
	key_t key;
	char *filename;	/* I need to store this somewhere */

	struct {
		long mtype;
		char mtext[MES_SIZE];
	} mbuf;
};

typedef struct message_t message;

#define MES_CLIENT_TO_SERVER	1
#define MES_SERVER_TO_CLIENT	2

#define MES_BEGINNING		1
#define MES_MIDDLE   		2
#define MES_END      		4
#define MES_EOF      		8
#define MES_OK			0
#define MES_NOT_OK		127

message *new_message(int messid);
int message_send(message *mes, string *str, int complete, int direction);
int message_end(message *mes, int direction);
int message_status(message *mes, int status, char *s, int direction);
int message_receive(message *mes, string *str, int *complete, int direction);
void message_destroy(message *mes);
void message_delete(message *mes);
int message_id(message *mes);

#endif
