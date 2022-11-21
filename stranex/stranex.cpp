// r3rs interpreter with boost for numeric tower

#include "lex.h"
#include "parse.h"
#include "eval.h"

#include <iostream>

int main(int argc, const char **argv) {
	token_list tokens(lex(std::cin));

	std::cout << "===-- tokens --===\n" << std::flush;
	for (size_t i(0); i != tokens.size(); ++i) {
		std::cout << *tokens[i] << std::endl;
	}

	syntax_tree tree(parse(tokens));

	std::cout << "===-- subtrees --===\n" << std::flush;
	for (size_t i(0); i != tree.size(); ++i) {
		std::cout << *tree[i] << std::endl;
	}

	std::cout << "===-- output --===\n" << std::flush;
	eval(tree, std::cout);
}
