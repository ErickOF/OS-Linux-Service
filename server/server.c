// Server side C program to demonstrate HTTP Server programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

// For timed names
#include <time.h>
/*
Paginas usadas:

https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa

https://stackoverflow.com/questions/30440188/sending-files-from-client-to-server-using-sockets-in-c 
 */


#define PORT 8081
int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Only this line has been changed. Everything is same.
    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    // Temporal until config file is used
    
    int inpPort;
    printf("Enter port: ");
    scanf("%d", &inpPort);
    address.sin_port = htons( inpPort );
    //address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[1025];
        memset(buffer, '0', sizeof(buffer));
        size_t  b,tot;
        
        b = recv(new_socket, buffer, 1024,0) ;
        
        char * dataStart = strstr(buffer,"\r\n\r\n");
        dataStart += 4;  
        
        size_t headerSize  = dataStart - buffer;
        
        fwrite(buffer, 1, headerSize, stdout); // Prints the header of POST-GET
        
        char * contentSizeP = strstr(buffer,"Content-Length:");
        if ((strstr(buffer,"POST") != NULL) && (contentSizeP != NULL)){
                                    
            contentSizeP += 15; //Size of "Content-Length:"
            size_t contentSize =  strtol(contentSizeP, NULL, 10); //Converts to size_t the size
                                                          
            char dir [] = "./files/"; // Dir to save                            
            char fname[50] = ""; // Filename
            
            // If the header contains the name for the new file using the format:
            // Filename: example.png
            char * filenameP = strstr(buffer,"Filename");
            if (filenameP != NULL){
                char * filenamePE = strstr(filenameP,"\n"); // Points to the end of this line
                filenameP += 10; //Size of "Filename:"
                int fNameLen = filenamePE - filenameP;
                if (fNameLen > 1){
                    memcpy(fname,filenameP,fNameLen);      // Copies name to fname 
                }else{
                    filenameP = NULL; //If the given name is too small it uses the time format name below
                }
            } 
            /* If the header doesn't contain the name of the file, the name will be taken 
             * from the current time Y-m-d-H-M-S
             * if the Content-Type header is included, appends the corresponding extension .jpg or .png
             * Content-Type: *png*   OR   Content-Type: *jpeg*
             */
            if(filenameP == NULL) {                
                time_t rawtime;
                time(&rawtime);
                strftime(fname,49,"%Y-%m-%d-%H-%M-%S", localtime(&rawtime)); //Formats time
                char * contentTypeP = strstr(buffer,"Content-Type:");
                if (contentTypeP != NULL){
                    int isJPEG = (strstr(contentTypeP,"jpeg") != NULL)||(strstr(contentTypeP,"jpg") != NULL);
                    int isPNG = strstr(contentTypeP,"png") != NULL;
                    if (isJPEG){
                        strcat(fname, ".jpeg");
                    }else if (isPNG){
                        strcat(fname, ".png");
                    }
                }                             
            }
            
            
            char fullname [256] = "";  // Fullname = dir + filename
            strcat(fullname, dir);
            strcat(fullname, fname); 
            
            printf("Saving file in %s\n",fullname);

            FILE* fp = fopen( fullname, "wb");                        
            tot = 0;   //Total bytes of content counter                
            
            
            if (headerSize != b){
                if(fp != NULL  ){    
                    
                    fwrite(dataStart, 1, b-headerSize, fp);
                    tot += (b-headerSize);                    
                    
                    while( tot < contentSize ){
                        b = recv(new_socket, buffer, 1024,0) ;
                        if (b < 1){
                            printf("\nError reading contents!\n");break; // Handles disconnection from client                            
                        }
                        tot += b;
                        fwrite(buffer, 1, b, fp);
                        
                    }
                    
                    printf("Received bytes: %ld, header size %ld, content size %ld\n",tot+headerSize, headerSize, contentSize);
                    
                    fclose(fp);                                
                }else{
                    perror("File\n");
                } 
            }else{
                fclose(fp);   
                printf("No Content Received!\n");
            }
        }else if (strstr(buffer,"GET") != NULL){
            1;
        }
        
        
        
        write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------");
        close(new_socket);
    }
    return 0;
}
