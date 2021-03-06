#include <stdio.h>
#include <string.h>
#include "bsdconv.h"

int item_in_array(char *item, char **array, int size){
	int i;
	for(i=0;i<size;++i){
		if(strcmp(item, array[i])==0)
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[]){
	char *arg="";
	char *arg_base;
	char *c;
	char **p;
	char *part;
	int mode;
	char **inter_list;
	char **fromto_list;
	char **codecs_list;
	int i;

	if(argc>1)
		arg=argv[1];

	arg=arg_base=strdup(arg);

	if(arg[0]=='\'' || arg[0]=='"'){
		arg+=1;
	}
	i=strlen(arg)-1;
	if(arg[i]=='\'' || arg[i]=='"'){
		arg[i]=0;
	}

	part=arg;
	mode=FROM;
	strtoupper(arg);
	for(c=arg;*c;++c){
		switch(*c){
			case ':':
				mode=TO;
				part=c+1;
				break;
			case '|':
				mode=FROM;
				part=c+1;
				break;
			case ',':
				part=c+1;
				break;
		}
	}

	int size=0;
	int num=0;
	inter_list=bsdconv_modules_list(INTER);
	fromto_list=bsdconv_modules_list(mode);
	for(p=inter_list;*p;++p)
		num+=1;
	for(p=fromto_list;*p;++p)
		num+=1;

	codecs_list=malloc(sizeof(char *) * (num+1));
	for(p=inter_list;*p;++p){
		if(strstr(*p,part)==*p && !item_in_array(*p, codecs_list, size)){
			codecs_list[size]=*p;
			size+=1;
		}
		codecs_list[size]=NULL;
	}
	for(p=fromto_list;*p;++p){
		if(strstr(*p,part)==*p && !item_in_array(*p, codecs_list, size)){
			codecs_list[size]=*p;
			size+=1;
		}
		codecs_list[size]=NULL;
	}

	for(i=0;i<size;++i){
		char *res=malloc(strlen(arg)+strlen(codecs_list[i])+1);
		strncpy(res, arg, part-arg);
		res[part-arg]=0;
		strcat(res, codecs_list[i]);

		printf("%s\n", res);

		free(res);
	}

	for(p=inter_list;*p;++p){
		bsdconv_free(*p);
	}
	bsdconv_free(inter_list);
	for(p=fromto_list;*p;++p){
		bsdconv_free(*p);
	}
	bsdconv_free(fromto_list);

	bsdconv_free(codecs_list);

	free(arg_base);
	return 0;
}
