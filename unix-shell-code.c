#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include <string.h>

//Function that parses out unnecessary characters
//such as ' ', and '\t'
char** parse(char* s) {
  static char* words[500];
  memset(words, 0, sizeof(words));

  char break_chars[] = " \t";

  int i = 0;
  char* p = strtok(s, break_chars);
  words[i++] = p;
  while (p != NULL) {
    p = strtok(NULL, break_chars);
    words[i++] = p;
  }
  return words;
}

int main(int argc, const char * argv[]) {  

  //Initializes two arrays: 
  //1. For the current input
  //2. To keep track of all inputs
  char input[BUFSIZ];
  char last_command[BUFSIZ];
  memset(input, 0, BUFSIZ * sizeof(char));
  memset(last_command, 0, BUFSIZ * sizeof(char));
  //Condition to keep the command line going
bool finished = false;
bool redirectoutput = false;
bool redirectinput = false;

  //While loop that runs the program
  while (!finished) {
    printf("osh> ");
    
    fflush(stdout);

    //gets input and stores it in the array
    //also get rid of the new line at the end
    fgets(input, BUFSIZ, stdin);
    input[strlen(input) - 1] = '\0';

    //If input is empty, it will give an error and pop up the command line until a input is not empty
    if (strcmp(input, "\0") == 0) {   
      fprintf(stderr, "no command entered\n\n");
    }
    //If input is exit, then it will end the program
    else if(strncmp(input, "exit", 4) == 0){
        return 0;
    }
    
    //this is the case where the input is not an empty command or
    //not "exit" to end the program

        //Scenario where "!!" or use previous command 
        if(strncmp(input, "!!", 2) == 0){
            //if there is no last command,it will give an error
            if(strlen(last_command) == 0){
                printf("There is no last command!\n");
            }
            else {
            //if there is a previous command it will, execute it again
                strcpy(input, last_command);
                printf("The last command was: %s\n\n", last_command);
            }
        }

        //scenario where it is an actual commmand and not "!!"
        if (strncmp(input, "!!", 2 ) != 0) {
            strcpy(last_command, input);
            char** words = parse(input);  

            //creates child
            pid_t pid = fork();
           //printf("\n Checking output\n");

            //if child wasn't created
            if(pid < 0){
                fprintf(stderr, "Unable to fork a child\n");
                return 0;
            }
            //parent process
            else if (pid > 0) {
                    wait(NULL);
            }
            //child process
            else if(pid == 0){   
                //printf("Child Process created...\n");

                //printf("\nFirst word: %s\n", words[0]);
                //printf("Second word: %s\n\n", words[1]);
                
                //regular commands with no redirection of output and input
                if (words[1] == '\0'){
                  execvp(words[0], words);
                  printf("\n\n");
                }

                //instance when cat "filename" is used
                if(words[2] == '\0'){
                  execvp(words[0], words);
                  printf("\n\n");
                }

                //redirects input from file and executes it
                //format :
                // < "filename" 
                if(strncmp(words[0], "<", 1) == 0){
                  printf("getting input from %s\n\n", words[1]);
                  char infromfile[BUFSIZ];
                  memset(infromfile,0, BUFSIZ * sizeof(char));
                  int fd_in = open(words[1], O_RDONLY);
                  dup2(fd_in, STDIN_FILENO);
                  fgets(infromfile, BUFSIZ, stdin);
                  char** inputcommands = parse(infromfile);

                  //printf("Input from File: %s %s\n", inputcommands[0], inputcommands[1]);
                  execvp(inputcommands[0], inputcommands);
                }
                
                // case for ls > text.txt
                if(strncmp(words[1], ">", 1) == 0){
                  printf("output will be stored in %s\n\n", words[2]);
                  int fd_out = open(words[2], O_WRONLY | O_TRUNC | O_CREAT);
                  char* out[BUFSIZ];
                  out[0] = words[0];
                  dup2(fd_out, STDOUT_FILENO);
                  execvp(out[0], out);
                  close(fd_out);
                  printf("\n\n");
                }

                // //case for cat "filename" > "outputfile"
                if(strncmp(words[2], ">", 1) == 0 && (strncmp(words[0], "cat", 3) == 0)){
                  printf("output will be stored in %s\n\n", words[3]);
                  int fd_out = open(words[3], O_WRONLY | O_TRUNC | O_CREAT);
                  dup2(fd_out, STDOUT_FILENO);
                  char* out[BUFSIZ];
                  out[0]= words[0];
                  out[1] = words[1];
                  execvp(out[0], out);
                  close(fd_out);
                }
            }
        }
   }
  }
