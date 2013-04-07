/*
 * Copyright (c) 2009-2013 Kuan-Chung Chiu <buganini@gmail.com>
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

#define USE_HEX_MAP

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "bsdconv.h"
#ifdef WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#ifndef MAP_PREFAULT_READ
#define MAP_PREFAULT_READ 0
#endif

#ifdef WIN32
#define CODECS_SUBPATH "codecs"
#else
#define CODECS_SUBPATH "share/bsdconv"
#endif

inline void strtoupper(char *s){
	char *c;
	for(c=s;*c;++c){
		if(*c>='a' && *c<='z'){
			*c=*c-'a'+'A';
		}
	}
}

inline int _cbcreate(struct bsdconv_instance *ins, int p, int c){
	int r;
	char *argv;
	if(ins->phase[p].codec[c].argv)
		argv=strdup(ins->phase[p].codec[c].argv);
	else
		argv=strdup("");
	char *cur=argv;
	char *k;
	struct hash_entry *arg=NULL, *tmp;
	struct hash_entry **last=&arg;
	if(*cur){
		while((k=strsep(&cur, "&"))!=NULL){
			*last=malloc(sizeof(struct hash_entry));
			(*last)->key=k;
			(*last)->ptr=strchr(k, '=');
			if((*last)->ptr){
				*CP((*last)->ptr)=0;
				(*last)->ptr+=1;
			}
			(*last)->next=NULL;
			last=&((*last)->next);
		}		
	}

	r=ins->phase[p].codec[c].cbcreate(ins, arg);
	free(argv);
	while(arg){
		tmp=arg->next;
		free(arg);
		arg=tmp;
	}
	return r;
}

int str2data(const char *s, struct data_st *d){
	d->data=NULL;
	d->len=0;
	if(!s || !*s)
		return EINVAL;
	d->data=malloc(strlen(s)/2);
	char f=0;
	while(*s){
		if(hex[(unsigned char) *s]<0){
			free(d->data);
			d->data=NULL;
			return EINVAL;			
		}
		switch(f){
			case 0:
				f=1;
				d->data[d->len]=hex[(unsigned char)*s];
				break;
			case 1:
				f=0;
				d->data[d->len]*=16;
				d->data[d->len]+=hex[(unsigned char)*s];
				d->len+=1;
				break;
		}
		s+=1;
	}
	return 0;
}

int _loadcodec(struct bsdconv_codec_t *cd, char *path){
#ifdef WIN32
	if ((cd->fd=CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))==INVALID_HANDLE_VALUE){
		SetLastError(EOPNOTSUPP);
		return 0;
	}
	cd->md=CreateFileMapping(cd->fd, NULL, PAGE_READONLY, 0,0, NULL);
	if(!cd->md){
		CloseHandle(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
	cd->data_z=cd->z=MapViewOfFile(cd->md, FILE_MAP_READ, 0,0,0);
	if(!cd->z){
		CloseHandle(cd->md);
		CloseHandle(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
#else
	struct stat stat;
	if((cd->fd=open(path, O_RDONLY))==-1){
		SetLastError(EOPNOTSUPP);
		return 0;
	}
	fstat(cd->fd, &stat);
	cd->maplen=stat.st_size;
	if((cd->data_z=cd->z=mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE | MAP_PREFAULT_READ, cd->fd, 0))==MAP_FAILED){
		close(cd->fd);
		SetLastError(ENOMEM);
		return 0;
	}
#endif

	cd->dl=NULL;
	cd->cbcreate=NULL;
	cd->cbinit=NULL;
	cd->cbctl=NULL;
	cd->cbconv=NULL;
	cd->cbflush=NULL;
	cd->cbdestroy=NULL;
	strcat(path, "." SHLIBEXT);

#ifdef WIN32
	if((cd->dl=LoadLibrary(path))){
		cd->cbconv=(void *)GetProcAddress(cd->dl,"cbconv");
		cd->cbflush=(void *)GetProcAddress(cd->dl,"cbflush");
		cd->cbcreate=(void *)GetProcAddress(cd->dl,"cbcreate");
		cd->cbinit=(void *)GetProcAddress(cd->dl,"cbinit");
		cd->cbctl=(void *)GetProcAddress(cd->dl,"cbctl");
		cd->cbdestroy=(void *)GetProcAddress(cd->dl,"cbdestroy");
	}
#else
	if((cd->dl=dlopen(path, RTLD_LAZY))){
		cd->cbconv=dlsym(cd->dl,"cbconv");
		cd->cbflush=dlsym(cd->dl,"cbflush");
		cd->cbcreate=dlsym(cd->dl,"cbcreate");
		cd->cbinit=dlsym(cd->dl,"cbinit");
		cd->cbctl=dlsym(cd->dl,"cbctl");
		cd->cbdestroy=dlsym(cd->dl,"cbdestroy");
		if(cd->cbcreate && cd->cbdestroy==NULL){
			fprintf(stderr,"Possible memory leak in %s\n", path);
		}
	}
#endif
	return 1;
}

char * bsdconv_solve_alias(int type, char *_codec){
	char *ret;
	char *codec;
	struct bsdconv_instance *ins;
	switch(type){
		case FROM:
			ins=bsdconv_create("ASCII:FROM_ALIAS:ASCII");
			break;
		case INTER:
			ins=bsdconv_create("ASCII:INTER_ALIAS:ASCII");
			break;
		case TO:
			ins=bsdconv_create("ASCII:TO_ALIAS:ASCII");
			break;
		default:
			return NULL;
	}
	if(ins==NULL){
		return NULL;
	}
	codec=strdup(_codec);
	strtoupper(codec);
	bsdconv_init(ins);
	ins->output_mode=BSDCONV_AUTOMALLOC;
	ins->output.len=1;
	ins->input.data=codec;
	ins->input.len=strlen(codec);
	ins->input.flags=F_FREE;
	ins->flush=1;
	bsdconv(ins);
	ret=ins->output.data;
	ret[ins->output.len]=0;
	bsdconv_destroy(ins);
	return ret;
}

int loadcodec(struct bsdconv_codec_t *cd, int type){
	char *cwd;
	char *c;
	char buf[PATH_MAX+1];
	cwd=getcwd(NULL, 0);
	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}
	chdir(CODECS_SUBPATH);
	switch(type){
		case FROM:
			chdir("from");
			break;
		case INTER:
			chdir("inter");
			break;
		case TO:
			chdir("to");
			break;
	}
	REALPATH(cd->desc, buf);
	chdir(cwd);
	free(cwd);
	if(!_loadcodec(cd, buf))
		return 0;
	return 1;
}

void unloadcodec(struct bsdconv_codec_t *cd){
#ifdef WIN32
	if(cd->dl){
		FreeLibrary(cd->dl);
	}
	UnmapViewOfFile(cd->z);
	CloseHandle(cd->md);
	CloseHandle(cd->fd);
#else
	if(cd->dl){
		dlclose(cd->dl);
	}
	munmap(cd->z, cd->maplen);
	close(cd->fd);
#endif
}

void bsdconv_hash_set(struct bsdconv_instance *ins, const char *key, void *ptr){
	char *tk;
	void *tp;
	struct hash_entry *p=ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			tp=ptr;
			tk=p->key;
			p->key=ins->hash->key;
			p->ptr=ins->hash->ptr;
			ins->hash->key=tk;
			ins->hash->ptr=tp;
			return;
		}
		p=p->next;
	}
	p=malloc(sizeof(struct hash_entry));
	p->next=ins->hash;
	ins->hash=p;
	p->key=strdup(key);
	p->ptr=ptr;
	return;
}

void *bsdconv_hash_get(struct bsdconv_instance *ins, const char *key){
	char *tk;
	void *tp;
	struct hash_entry *p=ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			tk=p->key;
			tp=p->ptr;
			p->key=ins->hash->key;
			p->ptr=ins->hash->ptr;
			ins->hash->key=tk;
			ins->hash->ptr=tp;
			return p->ptr;
		}
		p=p->next;
	}
	return NULL;
}

int bsdconv_hash_has(struct bsdconv_instance *ins, const char *key){
	char *tk;
	void *tp;
	struct hash_entry *p=ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			tk=p->key;
			tp=p->ptr;
			p->key=ins->hash->key;
			p->ptr=ins->hash->ptr;
			ins->hash->key=tk;
			ins->hash->ptr=tp;
			return 1;
		}
		p=p->next;
	}
	return 0;
}

void bsdconv_hash_del(struct bsdconv_instance *ins, const char *key){
	struct hash_entry *p=ins->hash;
	struct hash_entry **q=&ins->hash;
	while(p!=NULL){
		if(strcmp(p->key, key)==0){
			free(p->key);
			*q=p->next;
			free(p);
			return;
		}
		p=p->next;
		q=&p->next;
	}
}

void bsdconv_init(struct bsdconv_instance *ins){
	int i, j;
	struct data_rt *data_ptr;

	ins->flush=0;
	ins->input.data=NULL;
	ins->input.len=0;
	ins->output.data=NULL;
	ins->output.len=0;
	ins->output_mode=BSDCONV_HOLD;

	ins->ierr=0;
	ins->oerr=0;
	ins->score=0;
	ins->full=0;
	ins->half=0;
	ins->ambi=0;

	for(i=0;i<=ins->phasen;++i){
		ins->phase_index=i;
		ins->phase[i].flags=0;
		ins->phase[i].offset=0;
		while(ins->phase[i].data_head->next){
			data_ptr=ins->phase[i].data_head->next;
			ins->phase[i].data_head->next=ins->phase[i].data_head->next->next;
			if(data_ptr->flags & F_FREE)
				free(data_ptr->data);
			free(data_ptr);
		}
		ins->phase[i].data_tail=ins->phase[i].data_head;
		ins->phase[i].data_head->len=0;
		ins->phase[i].match_data=NULL;
		if(i>0){
			ins->phase[i].curr=ins->phase[i-1].data_head;
			for(j=0;j<=ins->phase[i].codecn;++j){
				ins->phase[i].index=j;
				if(ins->phase[i].codec[j].cbinit)
					ins->phase[i].codec[j].cbinit(ins);
			}
			RESET(i);
		}
	}
}

int bsdconv_get_phase_index(struct bsdconv_instance *ins, int phasen){
	/*
	 * phase[0] is a place holder for _INPUT
	 * real phases range is [1,len]=[1,phasen]
	 */
	/* logical new index = len */
	if(phasen /* logical */ >= ins->phasen /* len */){
		/* real  = logical + 1 */
		return ins->phasen + 1;
	}else{
		/* real  = (n + len) % (len) + 1*/
		return (phasen + ins->phasen) % (ins->phasen) + 1;
	}
}

