#ifndef _stranx_token_h_
#define _stranx_token_h_

#include "datum.h"

#include <deque>

struct token;

typedef std::deque<std::unique_ptr<token>> token_list;

struct token : object {
	// self-parses by default
	virtual p_datum parse(token_list &tokens) const;
};

struct begin_list : token {
	operator std::string() const override {
		return "(";
	}

	p_datum parse(token_list &tokens) const override;
};

struct end_list : token {
	operator std::string() const override {
		return ")";
	}

	p_datum parse(token_list &) const override {
		assert(0 && "unexpected end of list token");
		throw;
	}
};

struct dot : token {
	operator std::string() const override {
		return ".";
	}

	p_datum parse(token_list &) const override {
		assert(0 && "unexpected dot token");
		throw;
	}
};

struct identifier : token, datum {
	identifier(const std::string &n) : name(n) {}

	operator std::string() const override {
		return name;
	}

	p_datum eval(environment &env) override {
		return find(name, env);
	}

	const std::string name;
};

struct decimal : token, datum {
	decimal(const double &v) : val(v) {}

	operator std::string() const override {
		return std::to_string(val);
	}

	const double val;
};

#endif
