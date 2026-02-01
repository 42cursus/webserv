/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 22:51:57 by margo             #+#    #+#             */
/*   Updated: 2026/02/01 17:42:39 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Prefix_suffix.hpp"
#include "WebServer.hpp"
#include "webserv.hpp"
#include <csignal>
#include <vector>

typedef struct sigaction t_sigaction;

sig_atomic_t g_var = {0x00};

void sig_handler(int sig, siginfo_t *info, void *ctx) {
	int sipid = info->si_pid;
	if (sig == SIGINT)
		g_var = SIGINT;
	return;
	(void)ctx;
	(void)sipid;
}

int main(int argc, char **argv) {
	t_sigaction act;
	t_sigaction old_act;

	act.sa_flags	 = SA_SIGINFO;// Do NOT set SA_RESTART; we want syscalls to be interrupted.
	act.sa_sigaction = &sig_handler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGINT, &act, &old_act) != 0)
		exit(EXIT_FAILURE);

	std::string filename = "resources/webserv.conf";
	Parser		newParser(filename);

	if (argc > 1)
		filename = argv[1];
	try {
		newParser.init_parser();
		newParser.tokenise();
		printTokens(newParser.getTokens());
		std::cout << std::endl;
		newParser.parse();
		newParser.getConfig().printConfig();
		// exit(1);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	std::vector<Config> cfgs;
	cfgs.resize(newParser.getConfig().getServers().size());

	for (uint64_t i = 0; i < cfgs.size(); i++)
		newParser.getConfig().getServers()[i].get_config(cfgs[i]);

	WebServer srv;

	srv.init(cfgs);
	srv.start();
	srv.serve();
	srv.stop();

	return (0);
	(void) filename;
}