int bsdconv_get_codec_index(struct bsdconv_instance *ins, int phasen, int codecn){
	/*
	 * codecn is -=1 for convenient use as boundary
	 * real phases range is [0,len)=[0,codecn]
	 */
	phasen=bsdconv_get_phase_index(ins, phasen);

	/* logical new index = len */
	if(codecn /* logical */ >= ins->phase[phasen].codecn+1 /* len */ ){
		/* real  = logical */
		return ins->phase[phasen].codecn+1;
	}else{
		/* real  = (n + len) % (len) */
		return (codecn + ins->phase[phasen].codecn+1) % (ins->phase[phasen].codecn+1);
	}
}

int bsdconv_insert_phase(struct bsdconv_instance *ins, const char *codec, int phase_type, int ophasen){
	int i,len;
	const char *c;
	char *t, *cd=strdup(codec), *cd2;

	int phasen=bsdconv_get_phase_index(ins, ophasen);

	strtoupper(cd);
	len=1;
	for(c=codec;*c;++c) if(*c==',') ++len;

	cd2=strdup(cd);
	t=cd2;
	for(i=0;i<len;++i){
		if(!bsdconv_codec_check(phase_type, strsep(&t,","))){
			free(cd);
			free(cd2);
			return -1;
		}
	}
	free(cd2);

	++ins->phasen;
	ins->phase=realloc(ins->phase, sizeof(struct bsdconv_phase) * (ins->phasen+1));

	for(i=ins->phasen /* shifted index */;i>phasen;--i){
		ins->phase[i]=ins->phase[i-1];
	}
	ins->phase[phasen].type=phase_type;
	ins->phase[phasen].codec=malloc(sizeof(struct bsdconv_codec_t)*len);
	ins->phase[phasen].codecn=len-1 /* trimmed length */;

	ins->phase[phasen].data_head=malloc(sizeof(struct data_rt));
	ins->phase[phasen].data_head->next=NULL;
	ins->phase[phasen].data_head->flags=0;

	t=cd;
	for(i=0;i<len;++i){
		ins->phase[phasen].codec[i].desc=strdup(strsep(&t,","));
		ins->phase[phasen].codec[i].argv=strchr(ins->phase[phasen].codec[i].desc, '#');
		if(ins->phase[phasen].codec[i].argv){
			*(ins->phase[phasen].codec[i].argv)=0;
			ins->phase[phasen].codec[i].argv+=1;
		}

		if(!loadcodec(&ins->phase[phasen].codec[i], phase_type)){
			free(cd);
			free(cd2);
			return -1;
		}
		ins->phase_index=phasen;
		ins->phase[phasen].index=i;
		if(ins->phase[phasen].codec[i].cbcreate)
			_cbcreate(ins, phasen, i);
	}
	free(cd);

	return phasen;
}

