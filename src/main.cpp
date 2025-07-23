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
#include <csignal>
typedef struct sigaction	t_sigaction;


static sig_atomic_t g_var;

void	sig_handler(int sig, siginfo_t *info, void *ctx)
{
	int					sipid;

	sipid = info->si_pid;
	if (sig == SIGINT)
		g_var = SIGINT;
	return ;
	(void)ctx;
	(void)sipid;
}

int main(int argc, char **argv)
{
	t_sigaction	act;

	act.sa_flags = SA_SIGINFO | SA_RESTART;
	act.sa_sigaction = &sig_handler;
	sigemptyset(&act.sa_mask);

	try {
		std::cout << "Wello horld!" << std::endl;
		char *filename = argv[1];
		Config conf = Parser::parse(filename);
		Server srv = Server(conf);

		srv.start();

		while(g_var != SIGINT)
		{
			Client clnt(srv);
			clnt.acceptConnection();
			clnt.handleRequest();
		}
		srv.stop();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

    return (0);
	(void)argc;
}
