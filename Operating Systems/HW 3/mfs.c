/*

	Name: Deep Patel
	ID: 1001354110

*/



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

//The variables are used for the FAT32 Layout
char BS_OEMName[8];
int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;
int32_t RootClusAddress;
int32_t Current_Directory; //Stores our current working directory address
int32_t Parent_Directory; //Stores the previous/parent directory

//The below 2 structs are used to represent each record
struct __attribute__((__packed__)) DirectoryEntry{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

struct DirectoryEntry dir[16];

FILE *fp; //File pointer for the FAT32 we open
char open_file[1000] = ""; //This stores the currently open file name


#define WHITESPACE " \t\n" //Used to split command line into tokens

#define MAX_COMMAND_SIZE 255 //The max command size is 255

#define MAX_NUM_ARGUMENTS 5 //Up to 5 arguments can be entered, really we only need up to 4 though


//Takes a logical block address, finds the first FAT, and returns the logical block address of the block in the file. Is theres no more blocks, it will return -1.
int16_t NextLB ( uint32_t sector)
{
    uint32_t FATAddress = ( BPB_BytsPerSec * BPB_RsvdSecCnt ) + ( sector * 4 );
    int16_t val;
    fseek( fp, FATAddress, SEEK_SET );
    fread ( &val , 2, 1, fp );
    return val;
}



//Takes the current sector number that points to a block of data and returns the adress value for the block of data.
int LBAToOffset(int32_t sector)
{
    return (( sector - 2 ) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

void Directories(int32_t addressing)
{
        fseek(fp, addressing, SEEK_SET); //Moves the function pointer to current directory start.
        int i;
        for (i = 0; i < 16; i++)
            {
                fread(&dir[i], sizeof( struct DirectoryEntry ), 1, fp);
            }

        for (i = 0; i < 16; i++)
            {
                char name[12];
                memcpy(name, dir[i].DIR_Name, 11);
                name[11] = '\0';
                printf("%s is in cluster low %d\n", name, dir[i].DIR_FirstClusterLow);
            }
}


int main()
{

    char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE ); //This stores the commands given by the user.

    while(1==1)
    {
         printf ("mfs> "); //Print mfs prompt to show program is ready for input

         while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) ); //Read all text in commandline
         char *token[MAX_NUM_ARGUMENTS]; //Parse Input
         int token_count = 0;
         char *arg_ptr; //Pointer to token, parsed by strsep
         char *working_str = strdup(cmd_str);
         char *working_root = working_str; //Move the working_str pointer to remember its original value for deallocation
         while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && (token_count<MAX_NUM_ARGUMENTS))  //Tokenize the input string. Whitespace is the delimiter.
                {
                token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
                if( strlen( token[token_count] ) == 0 )
                    {
                    token[token_count] = NULL;
                    }
                token_count++;
                }

        if(token[0] == NULL) //User entered a space so we ignore
		{

		}