int bsdconv_insert_codec(struct bsdconv_instance *ins, const char *codec, int ophasen, int ocodecn){
	int i;

	int phasen=bsdconv_get_phase_index(ins, ophasen);
	int codecn=bsdconv_get_codec_index(ins, ophasen, ocodecn);

	if(!bsdconv_codec_check(ins->phase[phasen].type, codec)){
		return -1;
	}

	++ins->phase[phasen].codecn;
	ins->phase[phasen].codec=realloc(ins->phase[phasen].codec, sizeof(struct bsdconv_codec_t)*(ins->phase[phasen].codecn+1));

	for(i=ins->phase[phasen].codecn;i>codecn;--i){
		ins->phase[phasen].codec[i]=ins->phase[phasen].codec[i-1];
	}
	ins->phase[phasen].codec[codecn].desc=strdup(codec);
	strtoupper(ins->phase[phasen].codec[codecn].desc);
	ins->phase[phasen].codec[codecn].argv=strchr(ins->phase[phasen].codec[codecn].desc, '#');
	if(ins->phase[phasen].codec[codecn].argv){
		*(ins->phase[phasen].codec[codecn].argv)=0;
		ins->phase[phasen].codec[codecn].argv+=1;
	}

	if(!loadcodec(&ins->phase[phasen].codec[codecn], ins->phase[phasen].type)){
		return -1;
	}
	ins->phase_index=phasen;
	ins->phase[phasen].index=codecn;
	if(ins->phase[phasen].codec[codecn].cbcreate)
		_cbcreate(ins, phasen, codecn);
	return codecn;
}

int bsdconv_replace_phase(struct bsdconv_instance *ins, const char *codec, int phase_type, int ophasen){
	int i,len;
	const char *c;
	char *t, *cd=strdup(codec), *cd2;

	int phasen=bsdconv_get_phase_index(ins, ophasen);

	strtoupper(cd);
	len=1;
	for(c=codec;*c;++c) if(*c==',') ++len;

	cd2=strdup(cd);
	t=cd2;
	for(i=0;i<len;++i){
		if(!bsdconv_codec_check(phase_type, strsep(&t,","))){
			free(cd);
			free(cd2);
			return -1;
		}
	}
	free(cd2);

	for(i=0;i<=ins->phase[phasen].codecn;++i){
		free(ins->phase[phasen].codec[i].desc);
		if(ins->phase[phasen].codec[i].cbdestroy){
			ins->phase[phasen].codec[i].cbdestroy(ins->phase[phasen].codec[i].priv);
		}
		unloadcodec(&ins->phase[phasen].codec[i]);
	}

	ins->phase[phasen].type=phase_type;
	ins->phase[phasen].codec=realloc(ins->phase[phasen].codec, sizeof(struct bsdconv_codec_t)*len);
	ins->phase[phasen].codecn=len-1 /* trimmed length */;

	t=cd;
	for(i=0;i<len;++i){
		ins->phase[phasen].codec[i].desc=strdup(strsep(&t,","));
		ins->phase[phasen].codec[i].argv=strchr(ins->phase[phasen].codec[i].desc, '#');
		if(ins->phase[phasen].codec[i].argv){
			*(ins->phase[phasen].codec[i].argv)=0;
			ins->phase[phasen].codec[i].argv+=1;
		}

		if(!loadcodec(&ins->phase[phasen].codec[i], phase_type)){
			free(cd);
			return -1;
		}
		ins->phase_index=phasen;
		ins->phase[phasen].index=i;
		if(ins->phase[phasen].codec[i].cbcreate)
			_cbcreate(ins, phasen, i);
	}
	free(cd);

	return phasen;
}

