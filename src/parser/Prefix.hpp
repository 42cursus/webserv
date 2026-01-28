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

struct TrieNode
{
	Location	*location;
	TrieNode	*children[128];

	TrieNode(void);
};


void	loc_trie_insert(TrieNode *head, Location *location);
Location	*loc_trie_search(TrieNode *head, std::string const& path);
std::string apply_location(std::string& path, Location const *location);
void	test_trie_match(TrieNode *head, std::string path);
void 	test_trie(void);
void	free_loc_trie(TrieNode *node);

#endif
