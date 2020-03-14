#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/wait.h> 
#include <fcntl.h>
#include <string.h>

void exec_remaining(char *commands[], int size){
    if(size == 0){
        exit(0);    
    }
    char *path='\0';char *arg;int argc = 0;
    char *current_command = commands[0];
    path = strtok(current_command, " ");
    arg = strtok(NULL, "");
    char *args[200];
    for(int i=0;i<200;++i)
        args[i] = NULL;
    char *arg_it = strtok(arg, " ");
    args[0] = path;
    ++argc;
    while(arg_it != NULL){
        char *current_arg = arg_it;
        args[argc] = current_arg;
        ++argc;
        arg_it = strtok(NULL, " ");
    }
    int pid;
    int fd[2];
    int ret;
    ret = pipe (fd);
    pid = fork ();
    if (pid == 0) {
        for(int i=0;i<argc;++i){
            if(strcmp(args[i],"<")==0){
                int fd_file = open(args[i+1], O_RDWR  , 0666);
                close(0);
                dup(fd_file);
                // close(fd_file);
                args[i] = '\0';
                continue;
            }
            if(args[i][0]=='1' && args[i][1]=='>'){
                char file_name[200] ;
                for(int j=2;j<strlen(args[i]);++j){
                    file_name[j-2] = args[i][j];
                }
                int fd_file = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, 0666 );
                close(1);
                dup(fd_file);
                close(fd_file);
                args[i] = '\0';
                continue;
            }
            if(strcmp(args[i], "2>&1")==0){
                close(2);
                dup(1);
                args[i] = '\0';
                continue;
            }
            if(args[i][0]=='2' && args[i][1]=='>'){
                char file_name[200] ;
                for(int j=2;j<strlen(args[i]);++j){
                    file_name[j-2] = args[i][j];
                }
                int fd_file = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, 0666 );
                close(2);
                dup(fd_file);
                close(fd_file);
                args[i] = '\0';
                continue;
            }
            
            if(strcmp(args[i], ">") == 0){
                int fd_file = open(args[i+1],O_WRONLY | O_CREAT | O_TRUNC, 0666);
                close(fd[0]);
                close(1);
                dup(fd_file);
                close(fd[1]);
                close(fd_file);
                args[i] = '\0';
                continue;
            }
            if(strcmp(args[i], ">>") == 0){
                int fd_file = open(args[i+1],O_WRONLY | O_CREAT | O_APPEND, 0666);
                close(1);
                dup(fd_file);
                close(fd_file);
                args[i] = '\0';
                continue;
            }
        }
        if(size!=1){
            close(fd[0]);
            close(1);
            dup(fd[1]); 
            close(fd[1]);
        }
        ret = execv (args[0], args);
        exit(0);
	}
    else if (pid > 0) {
        close(fd[1]);
        close(0);
        dup(fd[0]);
        close(fd[0]);
        wait(NULL);
        exec_remaining (commands + 1, size - 1);
	}
}

void parse_and_run(char *inp){
    char *single = strtok(inp, "|");
    char *commands[20];
    int N_of_commands = 0;
    while(single != NULL){
        char *current_command = single;
        commands[N_of_commands] = current_command;
        ++N_of_commands;
        single = strtok(NULL, "|");
    }
    exec_remaining(commands, N_of_commands);
}

int main(){
    while (1) {
        printf("$ ");
        char input[2000];
        input[0] = '\0';
        scanf("%[^\n]%*c", input);
        if(strcmp(input,"exit")==0){
            return 0;
        }
        int pid = fork();
        if(pid == 0) {
            parse_and_run(input);
        }
        else {
            wait(NULL);
        }
    }
    return 0;
}