#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXN	512
#define E_INPUT	1
#define E_CMD		2

int
parse_line(char **p, char **myptr, char **_argv){
	while(1){
		switch(**p){
			case ' ':
				**p = 0;
				(*p)++;
				while(**p == ' ') (*p)++;
				break;
			case '\n':
				**p = 0;
				(*p)++;
				*myptr = 0;
				if(*(*p + 1) == 0) return E_INPUT;
				return E_CMD;
			case 0:
				*myptr = 0;
				return E_INPUT;
			default:
				*myptr = *p;
				myptr++;
				if(myptr - _argv >= MAXARG){
					fprintf(2, "xargs: too many arguments!\n");
					exit(1);
				}
				while(**p != 0 && **p != ' ' && **p != '\n') (*p)++;
		}
	}				
}

int
main(int argc, char *argv[])
{
  char *_argv[MAXARG];
  char *p;
  char line[MAXN];
  int ret;
  int i;
  
  if(argc == 1) {
  	fprintf(2, "Usage: xargs command...\n");
  	exit(1);
  }
  
  if(argc > MAXN){
  	fprintf(2, "xargs: too many arguments!\n");
  	exit(1);
  }
  
  for(i = 1;i < argc;++i) _argv[i - 1] = argv[i];
	for(i = 0;i < (sizeof line);++i) line[i] = 0;
	
	p = line;
	while((ret = read(0, p, MAXN)) > 0){
		p += ret;
		if(p - line >= MAXN){
			fprintf(2, "xargs: lines too long!\n");
			exit(1);
		}
	}
	if(ret < 0){
		fprintf(2, "Error occured when reading!\n");
		exit(1);
	}
	
	p = line;
	while(1){
		int pid, ret = parse_line(&p, _argv + argc - 1, _argv);
		if((pid = fork()) < 0){
			fprintf(2, "Fork error!\n");
			exit(1);
		}
		if(!pid)
			exec(_argv[0], _argv);
		else
			wait((int *)0);
		
		if(ret == E_INPUT) exit(0);
	}
	
	exit(1); // Controls should never reach here!
}
