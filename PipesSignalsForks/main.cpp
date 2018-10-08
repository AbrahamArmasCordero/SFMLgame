#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX 100

//fd[0] read
static int fdP1[2];
static int fdP2[2];
pid_t son1;
pid_t son0;

void IllSendIt(int i){
    char buffer[MAX];
    size_t t = read(fdP1[0],buffer, MAX);
    buffer[t] = '\0';
    write(fdP2[1], buffer, t);

    kill(son1, SIGUSR2);
}

void FatherIllRead(int i){
    char buffer[MAX];
    size_t t = read(fdP2[0], buffer, MAX);
    buffer[t] = '\0';
    std::cout << buffer << std::endl;
    std::cout << "I'm " << getpid() << " and i received " << buffer << std::endl;
}

int main()
{

    int state = pipe(fdP1);
    if(state <0) throw "pipe not found";

    son0 = fork();

    signal(SIGUSR1, IllSendIt);
    signal(SIGUSR2, FatherIllRead);

    if(son0 == 0){
        signal(SIGUSR1, SIG_IGN);
        signal(SIGUSR2, SIG_IGN);
        close(fdP1[0]);
        close(fdP2[0]);
        close(fdP2[1]);

        write(fdP1[1], "Toma hermano", 12);
        kill(getppid(), SIGUSR1);

        while(1){
            pause();
        }
    }
    else{

        int stateP2 = pipe(fdP2);

        son1 = fork();

        if(son1 == 0){
            signal(SIGUSR1, SIG_IGN);
            close(fdP2[1]);
            close(fdP1[0]);
            close(fdP1[1]);

            while(1){
                pause();
            }
        }
        else{
        //padre
            while(1){
                pause();
            }
        }
    }
    return 0;
}
