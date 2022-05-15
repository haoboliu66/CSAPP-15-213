// C program to compare two files and report
// mismatches by displaying line number and
// position of line.
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
  
void compareFiles(FILE *fp1, FILE *fp2)
{
    // fetching character of two file
    // in two variable ch1 and ch2
    char ch1 = getc(fp1);
    char ch2 = getc(fp2);
  
    // error keeps track of number of errors
    // pos keeps track of position of errors
    // line keeps track of error line
    int error = 0, pos = 0, line = 1;
  
    // iterate loop till end of file
    while (ch1 != EOF && ch2 != EOF)
    {
        pos++;
  
        // if both variable encounters new
        // line then line variable is incremented
        // and pos variable is set to 0
        if (ch1 == '\n' && ch2 == '\n')
        {
            line++;
            pos = 0;
        }
  
        // if fetched data is not equal then
        // error is incremented
        if (ch1 != ch2)
        {
            error++;
            printf("Line Number : %d \tError"
               " Position : %d \n", line, pos);
        }
  
        // fetching character until end of file
        ch1 = getc(fp1);
        ch2 = getc(fp2);
    }
  
    printf("Total Errors : %d\n", error);
}
  
// Driver code
int main(int argc, char **argv){
	if(argc < 2){
		printf("need 2 files path\n");
		return 0;
	}

    // opening both file in read only mode
    FILE *fp1 = fopen(argv[1], "r");
    FILE *fp2 = fopen(argv[2], "r");
  
    if (fp1 == NULL || fp2 == NULL)
    {
       printf("Error : Files not open");
       exit(0);
    }
  printf("comparing [ %s ], [ %s ]\n", argv[1], argv[2]);
    compareFiles(fp1, fp2);
  
    // closing both file
    fclose(fp1);
    fclose(fp2);
    return 0;
}
