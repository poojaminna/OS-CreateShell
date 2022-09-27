#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>


const char redirec = '>';
const char* redirection =">";
// const char space1 = ' \t\n';
const char* space = " \t\n";
const char* ampersand = "&";

char** allPaths;
int totalCountOfPaths = 1;

void error(){
    char errorMessage[25] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
    // exit(1);
}


void printDash(){
    char Dash[10] = "dash> ";
    write(STDOUT_FILENO, Dash, strlen(Dash));
}


char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    // https://stackoverflow.com/questions/656542/trim-a-string-in-c
    return rtrim(ltrim(s)); 
}


int numOfTokens(char* line, const char* delimiter){
    int count=0;
    char* start = strtok(strdup(line), delimiter);
    while(start!=NULL){
        count++;
        start=strtok(NULL, delimiter);
    }
    return count;
}

char** split(char* line, const char* delimiter){
    int numberOfTokens = numOfTokens(line, delimiter);
    char** splitted = malloc(numberOfTokens * sizeof(char*));
    char* start = strtok(strdup(line), delimiter);
    int i=0;
    while(start!= NULL && i<numberOfTokens){
        splitted[i++] = start;
        start = strtok(NULL, delimiter);
    }
    return splitted;
}



void duplicate(char** src, char** des, int len){
 int i=0;
 while(i<len){
    des[i] = trim(src[i]);
    i++;
 }
}

void inbuilt_exit(){
    exit(0);
}

void inbuilt_cd(char* folder){
    if(chdir(folder)!=0)
    error();
}

void inbuilt_path(char** paths, int tokensCount){
    int pathsCount = tokensCount-1;
    char** updatedPaths = malloc(pathsCount * sizeof(char*));
    int i=0;
    while(i<pathsCount){
        if(access(paths[i+1], F_OK)!=0) error();
        updatedPaths[i] = paths[i+1];
        i++;
    }
    free(allPaths);
    allPaths=updatedPaths;
    totalCountOfPaths = pathsCount;
}



char* findPath(char* searchFor){
    int found=-1;
    char* executableFile;
    int i=0;
    while(i<totalCountOfPaths){
        executableFile = malloc(strlen(allPaths[i])+ 1 + strlen(searchFor));
        strcpy(executableFile, allPaths[i]);
        strcat(executableFile,"/");
        strcat(executableFile,searchFor);
        found = access(executableFile, F_OK);
        if(found==0) return trim(executableFile);
        free(executableFile);
        i++;
    }
    return NULL;
}

int output(char* file)
{
    // printf("hii%shii",file);
  fclose(stdout);
  fclose(stderr);
  if (freopen(file, "w+", stdout) == NULL) return -1;
  if (freopen(file, "w+", stderr) == NULL) return -1;
  return 0;
}

int decode(char* command){
    // printf("This is each command b%sb\n",command);
    if(*command == 0 ) return 0;

    int tokensCount=0;
    char** commandDup =NULL;
    char* right = NULL;

    if(strchr(command, redirec)!=NULL){
        int tokensOfRedirec = numOfTokens(command, redirection);
        // printf("%d yes", tokensOfRedirec);
        if(tokensOfRedirec!=2) return -1;
        commandDup = split(command, redirection);
        command = trim(commandDup[0]);
        // printf("Left command b%sb\n", left);
        right = trim(commandDup[1]);
        // printf("File name b%sb\n",right);
        if(*right==0 || numOfTokens(command, space)!=1) {return -1;}
        // || numOfTokens(command, space)!=1
    }

    tokensCount = numOfTokens(command, space);
    commandDup = malloc((tokensCount+1) * sizeof(int));
    duplicate(split(command, space), commandDup, tokensCount);
    commandDup[tokensCount]=NULL;
    // printf("Trimmed splitted command is inside b%sb\n", commandDup[0]);
    // printf("Trimmed splitted command is inside b%sb\n", commandDup[1]);
    // printf("Trimmed splitted command is inside b%sb\n", commandDup[2]);
     if(strcmp(trim(commandDup[0]), "exit")==0){
        // printf("Helloooo");
        // exit(0);
        inbuilt_exit();
     }else if(strcmp(trim(commandDup[0]),"cd")==0){
        // printf("Heyy");
        inbuilt_cd(commandDup[1]);
     }else if(strcmp(trim(commandDup[0]),"path")==0){
        // printf("entered");
        inbuilt_path(commandDup,tokensCount);
        // printf("%s success",allPaths[0]);
        // printf("%s success",allPaths[1]);
     }else{
        //  printf("hii%shii",trim(commandDup[0]));
        char* executableFile = findPath(trim(commandDup[0]));
        int childPID = fork();
        if(childPID==0){
            if(right!=NULL) output(right);
            // printf("Argument b%sb is\n", commandDup[0]);
            execv(executableFile, commandDup);
        }else{
            int childProcessID = childPID;
            wait(NULL);
        }
     }
    
    return 1;
}

void parallelCommands(char* input){
    // printf("This is the command b%sb\n",input);
    int tokens = numOfTokens(input, ampersand);
    char** arrOfCommands = malloc(tokens* sizeof(char*));
    arrOfCommands = split(input, ampersand);
    for(int i=0;i<tokens;i++){
        decode(trim(arrOfCommands[i]));
        // if(val ==-1) printf("Error has occureed re ");

    }
}

int main(int argc, char** argv){

//     char* command = argv[0];
// int val = decode(command);
// printf("%d",val);

    allPaths = malloc(sizeof(char*) * totalCountOfPaths);
  allPaths[0] = "/bin";
    char* input = NULL;
    size_t inputLength = 0;
    if(argc==1){
        while(1){
        
            printDash();
            if(getline(&input, &inputLength, stdin) > 0) {
                parallelCommands(trim(input));
            }
        }
    }else if(argc==2){
        char* fileName = trim(argv[1]);
        FILE *fp= fopen(fileName,"r");
        while(getline(&input, &inputLength, fp) > 0) parallelCommands(trim(input));
    }else{
        error();
        exit(1);
    }

}