int bsdconv_replace_codec(struct bsdconv_instance *ins, const char *codec, int ophasen, int ocodecn){
	int phasen=bsdconv_get_phase_index(ins, ophasen);
	int codecn=bsdconv_get_codec_index(ins, ophasen, ocodecn);

	if(!bsdconv_codec_check(ins->phase[phasen].type, codec)){
		return -1;
	}

	free(ins->phase[phasen].codec[codecn].desc);
	if(ins->phase[phasen].codec[codecn].cbdestroy){
		ins->phase[phasen].codec[codecn].cbdestroy(ins->phase[phasen].codec[codecn].priv);
	}
	unloadcodec(&ins->phase[phasen].codec[codecn]);

	ins->phase[phasen].codec[codecn].desc=strdup(codec);
	strtoupper(ins->phase[phasen].codec[codecn].desc);
	ins->phase[phasen].codec[codecn].argv=strchr(ins->phase[phasen].codec[codecn].desc, '#');
	if(ins->phase[phasen].codec[codecn].argv){
		*(ins->phase[phasen].codec[codecn].argv)=0;
		ins->phase[phasen].codec[codecn].argv+=1;
	}

	if(!loadcodec(&ins->phase[phasen].codec[codecn], ins->phase[phasen].type)){
		return -1;
	}
	ins->phase_index=phasen;
	ins->phase[phasen].index=codecn;
	if(ins->phase[phasen].codec[codecn].cbcreate)
		_cbcreate(ins, phasen, codecn);
	return codecn;
}

void bsdconv_ctl(struct bsdconv_instance *ins, int ctl, void *p, int v){
	int i,j;
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(ins->phase[i].codec[j].cbctl){
				ins->phase_index=i;
				ins->phase[i].index=j;
				ins->phase[i].codec[j].cbctl(ins, ctl, p, v);
			}
		}
	}
}

char *bsdconv_pack(struct bsdconv_instance *ins){
	char *ret;
	char *t, *cur;
	const char *end;
	int len=0;
	int i,j,n;
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			len+=strlen(ins->phase[i].codec[j].desc);
			t=ins->phase[i].codec[j].desc;
			n=1;
			while(*t){
				if(*t==',')
					n+=1;
				t++;
			}
			if(ins->phase[i].codec[j].argv)
				len+=(strlen(ins->phase[i].codec[j].argv)+1)*n;
			len+=1;
		}
	}
	ret=malloc(sizeof(char) * len);
	ret[0]=0;
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(j==0){
				if(i>1){
					switch(ins->phase[i].type){
						case FROM:
							strcat(ret, "|");
							break;
						case INTER:
						case TO:
							strcat(ret, ":");
						break;
					}
				}
			}else{
				strcat(ret, ",");
			}
			t=ins->phase[i].codec[j].desc;
			while(1){
				cur=strchr(t, ',');
				if(cur){
					end=",";
					*cur=0;
				}else{
					end="";					
				}
				strcat(ret, t);
				if(ins->phase[i].codec[j].argv && *(ins->phase[i].codec[j].argv)){
					if(strchr(t,'#')==NULL)
						strcat(ret, "#");
					else
						strcat(ret, "&");
					strcat(ret, ins->phase[i].codec[j].argv);
				}
				strcat(ret,end);
				if(cur)
					t=cur+1;
				else
					break;
			}
		}
	}
	return ret;
}

struct bsdconv_instance *bsdconv_unpack(const char *_conversion){
	struct bsdconv_instance *ins=malloc(sizeof(struct bsdconv_instance));
	char *conversion;
	char *t, *t1;
	int i, j;
	int f=0;

	conversion=strdup(_conversion);
	t1=t=conversion;
	strtoupper(conversion);
	i=1;
	for(t=(char *)conversion;*t;t++){
		if(*t==':' || *t=='|')++i;
	}
	ins->phasen=i;
	char *opipe[i+1];

	ins->phase=malloc(sizeof(struct bsdconv_phase) * (i+1));

	i=1;
	t1=t=conversion;
	while((t1=strsep(&t, "|")) != NULL){
		if(f>1){
			ins->phase[i-f].type=FROM;
			ins->phase[i-1].type=TO;
		}
		f=0;
		while((opipe[i]=strsep(&t1, ":"))!=NULL){
			ins->phase[i].type=INTER;
			i+=1;
			f+=1;
		}
	}
	if(f>1){
		ins->phase[i-f].type=FROM;
		ins->phase[i-1].type=TO;
	}
	ins->phase[0].type=_INPUT;

	for(i=1;i<=ins->phasen;++i){
		if(*opipe[i]){
			ins->phase[i].codecn=1;
			for(t=(char *)opipe[i];*t;t++){
				if(*t==','){
					ins->phase[i].codecn+=1;
				}
			}
		}else{
			free(ins->phase);
			free(ins);
			free(conversion);
			return NULL;
		}
		ins->phase[i].codecn-=1;
	}
	for(i=1;i<=ins->phasen;++i){
		ins->phase[i].codec=malloc((ins->phase[i].codecn + 1)* sizeof(struct bsdconv_codec_t));
	}
	for(i=1;i<=ins->phasen;++i){
		t=opipe[i];
		for(j=0;j<=ins->phase[i].codecn;++j){
			ins->phase[i].codec[j].desc=strdup(strsep(&t, ","));
			ins->phase[i].codec[j].argv=strchr(ins->phase[i].codec[j].desc, '#');
			if(ins->phase[i].codec[j].argv){
				*(ins->phase[i].codec[j].argv)=0;
				ins->phase[i].codec[j].argv+=1;
			}
			if(ins->phase[i].codec[j].desc[0]==0){
				for(;j>=0;--j){
					free(ins->phase[i].codec[j].desc);
				}
				for(i=1;i<=ins->phasen;++i){
					free(ins->phase[i].codec);
				}
				free(ins->phase);
				free(ins);
				free(conversion);
				return NULL;
			}
		}
	}
	free(conversion);
	return ins;
}

