/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 22:51:57 by margo             #+#    #+#             */
/*   Updated: 2026/01/27 18:38:58 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "TCPServer.hpp"
#include "serve.hpp"
#include "Parser.hpp"
#include <csignal>
// #include <cstdlib>
#include <vector>
#include "Prefix_suffix.hpp"


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

	act.sa_flags = SA_SIGINFO; // Do NOT set SA_RESTART; we want syscalls to be interrupted.
	act.sa_sigaction = &sig_handler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGINT, &act, &old_act) != 0)
		exit(EXIT_FAILURE);


	std::string filename = "resources/webserv.conf";
	Parser  newParser(filename);

	if (argc > 1)
		filename = argv[1];

    try
    {
        newParser.init_parser();
        newParser.tokenise();
        //printTokens(newParser.getTokens());
        std::cout << std::endl;
        newParser.parse();
        // newParser.getConfig().printConfig();
		// exit(1);
    }
    catch (std::exception   &e)
    {
        std::cerr << e.what() << std::endl;
    }

	std::vector<TCPServer*>	srvs;
	std::vector<Config>	cfgs;
	cfgs.resize(newParser.getConfig().getServers().size());

	for (uint64_t i = 0; i < cfgs.size(); i++)
	{
		newParser.getConfig().getServers()[i].get_config(cfgs[i]);
	}

	// test_trie_match(cfgs[0].http.server.loc_trie, "/");
	// test_trie_match(cfgs[0].http.server.loc_trie, "/uploa");
	// test_trie_match(cfgs[0].http.server.loc_trie, "/upload/");
	// test_trie_match(cfgs[0].http.server.loc_trie, "/upload/hello");
	//
	// test_trie_match(cfgs[1].http.server.loc_trie, "/hello");
	// test_trie_match(cfgs[1].http.server.loc_trie, "/uploa");
	// test_trie_match(cfgs[1].http.server.loc_trie, "/upload/hello");
	// exit(1);

	for (uint64_t i = 0; i < cfgs.size(); i++)
	{
		srvs.push_back(new TCPServer(cfgs[i]));
		srvs[i]->start();
	}

	serve(srvs);

	for (uint64_t i = 0; i < srvs.size(); i++)
	{
		srvs[i]->stop();
		delete srvs[i];
	}

	
    return (0);
	(void)filename;
}
