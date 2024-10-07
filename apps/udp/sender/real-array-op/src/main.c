#include "main.h"

void usage(const char* exe_name)
{
    printf("Usage:\n");
    printf("\t%s -h <host> -p <port>\n", exe_name);
}

int start(int argc, char* argv[])
{
    char host[2048] = "";

    int port;

    if (argc >= 3)
    {
        char arg_line[4 * 1024] = "";

        combine_arg_line(arg_line, argv, 1, argc);

        int ret = sscanf(arg_line, "-h %s -p %d", host, &port);

        if (ret < 2) {
            usage(argv[0]);
            return -1;
        }
    }
    else {
        printf("Enter server address (-h <host> -p <port>): ");

        int ret = scanf("-h %s -p %d", host, &port);

        if (ret < 2)
        {
            usage(argv[0]);

            return -2;
        }
    }

    return init_client(host, port);
}

int init_client(const char* host, short port)
{
    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_socket <= 0)
    {
        printf("Cannot create client socket\n");
        return -1;
    }

    printf("Socket created\n");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    char target_host[2048] = "";
    resolve_address(host, target_host);

    server_address.sin_addr.s_addr = inet_addr(target_host);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address))) {
        printf("Cannot connect to port %s:%d\n", target_host, port);
        return -2;
    }

    printf("Success connection to server: %s:%d\n", target_host, port);

    return process_connection(client_socket);
}

int process_connection(SOCKET client_socket, struct sockaddr_in server_address)
{
    
    char buffer[4 * 1024] = "";
    
    struct NumberOperationData request;

    printf("Enter type of operation:\n0 for AVG, 1 for MAX, 2 for MIN: ");
    int ret = scanf("%d", &request.type);
    if (ret != 1 || (request.type < 0 || request.type > 2)) {
        printf("Error reading input or invalid operation type\n");
        return -11;
    }

    printf("\nEnter your list of numbers: ");
    for (int i = 0; i < 5; i++) {
        ret = scanf("%lf", &request.data[i]);
        if (ret != 1) {
            printf("Error reading input\n");
            return -12;
        }
    }

    for (int i = 5; i < 1024; i++) {
        request.data[i] = NAN;
    }

    ret = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&server_address, sizeof(server_address));
    if (ret <= 0) {
        printf("Sending data error\n");
        return -13;
    }

    printf("====> Sent: [%d bytes]\n", ret);

    struct NumberOperationResult response;

    memset(buffer, 0, sizeof(buffer));
    
    struct sockaddr_in from_address;
    int from_len = sizeof(from_address);

    ret = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&from_address, &from_len);
    
    if (ret <= 0) {
        printf("Receiving data error\n");
        return -14;
    }

    printf("<==== Received echo: [%d bytes]\n", ret);

    process_request(&request, &response);

    return closesocket(client_socket);
}

int process_request(struct NumberOperationData* request, struct NumberOperationResult* response)
{
    switch (response->type) {
    case AVG:
        printf("Average value: %.3f\n", response->result);
        break;
    case MAX:
        printf("Maximal value: %.3f\n", response->result);
        break;
    case MIN:
        printf("Minimal value: %.3f\n", response->result);
        break;
    default:
        printf("Unknown operation type.\n");
        break;
    }
    return 0;
}