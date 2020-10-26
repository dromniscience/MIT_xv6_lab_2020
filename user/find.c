#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
read_dir(int fd, char * const path, const char * const file)
{
	char buf[512], *p = path + strlen(path);
	struct dirent de;
	struct stat st;
	
	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		char *tmp = p;
		int tmpfd;
		
		/* Filter out distractions */
		if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0 || !strlen(de.name))
			continue;
		
		/* Bound check */
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      fprintf(2, "find: path too long\n");
      break;
    }
    
    /* In case that path is "/" */
    if(*(tmp - 1) != '/'){
    	*tmp = '/';
    	tmp++;
    }
    
    memmove(tmp, de.name, DIRSIZ);
    tmp[DIRSIZ] = 0; // End mark
    
    /* Once successfully opened, close it before leaving */
    if((tmpfd = open(path, 0)) < 0){
    	fprintf(2, "find: cannot open %s\n", path);
    }
    else if(fstat(tmpfd, &st) < 0){
    	fprintf(2, "find: cannot stat %s\n", path);
    	close(tmpfd);
    }
    else if(st.type != T_DIR){
    	if(strcmp(de.name, file) == 0)
    		printf("%s\n", path);
    	close(tmpfd);
    }
    else {
    	read_dir(tmpfd, path, file);
    	close(tmpfd);
    }
  }
  
  /* Restore path */
  *p = 0;
  return;
}

void
find(char * const path, const char *file)
{
  int fd;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
  
  if(st.type != T_DIR){
  	fprintf(2, "find: cannot search into %s. Not a directory.\n", path);
  	close(fd);
  	return;
  }
  
  read_dir(fd, path, file);
  close(fd);
  return;
}


int
main(int argc, char *argv[])
{
	char path[512];
	char file[512];
	int i = 1;
	
	for(; i < argc;i++)
		if(strlen(argv[i]) >= sizeof path){
			fprintf(2, "find: Input file is too long!\n");
			exit(1);
		}
	
	if(argc < 2 || argc > 3){
    fprintf(2, "Usage: find <dir> filename\n");
    exit(1);
	}
  else if(argc == 2){
    path[0] = '.';
    path[1] = 0;
    memmove(file, argv[1], strlen(argv[1]) + 1);   
    find(path, file);
  }
  else{
  	memmove(path, argv[1], strlen(argv[1]) + 1);
  	memmove(file, argv[2], strlen(argv[2]) + 1);
  	find(path, file);
	}
  exit(0);
}
