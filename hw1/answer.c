#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
 int main(int argc,char* argv[]){
     if(argc!=2 && argc!=3){
         perror("ERROR: This file should not be empty!");
         return EXIT_FAILURE;
     }
     char** words = NULL;
     int end=0;
     int t=16;
     int i=0;
     int x=0;
     int count=0;
     int count2=0;
     int num=0;
     char word[100];
     words = calloc( t, sizeof(char*) );
     
     int* n = NULL;
     n = calloc( t,sizeof(int) );
     
     DIR * dir = opendir( argv[1] );
     
     
     if ( dir == NULL )
  {
    perror( "ERROR:" );
    return EXIT_FAILURE;
  }
     int c=chdir(argv[1]);
     if(c != 0){
         perror("ERROR:");
         return EXIT_FAILURE;
     }
    struct dirent * file;
    printf("Allocated initial parallel arrays of size 16.\n");
    while ( ( file = readdir( dir ) ) != NULL )
  {

    struct stat buf;
    int rc = lstat( file->d_name, &buf );  /* e.g., "xyz.txt" */
     /* ==> "assignments/xyz.txt" */
    if ( rc == -1 )
    {
      perror( "lstat() failed" );
      return EXIT_FAILURE;
    }

    //printf( " (%d bytes)\n", (int)buf.st_size );

    if ( S_ISREG( buf.st_mode ) )
    {
      FILE *fp=fopen(file->d_name,"r");
      int ch;
      size_t idx ;
      end=0;
      while(1){
          word[0] = 0;
          for (idx=0; idx < sizeof word -1; ) {
              ch = fgetc(fp);
              if (ch == EOF){ 
                  end=1;
                  break;
              }
              if (!isalpha(ch) && !isdigit(ch)) {
                  if (idx==0) {
                  continue;
                  }
                  else if(idx==1){
                      word[0]=0;
                      idx=0;
                      continue;
                  }// Nothing read yet; skip this character
                  else {
                      break;
                     } // we are beyond the current word
                  }
                  word[idx++] = ch;
              
          }
          if(end==1){
                break;
            }
          word[idx] = '\0';
          num++;
            if(count<t){
                for(i=0;i<count;i++){
                    if(strcmp(words[i],word)==0){
                        n[i]++;
                        x=1;
                        break;
                    }
                    
                }
                if(x!=1){
                   words[count]=strdup(word);
                   count++;
                   n[count2] = 1;
                   count2++;
                }
                x = 0;
            }
            else{
                t=t+16;
                words=realloc(words,t*sizeof(char*));
                n=realloc(n,t*sizeof(int));
                
                printf("Re-allocated parallel arrays to be size %d.\n",t);
                for(i=0;i<count;i++){
                    if(strcmp(words[i],word)==0){
                        n[i]++;
                        x=1;
                        break;
                    }
                    
                }
                if(x!=1){
                   words[count]=strdup(word);
                   count++;
                   n[count2] = 1;
                   count2++;
                }
                x = 0;
            }
      }
      fclose(fp);
    }

    /* TO DO: determine whether each file is executable ...

                man lstat      */

  }
  printf("All done (successfully read %d words; %d unique words).\n",num,count);
  closedir( dir );
  if(argc!=3){
      printf("All words (and corresponding counts) are:\n");
      for(i=0;i<count;i++){
        printf("%s -- %d\n",words[i],n[i]);
      }
  }
  else{
      int nums=atoi(argv[2]);
    printf("First %d words (and corresponding counts) are:\n",nums);
    for(i=0;i<nums;i++){
      printf("%s -- %d\n",words[i],n[i]);
  }  
  }
  for(i=0;i<count;i++){
      free(words[i]);
  }
  free(words);
  free(n);
  
 return EXIT_SUCCESS;
 }