        else if(strcmp(token[0], "open") == 0) //Open faile and update all our variables with its information
        {
        fp = fopen(token[1], "r"); //Opens img file. fat32.img or whatever it is the user enters.
            if(fp == NULL)
            {
             printf("Error: File system image not found.\n");
            }
            else if(strcmp(token[1], open_file) == 0)
            {
             printf("Error: File system image already open.\n");
            }
            else
            {
        strcpy(open_file, token[1]);
        //Reading Bytes Per Sector
        fseek(fp, 11, SEEK_SET);
        fread(&BPB_BytsPerSec, 2, 1, fp);

        //Reading Sectors Per Cluster
        fseek(fp, 13, SEEK_SET);
        fread(&BPB_SecPerClus, 1, 1, fp);

        //Reading Reserved Sector Count
        fseek(fp, 14, SEEK_SET);
        fread(&BPB_RsvdSecCnt, 2, 1, fp);

        //Reading the count of FAT data structures
        fseek(fp, 16, SEEK_SET);
        fread(&BPB_NumFATs, 1, 1, fp);

        //Reading the count of FAT32 32-bit sectors that are occupied with one FAT.
        fseek(fp, 36, SEEK_SET);
        fread(&BPB_FATSz32, 4, 1, fp);

        //Reading BPB_RootEntCnt, which should be 0 for FAT32
        fseek(fp, 17, SEEK_SET);
        fread(&BPB_RootEntCnt, 2, 1, fp);

        //Reading BPB_RootClus, which is set to the cluster number of the first cluster of the root directory
     //   fseek(fp, 44, SEEK_SET);
       // fread(&BPB_RootClus, 4, 1, fp);

        //Reading BS_Vollab, which is the volume label
        fseek(fp, 71, SEEK_SET);
        fread(&BS_VolLab, 11, 1, fp);

        //Calculate the address of the root directory
        RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);

        Current_Directory = RootClusAddress; //Set our working directory to the root.
        Parent_Directory = RootClusAddress; //Set our parent directory to the root.
            }
        }

        else if(strcmp(token[0], "info") == 0) //Print the relevant information about the file
        {
            //Printing Bytes Per Sector
            printf("BPB_BytesPerSec: %d, %x\n", BPB_BytsPerSec, BPB_BytsPerSec);

            //Printing Sectors Per Cluster
            printf("BPB_SecPerClus: %d %x\n", BPB_SecPerClus, BPB_SecPerClus);

            //Printing Reserved Sector Count
            printf("BPB_RsvdSecCnt: %d %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);

            //Printing the count of FAT data structures
            printf("BPB_NumFATs: %d %x\n", BPB_NumFATs, BPB_NumFATs);

            //Printing the count of FAT32 32-bit sectors that are occupied with one FAT.
            printf("BPB_FATSz32: %d %x\n", BPB_FATSz32, BPB_FATSz32);
            }


        else if(strcmp(token[0], "close") == 0) //Close the file
        {
            if(strcmp(open_file, "") == 0) //Check to see if theres an open file
                printf("Error: File system image must be opened first.\n");   //print error is there is no file open
            else
            {
            strcpy(open_file, ""); //Make open file empty so that we know no files are open anymore
            fclose(fp); //Close the file
            }
        }

        else if(strcmp(token[0], "ls") == 0)
        {
                if(token[1] == NULL)
                Directories(Current_Directory);
                else if(strcmp(token[1], ".") == 0)
                Directories(Current_Directory);
                else if(strcmp(token[1], "..") == 0)
                Directories(Parent_Directory);
        }

        else if(strcmp(token[0], "volume") == 0) //Prints the volume name of the file system image.
        {
            if(strcmp(BS_VolLab, "          ") == 0)
            printf("Error: volume name not found.");

            printf("Volume name: %s\n", BS_VolLab); //Print out the volume name found when we opned the file
        }

        else if(strcmp(token[0], "stat") == 0)
        {
            int stat_check = 0; //When stat check is 0, this means no file was found.
            if(strstr(token[1], ".") != NULL) //Make sure input contains a . so we know its a filename and not a directory
            {
                //Step 1, take the users string and convert it into FAT32's format
                char names[12]; //Will be used to hold our properly formatted filename
                //char stat_file = token[1]; //Is a middle man string we will use
                char *token2; //Our token pointer for when we split the string
                token2 = strtok(token[1], "."); //filename string is split in two by the .

                memset(names, ' ', 12); //fill name with spaces
                memcpy(names, token2, strlen(token2));

                token2 = strtok(NULL, "."); //Get the rest of the filename
                memcpy(names+8, token2, strlen(token2));



                char *j = names;        //Using char pointer j, we can interate and convert all lowercase letters to uppercase ones.
                while(*j)
                {
                    *j = toupper((unsigned char) *j);
                    j++;
                }
                names[11] = '\0';



                //Now compare filenames until we find the correct one, then print its attributes
                int k;


                fseek(fp, Current_Directory, SEEK_SET); //Moves the function pointer to current directory start.
                for (k = 0; k < 16; k++)
                {
                    fread(&dir[k], sizeof( struct DirectoryEntry ), 1, fp);
                }


                for (k = 0; k < 16; k++)
                {
                    char named[12];
                    memcpy(named, dir[k].DIR_Name, 11);
                    named[11] = '\0';
                    //printf("%s is in cluster low %d\n", named, dir[k].DIR_FirstClusterLow);

                    if(strcmp(named, names) == 0) //User entered filename is same as one in current directory, print information
                    {
                    stat_check = 1; //File found, set stat check to 1.
                    printf("Filename: %s\n", named);
                    printf("Attributes: %d\n", dir[k].DIR_Attr);
                    printf("Unused1: %s\n", dir[k].Unused1);
                    printf("First Cluster High: %d\n", dir[k].DIR_FirstClusterHigh);
                    printf("Unused2: %s\n", dir[k].Unused2);
                    printf("First Cluster Low: %d\n", dir[k].DIR_FirstClusterLow);
                    printf("Filesize: %d\n", dir[k].DIR_FileSize);
                    }


                }
            }
            else if(strstr(token[1], ".") == NULL)  //This means input is a directory name
            {
                                //Step 1, take the users string and convert it into FAT32's format
                char names[12]; //Will be used to hold our properly formatted filename
                //char stat_file = token[1]; //Is a middle man string we will use
                            //char *token2; //Our token pointer for when we split the string
                            //token2 = strtok(token[1], "."); //filename string is split in two by the .

                memset(names, ' ', 12); //fill name with spaces
                memcpy(names, token[1], strlen(token[1]));

                            //token2 = strtok(NULL, "."); //Get the rest of the filename
                            //memcpy(names+8, token2, strlen(token2));



                char *j = names;        //Using char pointer j, we can interate and convert all lowercase letters to uppercase ones.
                while(*j)
                {
                    *j = toupper((unsigned char) *j);
                    j++;
                }
                names[11] = '\0';



                //Now compare filenames until we find the correct one, then print its attributes
                int k;


                fseek(fp, Current_Directory, SEEK_SET); //Moves the function pointer to current directory start.
                for (k = 0; k < 16; k++)
                {
                    fread(&dir[k], sizeof( struct DirectoryEntry ), 1, fp);
                }


                for (k = 0; k < 16; k++)
                {
                    char named[12];
                    memcpy(named, dir[k].DIR_Name, 11);
                    named[11] = '\0';
                    //printf("%s is in cluster low %d\n", named, dir[k].DIR_FirstClusterLow);

                    if(strcmp(named, names) == 0) //User entered directory is same as one in current directory, print information
                    {
                    stat_check = 1; //File found, set stat check to 1.
                    printf("Filename: %s\n", named);
                    printf("Attributes: %d\n", dir[k].DIR_Attr);
                    printf("Unused1: %s\n", dir[k].Unused1);
                    printf("First Cluster High: %d\n", dir[k].DIR_FirstClusterHigh);
                    printf("Unused2: %s\n", dir[k].Unused2);
                    printf("First Cluster Low: %d\n", dir[k].DIR_FirstClusterLow);
                    printf("Filesize: %d\n", dir[k].DIR_FileSize);
                    }


                }
            }

            if(stat_check == 0) //This means that we did not fine a file in our search
                printf("Error: File not found.\n"); //Print error for not finding file

        }

        else if(strcmp(token[0], "cd") == 0) //Works just like stat, but without output, and will change the current directory pointer to matching directory
        {
          /*  if(strcmp(token[1], "..") == 0) //Parent directory is requested
                {
                    Current_Directory = Parent_Directory; //Make current directory the parent one
                }
            else if(strstr(token[1], ".") != NULL) //Make sure input contains a . so we know its a filename and not a directory
            {
                //Step 1, take the users string and convert it into FAT32's format
                char names[12]; //Will be used to hold our properly formatted filename
                //char stat_file = token[1]; //Is a middle man string we will use
                char *token2; //Our token pointer for when we split the string
                token2 = strtok(token[1], "."); //filename string is split in two by the .

                memset(names, ' ', 12); //fill name with spaces
                memcpy(names, token2, strlen(token2));

                token2 = strtok(NULL, "."); //Get the rest of the filename
                memcpy(names+8, token2, strlen(token2));



                char *j = names;        //Using char pointer j, we can interate and convert all lowercase letters to uppercase ones.
                while(*j)
                {
                    *j = toupper((unsigned char) *j);
                    j++;
                }
                names[11] = '\0';



                //Now compare filenames until we find the correct one, then print its attributes
                int k;


                fseek(fp, Current_Directory, SEEK_SET); //Moves the function pointer to current directory start.
                for (k = 0; k < 16; k++)
                {
                    fread(&dir[k], sizeof( struct DirectoryEntry ), 1, fp);
                }


                for (k = 0; k < 16; k++)
                {
                    char named[12];
                    memcpy(named, dir[k].DIR_Name, 11);
                    named[11] = '\0';
                    //printf("%s is in cluster low %d\n", named, dir[k].DIR_FirstClusterLow);

                    if(strcmp(named, names) == 0) //User entered filename is same as one in current directory, print information
                    {
                    Parent_Directory = Current_Directory; //Make current directory the parent one
                    Current_Directory = LBAToOffset(dir[k].DIR_FirstClusterLow); //Set current to the requested current directory
                    }


                }
            } */
          //  else if(strstr(token[1], ".") == NULL)  //This means input is a directory name
           // {
                                //Step 1, take the users string and convert it into FAT32's format
                char names[12]; //Will be used to hold our properly formatted filename
                //char stat_file = token[1]; //Is a middle man string we will use
                            //char *token2; //Our token pointer for when we split the string
                            //token2 = strtok(token[1], "."); //filename string is split in two by the .

                memset(names, ' ', 12); //fill name with spaces
                memcpy(names, token[1], strlen(token[1]));

                            //token2 = strtok(NULL, "."); //Get the rest of the filename
                            //memcpy(names+8, token2, strlen(token2));



                char *j = names;        //Using char pointer j, we can interate and convert all lowercase letters to uppercase ones.
                while(*j)
                {
                    *j = toupper((unsigned char) *j);
                    j++;
                }
                names[11] = '\0';



                //Now compare filenames until we find the correct one, then print its attributes
                int k;


                fseek(fp, Current_Directory, SEEK_SET); //Moves the function pointer to current directory start.
                for (k = 0; k < 16; k++)
                {
                    fread(&dir[k], sizeof( struct DirectoryEntry ), 1, fp);
                }


                for (k = 0; k < 16; k++)
                {
                    char named[12];
                    memcpy(named, dir[k].DIR_Name, 11);
                    named[11] = '\0';
                    //printf("%s is in cluster low %d\n", named, dir[k].DIR_FirstClusterLow);
                    if(strcmp(token[1], ".") == 0) //"." is the first item in a directory list. so dir[0]
                    {
                    Parent_Directory = Current_Directory; //Move to current directory
                    Current_Directory = LBAToOffset(dir[0].DIR_FirstClusterLow); //Set current to the requested current directory
                    }
                    if(strcmp(token[1], "..") == 0) //"." is the 2nd item in a directory list. so dir[1]
                    {
                        if(0 == dir[0].DIR_FirstClusterLow) //The parent is also the root, DO NOT OFFSET
                            {
                                Parent_Directory = Current_Directory;
                                Current_Directory = RootClusAddress;
                            }
                        else  //The parent is not the root, offset
                            {
                                Parent_Directory = Current_Directory; //Move to parent directory
                                Current_Directory = LBAToOffset(dir[1].DIR_FirstClusterLow); //Set current to the requested current directory
                            }

                    }
                    else if(strcmp(named, names) == 0) //User entered directory is same as one in current directory, print information
                    {
                    Parent_Directory = Current_Directory; //Make current directory the parent one
                    Current_Directory = LBAToOffset(dir[k].DIR_FirstClusterLow); //Set current to the requested current directory
                    }
                }

            //}
        }






         free( working_root );
    }




    return 0;
}