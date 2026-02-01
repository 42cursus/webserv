/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Prefix.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/23 12:53:41 by fsmyth            #+#    #+#             */
/*   Updated: 2026/01/23 15:32:18 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PREFIX_HPP
# define PREFIX_HPP

#include "webserv.hpp"
#include "Location.hpp"

struct TrieNode
{
    static const int CHILDREN_SIZE = 128;
	void		*data;
	TrieNode	*children[CHILDREN_SIZE];

	TrieNode();
};

void		prefix_trie_insert(TrieNode *head, std::string const& path, void *data);
void		*prefix_trie_search(TrieNode *head, std::string const& path);
void		suffix_trie_insert(TrieNode *head, CGI *cgi);
CGI			*suffix_trie_search(TrieNode *head, std::string const& suffix);

std::string	apply_location(std::string& path, Location const *location);

void		test_trie_match(TrieNode *head, std::string path);
void		test_trie(void);
void		free_trie(TrieNode *node);

#endif
