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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../../src/bsdconv.h"

struct my_s{
	int status;
	char plane, buf[4];
	struct bsdconv_instance *uni;
};

void *cbcreate(void){
	struct my_s *r=malloc(sizeof(struct my_s));
	r->uni=bsdconv_create("PASS:UNICODE:PASS");
	if(r->uni==NULL){
		free(r);
		return NULL;
	}
	return r;
}

void cbinit(struct bsdconv_codec_t *cdc, struct my_s *r){
	r->status=0;
	r->plane=1;
}

void cbdestroy(void *p){
	struct my_s *r=p;
	bsdconv_destroy(r->uni);
	free(p);
}

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=&ins->phase[ins->phase_index];
	struct my_s *t=this_phase->codec[this_phase->index].priv;
	struct bsdconv_instance *uni=t->uni;
	char d;

	for(;this_phase->i<this_phase->data->len;this_phase->i+=1){
		d=CP(this_phase->data->data)[this_phase->i];
		switch(t->status){
			case 0:
				if(d==0x0){ //plane switch sequence
					t->status=10;
					continue;
				}else{ //data sequence
					t->status=1;
					t->buf[0]=0x02;
					t->buf[1]=t->plane;
					t->buf[2]=d;
					continue;
				}
				break;
			case 1:
				t->status=0;
				t->buf[3]=d;
				
				bsdconv_init(uni);
				uni->input.data=t->buf;
				uni->input.len=4;
				uni->input.flags=F_SKIP;
				uni->input.next=NULL;
				uni->flush=1;
				bsdconv(uni);
				this_phase->data_tail->next=uni->phase[uni->phasen].data_head->next;
				uni->phase[uni->phasen].data_head->next=NULL;

				this_phase->state.status=NEXTPHASE;
				return;
			case 10:
				t->status=0;
				t->plane=d;
				continue;
				break;
		}
	}
	this_phase->state.status=CONTINUE;
	return;
}
