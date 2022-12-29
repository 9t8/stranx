#include "closure.h"
#include "lexer.h"

#include <iostream>

using namespace stranx;

sp<datum> parse_next(const token_list &tokens, size_t &idx) {
	const auto get_next_token_type([&]() -> const std::type_info & {
		assert(idx < tokens.size() &&
		"expected a token but none found (too many opening parens?)");

		token &next_token(*tokens[idx]);
		return typeid(next_token);
	});

	if (get_next_token_type() == typeid(beginl)) {
		++idx;
		if (get_next_token_type() == typeid(endl)) {
			++idx;
			return std::make_shared<emptyl>();
		}

		sp<pair> p(std::make_shared<pair>(parse_next(tokens, idx)));
		sp<datum> start(p);
		while (get_next_token_type() != typeid(endl)) {
			if (get_next_token_type() == typeid(dot)) {
				p->cdr = parse_next(tokens, ++idx);
				assert(get_next_token_type() == typeid(endl) &&
					   "malformed improper list (misplaced dot token)");
				break;
			}
			sp<pair> p_new(std::make_shared<pair>(parse_next(tokens, idx)));
			p->cdr = p_new;
			p = p_new;
		}
		++idx;

		return start;
	}

	sp<datum> p(std::dynamic_pointer_cast<datum>(tokens.at(idx++)));
	assert(p && "malformed token list (tried to parse an unparsable token)");

	return p;
}

int main(int, const char *[]) {
	token_list tokens(lexer(std::cin).lex());

	std::cout << "===-- tokens --===\n";
	for (size_t i(0); i < tokens.size(); ++i) {
		std::cout << " " << *tokens[i];
	}
	std::cout << "\n";

	std::vector<sp<datum>> tree;
	for (size_t i(0); i < tokens.size();) {
		tree.push_back(parse_next(tokens, i));
	}

	std::cout << "\n===-- tree --===\n";
	for (size_t i(0); i < tree.size(); ++i) {
		std::cout << *tree[i] << "\n";
	}
	std::cout << "\n===-- output --===" << std::endl;

	const sp<context> ctx(std::make_shared<context>(nullptr));
	ctx->define("lambda", std::make_shared<lambda>());
	ctx->define("define", std::make_shared<define>());

	for (size_t i(0); i < tree.size(); ++i) {
		std::cout << *tree[i]->eval(ctx) << std::endl;
	}
}
