#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../src/bsdconv.h"

#define SWAP(a,b,i) ((i)=(a), (a)=(b), (b)=(i))

void cbconv(struct bsdconv_instance *ins){
	char *data, *p, c;
	unsigned int len, i;
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	data=this_phase->curr->data;

	data+=1;
	if(this_phase->curr->len > 3){
		this_phase->state.status=NEXTPHASE;

		DATA_MALLOC(ins, this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=4;
		this_phase->data_tail->flags=F_FREE;
		p=this_phase->data_tail->data=malloc(4);

		c=*data-1;
		*p=bb11011000;
		*p |= (c >> 2) & bb00000011;
		++p;
		*p=(c << 6) & bb11000000;
		++data;
		*p |= (*data >> 2) & bb00111111;
		++p;
		*p=bb11011100;
		*p |= *data & bb00000011;
		++p;
		++data;
		*p=*data;

		data=this_phase->data_tail->data;

		SWAP(data[0],data[1],i);
		SWAP(data[2],data[3],i);
	}else{
		this_phase->state.status=NEXTPHASE;
		len=this_phase->curr->len-1;

		DATA_MALLOC(ins, this_phase->data_tail->next);
		this_phase->data_tail=this_phase->data_tail->next;
		this_phase->data_tail->next=NULL;
		this_phase->data_tail->len=2;
		this_phase->data_tail->flags=F_FREE;
		this_phase->data_tail->data=malloc(2);
		for(i=0;i<2-len;++i){
			CP(this_phase->data_tail->data)[i]=0x0;
		}
		memcpy(CP(this_phase->data_tail->data)+i, data, len);
		data=this_phase->data_tail->data;

		SWAP(data[0],data[1],i);
	}
	return;
}
