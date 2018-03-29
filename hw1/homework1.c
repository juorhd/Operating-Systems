#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>



void insert_word( char* s, char*** words, int** num, int* size, int* total_word, int* count )
{		
	if ( strlen(s) <= 1 ) return;
	
	(*count)++;
		
	int found = 0;
	
	for ( int i = 0; i < *total_word; ++i )  //check repetition
	{
		if ( !strcmp( s, words[0][i] ) )
		{
			num[0][i]++;
			found = 1;
			break;
		}
	}
	if ( !found )  //new word
	{    
		if ( (*total_word)+1 > (*size) ) //reallocate new space in heap
		{
			(*size) += 32;
			words[0] = realloc( words[0], (*size) * sizeof( char* ) );
			num[0] = realloc( num[0], (*size) * sizeof(int) );
			printf( "Re-allocated parallel arrays to be size %d.\n", *size );
			
		}
		words[0][ (*total_word) ] = calloc( 81, sizeof(char) );
		strcpy( words[0][ (*total_word) ], s );
		num[0][ *total_word ] = 1;
		(*total_word)++;		
	}
}

int main(int argc, char *argv[]) {
	if ( argc < 2 || argc > 3)
	{
		fprintf( stderr, "ERROR: Invalid arguments\n" );
		return EXIT_FAILURE;
	}
	
	DIR * dir = opendir( argv[1] );
	
	if ( dir == NULL )
	{
		perror( "ERROR: opendir() failed" );
		return EXIT_FAILURE;
	}
	
	int c_dir = chdir( argv[1] );
	if( c_dir != 0 )
	{
		perror("ERROR: chdir() failed");
		return EXIT_FAILURE;
	}
	
	//initialize pointer
	struct dirent * file;
	int** num = calloc( 1, sizeof(int*) );
	num[0] = calloc( 32, sizeof(int) );
	char*** words = calloc( 1, sizeof(char**) );
	words[0] = calloc( 32, sizeof(char*));
	int* size = malloc( sizeof(int) );
	int* total_word = malloc( sizeof(int) );
	int* count = malloc( sizeof(int) );
	
	*size = 32;
	*total_word = 0;
	*count = 0;
	
	printf("Allocated initial parallel arrays of size 32.\n");

	while ( ( file = readdir( dir ) ) != NULL )
	{
		if ( !strcmp(file->d_name,".") || !strcmp(file->d_name,"..")) continue;
		//printf( "found %s\n", file->d_name );   /* (*file).d_name */

		struct stat buf;
		int rc = lstat( file->d_name, &buf );

		if ( rc == -1 )
		{
			perror( "ERROR: lstat() failed" );
			return EXIT_FAILURE;
		}

		if ( S_ISREG( buf.st_mode ) )
		{
			long int f_size = buf.st_size;
			char* text = calloc( f_size+1, sizeof(char) );
						
			FILE *fp = fopen(file->d_name, "r");
			if(fp == NULL) 
			{
				perror("ERROR: fail to open file");
				return EXIT_FAILURE;
			}
			
			//change non-alphanumerical word to space
			fread( text, f_size, 1, fp );
			for ( int i = 0; i<f_size; ++i )	
				if ( !isalnum(text[i]) )
					text[i] = ' ';
			text[f_size] = '\0';
			
		
			for (char *p = strtok(text," "); p != NULL; p = strtok(NULL, " "))
			{
				#if DEBUG
					printf("%s  ", p);
				#endif
				
				insert_word(p, words, num, size, total_word, count);
			}
			
			free(text);
			fclose(fp);
		}
		
	} 
	closedir( dir );


//print
	printf("All done (successfully read %d words; %d unique words).\n", *count, *total_word);

		
	if( argc != 3 || atoi(argv[2]) >= *total_word || 2*atoi(argv[2]) > *total_word ){
		printf("All words (and corresponding counts) are:\n");
		for(int i=0;i<*total_word;i++)
			printf("%s -- %d\n",words[0][i],num[0][i]);
	} else{
		int nums = atoi(argv[2]);
		printf("First %d words (and corresponding counts) are:\n",nums);
		for(int i = 0;i < nums; i++)
			printf("%s -- %d\n", words[0][i], num[0][i]);
		
		printf("Last %d words (and corresponding counts) are:\n",nums);
		for(int i = *total_word-nums;i < *total_word; i++)
			printf("%s -- %d\n", words[0][i], num[0][i]);
	}  

//free of memories
	for( int i = 0; i < *total_word; i++){ free(words[0][i]); }
	free(words[0]); free(num[0]); 
	free(words); free(num);
	free(size); free(count);
	free(total_word);
	return EXIT_SUCCESS;
}