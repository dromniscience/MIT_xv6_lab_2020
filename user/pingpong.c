#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define	LEN	50

int
main(int argc, char *argv[])
{
	int p1[2], p2[2];
	int pid;
	char buf[LEN + 1] = {'x', 'v', '6'};
	
  if(pipe(p1) < 0) printf("pipe error!\n");
  if(pipe(p2) < 0) printf("pipe error!\n");
  
  if((pid = fork()) == 0){
  	close(p1[1]);
  	close(p2[0]);
  	
  	if(read(p1[0], buf, 1) <= 0){
  		printf("Error occured when reading from a pipe.\n");
  		exit(1);
  	}
  	else printf("%d: received ping\n", getpid());
  	
  	if(write(p2[1], buf, 1) != 1){
  		printf("Error occured when writing to a pipe.\n");
  		exit(1);
  	}
  	
  	exit(0);
  }
  else if(pid > 0){
  	close(p1[0]);
  	close(p2[1]);
  	
  	if(write(p1[1], buf, 1) != 1){
  		printf("Error occured when writing to a pipe.\n");
  		exit(1);
  	}
  	
  	if(read(p2[0], buf, 1) <= 0){
  		printf("Error occured when reading from a pipe.\n");
  		exit(1);
  	}
  	else printf("%d: received pong\n", getpid());
  	
  	wait((int *)0);
  	exit(0);
  }
  else {
  	printf("Fork error!\n");
  	exit(1);
  }
}
