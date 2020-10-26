#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int prime, mark = 0;
static int pipe_in = -1, pipe_out = -1;
static int pid, ret;
static int p[2];
static char buf[100];

void
sift_down(int base){
	 pipe(p);
	 if((pid = fork()) < 0){
	 	printf("Fork error!\n");
	 	exit(1);
	 }
	 
	 if(!pid){
	 	printf("prime %d\n", base);
	 	
	 	close(pipe_in);
	 	close(p[1]);
	 	pipe_out = -1;
	 	pipe_in = p[0];
	 	prime = base;
	 	mark = 0;
	 	return;
	 }
	 else{
	 	close(p[0]);
	 	pipe_out = p[1];
	 	mark = 1;
	 	return;
	 }
}

int
main(int argc, char *argv[])
{ 
	close(0);
	close(2);
	
  if(pipe(p) < 0){
  	printf("Pipe error!\n");
  	exit(1);
  }
  
  if((pid = fork()) < 0){
  	printf("Fork error!\n");
  	exit(1);
  }
  
  /* Parent */
  if(pid > 0){
  	int feed = 3;
  	
  	close(p[0]);
  	pipe_out = p[1];
  	
  	for(; feed <= 35;++feed){
  		buf[0] = feed;
  		if(write(pipe_out, buf, 1) != 1){
  			printf("primes: pipe error when writing!\n");
  			exit(1);
  		}
  	}
  	
  	close(pipe_out);
  	wait((int *) 0);
  	exit(0);
  }
  
  /* Child */
  close(p[1]);
  
  pipe_in = p[0];
  prime = 2;
  
  printf("prime 2\n");
  
  while((ret = read(pipe_in, buf, 1)) > 0){
  	int tmp = buf[0];
  	if(tmp % prime){
  		if(!mark)
  			sift_down(tmp);
  		else if(write(pipe_out, buf, 1) != 1){
  			printf("primes: pipe error when writing!\n");
  			exit(1);
  		}
  	}
  }
  
  if(ret < 0){
  	printf("primes: pipe error when reading!\n");
  	exit(1);
  }
  close(pipe_in);
  if(pipe_out > 0){
  	close(pipe_out);
  	wait((int *)0);
  }
  exit(0);
}
