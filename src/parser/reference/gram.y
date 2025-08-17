%token QuotedString EOF Keyword Variable BlockStart BlockEnd Semicolon Comment EndOfLine Illegal Regex

%%

config		: /* empty */
			| config statement
			;

statement	: directive
			| comment
			;

comment		: Comment
			;

block		: BlockStart directives BlockEnd
			;

directives	: /* empty */
			| directives directive
			;

directive	: Keyword parameters semicolon
			| Keyword parameters block
			| Keyword block
			| Keyword semicolon
			;

semicolon	: Semicolon
			;

parameters	: /* empty */
			| parameters parameter
			;

parameter	: Keyword
			| Variable
			| Regex
			| QuotedString
			;

%%
