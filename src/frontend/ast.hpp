#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <optional>
#include <vector>
#include <variant>
#include "frontend/token.hpp"

namespace TwoPy::Frontend {
    /* Forward Decl - Expressions */
    struct ExprNode;
    using ExprPtr = std::unique_ptr<ExprNode>;

    struct BoolLiteral;
    struct IntegerLiteral;
    struct FloatLiteral;
    struct StringLiteral;
    struct Identifier;
    struct FactorOp;
    struct TermOp;
    struct AssignmentOp;
    struct AugmentedAssignmentOp;
    struct BitwiseOp;
    struct EqualityOp;
    struct ComparisonOp;
    struct PowerOp;
    struct CallExpr;
    struct AttributeExpr;
    struct ListExpr;
    struct DictExpr;
    struct SelfExpr;
    struct ConstructorCallExpr;
    struct OrOp;
    struct AndOp;

    /* Forward Decl - Statements */
    struct StmtNode;
    using StmtPtr = std::unique_ptr<StmtNode>;

    struct ReturnStmt;
    struct PassStmt;
    struct BreakStmt;
    struct ContinueStmt;
    struct IfStmt;
    struct ElifStmt;
    struct ElseStmt;
    struct WhileStmt;
    struct ForStmt;
    struct MatchStmt;
    struct CaseStmt;
    struct TryStmt;
    struct ExceptStmt;
    struct FinallyStmt;
    struct FunctionDef;
    struct MethodDef;
    struct ClassDef;
    struct LambdaStmt;
    struct Block;
    struct ExpressionStmt;

    using Literals = std::variant<IntegerLiteral, FloatLiteral, StringLiteral, BoolLiteral>;
    using OperatorsType = std::variant<AssignmentOp, AugmentedAssignmentOp, FactorOp, 
            TermOp, BitwiseOp, EqualityOp, ComparisonOp, PowerOp, OrOp, AndOp>;

    /* Begin Node define */

    struct IntegerLiteral {
        token_class token;
    };

    struct FloatLiteral {
        token_class token;
    };

    struct StringLiteral {
        token_class token;
    };

    struct BoolLiteral {
        token_class token;
    };

    struct Identifier {
        token_class token;
    };

    struct AndOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct OrOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct FactorOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct BitwiseOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct EqualityOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct ComparisonOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct PowerOp {
        token_class op;
        ExprPtr base;
        ExprPtr exponent;
    };

    struct TermOp {
        token_class op;
        ExprPtr left;
        ExprPtr right;
    };

    struct CallExpr {
        token_class token;
        ExprPtr callee;
        std::vector<ExprPtr> arguments;
    };
   
    struct ConstructorCallExpr {
        token_class token;
        ExprPtr constructor;
        std::vector<ExprPtr> arguments;
    };
    
    struct AttributeExpr {
        token_class token;
        Identifier constructor;
        Identifier attribute;
    };

    struct ListExpr {
        token_class token;
        std::vector<ExprPtr> elements;
    };

    struct ListIndexExpr {
        token_class token;
        ExprPtr list_name;
        ExprPtr index;
    };

    struct DictExpr {
        token_class token;
        std::vector<std::pair<ExprPtr, ExprPtr>> entries;
    };

    struct SelfExpr {
        token_class token;
        std::unique_ptr<Identifier> attribute;
    };

    struct Block {
        token_class token;
        std::vector<StmtPtr> statements;
    };

    struct Parameter {
        token_class token;
    };

    struct ParameterList {
        std::vector<Parameter> params;
    };

    struct AssignmentOp {
        token_class token;
        ExprPtr target;
        ExprPtr value;
    };

    struct AugmentedAssignmentOp {
        token_class op;
        ExprPtr target;
        ExprPtr value;
    };

    struct ReturnStmt {
        token_class token;
        ExprPtr value;
    };

    struct PassStmt {
        token_class token;
    };

    struct BreakStmt {
        token_class token;
    };

    struct ContinueStmt {
        token_class token;
    };

    struct ElifStmt {
        token_class token;
        ExprPtr condition;
        Block body;
    };

    struct ElseStmt {
        token_class token;
        Block body;
    };

    struct IfStmt {
        token_class token;
        ExprPtr condition;
        Block body;
        std::vector<ElifStmt> elifs;
        std::unique_ptr<ElseStmt> else_branch;
    };

    struct WhileStmt {
        token_class token;
        ExprPtr condition;
        Block body;
    };

    struct ForStmt {
        token_class token;
        Identifier variable;
        std::optional<ExprPtr> iterable;
        Block body;
    };

    struct CaseStmt {
        token_class token;
        ExprPtr pattern;
        Block body;
    };

    struct MatchStmt {
        token_class token;
        ExprPtr subject;
        std::vector<CaseStmt> cases;
    };

    struct ExceptStmt {
        token_class token;
        Block body;
    };

    struct FinallyStmt {
        token_class token;
        Block body;
    };

    struct TryStmt {
        token_class token;
        Block body;
        std::unique_ptr<ExceptStmt> except_branch;
        std::unique_ptr<FinallyStmt> finally_branch;
        std::unique_ptr<ElseStmt> else_branch;
    };

    struct FunctionDef {
        token_class token;
        ParameterList params;
        Block body;
    };

    struct MethodDef {
        token_class token;
        ParameterList params;
        Block body;
    };

    struct ClassDef {
        token_class token;
        Block body;
    };

    struct LambdaStmt {
        token_class token;
        ParameterList params;
        std::vector<StmtPtr> body;
    };

    struct ExpressionStmt {
        token_class token;
        ExprPtr expression;
    };

    /* Where the program inits */
    struct Program {
        std::vector<StmtPtr> statements;
    };

    /* Expr node */
    struct ExprNode {
        std::variant<
            Identifier,
            CallExpr,
            ListIndexExpr,
            ConstructorCallExpr,
            AttributeExpr,
            ListExpr,
            DictExpr,
            SelfExpr,
            Literals,
            OperatorsType> node;
    };

     /* Statement Node */
    struct StmtNode {
        std::variant<
            ReturnStmt,
            PassStmt,
            BreakStmt,
            ContinueStmt,
            IfStmt,
            WhileStmt,
            ForStmt,
            MatchStmt,
            TryStmt,
            FunctionDef,
            MethodDef,
            ClassDef,
            LambdaStmt,
            CaseStmt,
            Block,
            ExpressionStmt> node;
    };
}

#endif