#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int femtoshell_main(int argc, char *argv[]) {
    
        int size = 10240;
        char buf[size];
        bool done = 1;
        int last =0;
        while(1)
        {
                if(done)
                    printf("Femto shell prompt > ");

                if (fgets(buf,sizeof(buf),stdin) == NULL )
                {
                        printf("\n");
                        return last;
                }
                size_t len = strcspn(buf,"\n");
                buf[len] = 0;
                if(len == 0)
                {
                    printf("\n");
                    continue;
                }
                
                const char *x = "exit";
                int len3 = strlen(x);
                const char *e = "echo";
                int len2 = strlen(e);
                if(strncmp(buf,x,len3) == 0)
                {
                        printf("\nGood Bye\n");
                        return 0;
                }
                else if((strncmp(buf,e,len2) == 0) && (buf[len2]==' ' || buf[len2]=='\0'))
                {
                        if(!done) done = 1;
                        char *st = buf + len2;
                        if(*st == ' ')
                        {
                                st++;
                        }
                        if(strlen(st) < 2)
                        {
                            done = 0;
                        }
                        printf("\n%s\n",st?st:"");
                       last =0;
                }
                else
                {
                        printf("\nInvalid command\n");
                        last =-1;
                }
        }
        return last;


}