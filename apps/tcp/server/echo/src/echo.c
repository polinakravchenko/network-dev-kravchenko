#include "echo.h"

SOCKET server_socket = -1;

void free_socket()
{
	if (server_socket > 0)
	{
		closesocket(server_socket);
	}
}
void usage(const char* exe_name)
{
	printf("Usage:\n");
	printf("\t%s -p <port>", exe_name);
}

int start(int argc, char* argv[])
{
	int port = DEFAULT_PORT;

	if (argc >= 3)
	{
		char arg_line[128];

		memset(arg_line, 0, sizeof(arg_line));

		combine_arg_line(arg_line, argv, 1, 3);

		int ret = sscanf(arg_line, "-p %d", &port);

		if (ret < 1) {
			usage(argv[0]);
			return -1;
		}
	}

	return init_server(port);
}

int init_server(short port)
{
	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket <= 0)
	{
		printf("Cannot create socket\n");
		return -1;
	}

	if (bind(server_socket, (struct sockaddr*)&address, sizeof(address))) {
		printf("Cannot bind socket to port %d\n", port);
		return -2;
	}

	if (listen(server_socket, DEFAULT_QUEUE))
	{
		printf("Cannot listen socket on port %d\n", port);
		return -3;
	}

	printf("Sever run on port %d\n", port);

	return process_connection();
}

int process_connection()
{
	struct sockaddr_in income_addr;

	int len = sizeof(income_addr);

	SOCKET client_socket = accept(server_socket, (struct sockaddr*)&income_addr, &len);

	if (client_socket <= 0)
	{
		printf("Error incomming connection\n");
		return -10;
	}

	printf("Establish connection from: %s\n", inet_ntoa(income_addr.sin_addr));

	char buffer[4 * 1024];
	memset(buffer, 0, sizeof(buffer));

	int ret = recv(client_socket, buffer, sizeof(buffer), 0);

	if (ret <= 0)
	{
		printf("Receiving data error\n");
		return -11;
	}

	printf("<==== Received: %s [%d bytes]\n", buffer, ret);

	ret = send(client_socket, buffer, ret, 0);

	printf("====> Sent echo: %s [%d bytes]\n", buffer, ret);

	return closesocket(client_socket);
}