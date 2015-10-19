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
 * traperr - A library used to trap an error
 * and set the errind accordingly
 *
 * This is to give a chance for shsql and others
 * to clean up after an error has occured.
 *
 * Version 0.7.3 - Created
 */
#include <stdio.h>
#include <signal.h>


/*
 * The global indicator to say if we are in error
 */

int trap_iserror = 0;

void trap_handler(int i);

/*
 * Initialization process, set the signal traps up
 *
 * Returns 0 on success, -1 on error
 */

int trap_init()
{
	int i = 0;

	/*
	 * This list maybe incomplete....
	 *
	 * Basically - I want to register even if the process
	 * simply farts
	 */

	if(signal(SIGHUP, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGINT, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGQUIT, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGILL, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGABRT, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGFPE, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGSEGV, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGPIPE, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGTERM, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGUSR1, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGUSR2, trap_handler) == SIG_ERR) i = -1;

	if(signal(SIGBUS, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGPOLL, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGPROF, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGSYS, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGTRAP, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGVTALRM, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGXCPU, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGXFSZ, trap_handler) == SIG_ERR) i = -1;

	if(signal(SIGIOT, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGSTKFLT, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGIO, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGPWR, trap_handler) == SIG_ERR) i = -1;
	if(signal(SIGUNUSED, trap_handler) == SIG_ERR) i = -1;

	return i;
}

void trap_handler(int i)
{
	trap_iserror = -1;
}

