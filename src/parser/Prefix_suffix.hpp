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

template<typename T>
T *prefix_trie_search(TrieNode *head, std::string const &path)
{
	TrieNode *current  = head;
	void	 *last_loc = NULL;

	for (uint64_t i = 0; i < path.length() && current != NULL; i++) {
		// std::cout << path[i] << std::endl;
		size_t idx = static_cast<size_t>(path[i]);

		current = current->children[idx];
		if (current != NULL && current->data != NULL)
			last_loc = current->data;
	}

	return reinterpret_cast<T *>(last_loc);
}

void		suffix_trie_insert(TrieNode *head, CGI *cgi);
CGI			*suffix_trie_search(TrieNode *head, std::string const& suffix);

std::string	apply_location(std::string& path, Location const *location);

void		test_trie_match(TrieNode *head, std::string path);
void		test_trie(void);
void		free_trie(TrieNode *node);

#endif
