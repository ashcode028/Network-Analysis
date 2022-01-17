#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// argv[0] filename
// argv[1] server_ip_address
// argv[2] portn_no

int main(int argc, char **argv)
{
    //gives error if server_ip and port number is not passed
    if (argc < 3)
    {
        printf("Server ip address and port not provided\n");
        exit(1);
    }

    // Declaring the variables
    int sock_fd, port_no, read_write_ret, ch = 0, words, num = 0;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char message[256];

    port_no = atoi(argv[2]);

    //clearing the data in n bytes of address memory
    bzero((char *)&server_addr, sizeof(server_addr));

    //creating the socket at server
    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        error("ERROR while opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        error("ERROR: No such Host found");
    }

    //initialise the server_addr
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_no);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Connection Failed");
    }

    bzero(message, 256);
    fgets(message, 256, stdin);
    read_write_ret = write(sock_fd, message, strlen(message));
    if (read_write_ret < 0)
    {
        error("ERROR on sending message to server");
    }

    /****File Reading***/
    FILE *f = fopen("client_process_list", "w");
    read(sock_fd, &words, sizeof(int));

    if (read_write_ret < 0)
    {
        error("ERROR on recieving message from server");
    }

    while (ch != words)
    {
        read(sock_fd, message, 100);
        fputs(message, f);
        if (num == 0)
        {
            fputs(" ", f);
        }
        else if (num == 1)
        {
            fputs(" ", f);
        }
        else
        {
            fputs("\n", f);
            num = -1;
        }
        ch++;
        num++;
    }
    fclose(f);

    FILE *f1 = fopen("client_process_list", "r");
    for (int i = 0; i < 3; i++)
    {
        fscanf(f1, "%s", message);
        write(sock_fd, message, 100);
    }
    fclose(f1);

    close(sock_fd);
    return 0;
}