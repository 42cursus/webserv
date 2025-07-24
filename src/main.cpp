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
#include "app/TCPServer.hpp"
#include "app/Worker.hpp"
#include "utils/Parser.hpp"
#include <csignal>
#include <cstdlib>

typedef struct sigaction	t_sigaction;


sig_atomic_t g_var = {0x00};

void	sig_handler(int sig, siginfo_t *info, void *ctx)
{
	int	sipid;

	sipid = info->si_pid;
	if (sig == SIGINT)
		g_var = SIGINT;
	return ;
	(void)ctx;
	(void)sipid;
}

int	main(int argc, char **argv)
{
	t_sigaction	act;
	t_sigaction	old_act;
	char		*filename;

	act.sa_flags = SA_SIGINFO; // Do NOT set SA_RESTART; we want syscalls to be interrupted.
	act.sa_sigaction = &sig_handler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGINT, &act, &old_act) != 0)
		exit(EXIT_FAILURE);
	filename = (char *)"resources/webserv.conf";
	if (argc > 1)
		filename = argv[1];
	try {
		std::cout << "Wello horld!" << std::endl;
		Config conf = Parser::parse(filename);
		TCPServer srv = TCPServer(conf);
		srv.start();
		srv.serve(srv);
		srv.stop();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    return (0);
}