struct bsdconv_instance *bsdconv_create(const char *_conversion){
	int e=0;
	struct bsdconv_instance *ins=NULL;
	char *conversion=malloc(strlen(_conversion)+1);
	int i, j;
	char *c;
	const char *d;
	char whitespace[256]={0};
	whitespace['\r']=1;
	whitespace['\n']=1;
	whitespace['\t']=1;
	whitespace['\f']=1;
	whitespace[' ']=1;
	d=_conversion;
	c=conversion;
	while(*d){
		if(whitespace[*UCP(d)]==0){
			*c=*d;
			c+=1;
		}
		d+=1;
	}
	*c=0;

	i=0;
	while(i==0 || i<=ins->phasen){
		start_parse:
		ins=bsdconv_unpack(conversion);
		if(ins==NULL){
			free(conversion);
			SetLastError(EINVAL);
			return NULL;
		}
		for(i=1;i<=ins->phasen;++i){
			for(j=0;j<=ins->phase[i].codecn;++j){
				if(!bsdconv_codec_check(ins->phase[i].type, ins->phase[i].codec[j].desc)){
					c=ins->phase[i].codec[j].desc;
					ins->phase[i].codec[j].desc=bsdconv_solve_alias(ins->phase[i].type, ins->phase[i].codec[j].desc);
					if(strcmp(c, ins->phase[i].codec[j].desc)==0)
						e=1;
					free(conversion);
					conversion=bsdconv_pack(ins);
					free(c);
					for(i=1;i<=ins->phasen;++i){
						for(j=0;j<=ins->phase[i].codecn;++j){
							free(ins->phase[i].codec[j].desc);
						}
						free(ins->phase[i].codec);
					}
					free(ins->phase);
					free(ins);
					if(e){
						SetLastError(EOPNOTSUPP);
						free(conversion);
						return NULL;
					}
					goto start_parse;
				}
			}
		}
	}
	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(!loadcodec(&ins->phase[i].codec[j], ins->phase[i].type)){
				free(ins->phase[i].codec[j].desc);
				j-=1;
				for(;i>=1;j=ins->phase[--i].codecn){
					for(;j>=0;--j){
						free(ins->phase[i].codec[j].desc);
						unloadcodec(&ins->phase[i].codec[j]);
					}
				}
				goto bsdconv_create_error;
			}
		}
	}

	ins->pool=NULL;
	ins->hash=NULL;
	ins->input.flags=0;
	ins->output.flags=0;

	for(i=1;i<=ins->phasen;++i){
		for(j=0;j<=ins->phase[i].codecn;++j){
			if(ins->phase[i].codec[j].cbcreate){
				ins->phase_index=i;
				ins->phase[i].index=j;
				e=_cbcreate(ins, i, j);
				if(e){
					for(j=j-1;j>=0;j-=1){
						ins->phase[i].codec[j].cbdestroy(ins);
					}
					for(i=i-1;i>=1;i-=1){
						for(j=0;j<=ins->phase[i].codecn;++j){
							ins->phase[i].codec[j].cbdestroy(ins);
						}
					}
					for(i=1;i<=ins->phasen;++i){
						for(j=0;j<=ins->phase[i].codecn;++j){
							free(ins->phase[i].codec[j].desc);
							unloadcodec(&ins->phase[i].codec[j]);
						}
					}
					SetLastError(e);
					goto bsdconv_create_error;
				}
			}
		}
	}
	for(i=0;i<=ins->phasen;++i){
		ins->phase[i].data_head=malloc(sizeof(struct data_rt));
		ins->phase[i].data_head->next=NULL;
		ins->phase[i].data_head->flags=0;
	}

	free(conversion);
	return ins;

bsdconv_create_error:
	for(i=1;i<=ins->phasen;++i){
		free(ins->phase[i].codec);
	}

	free(conversion);
	free(ins->phase);
	free(ins);
	return NULL;
}

void bsdconv_destroy(struct bsdconv_instance *ins){
	int i,j;
	struct data_rt *data_ptr;

	for(i=0;i<=ins->phasen;++i){
		if(i>0){
			for(j=0;j<=ins->phase[i].codecn;++j){
				free(ins->phase[i].codec[j].desc);
				if(ins->phase[i].codec[j].cbdestroy){
					ins->phase_index=i;
					ins->phase[i].index=j;
					ins->phase[i].codec[j].cbdestroy(ins);
				}
				unloadcodec(&ins->phase[i].codec[j]);
			}
			free(ins->phase[i].codec);
		}
		while(ins->phase[i].data_head){
			data_ptr=ins->phase[i].data_head;
			ins->phase[i].data_head=ins->phase[i].data_head->next;
			if(data_ptr->flags & F_FREE)
				free(data_ptr->data);
			free(data_ptr);
		}
	}
	while(ins->pool){
		data_ptr=ins->pool;
		ins->pool=ins->pool->next;
		free(data_ptr);
	}
	free(ins->phase);
	while(ins->hash){
		free(ins->hash->key);
		ins->hash=ins->hash->next;
	}
	free(ins);
}

