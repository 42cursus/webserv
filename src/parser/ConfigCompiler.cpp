/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigCompiler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:00:44 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:00:44 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigCompiler.hpp"
#include "Prefix_suffix.hpp"
#include <arpa/inet.h>
#include <cstring>

void ConfigCompiler::overwrite_error_pages(struct Config &cfg, const ConfigMap &error_pages)
{
	ConfigMap::const_iterator it;

	for (it = error_pages.begin(); it != error_pages.end(); it++)
		cfg.http.server.error_pages[it->first] = it->second;
}

TrieNode *ConfigCompiler::build_cgi_trie(std::vector<CGI> &cgis)
{
	TrieNode *head = new TrieNode();

	for (uint64_t i = 0; i < cgis.size(); i++)
		suffix_trie_insert(head, &cgis[i]);
	return head;
}

void ConfigCompiler::compileServer(Server &server, struct Config &cfg)
{
	cfg.http.server.locations = server.getLocations();
	cfg.http.server.redirects = server.getRedirects();
	cfg.http.server.server_name = server.getHost();
	cfg.http.server.ipv4_listen.sin_family = AF_INET;
	cfg.http.server.ipv4_listen.sin_addr.s_addr = htonl(INADDR_ANY);
	std::memset(cfg.http.server.ipv4_listen.sin_zero, 0, 8);
	cfg.http.server.ipv4_listen.sin_port = htons(server.getPort());
	cfg.http.server.loc_trie = new TrieNode();
	cfg.http.server.redirect_trie = new TrieNode();

	overwrite_error_pages(cfg, server.getErrorPages());
	for (uint64_t i = 0; i < cfg.http.server.locations.size(); i++) {
		prefix_trie_insert(cfg.http.server.loc_trie, cfg.http.server.locations[i]._path, &cfg.http.server.locations[i]);
		cfg.http.server.locations[i].cgi_trie = build_cgi_trie(cfg.http.server.locations[i]._cgi);
	}
	for (uint64_t i = 0; i < cfg.http.server.redirects.size(); i++)
		prefix_trie_insert(cfg.http.server.redirect_trie, cfg.http.server.redirects[i]._path, &cfg.http.server.redirects[i]);
}
