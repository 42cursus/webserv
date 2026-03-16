/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:01:18 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:01:18 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGVALIDATOR_HPP
#define CONFIGVALIDATOR_HPP

#include <string>

class ConfigValidator {
public:
	static bool validateCgiParam(std::string cgi_param);
	static bool validateMethod(std::string method);
	static bool validateErrorPage(std::string error_page);
	static bool validateErrorCode(std::string error_code);
	static bool validateBool(std::string boolean);
	static bool validateCgiScriptExt(std::string ext, std::string script);
	static bool validateLocationRedirect(std::string path, std::string root);
};

#endif//CONFIGVALIDATOR_HPP
