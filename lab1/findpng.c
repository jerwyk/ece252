#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../com/png.h"

/*recursively search all entries in directory and subdirectory
  print file name if it is an png*/
int find_dir_r(char* dir);

int main(int argc, char *argv[]) 
{
    if(argc == 2)
    {
        find_dir_r(argv[1]);
    }
    else
    {
        printf("Invalid arguments\n");
    }
    
}

int find_dir_r(char* dir)
{
    DIR *p_dir;

    /*invalid directory*/
    if((p_dir = opendir(dir)) == NULL)
    {
        printf("Invalid directory\n");
        exit(2);
    }

    struct dirent *p_dirent;
    while ((p_dirent = readdir(p_dir)) != NULL) 
    {
        char full_path[256];
        char *str_path = p_dirent->d_name;  /* relative path name! */
        snprintf(full_path, sizeof full_path, "%s/%s", dir, str_path);
    
        if (str_path == NULL) 
        {
            fprintf(stderr,"Null pointer found!"); 
            exit(3);
        } 
        else if(strcmp(str_path, "..") && strcmp(str_path, "."))/*ignore parent and current directory*/
        {
            struct stat buf;
            
            if (stat(full_path, &buf) < 0) 
            {
                printf("%s\n", full_path);
                perror("stat error");
                continue;
            }
            /*regular file*/
            if(S_ISREG(buf.st_mode))
            {
                /*read file and check if it is png*/
                FILE *fptr;
                if ((fptr = fopen(full_path,"r")) != NULL)
                {
                    uint8_t sig[PNG_SIG_SIZE];
                    fread(&sig, sizeof(sig), 1, fptr);
                    if(is_png(sig, PNG_SIG_SIZE))
                    { 
                        printf("%s\n", full_path);
                    }
                }
            }
            else if(S_ISDIR(buf.st_mode)) /*is a directory*/
            {
                /*recursively check directory*/
                find_dir_r(full_path);
            }
        }
    }

    closedir(p_dir);
    return 0;
}