/*
Copyright 2011 Aiko Barz

This file is part of torrentkino.

torrentkino is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

torrentkino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with torrentkino.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <netdb.h>

#include "worker.h"
#include "udp.h"
#include "../dns/dns.h"
#include "torrentkino.h"

struct obj_work *work_init(void)
{
	struct obj_work *work =
	    (struct obj_work *)myalloc(sizeof(struct obj_work));
	work->mutex = mutex_init();
	work->threads = NULL;
	work->id = 0;
	work->active = 0;

	/* The bootstrap thread immediately stops after the start procedure. */
	work->number_of_threads = 3;
	return work;
}

void work_free(void)
{
	mutex_destroy(_main->work->mutex);
	myfree(_main->work);
}

void work_start(void)
{
	int number_of_worker = _main->work->number_of_threads - 1;

	info(_log, NULL, "Worker: %i", number_of_worker);

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&_main->work->attr);
	pthread_attr_setdetachstate(&_main->work->attr,
				    PTHREAD_CREATE_JOINABLE);

	_main->work->threads =
	    (pthread_t **) myalloc(_main->work->number_of_threads *
				   sizeof(pthread_t *));

	/* P2P Server */
	_main->work->threads[0] = (pthread_t *) myalloc(sizeof(pthread_t));
	if (pthread_create(_main->work->threads[0], &_main->work->attr,
			   udp_thread, _main->udp) != 0) {
		fail("pthread_create()");
	}

	/* DNS Server */
	_main->work->threads[1] = (pthread_t *) myalloc(sizeof(pthread_t));
	if (pthread_create(_main->work->threads[1], &_main->work->attr,
			   udp_thread, _main->dns) != 0) {
		fail("pthread_create()");
	}

	/* Send 1st request while the P2P worker is starting */
	_main->work->threads[2] = (pthread_t *) myalloc(sizeof(pthread_t));
	if (pthread_create(_main->work->threads[2], &_main->work->attr,
			   udp_client, _main->udp) != 0) {
		fail("pthread_create()");
	}
}

void work_stop(void)
{
	int i = 0;

	/* Join threads */
	pthread_attr_destroy(&_main->work->attr);
	for (i = 0; i < _main->work->number_of_threads; i++) {
		if (pthread_join(*_main->work->threads[i], NULL) != 0) {
			fail("pthread_join() failed");
		}
		myfree(_main->work->threads[i]);
	}
	myfree(_main->work->threads);
}
