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

#include "Logging.hpp"
#include "Parser.hpp"
#include "Prefix_suffix.hpp"
#include "WebServer.hpp"
#include "webserv.hpp"
#include <csignal>
#include <cstdlib>
#include <vector>

typedef struct sigaction t_sigaction;

sig_atomic_t g_var = {0x00};

void sig_handler(int sig, siginfo_t *info, void *ctx) {
	int sipid = info->si_pid;
	if (sig == SIGINT)
		g_var = SIGINT;
	// std::cout << "\e[1k" << std::endl;
	return;
	(void)ctx;
	(void)sipid;
}

void	sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	// std::cout << "handler called" << std::endl;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) ;
	return ;
	(void)signo;
}

int main(int argc, char **argv) {
	t_sigaction act;
	t_sigaction old_act;

	act.sa_flags	 = SA_SIGINFO;// Do NOT set SA_RESTART; we want syscalls to be interrupted.
	act.sa_sigaction = &sig_handler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGINT, &act, &old_act) != 0)
		exit(EXIT_FAILURE);

	signal(SIGCHLD, sig_chld);
	signal(SIGPIPE, SIG_IGN);

	log_title();

	std::string filename = "resources/webserv.conf";
	if (argc > 1)
		filename = argv[1];
	Parser		newParser(filename);

	try {
		log_parsing(filename);
		newParser.init_parser();
		newParser.tokenise();
		// printTokens(newParser.getTokens());
		// std::cout << std::endl;
		newParser.parse();
		// newParser.getConfig().printConfig();
		// exit(1);
	} catch (std::exception &e) {
		// std::cerr << e.what() << std::endl;
		log_parsing_error(e);
		exit(1);
	}

	std::vector<Config> cfgs;
	cfgs.resize(newParser.getConfig().getServers().size());

	for (uint64_t i = 0; i < cfgs.size(); i++)
		newParser.getConfig().getServers()[i].get_config(cfgs[i]);

	WebServer srv;

	srv.init(cfgs);
	if (srv.start() != 0)
		return 1;
	srv.serve();
	srv.stop();

	return (EXIT_SUCCESS);
	(void) filename;
}
