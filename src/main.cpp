/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 22:51:57 by margo             #+#    #+#             */
/*   Updated: 2025/07/17 22:52:21 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "webserv.hpp"
#include "app/Server.hpp"
#include "app/Client.hpp"
#include "utils/Parser.hpp"

int main(int argc, char **argv)
{
	try {
		std::cout << "Wello horld!" << std::endl;
		char *filename = argv[1];
		Config conf = Parser::parse(filename);
		Server srv = Server(conf);

		srv.start();

		while(1) {
			Client clnt;
			clnt.acceptConnection(srv);
			clnt.handleRequest();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

    return (0);
	(void)argc;
}
