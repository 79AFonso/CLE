#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int check4morethan2bytes(char *c1, char *c2)
{
    for (int i=0; i<4; i++){
        if (c1[i] != c2[i])
            return 0;
    }

    return 1;
}



int isVowel(const char *c){

    char c_lower[4] = {0xC3, 0xA7, 0x00, 0x00 };
    char c_upper[4] = {0xC3, 0x87, 0x00, 0x00 };

    if(check4morethan2bytes(c, c_lower) || check4morethan2bytes(c, c_upper)  ) return 0;

    const char str[] = "aáàãâAÁÀÃÂeéèêEÉÈÊiíìIÍÌoóòõôOÓÒÕÔuúùUÚÙ";
    const char ch = *c;
    char *ret;

    ret = strchr(str, ch);

    // printf("String after |%c| is - |%s|\n", ch, ret);
    
    
    if (ret==NULL) return 0;

    return 1;
		
}

int isSeparationSymbol(char *c){ 
    char en_dash[4] = {0xE2, 0x80, 0x93, 0x00};
    char ellipsis[4] =  {0xE2, 0x80, 0xA6, 0x00};
    char quot1[4] = {0xE2, 0x80, 0x9C, 0x00};
    char quot2[4] = {0xE2, 0x80, 0x9D, 0x00};
    return *c == 0x2D || *c == 0x22|| *c == 0x20 || *c == 0x9 || *c == 0xA || *c == 0xD || *c == '.' || *c == '?' || *c == ';' || *c == ':' || *c == ',' || *c == '!'|| check4morethan2bytes(c, en_dash) || check4morethan2bytes(c, ellipsis) || *c=='[' || *c==']' || *c=='(' || *c==')' || check4morethan2bytes(c, quot1) || check4morethan2bytes(c, quot2);
}

int isDigCharUnd(char *c){
    if (isalpha(*c) || isdigit(*c)) return 1;
    const char str[] = "áàãâÁÀÃÂéèêÉÈÊíìIÍÌóòõôÓÒÕÔúùUÚÙçÇ_";
    const char ch = *c;
    char *ret;

    ret = strchr(str, ch);
    
    if (ret==NULL) return 0;

    return 1;
}

int isPTChar(char *c){
    if (isalpha(*c)) return 1;
    const char str[] = "áàãâÁÀÃÂéèêÉÈÊíìIÍÌóòõôÓÒÕÔúùUÚÙ";
    const char ch = *c;
    char *ret;

    ret = strchr(str, ch);
    
    if (ret==NULL) return 0;

    return 1;
}

void readUTFchar(FILE* f, char* fname){
    int count=0, countVowels=0, countConsonants=0;
    int bool_check_word = 0;
    char last_char [20];
    while(1){
        char * buffer = (char*) malloc (sizeof(char)*4);
        int n = fread (buffer,1,1,f);
        if (n==0) break;

        if (buffer[0]>=-64 && buffer[0]<-32) fread (buffer+1,1,1,f);
        else if (buffer[0]>=-32 && buffer[0]<-16) fread (buffer+1,1,2,f);
        else if (buffer[0]>=-16 && buffer[0]<-8) fread(buffer+1,1,3,f);

        if (bool_check_word == 0){

            if(isDigCharUnd(buffer)){
                bool_check_word = 1;
                strcpy(last_char, buffer);
                count++;
                // printf("%s : %d\n",buffer, isVowel(buffer));
                if (isVowel(buffer)){
                    countVowels++;
                }
            }
            
        }
        else {
            

            if (isDigCharUnd(buffer) || (*buffer== '\'' || *buffer == '’' || *buffer == '‘')){
                strcpy(last_char, buffer);
            }
            else if (isSeparationSymbol(buffer)){
                if (!isVowel(last_char) && !(*last_char == '_') && !(*last_char== '\'' || *last_char == '’' || *last_char == '‘') && !isdigit(*last_char)){
                    countConsonants++;
                }
                bool_check_word = 0;
            }
        }

        

        free(buffer);
    }
    printf("File : %s\n",fname);
    printf("Words : %d\n", count);
    printf("Words starting with Vowel : %d\n", countVowels);
    printf("Words ending in Consonant : %d\n", countConsonants);
    printf("*******************************\n");
}


int main(int argc, char **argv) { 

    clock_t t;
    t = clock();

    for (int i=1; i<argc; i++){
        FILE * f;
        f = fopen(argv[i], "rb");

        readUTFchar(f, argv[i]);
        fclose (f);
    }

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("Elapsed time %f s\n", time_taken);
    
    return 0;

}

