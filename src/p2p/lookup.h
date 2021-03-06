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

#ifndef LOOKUP_H
#define LOOKUP_H

#include "../shr/malloc.h"
#include "../shr/list.h"
#include "../shr/str.h"
#include "../dns/dns.h"
#include "token.h"

typedef struct {
	/* What are we looking for */
	UCHAR target[SHA1_SIZE];

	LIST *list;
	HASH *hash;

	/* Caller */
	IP c_addr;
	DNS_MSG msg;
	int send_response_to_initiator;
	int number_of_dns_responses;

} LOOKUP;

typedef struct {
	UCHAR id[SHA1_SIZE];
	IP c_addr;
	UCHAR token[TOKEN_SIZE_MAX];
	int token_size;
} NODE_L;

LOOKUP *ldb_init(UCHAR * target, IP * from, DNS_MSG * msg);
void ldb_free(LOOKUP * l);

LONG ldb_put(LOOKUP * l, UCHAR * node_id, IP * from);

NODE_L *ldb_find(LOOKUP * l, UCHAR * node_id);
void ldb_update(LOOKUP * l, UCHAR * node_id, BEN * token, IP * from);

int ldb_number_of_dns_responses(LOOKUP * l);

#endif
