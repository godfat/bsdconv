/*
 * Copyright (c) 2009-2011 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include "../../src/bsdconv.h"

#define TAILIZE(p) while(*p){ p++ ;}

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	CURRENT_CODEC(ins)->priv=fopen(getenv("BSDCONV_FROM_LOG"),"a");
	return 0;
}

void cbdestroy(struct bsdconv_instance *ins){
	void *fp=CURRENT_CODEC(ins)->priv;
	fclose(fp);
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	FILE *fp=this_phase->codec[this_phase->index].priv;
	fprintf(fp,"%02X\n", (int)UCP(this_phase->curr->data)[this_phase->i]);
	this_phase->state.status=NEXTPHASE;
	fflush(fp);
}