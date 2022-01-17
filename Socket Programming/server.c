#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>

int client_num = 0;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct my_process
{
    int pid;
    char name[100];
    int cpu_time;
};

int isDigit(const char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (!(str[i] >= '0' && str[i] <= '9'))
        {
            return 0;
        }
    }
    return 1;
}

int count_process()
{
    int index = 0, res = 0;
    DIR *directory;
    struct dirent *d;
    directory = opendir("/proc/");
    if (directory)
    {
        while ((d = readdir(directory)) != NULL)
        {
            if (d->d_type != 4 || !isDigit(d->d_name))
            {
                continue;
            }
            res++;
        }
        closedir(directory);
    }
    return res;
}

void get_all_pid(struct my_process *list)
{
    int index = 0;
    DIR *directory;
    struct dirent *d;
    directory = opendir("/proc/");

    if (directory)
    {
        while ((d = readdir(directory)) != NULL)
        {
            if (d->d_type != 4 || isDigit(d->d_name) == 0)
            {
                continue;
            }
            struct my_process temp;
            temp.pid = atoi(d->d_name);
            list[index++] = temp;
        }
        closedir(directory);
    }
}

void get_full_process_details(struct my_process *list, int n)
{
    for (int i = 0; i < n; i++)
    {
        int num = 1;
        char path[100], num1[100], num2[100], x[100], name[100];

        sprintf(path, "/proc/%d/stat", list[i].pid);
        FILE *f = fopen(path, "r");
        while (fscanf(f, "%1023s", x) == 1)
        {
            if (num == 2)
            {
                strcpy(name, x);
            }
            else if (num == 14)// kernalspace cpu time
            {
                strcpy(num1, x);
            }
            else if (num == 15)// userspace cpu time
            {
                strcpy(num2, x);
            }
            else if (num > 15)
            {
                break;
            }
            num++;
        }
        fclose(f);
        strcpy(list[i].name, name);
        list[i].cpu_time = atoi(num1) + atoi(num2);
    }
}

int my_cmp(const void *a, const void *b)
{
    struct my_process *ia = (struct my_process *)a;
    struct my_process *ib = (struct my_process *)b;
    return (int)(ia->cpu_time - ib->cpu_time);
}

void write_to_server_file(struct my_process *list, int n)
{
    char endl[] = "\n";
    FILE *f = fopen("server_process_list.txt", "w");
    for (int i = 0; i < n; i++)
    {
        char pid[100], cpu_time[100];
	// int to string then insert in file
        sprintf(pid, "%d", list[i].pid);
        sprintf(cpu_time, "%d", list[i].cpu_time);

        fputs(list[i].name, f);
        fputs(" ", f);
        fputs(pid, f);
        fputs(" ", f);
        fputs(cpu_time, f);
        fputs(endl, f);
    }
    fclose(f);
}

int cmp(const void *a, const void *b)
{
    struct my_process *ia = (struct my_process *)a;
    struct my_process *ib = (struct my_process *)b;
    return (int)(100.f * ib->cpu_time - 100.f * ia->cpu_time);
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int new_sock_fd = *(int *)socket_desc, client_number = (client_num++) + 1, read_size, read_write_ret, n, max, words = 0;
    char message[256], c, buffer[100];
    // read from socket
    read_write_ret = read(new_sock_fd, message, 256);
    printf("\n\nNew Connection Established: Client Id %d\n", client_number);

    if (read_write_ret < 0)
    {
        error("ERROR in recieving client message");
    }

    if (isDigit(message))
    {
        error("Client Entered input is not an Integer");
    }
    n = atoi(message);
    max = count_process();
    struct my_process list[max];

    get_all_pid(list);
    get_full_process_details(list, max);
    /*****sort the object array******/
    size_t struct_len = sizeof(list) / sizeof(struct my_process);
    qsort(list, struct_len, sizeof(struct my_process), cmp);
    /****end of sorting*****/
    if (n > max)
    {
        printf("\nWarning: Only %d process are running, fetching %d process instead of %d\n", max, max, n);
        n = max;
    }
    write_to_server_file(list, n);

    /**Transferring file***/
    bzero(buffer, 100);
    FILE *f = fopen("server_process_list.txt", "r");
    while ((c = getc(f)) != EOF)
    {
        fscanf(f, "%s", buffer);
        if (c == ' ' || c == '\n')
        {
            words++;
        }
    }
    write(new_sock_fd, &words, sizeof(int));
    rewind(f);

    int ch = 0;
    while (ch != words)
    {
        fscanf(f, "%s", buffer);
        write(new_sock_fd, buffer, 100);
        ch++;
    }
    fclose(f);

    sleep(5);
    //get the top 3 process
    printf("\nClient #%d: The most CPU consuming process\n", client_number);
    printf("Process Name\t Process id\t CPU consumption\n");
    for (int i = 0; i < 3; i++)
    {
        read(new_sock_fd, buffer, 100);
        fputs(buffer, stdout);
        if (i < 2)
        {
            fputs("       \t", stdout);
        }
    }
    fputs(" Clock Ticks\n", stdout);
    /**Transferring done***/

    close(new_sock_fd);
    sleep(5);
    printf("\nClient #%d: Connection disconnected\n", client_number);
    return 0;
}

int main(int argc, char **argv)
{
    //gives error if port number is not passed
    if (argc < 2)
    {
        printf("Port Number NOT Provided\nProgram terminated\n");
        exit(1);
    }

    // Declaring the variables
    int sock_fd, port_no, new_sock_fd, client_addr_len, read_write_ret;
    struct sockaddr_in server_addr, client_addr;
    char message[256];

    port_no = atoi(argv[1]);
    client_addr_len = sizeof(client_addr);

    //clearing the data in n bytes of address memory
    bzero((char *)&server_addr, sizeof(server_addr));

    //initialise the server_addr
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    //creating the socket at server
    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        error("ERROR in opening socket");
    }

    //binding socket to address
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("ERROR while binding socket");
    }

    //initiating the listen to every incoming request to the server
    if (listen(sock_fd, 5) < 0)
    {
        error("ERROR while server put in listen");
    }

    pthread_t thread_id;
    while (new_sock_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_len))
    {
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_sock_fd) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        if (new_sock_fd < 0)
        {
            perror("accept failed");
            return 1;
        }
    }
    close(sock_fd);
    close(new_sock_fd);
    return 0;
}