void bsdconv(struct bsdconv_instance *ins){
	uintptr_t i;
	struct data_rt *data_ptr;
	char *ptr;
	FILE *fp;
	int fd;
	unsigned char c;
	struct bsdconv_phase *this_phase;
	struct bsdconv_phase *prev_phase;

	if(ins->input.data!=NULL){
		DATA_MALLOC(ins->phase[0].data_tail->next);
		ins->phase[0].data_tail=ins->phase[0].data_tail->next;
		*(ins->phase[0].data_tail)=ins->input;
		ins->phase[0].data_tail->next=NULL;
		ins->input.data=NULL;
		ins->input.len=0;
		ins->input.flags=0;
	}

	ins->phase_index=1;

	phase_begin:
	if(ins->phase_index>0 && ins->phase_index<=ins->phasen){
		this_phase=&ins->phase[ins->phase_index];
		prev_phase=&ins->phase[ins->phase_index-1];
		switch(this_phase->type){
			case FROM:
				while(this_phase->curr->next){
					if(this_phase->curr == prev_phase->data_head) this_phase->i=this_phase->data_head->len;
					else this_phase->i=0;
					this_phase->curr=this_phase->curr->next;
					while(this_phase->i<this_phase->curr->len){
						c=UCP(this_phase->curr->data)[this_phase->i];
						if(c>=this_phase->state.beg && c<this_phase->state.end)
							memcpy(&this_phase->offset, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.base + (c - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
						else if(!(this_phase->flags & F_LOOPBACK))
							this_phase->offset=0;
						memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + this_phase->offset, sizeof(struct state_st));
						from_x:
						switch(this_phase->state.status){
							case DEADEND:
								from_deadend:
								this_phase->flags &= ~(F_PENDING | F_LOOPBACK);
								if(this_phase->flags & F_MATCH){
									if(this_phase->match_data){
										LISTCPY(this_phase->data_tail, this_phase->match_data, this_phase->codec[this_phase->index].data_z);

										LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
										this_phase->curr=prev_phase->data_head;
										this_phase->i=this_phase->data_head->len;
									}else if(this_phase->codec[this_phase->index].cbflush){
										this_phase->codec[this_phase->index].cbflush(ins);
									}
									this_phase->flags &= ~F_MATCH;
									RESET(ins->phase_index);
									goto phase_begin;
								}else if(this_phase->index < this_phase->codecn){
									this_phase->index++;
									memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
									this_phase->curr=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len;
									continue;
								}else{
									ins->ierr++;

									RESET(ins->phase_index);

									this_phase->bak=this_phase->curr;
									LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
									this_phase->bak=this_phase->curr=prev_phase->data_head;
									this_phase->i=this_phase->data_head->len=this_phase->data_head->len+1;
									continue;
								}
								break;
							case MATCH:
								this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
								this_phase->match_data=NULL;

								LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

								this_phase->bak=this_phase->curr;
								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
								this_phase->data_head->len=this_phase->i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;
							case SUBMATCH:
								this_phase->flags |= (F_MATCH | F_PENDING);
								this_phase->match_data=this_phase->state.data;

								this_phase->bak=this_phase->curr;
								this_phase->data_head->len=this_phase->i+1;
								break;
							case SUBROUTINE:
							case SUBMATCH_SUBROUTINE:
								this_phase->codec[this_phase->index].cbconv(ins);
								this_phase->flags |= F_LOOPBACK;
								goto from_x;
							case NEXTPHASE:
								this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
								this_phase->match_data=NULL;

								this_phase->bak=this_phase->curr;
								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
								this_phase->data_head->len=this_phase->i+1;

								RESET(ins->phase_index);

								ins->phase_index+=1;
								goto phase_begin;
							case CONTINUE:
								this_phase->flags |= F_PENDING;
								break;
							case NOOP:
								goto phase_begin;
						}
						this_phase->i+=1;
					}
				}
			break;

		case INTER:
			while(this_phase->curr->next){
				this_phase->curr=this_phase->curr->next;
				this_phase->state.status=NOMATCH;
				for(this_phase->i=0;this_phase->i<this_phase->curr->len;this_phase->i+=1){
					c=UCP(this_phase->curr->data)[this_phase->i];
					if(c>=this_phase->state.beg && c<this_phase->state.end){
						memcpy(&this_phase->offset, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.base + (c - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
					}else if(!(this_phase->flags & F_LOOPBACK)){
						this_phase->offset=0;
					}
					memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + this_phase->offset, sizeof(struct state_st));
					switch(this_phase->state.status){
						case DEADEND:
							goto inter_deadend;
							break;
						case SUBROUTINE:
						case SUBMATCH_SUBROUTINE:
							this_phase->flags |= F_LOOPBACK;
							break;
					}
				}
				inter_x:
				switch(this_phase->state.status){
					case NOMATCH:
						ins->phase_index+=1;
						goto phase_begin;
					case DEADEND:
						inter_deadend:
						this_phase->flags &= ~(F_PENDING | F_LOOPBACK);
						if(this_phase->flags & F_MATCH){
							if(this_phase->match_data){
								LISTCPY(this_phase->data_tail, this_phase->match_data, this_phase->codec[this_phase->index].data_z);

								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
							}else if(this_phase->codec[this_phase->index].cbflush){
								this_phase->codec[this_phase->index].cbflush(ins);
							}

							this_phase->flags &= ~F_MATCH;
							RESET(ins->phase_index);
							goto phase_begin;
						}else if(this_phase->index < this_phase->codecn){
							this_phase->index++;
							memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
							this_phase->curr=prev_phase->data_head;
							continue;
						}else{
							data_ptr=prev_phase->data_head->next;
							prev_phase->data_head->next=prev_phase->data_head->next->next;
							this_phase->curr=prev_phase->data_head;
							data_ptr->next=NULL;
							this_phase->data_tail->next=data_ptr;
							this_phase->data_tail=data_ptr;
							if(prev_phase->data_tail==data_ptr){
								prev_phase->data_tail=prev_phase->data_head;
							}
							this_phase->curr=prev_phase->data_head;

							RESET(ins->phase_index);

							ins->phase_index+=1;
							goto phase_begin;
						}
						break;
					case MATCH:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

						this_phase->bak=this_phase->curr->next;
						LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case SUBMATCH:
						this_phase->flags |= (F_MATCH | F_PENDING);
						this_phase->match_data=this_phase->state.data;

						if(this_phase->curr->next){
							this_phase->bak=this_phase->curr->next;
						}else{
							DATA_MALLOC(prev_phase->data_tail->next);
							this_phase->bak=prev_phase->data_tail->next;
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->flags=0;
						}

						break;
					case SUBROUTINE:
					case SUBMATCH_SUBROUTINE:
						this_phase->codec[this_phase->index].cbconv(ins);
						goto inter_x;
					case NEXTPHASE:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						this_phase->bak=this_phase->curr->next;
						LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case CONTINUE:
						this_phase->flags |= F_PENDING;
						this_phase->flags &= ~F_LOOPBACK;
						break;
					case NOOP:
						goto phase_begin;

				}
				if(256<this_phase->state.end)
					memcpy(&this_phase->offset, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.base + (256 - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
				else if(!(this_phase->flags & F_LOOPBACK))
					this_phase->offset=0;
				memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + this_phase->offset, sizeof(struct state_st));
				if(this_phase->state.status==DEADEND){ goto inter_deadend;}
			}
			break;

		case TO:
			while(this_phase->curr->next){
				this_phase->curr=this_phase->curr->next;
				this_phase->state.status=NOMATCH;
				for(this_phase->i=0;this_phase->i<this_phase->curr->len;this_phase->i+=1){
					c=UCP(this_phase->curr->data)[this_phase->i];
					if(c>=this_phase->state.beg && c<this_phase->state.end)
						memcpy(&this_phase->offset, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.base + (c - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
					else if(!(this_phase->flags & F_LOOPBACK))
						this_phase->offset=0;
					memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + this_phase->offset, sizeof(struct state_st));
					switch(this_phase->state.status){
						case DEADEND:
							goto to_deadend;
							break;
						case SUBROUTINE:
						case SUBMATCH_SUBROUTINE:
							this_phase->flags |= F_LOOPBACK;
							break;
					}
				}
				to_x:
				switch(this_phase->state.status){
					case NOMATCH:
						continue;
					case DEADEND:
						to_deadend:
						this_phase->flags &= ~(F_PENDING | F_LOOPBACK);
						if(this_phase->flags & F_MATCH){
							if(this_phase->match_data){
								LISTCPY(this_phase->data_tail, this_phase->match_data, this_phase->codec[this_phase->index].data_z);

								LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
								this_phase->curr=prev_phase->data_head;
							}else if(this_phase->codec[this_phase->index].cbflush){
								this_phase->codec[this_phase->index].cbflush(ins);
							}

							this_phase->flags &= ~F_MATCH;
							RESET(ins->phase_index);
							ins->phase_index+=1;
							goto phase_begin;
						}else if(this_phase->index < this_phase->codecn){
							this_phase->index++;
							memcpy(&this_phase->state, this_phase->codec[this_phase->index].z, sizeof(struct state_st));
							this_phase->curr=prev_phase->data_head;
							continue;
						}else{
							ins->oerr++;

							RESET(ins->phase_index);

							this_phase->bak=this_phase->curr->next;
							LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
							this_phase->bak=this_phase->curr=prev_phase->data_head;

							continue;
						}
						break;
					case MATCH:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						LISTCPY(this_phase->data_tail, this_phase->state.data, this_phase->codec[this_phase->index].data_z);

						this_phase->bak=this_phase->curr->next;
						LISTFREE(prev_phase->data_head, this_phase->bak,prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);
						ins->phase_index+=1;
						goto phase_begin;
					case SUBMATCH:
						this_phase->flags |= (F_MATCH | F_PENDING);
						this_phase->match_data=this_phase->state.data;
						if(this_phase->curr->next){
							this_phase->bak=this_phase->curr->next;
						}else{
							DATA_MALLOC(prev_phase->data_tail->next);
							this_phase->bak=prev_phase->data_tail->next;
							prev_phase->data_tail=prev_phase->data_tail->next;
							prev_phase->data_tail->next=NULL;
							prev_phase->data_tail->len=0;
							prev_phase->data_tail->flags=0;
						}

						break;
					case SUBROUTINE:
					case SUBMATCH_SUBROUTINE:
						this_phase->codec[this_phase->index].cbconv(ins);
						goto to_x;
					case NEXTPHASE:
						this_phase->flags &= ~(F_MATCH | F_PENDING | F_LOOPBACK);
						this_phase->match_data=NULL;

						this_phase->bak=this_phase->curr->next;
						LISTFREE(prev_phase->data_head,this_phase->bak,prev_phase->data_tail);
						this_phase->curr=prev_phase->data_head;

						RESET(ins->phase_index);

						ins->phase_index+=1;
						goto phase_begin;
					case CONTINUE:
						this_phase->flags|=F_PENDING;
						this_phase->flags &= ~F_LOOPBACK;
						break;
					case NOOP:
						goto phase_begin;
				}
				if(256<this_phase->state.end)
					memcpy(&this_phase->offset, this_phase->codec[this_phase->index].z + (uintptr_t)this_phase->state.base + (256 - this_phase->state.beg) * sizeof(offset_t), sizeof(offset_t));
				else if(!(this_phase->flags & F_LOOPBACK))
					this_phase->offset=0;
				memcpy(&this_phase->state, this_phase->codec[this_phase->index].z + this_phase->offset, sizeof(struct state_st));
				if(this_phase->state.status==DEADEND){ goto to_deadend;}
			}
			break;
		}
		ins->phase_index+=1;
	}

	//check back (phase-loop)
	for(ins->phase_index=ins->phasen;ins->phase_index>0;ins->phase_index-=1){
		if(ins->phase[ins->phase_index].curr->next){
			goto phase_begin;
		}
	}

	//flush
	if(ins->flush){
		for(ins->phase_index=1;ins->phase_index<=ins->phasen;++(ins->phase_index)){
			if(ins->phase[ins->phase_index].flags & F_PENDING){
				this_phase=&ins->phase[ins->phase_index];
				prev_phase=&ins->phase[ins->phase_index-1];
				switch(this_phase->type){
					case FROM:	goto from_deadend;
					case INTER:	goto inter_deadend;
					case TO:	goto to_deadend;
				}
			}
		}
	}

	//output
	switch(ins->output_mode){
		case BSDCONV_HOLD:
			ins->output.len=0;
			ins->output.flags=0;
			break;
		case BSDCONV_AUTOMALLOC:
			i=ins->output.len;
			data_ptr=ins->phase[ins->phasen].data_head;
			while(data_ptr){
				i+=data_ptr->len;
				data_ptr=data_ptr->next;
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
			ins->output.flags=1;
			ptr=ins->output.data=malloc(i);
			ins->output.len=i-ins->output.len;
			data_ptr=ins->phase[ins->phasen].data_head;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				memcpy(ptr, data_ptr->data, data_ptr->len);
				ptr+=data_ptr->len;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			break;
		case BSDCONV_PREMALLOCED:
			ins->output.flags=0;
			if(ins->output.data!=NULL && ins->output.len){
				i=0;
				while(ins->phase[ins->phasen].data_head->next && ins->phase[ins->phasen].data_head->next->len<=ins->output.len-i){
					memcpy(ins->output.data+i, ins->phase[ins->phasen].data_head->next->data, ins->phase[ins->phasen].data_head->next->len);
					i+=ins->phase[ins->phasen].data_head->next->len;
					if(ins->phase[ins->phasen].data_tail==ins->phase[ins->phasen].data_head->next){
						ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
					}
					data_ptr=ins->phase[ins->phasen].data_head->next;
					ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
					DATA_FREE(data_ptr);
				}
				ins->output.len=i;
			}else{
				i=0;
				data_ptr=ins->phase[ins->phasen].data_head;
				while(data_ptr){
					i+=data_ptr->len;
					data_ptr=data_ptr->next;
				}
				ins->output.len=i;
			}
			break;
		case BSDCONV_FILE:
			fp=ins->output.data;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				fwrite(data_ptr->data, data_ptr->len, 1, fp);
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
			break;
		case BSDCONV_FD:
			fd=(intptr_t)ins->output.data;
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				write(fd, data_ptr->data, data_ptr->len);
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
			break;
		case BSDCONV_NULL:
			while(ins->phase[ins->phasen].data_head->next){
				data_ptr=ins->phase[ins->phasen].data_head->next;
				ins->phase[ins->phasen].data_head->next=ins->phase[ins->phasen].data_head->next->next;
				DATA_FREE(data_ptr);
			}
			ins->phase[ins->phasen].data_tail=ins->phase[ins->phasen].data_head;
			break;
	}
	return;
}

char * bsdconv_error(void){
	switch(GetLastError()){
		case EDOOFUS:
				return strdup("Unexpected condition");
		case EOPNOTSUPP:
				return strdup("Unsupported charset/encoding");
		case ENOMEM:
				return strdup("Mmap failed");
		case EINVAL:
				return strdup("Conversion syntax error");
		default:
				return strdup("Unknown error");
	}
}

int bsdconv_codec_check(int type, const char *_codec){
	int ret=0;
	char *cwd;
	char *codec;
	FILE *fp;
	char *c;

	codec=strdup(_codec);
	strtoupper(codec);

	cwd=getcwd(NULL, 0);

	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}

	chdir(CODECS_SUBPATH);
	switch(type){
		case FROM:
			chdir("from");
			break;
		case INTER:
			chdir("inter");
			break;
		case TO:
			chdir("to");
			break;
	}

	fp=fopen(codec, "rb");
	if(fp!=NULL){
		fclose(fp);
		ret=1;
	}
	free(codec);

	chdir(cwd);
	free(cwd);
	return ret;
}

char ** bsdconv_codecs_list(int phase_type){
	char **list=malloc(sizeof(char *) * 8);
	int size=8;
	int length=0;
	char *cwd;
	char *c;
	DIR *dir;
	struct dirent *d;
	FILE *fp;
	char buf[256];
	const char *type;
	cwd=getcwd(NULL, 0);

	if((c=getenv("BSDCONV_PATH"))){
		chdir(c);
	}else{
		chdir(BSDCONV_PATH);
	}
	list[0]=NULL;
	chdir(CODECS_SUBPATH);
	switch(phase_type){
		case FROM:
			type="from";
			break;
		case INTER:
			type="inter";
			break;
		case TO:
			type="to";
			break;
		default:
			return list;
	}
	dir=opendir(type);
	if(dir!=NULL){
		while((d=readdir(dir))!=NULL){
			if(strstr(d->d_name, ".")!=NULL || strcmp(d->d_name, "alias")==0)
				continue;
			if(length>=size){
				size+=8;
				list=realloc(list, sizeof(char *) * size);
			}
			list[length]=strdup(d->d_name);
			length+=1;
		}
		closedir(dir);
	}
	chdir(type);
	fp=fopen("alias","rb");
	if(fp!=NULL){
		while(fgets(buf, sizeof(buf), fp)!=NULL){
			if(length>=size){
				size+=8;
				list=realloc(list, sizeof(char *) * size);
			}
			c=buf;
			list[length]=strdup(strsep(&c, "\t"));
			length+=1;
		}
		fclose(fp);
	}
	if(length>=size){
		size+=8;
		list=realloc(list, sizeof(char *) * size);
	}
	list[length]=NULL;
	length+=1;
	chdir(cwd);
	free(cwd);
	return list;
}

void *bsdconv_malloc(size_t size){
	return malloc(size);
}

void bsdconv_free(void *p){
	free(p);
}

int bsdconv_mkstemp(char *template){
	return mkstemp(template);
}
