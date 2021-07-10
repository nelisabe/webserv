/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_t.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nelisabe <nelisabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/05 17:40:26 by nelisabe          #+#    #+#             */
/*   Updated: 2021/07/09 17:37:17 by nelisabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "client_t.hpp"

int		create_socket(void)
{
	int		socket_ID;

	if ((socket_ID = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cout << "Error crearing socket\n";
		return -1;
	}
	int		set_value = 1;
	if (setsockopt(socket_ID, SOL_SOCKET, SO_REUSEADDR, \
		&set_value, sizeof(set_value)) == -1)
	{
		std::cout << "Error set socket options\n";
		return -1;
	}
	return socket_ID;
}

bool	parse_request(char const *reqest, char **response)
{
	if (reqest)
	{
		std::cout << "Request: ";
		std::cout << reqest << std::endl << "ENDL" << std::endl;
	}
	
	// std::cout << "Create your response: ";
	std::string	resp = "GET /hello HTTP/1.1\r\nHost: example.ru\r\nTransfer-Encoding: chunked\r\n\r\nD;name1=val1\r\nHello, World\n\r\n";
	// std::getline(std::cin, resp);

	// std::cout << resp.c_str() << std::endl;
	*response = new char[resp.size() + 1];
	memcpy(*response, resp.c_str(), resp.size() + 1);
	return false;
}

int		main(void)
{
	int		socket_ID = create_socket();

	if (socket_ID == -1)
		return 1;
	t_sockaddr_in	server;

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	if ((server.sin_addr.s_addr = inet_addr("192.168.151.101")) == -1)
	{
		std::cout << "Error inet_addr\n";
		close(socket_ID);
		return 1;
	}
	server.sin_port = htons(8080);
	if (connect(socket_ID, (sockaddr*)&server, sizeof(server)) == -1)
	{
		std::cout << "Error connect " << strerror(errno) << std::endl;
		close(socket_ID);
		return 1;
	}

		char	*response;
		
		parse_request(NULL, &response);
		send(socket_ID, response, strlen(response), 0);

		std::string temp;
		std::cin >> temp;
		
		std::string sendto = "13;name2=val2;name3;name4=\"sdfsdf\\\\fs\"\r\nI'm chunked coding!\r\n";
		send(socket_ID, sendto.c_str(), sendto.length(), 0);
		std::cin >> temp;

		sendto = "0\r\nField1:value1\r\n\r\n";
		send(socket_ID, sendto.c_str(), sendto.length(), 0);

		int		buffer_size = 64000;
		char	buffer[buffer_size];
		
		int bytes;
		std::cout << "Data send\n";
		if ((bytes = recv(socket_ID, buffer, buffer_size - 1, 0)) == -1)
		{
			std::cout << "Error recv\n";
			return 1;
		}
		std::cout << "Data recv\n";
		// if (!bytes)
		// 	break ;
		// std::cout << "Bytes recieved: " << bytes << std::endl;
		std::cout << "DATA RECIEVED FROM SERVER \n";
		std::cout << buffer << std::endl;
		
		// std::string	stop;
		// std::getline(std::cin, stop);
		// if (stop == "stop")
		// 	break;

	close(socket_ID);
	return 0;
}