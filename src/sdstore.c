#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "declarations.h"

int has_priority(char *arg)
{
    int r = 0;
    if (strcmp(arg, "0") == 0 || strcmp(arg, "1") == 0 || strcmp(arg, "2") == 0 || strcmp(arg, "3") == 0 || strcmp(arg, "4") == 0 || strcmp(arg, "5") == 0)
    {
        r = 1;
    }
    return r;
}

int check_commands(int argc, char *argv[], int first_index)
{
    int i, r = 1;
    for(i=first_index; i<argc && r==1; i++)
    {
        if (strcmp(argv[i], "nop") != 0 && strcmp(argv[i], "bcompress") != 0 && strcmp(argv[i], "bdecompress") != 0 && strcmp(argv[i], "gcompress") != 0 && strcmp(argv[i], "gdecompress") != 0 && strcmp(argv[i], "encrypt") != 0 && strcmp(argv[i], "decrypt") != 0)
        {
            r = 0;
        }
    }
    return r;
}

int main(int argc, char *argv[])
{
    int fd_srv_fifo = open(SERVER_FIFO_NAME, O_WRONLY);
    int i, test_fd;

    if (argc == 1)
    {
        write(1, "./sdstore status\n", 18);
        write(1, "./sdstore proc-file input-filename output-filename transformation-id-1 transformation-id-2 ...\n", 96);
        write(1, "./sdstore proc-file -p priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n", 108);
        _exit(EXIT_SUCCESS);
    }

    if (fd_srv_fifo > 0)
    {
        if (argc >= 2)
        {
            if (strcmp(argv[1], "status") == 0 || strcmp(argv[1], "proc-file") == 0 || strcmp(argv[1], "exit") == 0)
            {
                message st_message;
                st_message.client_pid = getpid();
                st_message.op.bcompress = 0;
                st_message.op.bdecompress = 0;
                st_message.op.gcompress = 0;
                st_message.op.gdecompress = 0;
                st_message.op.encrypt = 0;
                st_message.op.decrypt = 0;
                st_message.op.nop = 0;
                char cli_fifo[1024], buf[1024];
                snprintf(cli_fifo, sizeof(cli_fifo), CLIENT_FIFO_NAME, (int)st_message.client_pid);
                mkfifo(cli_fifo, 0777);
                int fd_clififowr, fd_clififord, bytes_read, first_index = 0;
                if (argc > 2)
                {
                    if (strcmp(argv[1], "proc-file") != 0)
                    {
                        write(2, "Error: Command not valid\n", 26);
                        unlink(cli_fifo);
                        _exit(EXIT_FAILURE);
                    }
                    st_message.type = 0;
                    st_message.task_number = 0;
                    int arguments = 0;

                    if (strcmp(argv[2], "-p") == 0)
                    {
                        if (argc > 3 && has_priority(argv[3]) == 1)
                        {
                            arguments = argc - 4;
                            first_index = 4;
                            st_message.priority = atoi(argv[3]);
                            if (argc > 4)
                            {
                                if ((test_fd = open(argv[4], O_RDONLY)) > 0)
                                {
                                    close(test_fd);
                                }
                                else
                                {
                                    write(2, "Error: Input file does not exist\n", 34);
                                    _exit(EXIT_FAILURE);
                                }
                                if (argc == 5)
                                {
                                    write(2, "Error: No output file path\n", 28);
                                    _exit(EXIT_FAILURE);
                                }
                            }
                            else if (argc == 4)
                            {
                                write(2, "Error: No input file path\n", 27);
                                _exit(EXIT_FAILURE);                                
                            }
                        }
                        else
                        {
                            write(2, "Error: Priority not valid\n", 26);
                            _exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        arguments = argc - 2;
                        first_index = 2;
                        st_message.priority = 0;
                        if ((test_fd = open(argv[2], O_RDONLY)) > 0)
                        {
                            close(test_fd);
                        }
                        else
                        {
                            write(2, "Error: Input file does not exist\n", 34);
                            _exit(EXIT_FAILURE);
                        }
                        if (argc == 3)
                        {
                            write(2, "Error: No output file path\n", 28);
                            _exit(EXIT_FAILURE);
                        }
                    }
                    st_message.n_args = arguments;

                    if (check_commands(argc, argv, first_index+2) == 0)
                    {
                        write(2, "Error: Command not valid\n", 26);
                        _exit(EXIT_FAILURE);
                    }

                    char *args = calloc(1024, sizeof(char));
                    if (arguments >= 2)
                    {
                        for (i = first_index; i < argc; i++)
                        {
                            strcat(args, argv[i]);
                            if (i != argc - 1)
                            {
                                strcat(args, " ");
                            }
                            if (strcmp(argv[i], "nop") == 0)
                            {
                                st_message.op.nop++;
                            }
                            else if (strcmp(argv[i], "bcompress") == 0)
                            {
                                st_message.op.bcompress++;
                            }
                            else if (strcmp(argv[i], "bdecompress") == 0)
                            {
                                st_message.op.bdecompress++;
                            }
                            else if (strcmp(argv[i], "gcompress") == 0)
                            {
                                st_message.op.gcompress++;
                            }
                            else if (strcmp(argv[i], "gdecompress") == 0)
                            {
                                st_message.op.gdecompress++;
                            }
                            else if (strcmp(argv[i], "encrypt") == 0)
                            {
                                st_message.op.encrypt++;
                            }
                            else if (strcmp(argv[i], "decrypt") == 0)
                            {
                                st_message.op.decrypt++;
                            }
                        }
                    }
                    strcat(args, "\0");
                    strcpy(st_message.commands, args);
                }
                else if (argc == 2)
                {
                    if (strcmp(argv[1], "proc-file") == 0)
                    {
                        close(fd_srv_fifo);
                        write(2, "Error: Not enough arguments\n", 29);
                        unlink(cli_fifo);
                        _exit(0);
                    }
                    else
                    {
                        st_message.type = strcmp("status", argv[1]) == 0 ? 1 : 2;
                        st_message.priority = 6;
                        strcpy(st_message.commands, "");
                        st_message.n_args = 0;
                    }
                }

                write(fd_srv_fifo, &st_message, sizeof(message));
                close(fd_srv_fifo);

                fd_clififord = open(cli_fifo, O_RDONLY);
                fd_clififowr = open(cli_fifo, O_WRONLY);
                while ((bytes_read = read(fd_clififord, buf, sizeof(buf))) > 0)
                {
                    printf("%s\n", buf);
                    if (strstr(buf, "concluded") != NULL)
                        break;
                }
                close(fd_clififord);
                close(fd_clififowr);
                unlink(cli_fifo);
            }
            else
            {
                write(2, "Error: Command not valid\n", 26);
            }
        }
        else
        {
            write(2, "Error: Command not valid\n", 26);
        }
    }
    else
    {
        write(2, "Error: FIFO does not exist\n", 28);
        close(fd_srv_fifo);
        _exit(EXIT_FAILURE);
    }

    _exit(EXIT_SUCCESS);
}