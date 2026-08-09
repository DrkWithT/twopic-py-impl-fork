#ifndef AST_TREE_HPP
#define AST_TREE_HPP

#include <string>
#include <variant>
#include <print>
#include "frontend/ast.hpp"

namespace AstPrinter {
    namespace Ast = TwoPy::Frontend;
    namespace Token = TwoPy::Frontend;

inline void print_indent(int depth) {
    for (int i = 0; i < depth; ++i) {
        std::print("  ");
    }
}

inline std::string token_value(const Token::token_class& token) {
    return std::string(token.value);
}

// Forward declarations
inline void print_expr(const Ast::ExprPtr& expr, int depth);
inline void print_stmt(const Ast::StmtPtr& stmt, int depth);
inline void print_block(const Ast::Block& block, int depth);

// Expression printers
inline void print_expr_node(const Ast::IntegerLiteral& node, int depth) {
    print_indent(depth);
    std::print("IntegerLiteral: {}\n", token_value(node.token));
}

inline void print_expr_node(const Ast::FloatLiteral& node, int depth) {
    print_indent(depth);
    std::print("FloatLiteral: {}\n", token_value(node.token));
}

inline void print_expr_node(const Ast::StringLiteral& node, int depth) {
    print_indent(depth);
    std::print("StringLiteral: {}\n", token_value(node.token));
}

inline void print_expr_node(const Ast::BoolLiteral& node, int depth) {
    print_indent(depth);
    std::print("BoolLiteral: {}\n", token_value(node.token));
}

inline void print_expr_node(const Ast::Identifier& node, int depth) {
    print_indent(depth);
    std::print("Identifier: {}\n", token_value(node.token));
}

inline void print_expr_node(const Ast::FactorOp& node, int depth) {
    print_indent(depth);
    std::print("FactorOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::TermOp& node, int depth) {
    print_indent(depth);
    std::print("TermOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::BitwiseOp& node, int depth) {
    print_indent(depth);
    std::print("BitwiseOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::EqualityOp& node, int depth) {
    print_indent(depth);
    std::print("EqualityOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::ComparisonOp& node, int depth) {
    print_indent(depth);
    std::print("ComparisonOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::PowerOp& node, int depth) {
    print_indent(depth);
    std::print("PowerOp: {}\n", token_value(node.op));
    if (node.base) print_expr(node.base, depth + 1);
    if (node.exponent) print_expr(node.exponent, depth + 1);
}

inline void print_expr_node(const Ast::AndOp& node, int depth) {
    print_indent(depth);
    std::print("AndOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::OrOp& node, int depth) {
    print_indent(depth);
    std::print("OrOp: {}\n", token_value(node.op));
    if (node.left) print_expr(node.left, depth + 1);
    if (node.right) print_expr(node.right, depth + 1);
}

inline void print_expr_node(const Ast::AssignmentOp& node, int depth) {
    print_indent(depth);
    std::print("AssignmentOp: {}\n", token_value(node.token));
    print_indent(depth + 1);
    std::print("target:\n");
    if (node.target) print_expr(node.target, depth + 2);
    print_indent(depth + 1);
    std::print("value:\n");
    if (node.value) print_expr(node.value, depth + 2);
}

inline void print_expr_node(const Ast::AugmentedAssignmentOp& node, int depth) {
    print_indent(depth);
    std::print("AugmentedAssignmentOp: {}\n", token_value(node.op));
    print_indent(depth + 1);
    std::print("target:\n");
    if (node.target) print_expr(node.target, depth + 2);
    print_indent(depth + 1);
    std::print("value:\n");
    if (node.value) print_expr(node.value, depth + 2);
}

inline void print_expr_node(const Ast::CallExpr& node, int depth) {
    print_indent(depth);
    std::print("CallExpr\n");
    print_indent(depth + 1);
    std::print("callee:\n");
    print_expr(node.callee, depth + 2);
    if (!node.arguments.empty()) {
        print_indent(depth + 1);
        std::print("arguments:\n");
        for (const auto& arg : node.arguments) {
            print_expr(arg, depth + 2);
        }
    }
}

inline void print_expr_node(const Ast::ConstructorCallExpr& node, int depth) {
    print_indent(depth);
    std::print("ConstructorCallExpr\n");
    print_indent(depth + 1);
    std::print("callee:\n");
    print_expr(node.constructor, depth + 2);
    if (!node.arguments.empty()) {
        print_indent(depth + 1);
        std::print("arguments:\n");
        for (const auto& arg : node.arguments) {
            print_expr(arg, depth + 2);
        }
    }
}

inline void print_expr_node(const Ast::AttributeExpr& node, int depth) {
    print_indent(depth);
    std::print("AttributeExpr\n");
    print_indent(depth + 1);
    std::print("constructor: {}\n", token_value(node.constructor.token));
    print_indent(depth + 1);
    std::print("attribute: {}\n", token_value(node.attribute.token));
}

inline void print_expr_node(const Ast::ListExpr& node, int depth) {
    print_indent(depth);
    std::print("ListExpr\n");
    for (const auto& elem : node.elements) {
        print_expr(elem, depth + 1);
    }
}

inline void print_expr_node(const Ast::ListIndexExpr& node, int depth) {
    print_indent(depth);
    std::print("ListIndexExpr\n");
    print_indent(depth + 1);
    std::print("list:\n");
    print_expr(node.list_name, depth + 2);
    print_indent(depth + 1);
    std::print("index:\n");
    print_expr(node.index, depth + 2);
}

inline void print_expr_node(const Ast::DictExpr& node, int depth) {
    print_indent(depth);
    std::print("DictExpr\n");
    for (const auto& [key, value] : node.entries) {
        print_indent(depth + 1);
        std::print("entry:\n");
        print_indent(depth + 2);
        std::print("key:\n");
        print_expr(key, depth + 3);
        print_indent(depth + 2);
        std::print("value:\n");
        print_expr(value, depth + 3);
    }
}

inline void print_expr_node(const Ast::SelfExpr& node, int depth) {
    print_indent(depth);
    if (node.attribute) {
        std::print("SelfExpr.{}\n", token_value(node.attribute->token));
    } else {
        std::print("SelfExpr\n");
    }
}

// Visitor for Literals variant
inline void print_literal(const Ast::Literals& lit, int depth) {
    std::visit([depth](const auto& node) {
        print_expr_node(node, depth);
    }, lit);
}

// Visitor for Operators variant
inline void print_operator(const Ast::OperatorsType& op, int depth) {
    std::visit([depth](const auto& node) {
        print_expr_node(node, depth);
    }, op);
}

// Main expression printer
inline void print_expr(const Ast::ExprPtr& expr, int depth) {
    if (!expr) {
        print_indent(depth);
        std::print("(null)\n");
        return;
    }

    std::visit([depth](const auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Ast::Literals>) {
            print_literal(node, depth);
        } else if constexpr (std::is_same_v<T, Ast::OperatorsType>) {
            print_operator(node, depth);
        } else {
            print_expr_node(node, depth);
        }
    }, expr->node);
}

// Block printer
inline void print_block(const Ast::Block& block, int depth) {
    print_indent(depth);
    std::print("Block:\n");
    for (const auto& stmt : block.statements) {
        print_stmt(stmt, depth + 1);
    }
}

// Statement printers
inline void print_stmt_node(const Ast::ReturnStmt& node, int depth) {
    print_indent(depth);
    std::print("ReturnStmt\n");
    if (node.value) {
        print_expr(node.value, depth + 1);
    }
}

inline void print_stmt_node(const Ast::PassStmt& node, int depth) {
    print_indent(depth);
    std::print("PassStmt\n");
}

inline void print_stmt_node(const Ast::BreakStmt& node, int depth) {
    print_indent(depth);
    std::print("BreakStmt\n");
}

inline void print_stmt_node(const Ast::ContinueStmt& node, int depth) {
    print_indent(depth);
    std::print("ContinueStmt\n");
}

inline void print_stmt_node(const Ast::IfStmt& node, int depth) {
    print_indent(depth);
    std::print("IfStmt\n");
    print_indent(depth + 1);
    std::print("condition:\n");
    print_expr(node.condition, depth + 2);
    print_indent(depth + 1);
    std::print("body:\n");
    print_block(node.body, depth + 2);

    for (const auto& elif : node.elifs) {
        print_indent(depth + 1);
        std::print("elif:\n");
        print_indent(depth + 2);
        std::print("condition:\n");
        print_expr(elif.condition, depth + 3);
        print_block(elif.body, depth + 2);
    }

    if (node.else_branch) {
        print_indent(depth + 1);
        std::print("else:\n");
        print_block(node.else_branch->body, depth + 2);
    }
}

inline void print_stmt_node(const Ast::WhileStmt& node, int depth) {
    print_indent(depth);
    std::print("WhileStmt\n");
    print_indent(depth + 1);
    std::print("condition:\n");
    print_expr(node.condition, depth + 2);
    print_indent(depth + 1);
    std::print("body:\n");
    print_block(node.body, depth + 2);
}

inline void print_stmt_node(const Ast::ForStmt& node, int depth) {
    print_indent(depth);
    std::print("ForStmt\n");
    print_indent(depth + 1);
    std::print("variable: {}\n", token_value(node.variable.token));
    if (node.iterable) {
        print_indent(depth + 1);
        std::print("iterable:\n");
        print_expr(*node.iterable, depth + 2);
    }
    print_indent(depth + 1);
    std::print("body:\n");
    print_block(node.body, depth + 2);
}

inline void print_stmt_node(const Ast::CaseStmt& node, int depth) {
    print_indent(depth);
    std::print("CaseStmt\n");
    print_indent(depth + 1);
    std::print("pattern:\n");
    print_expr(node.pattern, depth + 2);
    print_block(node.body, depth + 1);
}

inline void print_stmt_node(const Ast::MatchStmt& node, int depth) {
    print_indent(depth);
    std::print("MatchStmt\n");
    print_indent(depth + 1);
    std::print("subject:\n");
    print_expr(node.subject, depth + 2);
    for (const auto& case_stmt : node.cases) {
        print_stmt_node(case_stmt, depth + 1);
    }
}

inline void print_stmt_node(const Ast::TryStmt& node, int depth) {
    print_indent(depth);
    std::print("TryStmt\n");
    print_block(node.body, depth + 1);
    if (node.except_branch) {
        print_indent(depth + 1);
        std::print("except:\n");
        print_block(node.except_branch->body, depth + 2);
    }
    if (node.finally_branch) {
        print_indent(depth + 1);
        std::print("finally:\n");
        print_block(node.finally_branch->body, depth + 2);
    }
    if (node.else_branch) {
        print_indent(depth + 1);
        std::print("else:\n");
        print_block(node.else_branch->body, depth + 2);
    }
}

inline void print_stmt_node(const Ast::FunctionDef& node, int depth) {
    print_indent(depth);
    std::print("FunctionDef: {}\n", token_value(node.token));
    if (!node.params.params.empty()) {
        print_indent(depth + 1);
        std::print("params: ");
        for (size_t i = 0; i < node.params.params.size(); ++i) {
            if (i > 0) std::print(", ");
            std::print("{}", token_value(node.params.params[i].token));
        }
        std::print("\n");
    }
    print_block(node.body, depth + 1);
}

inline void print_stmt_node(const Ast::MethodDef& node, int depth) {
    print_indent(depth);
    std::print("MethodDef: {}\n", token_value(node.token));
    if (!node.params.params.empty()) {
        print_indent(depth + 1);
        std::print("params: ");
        for (size_t i = 0; i < node.params.params.size(); ++i) {
            if (i > 0) std::print(", ");
            std::print("{}", token_value(node.params.params[i].token));
        }
        std::print("\n");
    }
    print_block(node.body, depth + 1);
}

inline void print_stmt_node(const Ast::ClassDef& node, int depth) {
    print_indent(depth);
    std::print("ClassDef: {}\n", token_value(node.token));
    print_block(node.body, depth + 1);
}

inline void print_stmt_node(const Ast::LambdaStmt& node, int depth) {
    print_indent(depth);
    std::print("LambdaStmt\n");
    if (!node.params.params.empty()) {
        print_indent(depth + 1);
        std::print("params: ");
        for (size_t i = 0; i < node.params.params.size(); ++i) {
            if (i > 0) std::print(", ");
            std::print("{}", token_value(node.params.params[i].token));
        }
        std::print("\n");
    }
    print_indent(depth + 1);
    std::print("body:\n");
    for (const auto& stmt : node.body) {
        print_stmt(stmt, depth + 2);
    }
}

inline void print_stmt_node(const Ast::Block& node, int depth) {
    print_block(node, depth);
}

inline void print_stmt_node(const Ast::ExpressionStmt& node, int depth) {
    print_indent(depth);
    std::print("ExpressionStmt\n");
    if (node.expression) {
        print_expr(node.expression, depth + 1);
    }
}

// Main statement printer
inline void print_stmt(const Ast::StmtPtr& stmt, int depth) {
    if (!stmt) {
        print_indent(depth);
        std::print("(null)\n");
        return;
    }

    std::visit([depth](const auto& node) {
        print_stmt_node(node, depth);
    }, stmt->node);
}

// Program printer
inline void print_program(const Ast::Program& program) {
    std::print("Program\n");
    for (const auto& stmt : program.statements) {
        print_stmt(stmt, 1);
    }
}

// Convenience function
inline void print_ast(const Ast::Program& program) {
    print_program(program);
}

} // namespace AstPrinter

#endif
