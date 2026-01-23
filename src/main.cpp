/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 22:51:57 by margo             #+#    #+#             */
/*   Updated: 2026/01/23 15:42:55 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "TCPServer.hpp"
#include "serve.hpp"
#include "ConfigParser.hpp"
#include <csignal>
#include <cstdlib>
#include <vector>

#include "Prefix.hpp"

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

	// test_trie();
	// exit(1);

	act.sa_flags = SA_SIGINFO; // Do NOT set SA_RESTART; we want syscalls to be interrupted.
	act.sa_sigaction = &sig_handler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGINT, &act, &old_act) != 0)
		exit(EXIT_FAILURE);
	filename = (char *)"resources/webserv.conf";
	if (argc > 1)
		filename = argv[1];
	std::vector<TCPServer*>	srvs;
	try {
		std::cout << "Wello horld!" << std::endl;
		Config conf = Parser::make_default_config();

		// test_trie_match(conf.http.server.loc_trie, "/hello");
		// test_trie_match(conf.http.server.loc_trie, "/upload");
		// test_trie_match(conf.http.server.loc_trie, "/upload/wee");
		// test_trie_match(conf.http.server.loc_trie, "/data/file.mp4");
		// exit(1);

		TCPServer srv = TCPServer(conf);
		Config conf2 = Parser::make_default_config();
		conf2.http.server.ipv4_listen.sin_port = htons(5000);
		conf2.http.server.location.config.root = "./resources/web2";
		TCPServer srv2 = TCPServer(conf2);
		srvs.push_back(&srv);
		srvs.push_back(&srv2);
		srv.start();
		srv2.start();

		serve(srvs);

		srv.stop();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
    return (0);
	(void)filename;
}
