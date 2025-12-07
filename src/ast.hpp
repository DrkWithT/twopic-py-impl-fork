#ifndef AST_HPP
#define AST_HPP 

#include <memory>

namespace Ast {
    class ast_class {
        private:
            template<typename T>
            struct node {
                T data;
                std::unique_ptr<node> left;
                std::unique_ptr<node> right;
            };
        public:
    };
}

#endif