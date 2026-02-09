#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <string>
#include <vector>

/* Each of these would be local bytecode scope */
namespace TwoObject {
    enum class RuntimeDetection {
        FUNCTION,
    };

    /* Insprided by Derkt's ObjectBase class which allows for Polymophric virutal representation */
    struct ObjectBase {
        RuntimeDetection runtime_type;
        virtual ~ObjectBase() = default;
    };

    class function_object : public ObjectBase {
        public:
            std::string name;
            std::vector<std::string> parmas;
            TwoPyOpByteCode::FullByteCode bytecode;
            function_object() {
                runtime_type = RuntimeDetection::FUNCTION;
            }
    };

}

#endif